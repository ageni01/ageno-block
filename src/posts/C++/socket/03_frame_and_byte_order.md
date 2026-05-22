# 03. 数据帧与字节序

## 1. TCP 没有消息边界

很多初学者会误以为：

```cpp
send(fd, "hello", 5, 0);
recv(fd, buffer, 5, 0);
```

一次 `send` 就会对应一次 `recv`。

这是错的。

TCP 是字节流，只保证字节顺序可靠，不保证消息边界。

可能出现：

```text
发送端 send("hello")
发送端 send("world")

接收端 recv() 得到 "helloworld"
```

也可能：

```text
发送端 send("helloworld")

接收端第一次 recv() 得到 "hel"
接收端第二次 recv() 得到 "loworld"
```

这就是常说的：

- 粘包：多条消息粘在一起。
- 半包：一条消息被拆开。

## 2. 解决方法：自己设计应用层协议

本项目第 3 课使用最常见的长度前缀协议：

```text
4 字节正文长度 + 正文内容
```

例如正文是：

```text
hello
```

正文长度是 5，所以发送：

```text
[00 00 00 05][h e l l o]
```

接收端就知道：

1. 先读 4 字节。
2. 解析出正文长度。
3. 再继续读这么多字节。

代码：

- [src/03_frame_client.cpp](../src/03_frame_client.cpp)
- [src/03_frame_server.cpp](../src/03_frame_server.cpp)

## 3. 为什么需要 send_all

`send` 不保证一次把你给的数据全发出去。

所以严谨写法是循环：

```cpp
while (还没发完) {
    send(...);
}
```

本项目封装成：

```cpp
send_all(fd, frame.data(), frame.size());
```

## 4. 为什么需要 recv_all

`recv` 不保证一次读到你想要的长度。

所以读 4 字节长度时，也要循环读满 4 字节。

读正文时，也要循环读满正文长度。

本项目封装成：

```cpp
recv_all(fd, &net_len, sizeof(net_len));
recv_all(fd, body.data(), body.size());
```

## 5. 字节序是什么

一个 `uint32_t` 有 4 个字节。

数字：

```text
0x01020304
```

大端存储：

```text
01 02 03 04
```

小端存储：

```text
04 03 02 01
```

不同 CPU 可能使用不同字节序。

如果你直接把整数内存发到网络上，不同机器可能解析出不同结果。

## 6. 网络字节序

网络协议统一使用大端，叫网络字节序。

所以发送整数前：

```cpp
uint32_t net_len = htonl(len);
```

接收整数后：

```cpp
uint32_t len = ntohl(net_len);
```

常用函数：

```text
htons: host to network short, 16 位，常用于端口
htonl: host to network long,  32 位，常用于长度字段
ntohs: network to host short
ntohl: network to host long
```

端口号为什么写：

```cpp
server_addr.sin_port = htons(port);
```

因为端口号也是网络协议里的 16 位整数，也必须转网络字节序。
