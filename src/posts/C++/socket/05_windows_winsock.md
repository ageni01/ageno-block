# 05. Windows Winsock 从 0 开始

## 1. Windows socket 和 Linux/macOS socket 不完全一样

Linux/macOS 使用 POSIX socket：

```cpp
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
```

Windows 使用 Winsock：

```cpp
#include <WinSock2.h>
#include <WS2tcpip.h>
```

并且 CMake 里要链接：

```cmake
target_link_libraries(05_winsock_server PRIVATE ws2_32)
target_link_libraries(05_winsock_client PRIVATE ws2_32)
```

`ws2_32` 是 Windows 的 Winsock 库。

## 2. Windows socket 必须先 WSAStartup

Windows 程序使用 socket 前必须初始化 Winsock：

```cpp
WSADATA data{};
int rc = WSAStartup(MAKEWORD(2, 2), &data);
```

程序结束时清理：

```cpp
WSACleanup();
```

本项目用一个小类自动管理：

```cpp
class WinsockRuntime {
public:
    WinsockRuntime() {
        WSADATA data{};
        WSAStartup(MAKEWORD(2, 2), &data);
    }

    ~WinsockRuntime() {
        WSACleanup();
    }
};
```

这样 `main` 一开始写：

```cpp
WinsockRuntime winsock;
```

就不容易忘。

## 3. Windows 服务端流程

代码看 [src/05_winsock_server.cpp](../src/05_winsock_server.cpp)。

流程和 Linux/macOS 很像：

```text
WSAStartup
socket
setsockopt
bind
listen
accept
recv/send
closesocket
WSACleanup
```

注意差异：

- socket 类型是 `SOCKET`，不是 `int`。
- 失败值是 `INVALID_SOCKET` 或 `SOCKET_ERROR`。
- 错误码用 `WSAGetLastError()`。
- 关闭 socket 用 `closesocket()`，不是 `close()`。

## 4. Windows 客户端流程

代码看 [src/05_winsock_client.cpp](../src/05_winsock_client.cpp)。

流程：

```text
WSAStartup
socket
connect
send/recv
closesocket
WSACleanup
```

客户端不需要 `bind`、`listen`、`accept`。

## 5. Windows 上怎么构建

在 Windows 的 PowerShell 或 Developer PowerShell 里：

```powershell
cd teaching_cmake_socket
cmake -S . -B build
cmake --build build
```

如果用 Visual Studio 生成器，程序一般在类似目录：

```text
build\Debug\05_winsock_server.exe
build\Debug\05_winsock_client.exe
```

运行服务端：

```powershell
.\build\Debug\05_winsock_server.exe 9000
```

另开一个 PowerShell 运行客户端：

```powershell
.\build\Debug\05_winsock_client.exe 127.0.0.1 9000
```

如果用 Ninja 或 Makefiles，程序可能在：

```text
build\05_winsock_server.exe
build\05_winsock_client.exe
```

## 6. Linux/macOS 和 Windows socket 对照表

| 概念 | Linux/macOS | Windows |
|---|---|---|
| 初始化 | 不需要 | `WSAStartup` |
| 清理 | 不需要 | `WSACleanup` |
| socket 类型 | `int` | `SOCKET` |
| 创建失败 | `-1` | `INVALID_SOCKET` |
| 普通调用失败 | `-1` | `SOCKET_ERROR` |
| 关闭 | `close(fd)` | `closesocket(s)` |
| 错误码 | `errno` | `WSAGetLastError()` |
| 链接库 | 通常不额外链接 | `ws2_32` |

## 7. 字节序仍然一样

Windows 也要使用：

```cpp
htons(port)
htonl(length)
ntohs(port)
ntohl(length)
```

原因一样：网络协议使用网络字节序，也就是大端。

## 8. 初学建议

如果你是 0 基础，先按这个顺序学：

1. 看 Linux/macOS 的 [02_socket_from_zero.md](02_socket_from_zero.md)，先理解 socket 通用流程。
2. 再看本章，记住 Windows 的初始化、关闭、错误处理差异。
3. 最后再把第 3 课的数据帧思想搬到 Windows 上。

Windows 和 Linux 的 API 名字很像，核心网络思想是一样的。
