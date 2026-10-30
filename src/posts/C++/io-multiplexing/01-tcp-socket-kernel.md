---
title: TCP 与 Socket 全过程：从客户端 A 到服务器 B，再到内核与应用协议
date: 2026-10-30
category:
  - C++
tag:
  - TCP
  - Socket
  - Linux
---

# TCP 与 Socket 全过程：从客户端 A 到服务器 B，再到内核与应用协议

这套教程把网络报文、内核 socket、进程 fd、I/O 就绪与应用解析放在同一条线上。建议顺序：本文 → [select/poll/epoll API](./02-select-poll-epoll.md) → [内核实现与 LT/ET](./03-kernel-reactor.md) → [C/C++ 可运行实验](./04-c-cpp-lab.md)。

<!-- more -->

## 1. 先建立五个不同层次

| 名称 | 它是什么 | 不是什么 |
| --- | --- | --- |
| TCP 连接 | 两端协议栈维护的通信状态 | 一个 C++ 对象或线程 |
| 内核 socket | 内核中的协议端点及相关状态 | 用户态整数本身 |
| fd | 当前进程描述符表里的索引 | 网络包里携带的编号 |
| I/O 就绪 | 某项 I/O 此刻可以尝试而不必等待对应条件 | 一条完整业务消息已处理 |
| 应用消息 | 由应用协议定义的消息边界与内容 | 一次 send、一个 TCP 段或一次 recv |

最重要的分工：**内核根据网络协议把字节送进正确连接，程序根据 fd 取出字节，再由应用协议解析内容。** select/poll/epoll 主要回答“哪些 fd 值得进行 I/O”，不负责解析 JSON 或业务帧。

## 2. 四元组：为什么同一个服务器端口能接很多人

假定同一网络内，没有 NAT：

```text
A：10.0.0.10:53001  → B：10.0.0.20:9090
C：10.0.0.11:53002  → B：10.0.0.20:9090
```

一条 TCP 连接通常用源 IP、源端口、目的 IP、目的端口四元组来描述；指定 TCP 协议后讨论四元组，跨协议分类时常说五元组。在内核里还存在网络命名空间、地址族、设备绑定等上下文，四元组不是对所有环境中所有内核查找键的完整描述。

两条连接都访问 B:9090，但远端不同，因此可以区分。B 上 accept 出的两个 socket 本地端口都可能是 9090，并不需要每接一个用户就给服务器另开一个业务端口。

NAT 可能改写地址和端口；B 的 getpeername 看到的是 B 所在网络视角下的对端。代理终止并新建连接后，原客户端地址也不能简单靠 TCP 对端地址恢复。

## 3. 用户态的 fd 与内核对象怎样对应

```text
B 进程描述符表
fd 3 → 文件对象 → 监听 socket：本地 0.0.0.0:9090
fd 4 → 文件对象 → 已连接 socket：B:9090 ↔ A:53001
fd 5 → 文件对象 → 已连接 socket：B:9090 ↔ C:53002
```

这是概念图，Linux 实际还有 struct file、struct socket、struct sock 等对象与引用关系。fd 4 在 A 进程可能指向完全不同的东西。dup 后两个 fd 可以引用同一打开文件描述；close 后数字可能复用，因此不能把 fd 当成永久连接 ID。

监听 fd 等“新连接可接受”，连接 fd 等“已有连接的数据/状态变化”。epoll 返回监听 fd 可读时应 accept，返回连接 fd 可读时应 recv，二者不是同一种业务操作。

## 4. API 时序：两边分别调用什么

```text
B：socket → setsockopt → bind → listen → 等待监听 fd 可读 → accept
A：socket → connect
                                          ↓
B：得到独立 connfd ← 内核完成的连接进入可接受队列
A：send → TCP/IP/链路 → B 内核连接接收队列 → B 等到 connfd 可读 → recv
B：解析请求 → 生成响应 → send → A recv
A/B：shutdown 或 close → 内核推进连接关闭状态机
```

connect 的成功不要求 B 应用已经调用 accept；握手和队列由内核协议栈推进。应用迟迟不 accept 会受到队列和资源限制，不能无限积压。

### 4.1 socket

```c
int fd = socket(AF_INET, SOCK_STREAM, 0);
```

AF_INET 表示 IPv4，SOCK_STREAM 表示字节流；此组合通常得到 TCP。返回非负 fd，失败为 -1 并设置 errno。此时还没有建立远程连接。

### 4.2 bind 与 listen

```c
struct sockaddr_in addr = {0};
addr.sin_family = AF_INET;
addr.sin_port = htons(9090);
inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
bind(fd, (struct sockaddr *)&addr, sizeof addr);
listen(fd, 128);
```

片段省略错误处理，完整程序见实验。bind 选择本地地址与端口；127.0.0.1 仅本机可达，0.0.0.0 是监听所有 IPv4 本地地址的通配绑定，不应当成远端连接目标。

listen 把 socket 变成监听端点，128 是 backlog 请求值，不是“最多一共只能接 128 个用户”。Linux 的有效队列容量还受系统配置限制。常说半连接队列与已完成队列，SYN cookies 等机制也会影响实际行为，不能把所有内核版本画成完全相同的固定队列实现。

### 4.3 connect

```c
connect(fd, (struct sockaddr *)&server, sizeof server);
```

客户端通常不显式 bind，内核通过路由选择本地地址并分配临时端口。阻塞 connect 等连接成功或失败；非阻塞 connect 常返回 -1/EINPROGRESS，之后等可写或错误，再用 getsockopt(SOL_SOCKET, SO_ERROR) 判断最终结果。

### 4.4 accept 与地址查询

```c
struct sockaddr_in peer;
socklen_t len = sizeof peer;
int connfd = accept(listenfd, (struct sockaddr *)&peer, &len);
```

connfd 是新连接 fd，listenfd 不变。len 既提供缓冲区大小，也接收实际地址长度。可用 inet_ntop 将 peer.sin_addr 转为文本，用 ntohs 读取对端端口。

getsockname 查询本地端点，getpeername 查询已连接 socket 的对端。两边都查一次，就能把程序里的 fd 与 ss/tcpdump 看到的四元组对上。

Linux accept 返回的 socket 不保证继承监听 socket 的 O_NONBLOCK；应显式 fcntl，或 Linux 使用 accept4(..., SOCK_NONBLOCK | SOCK_CLOEXEC)。accept4 不是通用 POSIX API。

## 5. 握手与数据：不要把它们都交给应用解析

```text
A                       B 内核                  B 应用
SYN(seq=x)   ─────────→ 查找监听端点
              ←──────── SYN+ACK(seq=y, ack=x+1)
ACK(ack=y+1) ─────────→ 推进连接状态、进入可接受状态
                                              accept 获取连接
数据(seq=...) ────────→ TCP 校验、序号处理、重组、有序字节
                                              recv 取得字节
```

这是普通握手的简化逻辑；丢包、重传、Fast Open 等需要扩展状态。应用通过普通 TCP socket 不需要自己解码 IP/TCP 头。它拿到的是 TCP 交付的字节流。抓包工具可以看到协议头，recv 的普通 payload 缓冲区不会自动包含这些头。

## 6. A 调用 send 后，实际发生什么

1. 用户态把数据交给 send 系统调用。
2. 内核尝试把可接受部分放进发送路径与缓冲，返回已接受字节数。
3. TCP 根据拥塞控制、对端接收窗口、分段与重传策略推进发送。
4. 数据经路由、链路与网卡等到达 B。

普通 send 成功表示本机接受了这些字节，不表示对方应用已经 recv，更不表示业务事务成功。对方 TCP ACK 也主要代表协议栈确认，不等价于应用完成处理。需要业务确认时，应定义应用层响应。

一次 send 可以只接受一部分；非阻塞情况下缓冲不足返回 EAGAIN/EWOULDBLOCK。发送程序必须保存尚未发送的偏移，稍后继续，不能丢尾部。

## 7. 数据到达 B：系统与内核工作路径

以下以常见 Linux 物理网卡接收路径作教学模型，实际路径随驱动、内核版本、卸载、XDP、GRO、虚拟设备等变化：

```text
网卡接收帧
  → DMA 到主机接收缓冲
  → 中断/驱动调度 NAPI 轮询等机制处理批量接收
  → 链路/IP 层检查与处理
  → TCP 连接查找、序号校验、乱序处理/重组
  → 可读字节进入对应 socket 的接收路径
  → 通知等待该 socket 状态的等待队列/回调
  → 唤醒相关等待者，例如 epoll_wait 的线程
  → 调度器让线程继续运行
  → 线程 recv，把字节取到用户态缓冲
```

DMA 主要涉及设备与主机内存，不表示普通 recv 自动零拷贝到应用内存。epoll 通知也不直接携带业务 payload。loopback 测试不会经过真实网线与物理网卡，不能拿 loopback 抓包推断完整硬件路径。

RSS 可把接收流量按哈希分配到不同硬件队列，RPS/RFS 等可调整处理 CPU；它们与应用层 epoll 不是同一个机制。[Linux 网络扩展文档](https://docs.kernel.org/networking/scaling.html)

## 8. recv 的四种结果

```c
ssize_t n = recv(fd, buffer, sizeof buffer, 0);
```

| 结果 | 含义 | 应对 |
| --- | --- | --- |
| n > 0 | 得到这些字节 | 按精确长度处理，不能假设字符串结尾 |
| n == 0 | 正长度读取请求下，有序 EOF | 对端发送方向已结束；排空自己待发输出后再关闭 |
| -1/EINTR | 被信号中断 | 通常重试，注意退出策略和超时 |
| -1/EAGAIN 或 EWOULDBLOCK | 当前没有可读数据且为非阻塞 | 回事件循环等待 |
| -1/其他错误 | 如 ECONNRESET | 按连接错误清理 |

readable 也可能是 EOF 或错误，未必是新业务数据。就绪后状态可能被其他线程消耗，所以事件循环仍应使用非阻塞 fd，正确处理 EAGAIN。

## 9. 应用层怎样“解析出一条消息”

A 连续发送两条消息：

```text
send("hello")
send("world")
```

B 可能一次 recv 得到 helloworld，也可能先 he 再 llowor 再 ld。TCP 保序，但不保留 send 的边界。所谓粘包/半包，实质是应用把字节流误当成消息流。

常用帧协议：定长、分隔符、长度前缀。长度前缀例如：

```text
4 字节无符号长度（大端） | payload
00 00 00 05              | hello
```

每个连接维护自己的输入缓冲：追加新字节 → 不够 4 字节就等待 → 读取长度 → 验证上限 → 不够完整 body 就等待 → 提取一帧 → 循环解析余下帧。不能为每个 recv 重新清空尚未组成一帧的数据。

安全读取头部（片段）：

```cpp
std::uint32_t netLength;
std::memcpy(&netLength, input.data(), sizeof netLength);
std::uint32_t length = ntohl(netLength);
if (length > 1024 * 1024) throw std::runtime_error("frame too large");
```

只有在 input.size() >= 4 后才能执行，完整解析器见实验篇。不要强转未对齐指针并解引用；还要设置每连接内存上限、超时与畸形帧策略。

## 10. 你说的 htols：应当是 htons/htonl

常见名字是 htons、htonl、ntohs、ntohl，没有标准的 htols：

| API | 方向 | 宽度 |
| --- | --- | --- |
| htons | host → network | 16 位 |
| htonl | host → network | 32 位 |
| ntohs | network → host | 16 位 |
| ntohl | network → host | 32 位 |

h=host，n=network，s=short，l=long 是历史命名；htonl 转换 32 位，不是“当前平台 sizeof(long) 位”，LP64 平台 long 为 64 位也不能把它当成 64 位转换函数。

网络字节序通常指大端：数值 0x12345678 依次发送 12 34 56 78。小端主机内存里同一个数值的字节排列可能是 78 56 34 12，所以发送整数前需要协议规定编码。

```c
addr.sin_port = htons(9090);        /* 16 位端口写入网络结构 */
uint32_t header = htonl(5);        /* 32 位应用帧长度 */
unsigned peer_port = ntohs(peer.sin_port);
```

不要对已经由 inet_pton 写出的网络序地址再次 htonl。字符串 "9090" 是字符序列，不是把 htons 返回值打印成文本。UTF-8 文本、结构体 padding 和浮点数也不是调用一个 htonl 就能统一序列化。

### 10.1 旧地址 API 与现代写法

- inet_addr：只针对 IPv4，返回错误的历史约定容易与广播地址混淆。
- inet_ntoa：历史接口返回内部缓冲结果，现代代码更适合 inet_ntop。
- gethostbyname：旧式解析接口，现代通常使用 getaddrinfo。
- bzero：旧式清零接口，现代用初始化或 memset。
- sockaddr_in：IPv4 地址结构；sockaddr_in6：IPv6；sockaddr_storage 可放多种地址。

inet_pton 解析数值地址，不查询 DNS；getaddrinfo 解析主机/服务，返回候选地址链，逐个 socket/connect 尝试并在最后 freeaddrinfo。现代 API 也不意味着直接跨平台无需适配：Windows 要初始化 Winsock、使用 SOCKET/closesocket 等，不能复制 POSIX 的 int fd/close 假设。

## 11. 关闭为什么也会变成可读事件

shutdown(fd, SHUT_WR) 关闭本端发送方向，仍可继续接收。TCP 支持半关闭，客户端发送完请求后半关闭，服务端可读到 EOF 再回完响应。close 是释放当前 fd 引用，协议结束还受到其他引用、缓冲、TCP 状态与选项影响。

FIN 前已经发送的数据可能仍待应用读取；不要一看到 HUP 就把尚未读的数据全部丢弃。RST 则表示异常终止的一种情况。向失效连接 send 在 POSIX 上可能触发 SIGPIPE；应使用平台支持的选项/标志，或在进程策略中处理该信号。

## 12. 自检：能否回答这些问题

同一 9090 端口为什么有几十个 connfd？为什么 epoll_wait 不返回 hello 的字符串？为什么 recv 一次不够一条消息？为什么 accept 前握手也能完成？为什么 htonl 与 TCP 四元组查找不是同一层的处理？如果能分别从端点、内核、fd、就绪、应用协议解释，基本模型就建立起来了。

参考：[TCP](https://man7.org/linux/man-pages/man7/tcp.7.html)、[accept](https://man7.org/linux/man-pages/man2/accept.2.html)、[connect](https://man7.org/linux/man-pages/man2/connect.2.html)、[字节序 API](https://man7.org/linux/man-pages/man3/byteorder.3.html)。
