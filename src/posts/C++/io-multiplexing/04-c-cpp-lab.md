---
title: C/C++ Socket 实战：同一服务切换 select、poll、epoll，配合抓包理解全过程
date: 2026-10-30
category:
  - C++
tag:
  - C
  - C++
  - Socket
  - I/O 多路复用
---

# C/C++ Socket 实战：同一服务切换 select、poll、epoll，配合抓包理解全过程

前面三篇分别建立 [TCP/字节序](./01-tcp-socket-kernel.md)、[API](./02-select-poll-epoll.md)、[内核与 Reactor](./03-kernel-reactor.md) 的模型。本篇把它们放到可编译程序中逐项观察。

<!-- more -->

## 1. 源代码与运行边界

配套完整源码保存在本博客 GitHub 仓库的 [examples/io-multiplexing](https://github.com/ageni01/ageno-block/tree/main/examples/io-multiplexing)：

- `echo_server.c`：C11，多连接、非阻塞读写、有界输出缓冲；命令行切换 select/poll/epoll。
- `client.cpp`：C++17，RAII fd、循环发送、半关闭、精确校验回显。
- `frame_decoder.cpp`：长度前缀解析器，验证半包、连续多帧、二进制和大小限制。

select/poll 部分适用于本示例使用的 POSIX 环境；epoll 分支只有 Linux 编译。示例不是 Windows Winsock 代码。本机 macOS 已编译并验证 select/poll 与 C++ 客户端；当前没有可用 Linux Docker 守护进程，因此 epoll 分支尚未完成 Linux 编译运行验证，不能把前两种通过当作第三种也已通过。

## 2. 编译与第一次通信

克隆仓库后进入 examples/io-multiplexing：

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror echo_server.c -o /tmp/echo-server
c++ -std=c++17 -O2 -Wall -Wextra -Werror client.cpp -o /tmp/echo-client
c++ -std=c++17 -O2 -Wall -Wextra -Werror frame_decoder.cpp -o /tmp/frame-decoder
/tmp/frame-decoder
/tmp/echo-server select
```

另一终端运行：

```bash
/tmp/echo-client 'hello ageno'
```

客户端应输出同样的 hello ageno，服务端记录新连接 fd 和 peer IP:端口。服务端 Ctrl+C 后换成 `/tmp/echo-server poll` 再试；Linux 上再运行 `/tmp/echo-server epoll`。

服务端绑定 127.0.0.1:9090，仅本机访问。若 A/B 是两台机器，B 需要绑定可达接口地址，A 将 inet_pton 中的目标改为 B 的地址；同时检查路由、防火墙、容器映射。不要把 localhost 当成远程服务器地址。

## 3. C 程序中的网络生命周期

main 依次做：

```text
忽略 SIGPIPE → 初始化连接槽 → socket → SO_REUSEADDR
  → 填 sockaddr_in → htons(9090) → inet_pton
  → bind → listen → 设置监听 fd 非阻塞
  → 按 mode 进入 select/poll/epoll 循环
```

SO_REUSEADDR 不是“任意多个进程都能抢同一监听地址”的万能开关，与 SO_REUSEPORT 也不是同一选项。操作失败要检查 errno，不要靠反复加选项掩盖端口占用。

accept_all 一直 accept 到 EAGAIN，每个新 fd 显式设非阻塞，保存到连接槽。select 模式还拒绝 fd >= FD_SETSIZE，避免越界。连接槽满时关闭新连接，作为明确的简化过载策略。

## 4. 连接状态必须保存什么

源码的每个 client 保存：

```c
struct client {
    int fd, eof;
    size_t off, len;
    unsigned char out[65536];
};
```

有效待发字节是 out[off..len)。eof 表示已确认对端发送方向结束，不代表可以立即丢掉待发响应。64KiB 是实验上限，最多 128 连接，静态总缓冲约 8MiB。

### 4.1 读入

如果前面已发送一部分，先把剩余输出紧凑搬到缓冲起点，再 recv 到剩余空间。每次正返回都增加 len；EINTR 重试，EAGAIN 暂停，0 标记 eof，其他错误关闭连接。

echo 服务不解析业务帧，因为它的协议就是原样回显所有字节。真正业务应把接收数据追加到独立输入缓冲，再调用 Decoder；输出缓冲不能兼作任意复杂业务解析的全部状态。

### 4.2 发送

从 out+off 继续 send，正返回推进 off，EAGAIN 保留偏移，全部发送后把 off/len 归零。eof 且待发为空才关闭。这支持客户端先 shutdown(SHUT_WR)，然后继续接收服务端回显。

### 4.3 关注事件

```text
未 EOF 且有缓冲空间 → 关注 IN
存在待发送字节       → 关注 OUT
EOF 且输出排空        → 清理连接
```

这个状态机与等待 API 分离，所以三种后端能够复用同一个 service 函数。epoll 模式用 MOD 更新；select 模式每轮重建集合；poll 模式每轮构建 pollfd 数组。

示例 epoll 使用 LT，未开启 EPOLLET。不要只加 ET 标志就宣称实现成熟 ET：满缓冲后暂不 drain、恢复读与公平性等状态需要重新设计与验证。

## 5. 为什么 C++ 客户端还要循环 send

C++ 没有另一套内核 TCP 协议，它仍调用同样的 POSIX socket API。主要区别是用类封装生命周期：

```cpp
class Fd {
    int value_;
public:
    explicit Fd(int value);
    ~Fd();
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& other) noexcept;
    int get() const noexcept;
};
```

禁止复制避免两个对象误 close 同一 fd，移动转移所有权。异常退出时析构函数仍释放已拥有的 fd。不要在析构里抛异常，也不要随意“失败后重试 close”而忽略 fd 可能复用的风险。

客户端为了突出 Socket 生命周期使用阻塞 connect/send/recv；这不妨碍它与非阻塞多路复用服务端通信。客户端也可以设计成事件驱动，但需要额外连接中、发送中、接收中的状态。

## 6. 给 TCP 字节流加长度前缀

frame_decoder.cpp 的协议：4 字节大端长度 + payload，最大 1MiB。一个 Decoder 对象对应一条连接，feed 接收每次 recv 的字节片段。

```text
feed(前2字节) → 不足头部，保留
feed(后2字节 + he) → 头部完整，body不足，保留
feed(llo + 下一帧...) → 产出 hello，再继续解析下一帧
```

解析器使用 memcpy 读 uint32_t，再 ntohl，而非把 char* 强制转换为 uint32_t* 直接读。它允许 payload 内含 '\0'，不使用 strlen。为简化内存边界，feed 约定一次不超过 4096 字节；接入网络时对应使用 4096 大小的 recv 缓冲。

调用示意（片段）：

```cpp
unsigned char buf[4096];
ssize_t n = recv(fd, buf, sizeof buf, 0);
if (n > 0) {
    decoder.feed(buf, static_cast<std::size_t>(n), [](std::string payload) {
        // 这里才得到完整应用消息，可继续解析 JSON/命令。
    });
}
```

网络 EOF 时如果 decoder.complete() 为 false，说明流结束在半帧位置，应按协议错误处理，而不是当成完整消息。示例解析器出错后应放弃连接，不继续复用其状态。

## 7. 抓包：把四元组和系统调用对起来

Linux 服务端运行时，另一终端观察：

```bash
ss -ltnp 'sport = :9090'
ss -tnp '( sport = :9090 or dport = :9090 )'
```

第一条看监听，第二条看连接。客户端默认很快结束，要观察 ESTABLISHED，可用另一个终端交互连接：

```bash
nc 127.0.0.1 9090
```

输入一行看到回显，暂时不要退出。不同 nc 实现的退出/半关闭参数不同，不要依赖所有平台都有同一个 -N/-q 行为。

抓包需要系统允许的权限：

```bash
# Linux loopback；macOS 将 lo 改为 lo0
sudo tcpdump -i lo -nn 'tcp port 9090'
```

观察源/目的地址和端口，SYN、SYN+ACK、ACK，数据与关闭。loopback 可能受抓包点、卸载和显示方式影响，不要把展示出来的每条记录当成“应用的一次 send”。NAT 和远程机器测试应选择正确接口，并在两端分别抓包对照。

## 8. 系统调用追踪：确认 epoll 不读业务内容

Linux 可使用：

```bash
strace -f -tt -e trace=network,epoll_create1,epoll_ctl,epoll_wait,select,pselect6,poll,ppoll,fcntl,close \
  /tmp/echo-server epoll
```

不同 libc 可能将用户函数转换为 pselect6 等底层系统调用；跟踪输出也随架构变化。macOS 不提供 strace，不能照抄此命令。

尝试把日志按时间连成：

```text
epoll_create1 → ADD listener → wait
客户端 connect → listener IN
accept → 得到 connfd → 设置非阻塞 → ADD connfd
wait → connfd IN
recv → 有效字节 → EAGAIN/EOF
send → 部分/全部 → 必要时 MOD OUT
DEL/close
```

有时候 accept 返回的连接已经有数据，ADD 之后很快就绪；不必假设每个步骤都严格对应一次独立 wait。查看 EAGAIN 是正确事件循环的正常结束条件，不是必然出错。

## 9. 几个最有帮助的实验

1. 连接后不输入，同时再开第二客户端：多路复用服务仍能处理第二个。
2. 发送超过 64KiB 的二进制，并同时接收：验证缓冲满后恢复和部分读写。
3. 多客户端并发：观察不同 connfd 与不同四元组。
4. 半关闭写方向：观察服务器仍排空回显，再关闭连接。
5. 断开客户端：区分有序 EOF 与 reset 等错误。
6. 将 echo 改为长度帧：分多次发送同一头部，再把多帧合成一次发送。
7. Linux 改 LT 为 ET 时故意只 recv 一次：观察剩余数据为何可能停住，然后按 ET 状态规则修复。

当客户端单线程先 send 巨量数据、直到发完才 recv 时，双方有界缓冲可能形成相互等待；测试大数据应并行发送和接收，或设计合适的请求/响应协议。不能看到这种测试死锁就直接断言 poll 或 epoll 失效。

## 10. 验证记录与下一步

本次 macOS 验证：C11/C++17 严格告警编译，select/poll 分别运行 C++ 回显客户端；8 并发连接、每连接 1MiB 二进制、分片发送和写半关闭；帧解析器验证多种切分长度、合并帧、包含零字节与超长拒绝。Linux epoll 尚未实测。

生产还需要连接/空闲超时、总内存上限、EMFILE 等过载退避、信号退出、TLS、安全策略、连接 ID 生命周期、性能测量与长期测试。把这些逐步加到状态机，远比只背三个函数名更能理解网络服务器。

参考：[recv](https://man7.org/linux/man-pages/man2/recv.2.html)、[send](https://man7.org/linux/man-pages/man2/send.2.html)、[epoll](https://man7.org/linux/man-pages/man7/epoll.7.html)。
