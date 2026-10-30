---
title: poll 完整实战：pollfd 数组、事件掩码与多连接回显
date: 2026-10-30
category:
  - C++
tag:
  - poll
  - Socket
---

# poll 完整实战：pollfd 数组、事件掩码与多连接回显

本篇单独按“建立服务 → 构造关注 → 等待 → 处理 → 更新/关闭”讲 poll，并直接展示配套完整 C 源码，便于边读边运行。前置：[Socket 传统与现代写法](./05-socket-old-modern.md)。

<!-- more -->

## 1. 本课要独立掌握的流程

先建立非阻塞 listener，连接表保存已 accept 的连接。每轮构造 pollfd 数组，第一项监听，其余项根据连接状态选择 POLLIN/POLLOUT，并建立数组项到连接槽的映射。

```c
struct pollfd pfds[MAX_CONN + 1];
int slots[MAX_CONN + 1];
int count = 1;
pfds[0] = (struct pollfd){listener, POLLIN, 0};
slots[0] = -1;
for (int i = 0; i < MAX_CONN; ++i) {
    struct client *c = &clients[i];
    if (c->fd < 0) continue;
    short interest = (short)((want_read(c) ? POLLIN : 0) |
                             (want_write(c) ? POLLOUT : 0));
    pfds[count] = (struct pollfd){c->fd, interest, 0};
    slots[count++] = i;
}
int ready = poll(pfds, (nfds_t)count, -1);
```

nfds 是元素数量，不是最大 fd+1。events 是输入关注，revents 是输出结果。poll 没有 fd_set 的固定编号上限，但仍受资源与规模成本限制。

## 2. 正确读取 revents

```c
if (ready > 0) {
    for (int k = 1; k < count; ++k) {
        struct client *c = &clients[slots[k]];
        short flags = pfds[k].revents;
        int r = flags & (POLLIN | POLLHUP);
        int w = flags & POLLOUT;
        int bad = flags & (POLLERR | POLLNVAL);
        if (bad || ((r || w) && service(c, r, w) < 0)) drop(c);
    }
    if (pfds[0].revents & POLLIN) accept_all(listener, 0, -1);
}
```

events 不被改写成结果，revents 才是本轮结果。使用位掩码，别用 flags==POLLIN，因为 IN 与 HUP 可以同时返回。ready 是非零 revents 的数组项数量，不是收到多少字节。

本教学示例遇 ERR/NVAL 直接释放连接；生产代码可以先查询 SO_ERROR、记录更具体信息并制定尾部处理策略。HUP 不能等同于“缓冲已空”，本例让读处理器继续 recv，并在已读 EOF 且输出排空时关闭。

## 3. 为什么需要 slots 映射

pollfd 数组通常紧凑排列，但连接槽可能有空洞。pfds[3] 对应的连接不一定是 clients[2]，更不是 fd=3。显式 slots[k] 防止混淆数组下标、连接 ID 与 fd。

也可以长期维护 pollfd 数组，删除时交换尾项，但必须同步修复映射；处理当前事件批次时原地换位，还可能跳过或重复处理事件。为了教学，本例每轮重建数组，逻辑更清楚。

## 4. 超时与特殊条件

timeout 毫秒，-1 无限等待，0 立即返回；返回 0 是超时，不是连接关闭。返回 -1 要检查 EINTR 等错误。fd<0 的项被忽略，可用来临时停用数组槽。

POLLERR/POLLHUP/POLLNVAL 不必在 events 里请求也可返回。不要给所有连接永远加 POLLOUT：大多数空闲 TCP socket 一直可写，poll 会立即返回造成空转。输出有剩余才关注，发完取消。

## 5. 背压怎样在 poll 上体现

out 缓冲达到上限时 want_read 返回假，本轮 events 不含 POLLIN；写事件使 out 排空后，下轮恢复 POLLIN。这样内存有界，但仍需要连接超时与整体内存策略。

poll 仍需要读取本轮关注数组、查询对象、返回各项结果；用户通常也扫数组。它比 select 更好表达大 fd，却不是 epoll 式持久关注机制。对应内核与 TCP 的过程没有改变：通知就绪后仍由应用 recv/send。


## 编译与实测步骤

下载完整源码 [echo_server.c](https://github.com/ageni01/ageno-block/blob/main/examples/io-multiplexing/echo_server.c) 与 [client.cpp](https://github.com/ageni01/ageno-block/blob/main/examples/io-multiplexing/client.cpp)，执行：

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror echo_server.c -o /tmp/echo-server
c++ -std=c++17 -O2 -Wall -Wextra -Werror client.cpp -o /tmp/echo-client
/tmp/echo-server poll
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

参考：[poll 官方手册](https://man7.org/linux/man-pages/man2/poll.2.html)。
