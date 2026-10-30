---
title: 网络编程学习路线
article: false
---

# 网络编程：从 Socket 到 select / poll / epoll

第一次学习，先读 **Socket 新旧写法对照**，然后选择对应 API 的独立实战。四元组与内核机制放在后面的原理篇，可反复对照。

## 写法与独立实战

1. [Socket 传统与现代写法：IPv4、getaddrinfo、C++ RAII](./05-socket-old-modern.md)
2. [select：集合、等待与完整多连接服务端](./06-select-tutorial.md)
3. [poll：pollfd、事件掩码与完整多连接服务端](./07-poll-tutorial.md)
4. [epoll：ADD/MOD/DEL、LT/ET 与完整多连接服务端](./08-epoll-tutorial.md)

## 从应用追到内核

1. [A 到 B：TCP 四元组、Socket、内核收包和字节序](./01-tcp-socket-kernel.md)
2. [三种 API 逐项对比](./02-select-poll-epoll.md)
3. [等待队列、就绪表、LT/ET 与 Reactor](./03-kernel-reactor.md)
4. [C/C++ 客户端、帧解析、抓包与追踪实验](./04-c-cpp-lab.md)

[完整配套源码](https://github.com/ageni01/ageno-block/tree/main/examples/io-multiplexing)
