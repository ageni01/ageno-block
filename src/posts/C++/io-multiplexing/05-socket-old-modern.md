---
title: Socket 传统写法与现代写法：IPv4、getaddrinfo、字节序和 C++ RAII
date: 2026-10-30
category:
  - C++
tag:
  - Socket
  - C
  - C++
---

# Socket 传统写法与现代写法：IPv4、getaddrinfo、字节序和 C++ RAII

所谓“新版 socket”通常不是 socket/connect/recv 被另一套函数取代，而是地址解析、资源管理、错误处理和并发模型变得更稳健。本文将这些变化分开比较，避免误以为 htons 已经过时。

<!-- more -->

## 1. 对照看，哪些变了，哪些没变

| 传统常见写法 | 更适合新代码的方式 | 原因 |
| --- | --- | --- |
| 固定 sockaddr_in + IPv4 | getaddrinfo + AF_UNSPEC + 候选遍历 | 支持 IPv4/IPv6 和主机名 |
| inet_addr | inet_pton | 更明确的错误结果与地址族 |
| inet_ntoa | inet_ntop / getnameinfo | 调用方提供输出缓冲，支持更多地址族 |
| gethostbyname | getaddrinfo | 地址与服务解析统一 |
| bzero | 初始化 / memset | 标准写法更清楚 |
| 手写多出口 close | C 保持集中清理；C++ 使用 RAII | 明确所有权，减少泄漏 |
| 一次 send/recv 当完整消息 | 发送偏移 + 接收缓冲 + 帧解析 | TCP 不保留消息边界 |
| 串行阻塞每个客户端 | 根据规模选线程或非阻塞多路复用 | 避免一个慢连接拖住所有人 |

socket、bind、listen、accept、connect、send、recv 仍是核心接口。htons/htonl/ntohs/ntohl 仍用于协议规定的大端整数编码。IPv6 也有 16 位端口，不能说 IPv6 不再需要考虑网络字节序。

## 2. 看懂旧代码：IPv4 固定地址

```c
struct sockaddr_in server;
memset(&server, 0, sizeof server);
server.sin_family = AF_INET;
server.sin_port = htons(9090);
server.sin_addr.s_addr = inet_addr("127.0.0.1");
int fd = socket(AF_INET, SOCK_STREAM, 0);
/* 检查 fd，再 connect，并检查返回值 */
```

AF_INET 选择 IPv4；sin_port 是网络字节序；inet_addr 返回 IPv4 网络序地址。它不是 DNS 查询，不能直接把 example.com 塞进去。inet_addr 的历史错误结果容易与某些合法地址值混淆，现代固定数值地址通常用 inet_pton。

这段只是旧写法识读，不是推荐复制的完整程序；缺少错误处理的代码不能直接变成生产代码。

## 3. 现代固定地址：inet_pton

```c
struct sockaddr_in server = {0};
server.sin_family = AF_INET;
server.sin_port = htons(9090);
int rc = inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
if (rc == 0) { /* 输入不是合法数值地址 */ }
else if (rc == -1) { /* 不支持的地址族等错误，检查 errno */ }
else { /* rc == 1：成功，可继续 socket/connect */ }
```

这里也没有替代 TCP，而是替代地址文本到二进制的转换。输出地址用 inet_ntop；已经是网络序的 sin_addr 不要再次 htonl。

## 4. 主机名与双栈：getaddrinfo

完整可编译代码在 [modern_connect.c](https://github.com/ageni01/ageno-block/blob/main/examples/io-multiplexing/modern_connect.c)。核心如下：

```c
struct addrinfo hints = {0}, *list = NULL;
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;
int rc = getaddrinfo("localhost", "9090", &hints, &list);
if (rc != 0) {
    fprintf(stderr, "%s\n", gai_strerror(rc));
    /* 结束本次操作 */
}
```

getaddrinfo 返回错误码用 gai_strerror 解释，不是简单用 perror。成功后 list 是候选链表；每个候选都带 ai_family、ai_socktype、ai_protocol、ai_addr 和 ai_addrlen。

```c
int fd = -1;
for (struct addrinfo *p = list; p; p = p->ai_next) {
    fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (fd < 0) continue;
    if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
    close(fd);
    fd = -1;
}
freeaddrinfo(list);
/* fd >= 0 才能通信；fd < 0 表示所有候选失败 */
```

localhost 可能先返回 ::1，服务端若仅监听 127.0.0.1，IPv6 候选失败后还需尝试 IPv4。候选里的 sockaddr 已由解析器填好，不要再手动给其端口重复 htons。

这是顺序阻塞尝试，不是完整 Happy Eyeballs；解析本身也可能阻塞，连接超时和并行候选策略需要进一步设计。不要把它放入高并发事件循环后就认为所有操作都非阻塞。

服务端解析可以使用 AI_PASSIVE 配合 node=NULL 获取通配绑定候选。需要决定监听哪些地址、是否建立多个监听 socket，以及 IPV6_V6ONLY 策略；AF_UNSPEC 不保证一个 fd 自动同时监听两种协议。

## 5. 运行并打印真实四元组

```bash
cc -std=c11 -Wall -Wextra -Werror modern_connect.c -o /tmp/modern-connect
/tmp/echo-server poll
# 另一终端
/tmp/modern-connect localhost 9090
```

预期看到本地临时端口与远端 9090。程序用 sockaddr_storage 保存地址，再用 getnameinfo(NI_NUMERICHOST | NI_NUMERICSERV) 格式化，避免硬编码所有结果都是 sockaddr_in。

getaddrinfo 的候选地址属于返回链，freeaddrinfo 后不能继续保留 ai_addr 指针使用；socket 连接成功后内核已经使用了必要地址，释放候选链是正常步骤。

## 6. C 与 C++ 的资源边界

C 可以用单一 cleanup 标签处理失败路径，初始化 fd=-1，仅在拥有有效 fd 时 close。C++ 则可用不可复制、可移动的 Fd 类，让析构统一释放。

```cpp
Fd socket_owner(::socket(AF_INET, SOCK_STREAM, 0));
// 抛异常、提前 return 时，owner 析构负责 close。
```

完整 Fd 类见 [client.cpp](https://github.com/ageni01/ageno-block/blob/main/examples/io-multiplexing/client.cpp)。RAII 不会自动解决 TCP 半包、线程竞争和业务超时；它解决的是资源所有权。fd 被交给异步任务时还要保证对象不会提前销毁。

## 7. 继续学多路复用

固定地址与 DNS 解析是一条维度，阻塞与非阻塞是一条维度，select/poll/epoll 又是等待接口的维度。可以组合使用 getaddrinfo + 非阻塞 connect + epoll，也可以 getaddrinfo + 阻塞客户端。

下一步分别阅读：[select 完整步骤](./06-select-tutorial.md)、[poll 完整步骤](./07-poll-tutorial.md)、[epoll 完整步骤](./08-epoll-tutorial.md)。

参考：[getaddrinfo](https://man7.org/linux/man-pages/man3/getaddrinfo.3.html)、[inet_pton](https://man7.org/linux/man-pages/man3/inet_pton.3.html)、[字节序](https://man7.org/linux/man-pages/man3/byteorder.3.html)。
