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
