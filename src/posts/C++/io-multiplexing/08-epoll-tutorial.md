---
title: epoll 完整实战：注册、等待、动态写事件与 LT 到 ET
date: 2026-10-30
category:
  - C++
tag:
  - epoll
  - Socket
---

# epoll 完整实战：注册、等待、动态写事件与 LT 到 ET

本篇单独按“建立服务 → 构造关注 → 等待 → 处理 → 更新/关闭”讲 epoll，并直接展示配套完整 C 源码，便于边读边运行。前置：[Socket 传统与现代写法](./05-socket-old-modern.md)。

<!-- more -->

## 1. Linux 前置条件与选择

epoll 是 Linux 专用。本例先实现 LT，让状态机正确，再考虑 ET。服务端必须把监听和新连接都设非阻塞。不要把 Linux epoll API 当作 macOS 可直接运行的 API。

```c
int epfd = epoll_create1(EPOLL_CLOEXEC);
if (epfd < 0) { /* 处理失败 */ }
struct epoll_event interest = {0};
interest.events = EPOLLIN;
interest.data.u32 = MAX_CONN; /* 本例保留此值表示监听项 */
if (epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &interest) < 0) { /* 处理失败 */ }
```

epfd 表示 epoll 实例，不是监听 socket。data.u32 由我们定义为连接槽索引，内核原样回传；MAX_CONN 是监听哨兵值，不是 fd。

## 2. accept 后登记新连接

```c
struct epoll_event interest = {0};
interest.events = EPOLLIN;
interest.data.u32 = slot;
if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &interest) < 0) {
    /* 不要把未成功注册的连接留在表里；关闭并清理 */
}
```

accept_all 会循环到 EAGAIN，并显式设置每个 connfd 非阻塞。新连接在 ADD 前可能已有数据，epoll 注册会检查相应状态，不需要人为再发送一个字节“触发第一次事件”。

## 3. wait 返回的是事件，不是 payload

```c
struct epoll_event events[MAX_CONN + 1];
int ready = epoll_wait(epfd, events, MAX_CONN + 1, -1);
for (int k = 0; k < ready; ++k) {
    unsigned slot = events[k].data.u32;
    unsigned flags = events[k].events;
    /* slot==MAX_CONN：监听；否则取 clients[slot] */
}
```

maxevents 是本次最多返回多少项，不是总连接数。IN 后 recv，OUT 后 send；recv 才得到业务字节。ERR/HUP 可在没有显式请求时返回。多个状态可以共存，因此用位运算。

## 4. 用 MOD 根据输出状态更新关注

```c
interest.events = (want_read(c) ? EPOLLIN : 0) |
                  (want_write(c) ? EPOLLOUT : 0);
interest.data.u32 = slot;
if (epoll_ctl(epfd, EPOLL_CTL_MOD, c->fd, &interest) < 0) {
    /* 记录错误，清理连接 */
}
```

业务有响应时先尝试 send。若只发出一部分，off 保存进度，MOD 加 OUT；下次 OUT 继续，发完再移除 OUT。读缓冲/输出缓冲无空间时暂停 IN，恢复时重新更新。

本例每次事件后执行 MOD，便于教学；高性能实现可以比较新旧掩码，仅在变化时 MOD，减少系统调用。优化前先证明状态转换正确。

## 5. 关闭与本批事件生命周期

```c
epoll_ctl(epfd, EPOLL_CTL_DEL, c->fd, NULL);
close(c->fd);
c->fd = -1;
```

这是清理顺序示意，真实代码检查/记录必要错误。DEL 不是清除用户已经取回的 events 数组。fd/连接槽复用时，要避免旧事件指向新对象。本例先处理整批旧连接事件，再 accept 新连接，减少同一批复用的危险；生产常用连接ID与 generation。

同时注意 dup/fork 的共享打开文件描述，不要死记“任意 close 一次就绝对再无事件”。data.ptr 的对象更不能提前 free。

## 6. LT 到 ET 怎样改才完整

LT 默认模式下，只要条件仍满足，下次 wait 还可能报告。ET 加 EPOLLET 后，不能指望未读部分不断得到提醒。需同时检查：

1. 监听与连接全非阻塞。
2. accept 持续到 EAGAIN。
3. recv 持续到 EAGAIN，或因预算/背压暂停时记住尚未 drain 的状态。
4. 输出先尝试 send，保存剩余，正确处理后续可写。
5. 若读写预算耗尽但未到 EAGAIN，放用户态 ready 队列继续调度。
6. ONESHOT 若启用，处理完必须明确 MOD rearm。

因此不能只给本例 interest.events 加一个 EPOLLET 就宣称完成改造：固定缓冲可能在 EAGAIN 前满，必须补恢复与公平性策略。本例完整源码保持 LT，ET 的原理和骨架见 [内核与 Reactor](./03-kernel-reactor.md)。

## 7. 内核结构对应哪一步

ADD 建立持久关注项和 readiness 通知关系；TCP 接收改变 socket 状态后，回调把相关项加入就绪管理结构；wait 处理候选项、复制事件，应用随后 recv。Linux 内部具体结构随版本变化，不要把红黑树或链表当成用户态可依赖 ABI。

当 N 很大但活跃 K 很少，避免每轮无差别扫描 N 有优势；返回 K 项和处理实际字节仍有成本，不能说整个 epoll 服务器是无条件 O(1)。


## 编译与实测步骤

下载完整源码 [echo_server.c](https://github.com/ageni01/ageno-block/blob/main/examples/io-multiplexing/echo_server.c) 与 [client.cpp](https://github.com/ageni01/ageno-block/blob/main/examples/io-multiplexing/client.cpp)，执行：

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror echo_server.c -o /tmp/echo-server
c++ -std=c++17 -O2 -Wall -Wextra -Werror client.cpp -o /tmp/echo-client
/tmp/echo-server epoll
# 另一终端：
/tmp/echo-client 'hello ageno'
```

打开多个客户端验证彼此不阻塞；用慢读客户端检查输出缓冲和动态写关注；用半关闭检查 EOF 后剩余回显。详细抓包与并发测试见 [实验篇](./04-c-cpp-lab.md)。

## 完整 C 服务端：可切换三种等待后端

以下是同一份完整源码。先按本课模式运行，再与另外两课比较，公共 socket 生命周期、连接缓冲与 service 不变，变化集中在等待和关注更新部分。单一源码避免复制出三份行为不一致的读写状态机。

```c
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#ifdef __linux__
#include <sys/epoll.h>
#endif
#define MAX_CONN 128
#define CAP 65536
struct client { int fd, eof; size_t off, len; unsigned char out[CAP]; };
static struct client clients[MAX_CONN];
static int nonblock(int fd) {
    int f = fcntl(fd, F_GETFL, 0);
    return f < 0 ? -1 : fcntl(fd, F_SETFL, f | O_NONBLOCK);
}
static void drop(struct client *c) {
    close(c->fd); c->fd = -1; c->off = c->len = 0; c->eof = 0;
}
/* Return -1 on fatal error or fully drained EOF, otherwise retain connection. */
static int service(struct client *c, int readable, int writable) {
    if (readable && !c->eof) {
        if (c->off) {
            memmove(c->out, c->out + c->off, c->len - c->off);
            c->len -= c->off; c->off = 0;
        }
        while (c->len < CAP) {
            ssize_t n = recv(c->fd, c->out + c->len, CAP - c->len, 0);
            if (n > 0) { c->len += (size_t)n; continue; }
            if (n == 0) { c->eof = 1; break; }
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            return -1;
        }
    }
    if (writable || c->off < c->len) {
        while (c->off < c->len) {
            ssize_t n = send(c->fd, c->out + c->off, c->len - c->off, 0);
            if (n > 0) { c->off += (size_t)n; continue; }
            if (n < 0 && errno == EINTR) continue;
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) break;
            return -1;
        }
        if (c->off == c->len) c->off = c->len = 0;
    }
    return c->eof && c->len == 0 ? -1 : 0;
}
static int want_read(const struct client *c) {
    return !c->eof && c->len - c->off < CAP;
}
static int want_write(const struct client *c) { return c->off < c->len; }
static void accept_all(int listener, int use_select, int epfd) {
    for (;;) {
        struct sockaddr_in peer; socklen_t size = sizeof peer;
        int fd = accept(listener, (struct sockaddr *)&peer, &size);
        if (fd < 0) {
            if (errno == EINTR) continue;
            if (errno != EAGAIN && errno != EWOULDBLOCK) perror("accept");
            return;
        }
        if ((use_select && fd >= FD_SETSIZE) || nonblock(fd) < 0) { close(fd); continue; }
        int i; for (i = 0; i < MAX_CONN && clients[i].fd >= 0; ++i) { }
        if (i == MAX_CONN) { close(fd); continue; }
        clients[i].fd = fd; clients[i].eof = 0; clients[i].off = clients[i].len = 0;
        char ip[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof ip))
            fprintf(stderr, "accept fd=%d peer=%s:%u\n", fd, ip, (unsigned)ntohs(peer.sin_port));
#ifdef __linux__
        if (epfd >= 0) {
            struct epoll_event ev = {0}; ev.events = EPOLLIN; ev.data.u32 = (unsigned)i;
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) { perror("epoll add"); drop(&clients[i]); }
        }
#else
        (void)epfd;
#endif
    }
}
int main(int argc, char **argv) {
    const char *mode = argc > 1 ? argv[1] : "poll";
    if (strcmp(mode, "select") && strcmp(mode, "poll") && strcmp(mode, "epoll")) {
        fprintf(stderr, "usage: %s select|poll|epoll\n", argv[0]); return 1;
    }
#ifndef __linux__
    if (!strcmp(mode, "epoll")) { fprintf(stderr, "epoll requires Linux\n"); return 1; }
#endif
    signal(SIGPIPE, SIG_IGN); /* Failed send becomes EPIPE instead of process termination. */
    for (int i = 0; i < MAX_CONN; ++i) clients[i].fd = -1;
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) { perror("socket"); return 1; }
    int yes = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) < 0) { perror("setsockopt"); return 1; }
    struct sockaddr_in addr = {0}; addr.sin_family = AF_INET;
    addr.sin_port = htons(9090); (void)inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (bind(listener, (struct sockaddr *)&addr, sizeof addr) < 0 || listen(listener, 128) < 0 || nonblock(listener) < 0) {
        perror("listener"); close(listener); return 1;
    }
    int use_select = !strcmp(mode, "select");
    if (use_select && listener >= FD_SETSIZE) { fprintf(stderr, "listener exceeds FD_SETSIZE\n"); close(listener); return 1; }
    fprintf(stderr, "%s listening on 127.0.0.1:9090\n", mode);
#ifdef __linux__
    if (!strcmp(mode, "epoll")) {
        int epfd = epoll_create1(EPOLL_CLOEXEC);
        if (epfd < 0) { perror("epoll_create1"); return 1; }
        struct epoll_event ev = {0}; ev.events = EPOLLIN; ev.data.u32 = MAX_CONN;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &ev) < 0) { perror("epoll listener"); return 1; }
        for (;;) {
            struct epoll_event events[MAX_CONN + 1];
            int n = epoll_wait(epfd, events, MAX_CONN + 1, -1);
            if (n < 0) { if (errno == EINTR) continue; perror("epoll_wait"); return 1; }
            /* Accept after processing this batch: no slot is reused while old events remain. */
            int accept_ready = 0;
            for (int k = 0; k < n; ++k) {
                unsigned i = events[k].data.u32; unsigned flags = events[k].events;
                if (i == MAX_CONN) { accept_ready = 1; continue; }
                struct client *c = &clients[i];
                if (c->fd < 0) continue;
                if ((flags & EPOLLERR) || service(c, flags & (EPOLLIN | EPOLLHUP), flags & EPOLLOUT) < 0) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, c->fd, NULL); drop(c); continue;
                }
                ev.events = (want_read(c) ? EPOLLIN : 0) | (want_write(c) ? EPOLLOUT : 0);
                ev.data.u32 = i;
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, c->fd, &ev) < 0) { perror("epoll mod"); drop(c); }
            }
            if (accept_ready) accept_all(listener, 0, epfd);
        }
    }
#endif
    for (;;) {
        fd_set reads, writes; FD_ZERO(&reads); FD_ZERO(&writes);
        struct pollfd pfds[MAX_CONN + 1]; int slots[MAX_CONN + 1];
        int nfds = 1, maxfd = listener;
        if (use_select) FD_SET(listener, &reads);
        pfds[0] = (struct pollfd){listener, POLLIN, 0}; slots[0] = -1;
        for (int i = 0; i < MAX_CONN; ++i) {
            struct client *c = &clients[i]; if (c->fd < 0) continue;
            if (use_select) {
                if (want_read(c)) FD_SET(c->fd, &reads);
                if (want_write(c)) FD_SET(c->fd, &writes);
                if (c->fd > maxfd) maxfd = c->fd;
            }
            pfds[nfds] = (struct pollfd){c->fd, (short)((want_read(c) ? POLLIN : 0) | (want_write(c) ? POLLOUT : 0)), 0};
            slots[nfds++] = i;
        }
        int n = use_select ? select(maxfd + 1, &reads, &writes, NULL, NULL) : poll(pfds, (nfds_t)nfds, -1);
        if (n < 0) { if (errno == EINTR) continue; perror("wait"); return 1; }
        int accept_ready = use_select ? FD_ISSET(listener, &reads) : (pfds[0].revents & POLLIN);
        for (int k = 1; k < nfds; ++k) {
            struct client *c = &clients[slots[k]];
            int r = use_select ? FD_ISSET(c->fd, &reads) : (pfds[k].revents & (POLLIN | POLLHUP));
            int w = use_select ? FD_ISSET(c->fd, &writes) : (pfds[k].revents & POLLOUT);
            int bad = !use_select && (pfds[k].revents & (POLLERR | POLLNVAL));
            if (bad || ((r || w) && service(c, r, w) < 0)) drop(c);
        }
        if (accept_ready) accept_all(listener, use_select, -1);
    }
}
```

## 验证范围

C11/C++17 编译与 select/poll 回显、并发、分片和半关闭已在 macOS 验证。epoll 分支需要 Linux，尚未在本机完成 Linux 编译运行。此例有连接数和内存上限，但未实现连接超时、生产过载退避、TLS 与平滑退出。

参考：[epoll 官方手册](https://man7.org/linux/man-pages/man7/epoll.7.html)。
