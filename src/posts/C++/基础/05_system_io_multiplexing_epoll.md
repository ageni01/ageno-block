# 系统网络模型与 epoll 详解

## 1. 文档目标

本文关注系统级网络 IO 模型，包括：

- 阻塞与非阻塞
- 同步与异步的基本概念
- `select`
- `poll`
- `epoll`
- Reactor 思路

---

## 2. 阻塞与非阻塞

### 2.1 阻塞 IO

阻塞模型中，调用 `recv()` 时如果没有数据，线程会挂起等待。

优点：

- 简单

缺点：

- 线程利用率低
- 连接多时扩展性差

### 2.2 非阻塞 IO

非阻塞模式下，若当前无数据，系统调用立即返回。

设置方式：

```cpp
#include <fcntl.h>

int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

---

## 3. `select`

### 3.1 特点

- 兼容性好
- 需要每次拷贝集合
- `fd` 数量受限

### 3.2 示例

```cpp
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int listen_fd = 0; // 假设已创建并监听
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listen_fd, &readfds);

    int ret = select(listen_fd + 1, &readfds, NULL, NULL, NULL);
    if (ret > 0 && FD_ISSET(listen_fd, &readfds)) {
        // 处理新连接
    }
    return 0;
}
```

---

## 4. `poll`

### 4.1 特点

- 无 `FD_SETSIZE` 限制
- 仍然是 O(n) 扫描

### 4.2 示例

```cpp
#include <poll.h>

int main() {
    struct pollfd fds[1];
    fds[0].fd = 0; // 示例占位
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, 1000);
    if (ret > 0 && (fds[0].revents & POLLIN)) {
        // 可读
    }
    return 0;
}
```

---

## 5. `epoll`

`epoll` 是 Linux 下高并发网络服务最常见的 IO 多路复用机制之一。

### 5.1 核心接口

- `epoll_create1`
- `epoll_ctl`
- `epoll_wait`

### 5.2 创建 epoll 实例

```cpp
#include <sys/epoll.h>
#include <unistd.h>

int main() {
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        return 1;
    }
    close(epfd);
    return 0;
}
```

### 5.3 注册关注事件

```cpp
#include <sys/epoll.h>

void addReadEvent(int epfd, int fd) {
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}
```

### 5.4 等待事件

```cpp
#include <sys/epoll.h>

int waitEvents(int epfd) {
    epoll_event events[64];
    int n = epoll_wait(epfd, events, 64, 1000);
    for (int i = 0; i < n; ++i) {
        int fd = events[i].data.fd;
        if (events[i].events & EPOLLIN) {
            // fd 可读
        }
    }
    return n;
}
```

---

## 6. 基于 epoll 的最小事件循环

```cpp
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

int setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        return 1;
    }

    setNonBlocking(listen_fd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(listen_fd);
        return 1;
    }

    if (::listen(listen_fd, 128) < 0) {
        ::close(listen_fd);
        return 1;
    }

    int epfd = epoll_create1(0);
    if (epfd < 0) {
        ::close(listen_fd);
        return 1;
    }

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    epoll_event events[64];

    while (true) {
        int n = epoll_wait(epfd, events, 64, -1);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if (fd == listen_fd) {
                sockaddr_in client{};
                socklen_t len = sizeof(client);
                int client_fd = ::accept(listen_fd, reinterpret_cast<sockaddr*>(&client), &len);
                if (client_fd >= 0) {
                    setNonBlocking(client_fd);
                    epoll_event client_ev{};
                    client_ev.events = EPOLLIN;
                    client_ev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev);
                }
            } else if (events[i].events & EPOLLIN) {
                char buf[1024];
                ssize_t m = ::recv(fd, buf, sizeof(buf), 0);
                if (m <= 0) {
                    ::close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                } else {
                    ::send(fd, buf, static_cast<size_t>(m), 0);
                }
            }
        }
    }

    ::close(epfd);
    ::close(listen_fd);
    return 0;
}
```

这个示例展示了最关键的结构：

1. 监听 fd 放入 `epoll`
2. 新连接到来时 `accept`
3. 客户端 fd 也放入 `epoll`
4. 读事件触发后接收数据

---

## 7. LT 与 ET

### 7.1 LT

Level Triggered，水平触发。

特点：

- 默认模式
- 只要缓冲区里还有数据，就会继续通知
- 更容易写对

### 7.2 ET

Edge Triggered，边沿触发。

特点：

- 状态变化时通知一次
- 必须一次读到 `EAGAIN`
- 复杂度更高，但事件量更少

### 7.3 ET 示例注册方式

```cpp
epoll_event ev{};
ev.events = EPOLLIN | EPOLLET;
ev.data.fd = fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
```

---

## 8. Reactor 思路

一个典型 Reactor 模型会拆成：

1. 事件监听器
2. 连接管理器
3. 输入缓冲区
4. 协议解析器
5. 输出缓冲区
6. 业务处理器

建议分层：

- 网络层只负责收发
- 协议层只负责解码编码
- 业务层只处理业务对象

---

## 9. 常见问题

### 9.1 忘记设非阻塞

在 `epoll` 下若仍然用阻塞 fd，容易卡住事件循环。

### 9.2 ET 模式没有循环读空

会导致后续数据不再触发读取。

### 9.3 没处理断开连接

需要关注：

- `recv == 0`
- `EPOLLHUP`
- `EPOLLERR`

### 9.4 没有输出缓冲

写大数据时不能假设一次 `send()` 完成。

---

## 10. 工程建议

1. 新手先用 LT 模式
2. 单线程事件循环先跑通
3. 再加连接对象和缓冲区
4. 再接入协议解析
5. 最后再考虑线程池和多 Reactor

