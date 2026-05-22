# 04. Linux epoll 入门

## 1. epoll 解决什么问题

第 2 课的服务端是阻塞式写法：

```cpp
accept()
recv()
```

这种写法简单，但有问题：

如果一个客户端连上后一直不发数据，服务端可能卡在这个客户端的 `recv` 上。

想让一个线程同时处理很多客户端，就需要 I/O 多路复用。

Linux 常用的是 `epoll`。

## 2. epoll 只属于 Linux

`epoll` 是 Linux 专用 API。

macOS/BSD 没有 `epoll`，它们有类似的 `kqueue`。

所以本项目在 CMake 里写了：

```cmake
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(04_epoll_server_linux src/04_epoll_server_linux.cpp)
endif()
```

含义：只有 Linux 才编译 epoll 示例。

## 3. epoll 的三个核心函数

### epoll_create1

```cpp
int epoll_fd = epoll_create1(0);
```

创建一个 epoll 实例。

你可以理解为创建了一个“事件管理器”。

### epoll_ctl

```cpp
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
```

告诉 epoll：请帮我关注这个 fd。

常见操作：

```text
EPOLL_CTL_ADD: 添加 fd
EPOLL_CTL_MOD: 修改 fd
EPOLL_CTL_DEL: 删除 fd
```

### epoll_wait

```cpp
int n = epoll_wait(epoll_fd, events, max_events, timeout);
```

等待事件发生。

例如某个客户端 socket 可读，`epoll_wait` 就会返回。

## 4. 为什么 epoll 常配合非阻塞 socket

epoll 常见写法会把 socket 设置成非阻塞：

```cpp
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

非阻塞含义：

- 没有新连接时，`accept` 不会一直卡住。
- 没有数据时，`recv` 不会一直卡住。
- 它们会返回错误，并设置 `errno = EAGAIN` 或 `EWOULDBLOCK`。

## 5. epoll 服务端流程

看 [src/04_epoll_server_linux.cpp](../src/04_epoll_server_linux.cpp)。

流程是：

```text
socket
set_non_blocking
bind
listen
epoll_create1
epoll_ctl 添加 listen_fd

while true:
    epoll_wait
    如果 listen_fd 可读:
        accept 新客户端
        set_non_blocking(client_fd)
        epoll_ctl 添加 client_fd
    如果 client_fd 可读:
        recv 数据
        send 回显
```

## 6. 初学者先别急着写复杂 epoll

建议顺序：

1. 先写阻塞式 server/client。
2. 理解 TCP 是字节流。
3. 会写 `send_all` 和 `recv_all`。
4. 会设计 `长度 + 正文` 协议。
5. 再学 epoll。

否则你会同时被 socket、字节序、粘包、非阻塞、事件循环一起打乱。
