---
cover: /assets/images/cover2.jpg
icon: pen-to-square
date: 2026-01-2
category:
  - 套接字
tag:
  - IO
star: true
sticky: true
---


# C++ Socket 套接字编程详解

## 1. 文档目标

本文件集中整理：

- TCP 与 UDP 基础
- Socket 编程流程
- 地址结构
- 字节序转换
- 服务端与客户端示例
- 收发中的典型问题

本文以 POSIX Socket 为主，Windows 项目可参照同样流程改成 Winsock。

---

## 2. 常用头文件

POSIX:

- `<sys/types.h>`
- `<sys/socket.h>`
- `<netinet/in.h>`
- `<arpa/inet.h>`
- `<unistd.h>`
- `<cerrno>`
- `<cstring>`

Windows:

- `<winsock2.h>`
- `<ws2tcpip.h>`

---

## 3. 网络基础概念

### 3.1 TCP

特点：

- 面向连接
- 可靠传输
- 按序到达
- 面向字节流

### 3.2 UDP

特点：

- 无连接
- 尽力而为
- 保留报文边界
- 延迟更低，控制更灵活

### 3.3 IP + Port

- IP 用于标识主机
- Port 用于标识进程服务

---

## 4. 地址结构与字节序

### 4.1 `sockaddr_in`

```cpp
#include <arpa/inet.h>
#include <netinet/in.h>

int main() {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return 0;
}
```

### 4.2 字节序转换

- `htons`
- `htonl`
- `ntohs`
- `ntohl`

网络字节序通常是大端。

---

## 5. TCP 服务端流程

1. `socket()`
2. `bind()`
3. `listen()`
4. `accept()`
5. `recv()`
6. `send()`
7. `close()`

### 5.1 最小 TCP 服务端

```cpp
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "socket failed" << std::endl;
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind failed" << std::endl;
        ::close(server_fd);
        return 1;
    }

    if (::listen(server_fd, 16) < 0) {
        std::cerr << "listen failed" << std::endl;
        ::close(server_fd);
        return 1;
    }

    sockaddr_in client{};
    socklen_t len = sizeof(client);
    int client_fd = ::accept(server_fd, reinterpret_cast<sockaddr*>(&client), &len);
    if (client_fd < 0) {
        std::cerr << "accept failed" << std::endl;
        ::close(server_fd);
        return 1;
    }

    char buf[1024] = {0};
    ssize_t n = ::recv(client_fd, buf, sizeof(buf), 0);
    if (n > 0) {
        std::cout << "recv: " << std::string(buf, buf + n) << std::endl;
        ::send(client_fd, buf, static_cast<size_t>(n), 0);
    }

    ::close(client_fd);
    ::close(server_fd);
    return 0;
}
```

---

## 6. TCP 客户端流程

1. `socket()`
2. `connect()`
3. `send()`
4. `recv()`
5. `close()`

### 6.1 最小 TCP 客户端

```cpp
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return 1;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(9000);
    ::inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (::connect(fd, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
        std::cerr << "connect failed" << std::endl;
        ::close(fd);
        return 1;
    }

    const char* text = "hello socket";
    ::send(fd, text, std::strlen(text), 0);

    char buf[1024] = {0};
    ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
    if (n > 0) {
        std::cout << "echo: " << std::string(buf, buf + n) << std::endl;
    }

    ::close(fd);
    return 0;
}
```

---

## 7. UDP 示例

### 7.1 UDP 服务端

```cpp
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9001);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return 1;
    }

    char buf[1024] = {0};
    sockaddr_in client{};
    socklen_t len = sizeof(client);
    ssize_t n = ::recvfrom(fd, buf, sizeof(buf), 0,
                           reinterpret_cast<sockaddr*>(&client), &len);
    if (n > 0) {
        ::sendto(fd, buf, static_cast<size_t>(n), 0,
                 reinterpret_cast<sockaddr*>(&client), len);
    }

    ::close(fd);
    return 0;
}
```

### 7.2 UDP 客户端

```cpp
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return 1;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(9001);
    ::inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    const char* msg = "hello udp";
    ::sendto(fd, msg, std::strlen(msg), 0,
             reinterpret_cast<sockaddr*>(&server), sizeof(server));

    char buf[1024] = {0};
    ssize_t n = ::recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
    if (n > 0) {
        std::cout << std::string(buf, buf + n) << std::endl;
    }

    ::close(fd);
    return 0;
}
```

---

## 8. Socket 常用系统调用

### 8.1 `socket`

创建套接字。

### 8.2 `bind`

绑定本地地址与端口。

### 8.3 `listen`

把套接字变成监听套接字。

### 8.4 `accept`

接受新的 TCP 连接。

### 8.5 `connect`

客户端发起连接。

### 8.6 `send` / `recv`

TCP 字节流发送与接收。

### 8.7 `sendto` / `recvfrom`

UDP 数据报发送与接收。

### 8.8 `close`

关闭文件描述符。

---

## 9. 选项配置

### 9.1 `SO_REUSEADDR`

```cpp
#include <sys/socket.h>

int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

用途：

- 服务重启时更容易复用地址

### 9.2 非阻塞模式

```cpp
#include <fcntl.h>

int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

---

## 10. 发送与接收的关键问题

### 10.1 TCP 没有消息边界

错误理解：

- “一次 `send()` 对应一次 `recv()`”

正确理解：

- TCP 只保证字节流顺序，不保证应用层消息边界

### 10.2 部分发送

```cpp
#include <cstddef>
#include <cstdint>
#include <sys/socket.h>

bool sendAll(int fd, const char* data, std::size_t len) {
    std::size_t sent = 0;
    while (sent < len) {
        ssize_t n = ::send(fd, data + sent, len - sent, 0);
        if (n <= 0) {
            return false;
        }
        sent += static_cast<std::size_t>(n);
    }
    return true;
}
```

### 10.3 部分接收

`recv()` 只是“现在收到多少”，不是“一条消息收完了多少”。

---

## 11. Windows 与 POSIX 差异

Windows 需要：

1. `WSAStartup`
2. `closesocket`
3. `WSACleanup`

POSIX 通常使用：

1. `socket`
2. `close`

---

## 12. 错误处理建议

建议记录：

- 系统调用名称
- 返回值
- `errno`
- 对端地址
- 当前阶段

---

## 13. 实战建议

1. 先写阻塞版，确认协议正确
2. 再切非阻塞
3. 再上 `select` / `poll` / `epoll`
4. 协议解析与网络收发分层
5. 不要把业务逻辑直接塞进收包函数

