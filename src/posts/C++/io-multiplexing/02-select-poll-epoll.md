---
title: select、poll、epoll API 逐项详解：设计差异与事件循环
date: 2026-10-30
category:
  - C++
tag:
  - select
  - poll
  - epoll
---

# select、poll、epoll API 逐项详解：设计差异与事件循环

前置阅读：[TCP、Socket 与四元组](./01-tcp-socket-kernel.md)。先明确目标：让一个线程在大量连接之间等待“有进展的可能”，然后只处理就绪连接；不是让一次 recv 同时读所有 socket。

<!-- more -->

## 1. 为什么需要多路复用

阻塞式串行服务器：accept A → recv A。A 不发送时线程停在 A，已经到达的 B 无法被同一线程及时处理。

每连接一线程能缓解串行阻塞，但线程栈、调度、同步和连接规模有成本。另一种方式是所有连接非阻塞、循环 recv，遇 EAGAIN 换下一个；这会在没有数据时反复空转。

I/O 多路复用把“等很多对象”交给内核：

```text
注册/提交关注对象
    ↓
等待集合里至少一项有状态变化
    ↓
得到就绪信息
    ↓
accept / recv / send
    ↓
更新连接状态和关注事件，再等待
```

线程可以阻塞在 select/poll/epoll_wait，但对单个连接的操作使用非阻塞模式。**阻塞等待事件与非阻塞处理连接可以同时成立。**

## 2. 阻塞、非阻塞、同步、异步别混淆

O_NONBLOCK 控制某个 fd 操作在条件不足时是否等待。多路复用是就绪通知：应用收到通知后仍自己执行 recv/send；普通用法仍属于同步 I/O。完成通知模型则可在提交操作后获得完成结果，例如某些异步 I/O 接口，但不能把所有异步接口概括为一模一样。

```c
int old = fcntl(fd, F_GETFL, 0);
if (old == -1 || fcntl(fd, F_SETFL, old | O_NONBLOCK) == -1) {
    /* 处理错误 */
}
```

保留原 flags，不要只写 F_SETFL, O_NONBLOCK 覆盖其他标志。Linux 上 O_NONBLOCK 属于打开文件描述的状态，dup 等共享引用要注意影响。CLOEXEC 则与 exec 时描述符继承相关，不是非阻塞开关。

## 3. select：使用位集合提交关注对象

```c
#include <sys/select.h>
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
```

### 3.1 参数与返回

| 项目 | 含义 |
| --- | --- |
| nfds | 最大关注 fd + 1，不是连接数 |
| readfds | 关注读取方向的集合，返回后变成该方向就绪集合 |
| writefds | 关注写入方向的集合，同样会被修改 |
| exceptfds | 特殊异常条件，例如带外数据；不是所有普通错误总入口 |
| timeout=NULL | 无限等待 |
| timeval={0,0} | 立即检查并返回 |
| 正返回值 | 三个输出集合中的就绪位总数，同一 fd 在不同集合可能重复计数 |
| 0 | 超时 |
| -1 | 失败，检查 errno，例如 EINTR、EBADF |

Linux 可能修改 timeval，因此每轮重新构造；跨平台也不要依赖返回后 timeout 保持原值。FD_SETSIZE 限制的是 fd 数值范围，不能把 >= FD_SETSIZE 的 fd 放进 fd_set。在常见 glibc 中该值为 1024；不是“系统最多只能打开 1024 个文件”。

### 3.2 四个宏

```c
FD_ZERO(&set);       /* 清空 */
FD_SET(fd, &set);    /* 添加 */
FD_CLR(fd, &set);    /* 移除 */
FD_ISSET(fd, &set);  /* 返回是否在集合 */
```

select 每次覆盖集合，所以保存 master，再复制出 working set：

```c
fd_set master;
FD_ZERO(&master);
FD_SET(listenfd, &master); /* 已确认 listenfd < FD_SETSIZE */
for (;;) {
    fd_set readable = master;
    struct timeval timeout = {5, 0};
    int n = select(maxfd + 1, &readable, NULL, NULL, &timeout);
    if (n < 0) { if (errno == EINTR) continue; /* 记录错误并退出 */ break; }
    if (n == 0) continue;
    for (int fd = 0; fd <= maxfd; ++fd) {
        if (!FD_ISSET(fd, &readable)) continue;
        /* listenfd: accept 到 EAGAIN；connfd: recv 并更新状态 */
    }
}
```

这是 API 骨架，省略了关闭后 FD_CLR、maxfd 更新和写缓冲。完整程序见实验篇。不能把第一次 select 改写后的集合直接作为下一次输入，否则未就绪连接会“消失”。

### 3.3 设计成本

位集合紧凑、接口历史悠久，但用户每轮重建/复制关注集合，内核检查给定范围并回传集合，用户再查找哪些位就绪。fd 很稀疏但最大值很大时也不理想。简单说有扫描成本，但具体位图优化与常数要看实现。

## 4. poll：数组让输入和输出更清楚

```c
#include <poll.h>
struct pollfd {
    int fd;
    short events;
    short revents;
};
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```

nfds 是数组元素数量；events 是输入关注，revents 是输出结果。timeout 以毫秒计，0 立即返回，负值无限等待。正返回是 revents 非零的数组项数量；0 超时；-1 失败。

```c
struct pollfd fds[2] = {
    { .fd = listenfd, .events = POLLIN },
    { .fd = connfd, .events = POLLIN }
};
int n = poll(fds, 2, -1);
if (n > 0) {
    if (fds[0].revents & POLLIN) { /* accept */ }
    if (fds[1].revents & POLLIN) { /* recv */ }
}
```

### 4.1 事件掩码

| 标志 | 含义与处理 |
| --- | --- |
| POLLIN | 可读，可能数据或 EOF |
| POLLOUT | 写入可能取得进展，不保证任意大缓冲一次发完 |
| POLLPRI | 特殊高优先级条件 |
| POLLERR | 对象有错误，检查连接并清理 |
| POLLHUP | 挂断条件，可能仍有未读数据 |
| POLLNVAL | fd 无效 |

ERR/HUP/NVAL 可以在未要求对应 events 的情况下返回。用位运算 `&` 检查，不能 `revents == POLLIN`，因为多个条件可以同时出现。`fd < 0` 的数组项可临时忽略。

poll 没有 select 的固定 fd_set 位数限制，但受进程 fd 限额、数组大小、资源及实现限制。它仍每次提交数组、扫描对象、返回数组；用户通常仍要遍历数组找 revents。它改变了表达方式，并未自动变成持久化内核关注表。

## 5. epoll：把关注集合留在内核

epoll 是 Linux API。macOS/BSD 常用 kqueue，Windows 的高并发模型常见 IOCP。不要拿 Linux 源码直接假设这些 API 有完全相同的语义。

```c
#include <sys/epoll.h>
int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events,
               int maxevents, int timeout);
```

### 5.1 创建

```c
int epfd = epoll_create1(EPOLL_CLOEXEC);
```

返回一个指向 epoll 实例的 fd，最后 close。EPOLL_CLOEXEC 避免 exec 后泄漏描述符。旧 epoll_create(size) 的 size 是历史参数，现代内核不把它当固定连接容量；优先 create1。

### 5.2 注册与修改

```c
struct epoll_event ev = {0};
ev.events = EPOLLIN;
ev.data.fd = connfd;
if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) == -1) { /* error */ }
```

| 操作 | 语义 | 常见错误 |
| --- | --- | --- |
| EPOLL_CTL_ADD | 新增关注对象与事件 | 已存在 EEXIST |
| EPOLL_CTL_MOD | 修改已有事件与 data | 不存在 ENOENT |
| EPOLL_CTL_DEL | 删除关注 | 生命周期管理错误 |

每次都检查返回值；不要认为所有 fd 类型都支持 epoll，比如普通磁盘文件通常不能按 socket 的方式注册。

`data` 是联合体，可以存 fd、u64 或 ptr。内核按注册值回传，它不是内核帮你分析的业务数据。若存指针，必须保证事件处理前对象没被释放；若存 fd，必须处理 fd 复用。实际服务器常用连接 ID 加 generation 管理生命周期。

### 5.3 等待

```c
struct epoll_event events[64];
int n = epoll_wait(epfd, events, 64, -1);
for (int i = 0; i < n; ++i) {
    int fd = events[i].data.fd;
    unsigned flags = events[i].events;
    if (flags & EPOLLIN) { /* accept 或 recv */ }
    if (flags & EPOLLOUT) { /* 从待发偏移继续 send */ }
}
```

maxevents 必须大于 0，限制这次输出事件数量，不限制总注册数。timeout 单位毫秒，-1 无限等待、0 立即返回。n 是本次返回事件数量，0 超时，-1 失败；EINTR 可按退出/重试策略处理。

EPOLLERR、EPOLLHUP 不必显式注册也可能返回。EPOLLRDHUP 可请求检测对端关闭发送方向；仍要通过 recv 处理尾部数据与 EOF。

## 6. 什么叫“可读”和“可写”

| fd/状态 | 就绪事件后通常做什么 |
| --- | --- |
| 监听 socket 可读 | accept 队列中可取得连接 |
| 连接 socket 可读 | recv 取得数据、EOF 或错误 |
| 连接 socket 可写 | send 可能接受部分字节；非阻塞 connect 也可能结束 |
| 非阻塞连接完成通知 | getsockopt(SO_ERROR) 确认成功还是失败 |

这里说“通常”，因为就绪是一个状态观察，可能被并发处理者消耗，也可能因错误变为就绪。因此处理器应始终准备面对 EAGAIN。

不要永久注册所有 socket 的 EPOLLOUT/POLLOUT。空闲连接通常一直可写，结果事件循环不断返回、CPU 空转。只在输出缓冲仍有剩余时关注写；排空后取消写关注。

非阻塞 connect 示例骨架：

```c
int rc = connect(fd, (struct sockaddr *)&addr, sizeof addr);
if (rc == -1 && errno == EINPROGRESS) {
    /* 注册可写和错误；收到事件后执行： */
    int error = 0; socklen_t length = sizeof error;
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length) == -1) { /* 系统调用失败 */ }
    else if (error != 0) { /* 连接失败，error 是具体错误号 */ }
    else { /* 连接成功，可以继续协议状态机 */ }
}
```

片段省略 connect 立即成功和其他错误分支。可写不是连接必定成功的凭证。

## 7. 相同连接，三种 API 的表达

假设监听 fd=3，A=4，B=5，只有 A 发数据：

```text
select 输入 readfds={3,4,5} → 输出 readfds={4} → 用户找第4位
poll 输入 [{3,IN},{4,IN},{5,IN}] → 第2项 revents=IN → 用户扫数组
epoll 先 ADD 3/4/5 → wait 输出 [{data=4,IN}] → 用户处理输出项
```

这只是示意 fd 数字。epoll 优化的是“跨次保存关注关系、通过就绪机制收集候选、按批输出事件”。它没有让 TCP 收包、内存复制、协议解析或业务计算消失。

## 8. 三种方案的选择

| 比较点 | select | poll | epoll |
| --- | --- | --- | --- |
| 集合表达 | 位图 | pollfd 数组 | 内核持久关注集合 |
| 每轮输入 | 重设/复制位图 | 提交数组 | 通常仅 wait，变化时 ctl |
| 用户查找结果 | 查集合 | 查 revents | 遍历返回事件 |
| fd 范围 | 受 fd_set 限制 | 无此固定限制 | 无此固定限制 |
| 触发方式 | 通常 LT 语义 | 通常 LT 语义 | LT / ET |
| 可移植性 | POSIX 等平台广泛支持，细节不同 | POSIX，Windows 有不同接口 | Linux 专用 |

大量空闲连接、少量活跃连接是 epoll 的典型受益场景；若所有连接一直活跃、频繁增删，或业务计算占主导，差距取决于实际负载。不要背成“select/poll 永远慢、epoll 永远 O(1)”。下一篇会进一步拆开内核工作。

## 9. 超时与信号的进阶问题

循环收到 EINTR 时，如果每次都重新给完整 timeout，总等待可能超出预期；严格超时应计算单调时钟 deadline 并扣减剩余时间。pselect/ppoll/epoll_pwait 可在等待时原子调整信号掩码，解决“检查退出标志后、进入等待前信号到达”的竞态，但不是简单更换函数名就完成信号设计。

参考：[select](https://man7.org/linux/man-pages/man2/select.2.html)、[poll](https://man7.org/linux/man-pages/man2/poll.2.html)、[epoll](https://man7.org/linux/man-pages/man7/epoll.7.html)。继续阅读：[内核等待队列与 LT/ET](./03-kernel-reactor.md)。
