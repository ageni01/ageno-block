# 02. Socket 从 0 开始

## 1. Socket 是什么

你可以先把 socket 理解成“网络文件描述符”。

在 Linux/macOS 里，很多东西都可以用整数 fd 表示：

- 文件 fd
- 终端 fd
- 网络连接 fd

socket 编程就是通过这些 fd 调用：

- `socket`
- `bind`
- `listen`
- `accept`
- `connect`
- `send`
- `recv`
- `close`

## 2. 服务端编写流程

服务端代码看 [src/02_tcp_server.cpp](../src/02_tcp_server.cpp)。

服务端固定套路：

```text
socket -> setsockopt -> bind -> listen -> accept -> recv/send -> close
```

### socket

```cpp
int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
```

含义：

- `AF_INET`：IPv4。
- `SOCK_STREAM`：TCP。
- `0`：默认协议。

### bind

```cpp
bind(listen_fd, ...);
```

含义：把 socket 绑定到一个端口。

服务端必须绑定端口，否则客户端不知道连哪里。

### listen

```cpp
listen(listen_fd, SOMAXCONN);
```

含义：开始监听连接。

调用 `listen` 后，这个 socket 就是“监听 socket”。

### accept

```cpp
int client_fd = accept(listen_fd, ...);
```

含义：等待客户端连接。

注意：`listen_fd` 和 `client_fd` 不是同一个东西。

- `listen_fd`：只负责等新连接。
- `client_fd`：代表某一个已经连上的客户端。

### recv / send

```cpp
recv(client_fd, buffer, sizeof(buffer), 0);
send(client_fd, buffer, n, 0);
```

含义：

- `recv`：从客户端读数据。
- `send`：给客户端发数据。

## 3. 客户端编写流程

客户端代码看 [src/02_tcp_client.cpp](../src/02_tcp_client.cpp)。

客户端固定套路：

```text
socket -> connect -> send/recv -> close
```

客户端不需要 `bind` 和 `listen`，因为客户端不是等别人连，而是主动连服务器。

## 4. 一定要理解的服务端/客户端关系

```text
server:
  socket()
  bind(0.0.0.0:9000)
  listen()
  accept()  <--------- client connect()
  recv()    <--------- client send()
  send()    ---------> client recv()

client:
  socket()
  connect(127.0.0.1:9000)
  send()
  recv()
```

## 5. 阻塞是什么意思

默认 socket 是阻塞的。

例如：

```cpp
accept(...)
```

如果没有客户端连接，程序会停在这里等。

```cpp
recv(...)
```

如果客户端没发数据，程序会停在这里等。

这对初学非常友好，因为逻辑简单。

但如果想一个线程同时处理很多连接，就要学习：

- `select`
- `poll`
- Linux `epoll`
- macOS/BSD `kqueue`

本项目第 4 课会讲 Linux `epoll`。
