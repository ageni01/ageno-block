# C/C++ I/O 多路复用练习

配套文章在 `src/posts/C++/io-multiplexing/`。

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror echo_server.c -o /tmp/echo-server
c++ -std=c++17 -O2 -Wall -Wextra -Werror client.cpp -o /tmp/echo-client
/tmp/echo-server select
# 另一终端
/tmp/echo-client 'hello ageno'
```

停止服务器后换 `poll`；Linux 上还可换 `epoll`。macOS 不提供 epoll，程序会明确报错。服务器统一绑定本机 127.0.0.1:9090，C 程序使用相同连接状态机对比三种等待 API，epoll 采用 LT。

每连接 64 KiB 输出缓冲、最多 128 连接；缓冲满时暂停读，输出排空后恢复，支持部分写和读到 EOF 后继续排空输出。没有超时、平滑退出、TLS、生产级过载恢复或应用层帧协议。进程退出由操作系统回收资源。accept 遇到 EMFILE 等错误仅记录，生产需要资源预留和退避；不能把此示例直接当作生产服务。
