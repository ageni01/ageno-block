---
title: select 完整实战：从监听集合到多连接非阻塞回显
date: 2026-10-30
category:
  - C++
tag:
  - select
  - Socket
---

# select 完整实战：从监听集合到多连接非阻塞回显

本篇单独按“建立服务 → 构造关注 → 等待 → 处理 → 更新/关闭”讲 select，并直接展示配套完整 C 源码，便于边读边运行。前置：[Socket 传统与现代写法](./05-socket-old-modern.md)。

<!-- more -->

## 1. 本课要独立掌握的流程

socket/bind/listen 建立监听后，先把监听 fd 设为非阻塞。每一轮根据连接状态构造读/写集合，计算最大 fd，然后 select。返回后先处理本批连接事件，最后 accept 新连接。关闭连接后，下轮不再把它加入集合。

```c
fd_set reads, writes;
FD_ZERO(&reads); FD_ZERO(&writes);
FD_SET(listener, &reads);
int maxfd = listener;
for (int i = 0; i < MAX_CONN; ++i) {
    struct client *c = &clients[i];
    if (c->fd < 0) continue;
    if (want_read(c)) FD_SET(c->fd, &reads);
    if (want_write(c)) FD_SET(c->fd, &writes);
    if (c->fd > maxfd) maxfd = c->fd;
}
int count = select(maxfd + 1, &reads, &writes, NULL, NULL);
```

这些变量与辅助函数来自下文完整源码，不是可以脱离上下文编译的 main。maxfd+1 不是连接数量。fd 必须小于 FD_SETSIZE；完整示例在 accept 时检查这一点。

## 2. 为什么每次重建

select 把输入集合改成输出集合。第一次某个连接没有就绪，它的位就会在输出中被清掉。如果直接把输出作为下一次输入，就丢失了对它的监控。可以保存 master 再复制，也可以像本例根据连接表重建。

读集合和写集合分开，FD_ISSET 只对本轮返回后的集合检查。count 不是唯一 fd 数量：一个 fd 同时在读/写集合就绪时可计为多个位。不要假设处理 count 个不同连接就一定结束。

## 3. 事件怎么执行

```c
if (count < 0) {
    if (errno == EINTR) { /* 重试或退出 */ }
    else { /* 记录错误并结束循环 */ }
} else if (count > 0) {
    int accept_ready = FD_ISSET(listener, &reads);
    for (int i = 0; i < MAX_CONN; ++i) {
        struct client *c = &clients[i];
        if (c->fd < 0) continue;
        int r = FD_ISSET(c->fd, &reads);
        int w = FD_ISSET(c->fd, &writes);
        if ((r || w) && service(c, r, w) < 0) drop(c);
    }
    if (accept_ready) accept_all(listener, 1, -1);
}
```

监听可读后循环 accept 到 EAGAIN；普通连接可读后 recv，0 表示 EOF，EAGAIN 表示本轮暂时没有数据。可写后续发 out[off..len)。写缓冲空时不加入写集合，避免无意义唤醒。

## 4. 超时、异常与容易错的地方

NULL timeout 无限等待；timeval={0,0} 立即返回；需要 2 秒则每轮构造 timeval={2,0}，不能在 Linux 上反复复用被修改过的 timeval。要求总 deadline 时用单调时钟扣减。

exceptfds 不是所有 socket 错误的总开关；读取和写入返回值仍是最终判断。对 EOF 不 close 就继续监控读，会不断立即返回。maxfd 变小可以优化扫描范围；本例每轮重算，因此关闭最大 fd 后不需要单独修补旧最大值。

## 5. 从用户态对应到内核

本轮 fd_set 送入内核，内核查询对象的就绪状态并在需要时登记等待。数据到达后唤醒等待者，返回就绪集合；用户程序遍历集合后才 recv。select 既没有拷贝业务数据给你的 buf，也没有替你解析一条消息。

实践中连接不多时 select 很直观，但 fd_set 范围是硬约束。把 fd 限额调大不能让现有 FD_SET 自动安全容纳更大的 fd。


## 编译与实测步骤

下载完整源码 [echo_server.c](https://github.com/ageni01/ageno-block/blob/main/examples/io-multiplexing/echo_server.c) 与 [client.cpp](https://github.com/ageni01/ageno-block/blob/main/examples/io-multiplexing/client.cpp)，执行：

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror echo_server.c -o /tmp/echo-server
c++ -std=c++17 -O2 -Wall -Wextra -Werror client.cpp -o /tmp/echo-client
/tmp/echo-server select
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

参考：[select 官方手册](https://man7.org/linux/man-pages/man2/select.2.html)。
