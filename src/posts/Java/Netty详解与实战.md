---
title: "Netty 详解与实战"

category:
  - Java
tag:
  - Java
---
# Netty 详解与实战

> 涵盖：核心组件、自定义协议、内置协议（HTTP/WebSocket/gRPC/SSL）、编解码、零拷贝、内存管理、最佳实践

---

## 目录

1. [Netty 核心架构](#1-netty-核心架构)
2. [快速开始：Echo Server](#2-快速开始echo-server)
3. [核心组件详解](#3-核心组件详解)
4. [ChannelHandler 与 Pipeline](#4-channelhandler-与-pipeline)
5. [编解码与自定义协议](#5-编解码与自定义协议)
6. [内置协议支持](#6-内置协议支持)
7. [零拷贝与 ByteBuf](#7-零拷贝与-bytebuf)
8. [内存管理](#8-内存管理)
9. [线程模型与 EventLoop](#9-线程模型与-eventloop)
10. [高级特性](#10-高级特性)
11. [最佳实践与常见陷阱](#11-最佳实践与常见陷阱)

---

## 1. Netty 核心架构

```
         ┌─────────────────────────────────────────┐
         │           ChannelPipeline               │
         │  ┌──────┐ ┌──────┐       ┌──────┐      │
         │  │H1 In │ │H2 In │  ...  │Hn In │      │
         │  └──┬───┘ └──┬───┘       └──┬───┘      │
         │     │        │              │           │
───> In ──────┴────────┴──────────────┴─────> 读取
         │     ▲        ▲              ▲           │
         │     │        │              │           │
         │  ┌──┴───┐ ┌──┴───┐       ┌──┴───┐      │
         │  │H1 Out│ │H2 Out│  ...  │Hn Out│      │
         │  └──────┘ └──────┘       └──────┘      │
Out <─── ──────┬────────┬──────────────┬─────      │
         └─────┼────────┼──────────────┼───────────┘
               │        │              │
         ┌─────┴────────┴──────────────┴───────┐
         │            Channel                   │
         │     (Socket / DatagramChannel)       │
         └─────────────────────────────────────┘
```

### 核心组件

| 组件 | 职责 |
|------|------|
| **Channel** | 网络连接抽象（Socket），负责 I/O 操作 |
| **EventLoop** | 处理 I/O 事件的线程（一个 EventLoop 处理多个 Channel） |
| **EventLoopGroup** | EventLoop 线程池 |
| **ChannelPipeline** | 双向链表，编排 Inbound/Outbound Handler |
| **ChannelHandler** | 业务逻辑处理器 |
| **ByteBuf** | 字节容器，替代 ByteBuffer |
| **ChannelFuture** | 异步结果，可添加 Listener |
| **Bootstrap / ServerBootstrap** | 客户端 / 服务端启动器 |
| **Codec** | 编解码器（Encoder / Decoder） |

---

## 2. 快速开始：Echo Server

### 2.1 Maven 依赖

```xml
<dependency>
    <groupId>io.netty</groupId>
    <artifactId>netty-all</artifactId>
    <version>4.1.100.Final</version>
</dependency>
```

### 2.2 服务端

```java
public class EchoServer {
    private final int port;

    public EchoServer(int port) {
        this.port = port;
    }

    public void start() throws InterruptedException {
        // Boss: 接受连接（1 个线程）
        EventLoopGroup bossGroup = new NioEventLoopGroup(1);
        // Worker: 处理 I/O（默认 CPU*2 线程）
        EventLoopGroup workerGroup = new NioEventLoopGroup();

        try {
            ServerBootstrap b = new ServerBootstrap();
            b.group(bossGroup, workerGroup)
             .channel(NioServerSocketChannel.class)
             .option(ChannelOption.SO_BACKLOG, 128)
             .childOption(ChannelOption.SO_KEEPALIVE, true)
             .childHandler(new ChannelInitializer<SocketChannel>() {
                 @Override
                 protected void initChannel(SocketChannel ch) {
                     ch.pipeline().addLast(new EchoServerHandler());
                 }
             });

            ChannelFuture f = b.bind(port).sync();
            System.out.println("Server started on port " + port);
            f.channel().closeFuture().sync();
        } finally {
            bossGroup.shutdownGracefully();
            workerGroup.shutdownGracefully();
        }
    }

    public static void main(String[] args) throws InterruptedException {
        new EchoServer(8080).start();
    }
}
```

### 2.3 Handler

```java
@ChannelHandler.Sharable  // 可共享（无状态时使用）
public class EchoServerHandler extends ChannelInboundHandlerAdapter {
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        ByteBuf buf = (ByteBuf) msg;
        System.out.println("Received: " + buf.toString(Charset.defaultCharset()));
        ctx.write(msg); // 写回（不释放 msg，后面 flush 会自动释放）
    }

    @Override
    public void channelReadComplete(ChannelHandlerContext ctx) {
        ctx.flush(); // 冲刷所有待写数据
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        cause.printStackTrace();
        ctx.close();
    }
}
```

### 2.4 客户端

```java
public class EchoClient {
    public void start(String host, int port) throws InterruptedException {
        EventLoopGroup group = new NioEventLoopGroup();
        try {
            Bootstrap b = new Bootstrap();
            b.group(group)
             .channel(NioSocketChannel.class)
             .option(ChannelOption.TCP_NODELAY, true)
             .handler(new ChannelInitializer<SocketChannel>() {
                 @Override
                 protected void initChannel(SocketChannel ch) {
                     ch.pipeline().addLast(new EchoClientHandler());
                 }
             });

            ChannelFuture f = b.connect(host, port).sync();
            f.channel().closeFuture().sync();
        } finally {
            group.shutdownGracefully();
        }
    }

    public static void main(String[] args) throws InterruptedException {
        new EchoClient().start("127.0.0.1", 8080);
    }
}

public class EchoClientHandler extends ChannelInboundHandlerAdapter {
    @Override
    public void channelActive(ChannelHandlerContext ctx) {
        ctx.writeAndFlush(Unpooled.copiedBuffer("Hello Netty!", Charset.defaultCharset()));
    }

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        ByteBuf buf = (ByteBuf) msg;
        System.out.println("Server reply: " + buf.toString(Charset.defaultCharset()));
        ctx.close();
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        cause.printStackTrace();
        ctx.close();
    }
}
```

---

## 3. 核心组件详解

### 3.1 EventLoopGroup 选择

| 类 | 适用场景 | 说明 |
|----|----------|------|
| `NioEventLoopGroup` | TCP NIO | 基于 Selector 的多路复用 |
| `EpollEventLoopGroup` | Linux TCP | native epoll（比 NIO 性能更好） |
| `KQueueEventLoopGroup` | macOS/BSD | native kqueue |
| `OioEventLoopGroup` | 阻塞 I/O | 已过时，不推荐 |
| `DefaultEventLoopGroup` | 非 Channel 任务 | 用于执行普通任务 |

### 3.2 Channel 类型

| 类 | 说明 |
|----|------|
| `NioServerSocketChannel` | 服务端 TCP NIO |
| `NioSocketChannel` | 客户端 TCP NIO |
| `OioServerSocketChannel` | 服务端阻塞 I/O |
| `OioSocketChannel` | 客户端阻塞 I/O |
| `EpollServerSocketChannel` | Linux 服务端 epoll |
| `EpollSocketChannel` | Linux 客户端 epoll |
| `KQueueServerSocketChannel` | macOS 服务端 |
| `KQueueSocketChannel` | macOS 客户端 |

### 3.3 ChannelOption 常用配置

```java
b.option(ChannelOption.SO_BACKLOG, 128)      // TCP 握手队列长度
 .option(ChannelOption.SO_REUSEADDR, true)   // 地址复用
 .option(ChannelOption.TCP_NODELAY, true)    // 禁用 Nagle 算法
 .option(ChannelOption.SO_KEEPALIVE, true)   // 心跳检测
 .option(ChannelOption.SO_SNDBUF, 32 * 1024) // 发送缓冲区
 .option(ChannelOption.SO_RCVBUF, 32 * 1024) // 接收缓冲区
 .option(ChannelOption.CONNECT_TIMEOUT_MILLIS, 5000) // 连接超时
 .childOption(ChannelOption.WRITE_BUFFER_WATER_MARK,
         new WriteBufferWaterMark(32 * 1024, 64 * 1024)); // 写缓冲水位
```

### 3.4 ChannelFuture / ChannelPromise

```java
// 异步回调（而非阻塞等待）
ChannelFuture future = ctx.writeAndFlush(msg);
future.addListener((ChannelFutureListener) f -> {
    if (f.isSuccess()) {
        System.out.println("Write succeeded");
    } else {
        f.cause().printStackTrace();
    }
});

// ChannelPromise 是 ChannelFuture 的可写版本
// 通常在 Handler 中创建用于异步转同步
ChannelPromise promise = ctx.newPromise();
promise.addListener(f -> {
    if (f.isSuccess()) { /* 完成回调 */ }
});
```

---

## 4. ChannelHandler 与 Pipeline

### 4.1 Inbound / Outbound 分类

```java
// Inbound: 处理入站数据（从网络读到 Java）
// 执行顺序：从头到尾
pipeline.addLast("decoder",   new ByteToMessageDecoder()  { ... }); // 1
pipeline.addLast("handler",   new SimpleChannelInboundHandler<>() { ... }); // 2

// Outbound: 处理出站数据（从 Java 写到网络）
// 执行顺序：从尾到头
pipeline.addLast("encoder",   new MessageToByteEncoder<>() { ... }); // 2
pipeline.addLast("handler",   new ChannelOutboundHandlerAdapter() { ... }); // 1
```

### 4.2 Handler 生命周期

```java
public class LifecycleHandler extends ChannelInboundHandlerAdapter {

    @Override
    public void handlerAdded(ChannelHandlerContext ctx) {
        // Handler 被添加到 Pipeline 时
    }

    @Override
    public void handlerRemoved(ChannelHandlerContext ctx) {
        // Handler 从 Pipeline 移除时
    }

    @Override
    public void channelRegistered(ChannelHandlerContext ctx) {
        // Channel 注册到 EventLoop
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) {
        // Channel 连接就绪（TCP 握手完成）
    }

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        // 读到数据
        ctx.fireChannelRead(msg); // 传给下一个 Inbound
    }

    @Override
    public void channelReadComplete(ChannelHandlerContext ctx) {
        // 一次读取完成
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) {
        // Channel 断开
    }

    @Override
    public void channelUnregistered(ChannelHandlerContext ctx) {
        // Channel 从 EventLoop 注销
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        // 异常
    }

    @Override
    public void userEventTriggered(ChannelHandlerContext ctx, Object evt) {
        // 用户自定义事件（IdleStateEvent 等）
    }
}
```

### 4.3 常用 Handler 基类

| 基类 | 类型 | 用途 |
|------|------|------|
| `ChannelInboundHandlerAdapter` | Inbound | 入站消息处理（需手动释放 msg） |
| `SimpleChannelInboundHandler<T>` | Inbound | 入站消息处理（自动释放 msg） |
| `ChannelOutboundHandlerAdapter` | Outbound | 出站消息处理 |
| `ByteToMessageDecoder` | Inbound | 字节解码为对象 |
| `MessageToMessageDecoder` | Inbound | 对象转对象解码 |
| `MessageToByteEncoder` | Outbound | 对象编码为字节 |
| `MessageToMessageEncoder` | Outbound | 对象转对象编码 |
| `ChannelDuplexHandler` | Both | 同时处理入站和出站 |

### 4.4 @Sharable 注解

```java
// 无状态的 Handler 可以标注 @Sharable 以复用实例
// 有状态的 Handler 必须每个 Channel 新建（不能标注）

@ChannelHandler.Sharable
public class StatelessHandler extends ChannelInboundHandlerAdapter {
    // 安全：不包含成员变量
}

// 有状态的 Handler
public class StatefulHandler extends ChannelInboundHandlerAdapter {
    private int count; // 每个 Channel 独立 => 不能 @Sharable
}
```

---

## 5. 编解码与自定义协议

### 5.1 自定义协议设计

```
二进制协议格式（示例）:
┌─────────┬──────────┬──────────┬──────────┐
│ Magic   │ Version  │  Length  │  Payload │
│ (4B)    │  (1B)    │  (4B)    │  (N B)   │
│ 0xDA DA │  0x01    │  N       │  data    │
└─────────┴──────────┴──────────┴──────────┘
┌──────────────────────────────────────────────┐
│ Header (9B)              │ Body              │
└──────────────────────────┴───────────────────┘
```

### 5.2 协议编解码器实现

```java
// ========== POJO ==========
public class CustomPacket {
    private int    magic;      // 魔数
    private byte   version;    // 版本号
    private int    length;     // 数据长度
    private byte[] payload;    // 数据体

    // constructors / getters / setters
    public CustomPacket(byte[] payload) {
        this.magic   = 0xDADA;
        this.version = 0x01;
        this.payload = payload;
        this.length  = payload.length;
    }
}

// ========== Encoder ==========
public class CustomPacketEncoder extends MessageToByteEncoder<CustomPacket> {
    @Override
    protected void encode(ChannelHandlerContext ctx, CustomPacket msg, ByteBuf out) {
        out.writeInt(msg.getMagic());
        out.writeByte(msg.getVersion());
        out.writeInt(msg.getLength());
        out.writeBytes(msg.getPayload());
    }
}

// ========== Decoder ==========
public class CustomPacketDecoder extends ByteToMessageDecoder {
    private static final int HEADER_SIZE = 9; // 4 + 1 + 4

    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
        // 1. 检查可读字节数
        if (in.readableBytes() < HEADER_SIZE) return;

        // 2. 标记当前位置（方便回退）
        in.markReaderIndex();

        // 3. 读魔数
        int magic = in.readInt();
        if (magic != 0xDADA) {
            // 魔数不对 => 非法连接 / 粘包
            ctx.close();
            return;
        }

        // 4. 读版本 + 长度
        byte version = in.readByte();
        int length   = in.readInt();

        // 5. 检查数据体是否完整
        if (in.readableBytes() < length) {
            // 数据不完整 => 回退，等待更多数据
            in.resetReaderIndex();
            return;
        }

        // 6. 读取完整包
        byte[] payload = new byte[length];
        in.readBytes(payload);

        CustomPacket packet = new CustomPacket(payload);
        out.add(packet);
    }
}

// ========== Pipeline 装配 ==========
public class CustomProtocolInitializer extends ChannelInitializer<SocketChannel> {
    @Override
    protected void initChannel(SocketChannel ch) {
        ch.pipeline()
          .addLast("frameDecoder", new LengthFieldBasedFrameDecoder(1024, 5, 4, 0, 0))
          // 也可以直接用自定义的 CustomPacketDecoder（内部实现了长度检查）
          .addLast("decoder",      new CustomPacketDecoder())
          .addLast("encoder",      new CustomPacketEncoder())
          .addLast("handler",      new CustomServerHandler());
    }
}
```

### 5.3 Netty 内置编解码器

```java
// ---------- 基于长度 ----------
// 根据长度字段拆包（最常用）
pipeline.addLast(new LengthFieldBasedFrameDecoder(
    1024,          // maxFrameLength
    0,             // lengthFieldOffset
    4,             // lengthFieldLength
    0,             // lengthAdjustment
    0));           // initialBytesToStrip
// + LengthFieldPrepender 在发送时自动加长度头
pipeline.addLast(new LengthFieldPrepender(4));

// ---------- 基于分隔符 ----------
// 按行分割
pipeline.addLast(new LineBasedFrameDecoder(1024));
// 自定义分隔符
pipeline.addLast(new DelimiterBasedFrameDecoder(1024,
        Unpooled.copiedBuffer("END".getBytes())));

// ---------- 固定长度 ----------
pipeline.addLast(new FixedLengthFrameDecoder(100));

// ---------- 字符串编解码 ----------
pipeline.addLast(new StringDecoder(Charset.forName("UTF-8")));
pipeline.addLast(new StringEncoder(Charset.forName("UTF-8")));
```

### 5.4 JSON 协议

```java
// 服务端使用 JSON
public class JsonServerInitializer extends ChannelInitializer<SocketChannel> {
    @Override
    protected void initChannel(SocketChannel ch) {
        ch.pipeline()
          .addLast(new LengthFieldBasedFrameDecoder(1024 * 1024, 0, 4, 0, 4))
          .addLast(new JsonDecoder())          // 自定义：ByteBuf → JSON
          .addLast(new LengthFieldPrepender(4))
          .addLast(new JsonEncoder())          // 自定义：JSON → ByteBuf
          .addLast(new JsonServerHandler());
    }
}

// 使用 Jackson 实现 JSON 编解码
public class JsonDecoder extends ByteToMessageDecoder {
    private static final ObjectMapper MAPPER = new ObjectMapper();

    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out)
            throws Exception {
        int len = in.readableBytes();
        byte[] bytes = new byte[len];
        in.readBytes(bytes);
        out.add(MAPPER.readValue(bytes, Map.class));
    }
}

public class JsonEncoder extends MessageToByteEncoder<Object> {
    private static final ObjectMapper MAPPER = new ObjectMapper();

    @Override
    protected void encode(ChannelHandlerContext ctx, Object msg, ByteBuf out)
            throws Exception {
        byte[] bytes = MAPPER.writeValueAsBytes(msg);
        out.writeBytes(bytes);
    }
}
```

### 5.5 粘包与半包

```
         TCP 流（无边界）
┌─────────────────────────────────────────────┐
│ Packet1         Packet2         Packet3      │
│ [header|body]  [header|body]  [header|body] │
└─────────────────────────────────────────────┘

可能出现的四种情况：

1. 正常：decode 拿到一个完整包
2. 粘包：一次读到 Packet1 + Packet2
   ┌──────────────────┐
   │ Packet1 | Packet2 │
   └──────────────────┘
3. 半包：只读到 Packet1 的一部分
   ┌────────────┐
   │ Packet1 的前半 │
   └────────────┘
4. 粘包+半包：Packet2 完整 + Packet3 前半
   ┌──────────────────┐
   │ Packet2 | Packet3头│
   └──────────────────┘
```

**解决方案**: 三种拆包方式

| 方式 | 原理 | 内置 Handler |
|------|------|-------------|
| 定长 | 固定长度 | `FixedLengthFrameDecoder` |
| 分隔符 | 特殊标记结束 | `DelimiterBasedFrameDecoder` |
| **长度域** | 头部记录长度 | `LengthFieldBasedFrameDecoder` (推荐) |

---

## 6. 内置协议支持

### 6.1 HTTP

```java
public class HttpServerInitializer extends ChannelInitializer<SocketChannel> {
    @Override
    protected void initChannel(SocketChannel ch) {
        ch.pipeline()
          // HTTP 编解码
          .addLast("codec",          new HttpServerCodec())
          // 聚合 HTTP 消息（避免分块处理）
          .addLast("aggregator",     new HttpObjectAggregator(64 * 1024))
          // 压缩（可选）
          .addLast("compressor",     new HttpContentCompressor())
          // 业务
          .addLast("handler",        new HttpServerHandler());
    }
}

public class HttpServerHandler extends SimpleChannelInboundHandler<FullHttpRequest> {
    @Override
    protected void channelRead0(ChannelHandlerContext ctx, FullHttpRequest req) {
        String uri = req.uri();
        String body = req.content().toString(Charset.defaultCharset());
        System.out.println("URI: " + uri + ", Body: " + body);

        // 构建响应
        FullHttpResponse resp = new DefaultFullHttpResponse(
                HTTP_1_1, OK,
                Unpooled.copiedBuffer("Hello HTTP", Charset.defaultCharset()));
        resp.headers()
            .set(CONTENT_TYPE, "text/plain")
            .setInt(CONTENT_LENGTH, resp.content().readableBytes());

        ctx.writeAndFlush(resp);
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        cause.printStackTrace();
        ctx.close();
    }
}
```

### 6.2 WebSocket

```java
public class WebSocketServerInitializer extends ChannelInitializer<SocketChannel> {
    @Override
    protected void initChannel(SocketChannel ch) {
        ch.pipeline()
          .addLast("http-codec",     new HttpServerCodec())
          .addLast("aggregator",     new HttpObjectAggregator(64 * 1024))
          .addLast("chunked-writer", new ChunkedWriteHandler())
          // WebSocket 升级处理
          .addLast("ws-handler",     new WebSocketServerProtocolHandler("/ws"))
          // 业务
          .addLast("handler",        new WebSocketFrameHandler());
    }
}

public class WebSocketFrameHandler extends SimpleChannelInboundHandler<WebSocketFrame> {
    @Override
    protected void channelRead0(ChannelHandlerContext ctx, WebSocketFrame frame) {
        if (frame instanceof TextWebSocketFrame) {
            String text = ((TextWebSocketFrame) frame).text();
            System.out.println("WS received: " + text);

            // 广播到所有客户端
            ctx.writeAndFlush(new TextWebSocketFrame("Echo: " + text));
        } else if (frame instanceof PingWebSocketFrame) {
            ctx.writeAndFlush(new PongWebSocketFrame(frame.content().retain()));
        } else if (frame instanceof CloseWebSocketFrame) {
            ctx.close();
        }
    }
}
```

### 6.3 SSL/TLS

```java
public class SslServerInitializer extends ChannelInitializer<SocketChannel> {
    private final SslContext sslCtx;

    public SslServerInitializer() {
        // 自签名证书（生产环境使用 KeyStore）
        SelfSignedCertificate cert = new SelfSignedCertificate();
        sslCtx = SslContextBuilder.forServer(cert.certificate(), cert.privateKey())
                 .sslProvider(SslProvider.OPENSSL)  // 推荐使用 netty-tcnative
                 .build();
    }

    @Override
    protected void initChannel(SocketChannel ch) {
        ch.pipeline()
          .addLast(sslCtx.newHandler(ch.alloc()))
          .addLast("handler", new EchoServerHandler());
    }
}

// 客户端
public class SslClientInitializer extends ChannelInitializer<SocketChannel> {
    @Override
    protected void initChannel(SocketChannel ch) {
        SslContext sslCtx = SslContextBuilder.forClient()
                .trustManager(InsecureTrustManagerFactory.INSTANCE) // 信任所有证书
                .sslProvider(SslProvider.OPENSSL)
                .build();

        ch.pipeline()
          .addLast(sslCtx.newHandler(ch.alloc(), "localhost", 8443))
          .addLast("handler", new EchoClientHandler());
    }
}
```

### 6.4 心跳 / Idle 检测

```java
// 服务端加 IdleStateHandler
pipeline.addLast("idle", new IdleStateHandler(
        30,  // readerIdleTime    — 30s 没读到数据触发
        0,   // writerIdleTime    — 0 不检测
        0,   // allIdleTime       — 0 不检测
        TimeUnit.SECONDS))
      .addLast("handler", new HeartbeatHandler());

// 业务 Handler 处理空闲事件
public class HeartbeatHandler extends ChannelInboundHandlerAdapter {
    @Override
    public void userEventTriggered(ChannelHandlerContext ctx, Object evt) {
        if (evt instanceof IdleStateEvent) {
            IdleStateEvent e = (IdleStateEvent) evt;
            if (e.state() == IdleState.READER_IDLE) {
                System.out.println("Read idle, closing: " + ctx.channel().remoteAddress());
                ctx.close();
            }
        }
    }
}

// 客户端定时发送心跳
public class HeartbeatClientHandler extends ChannelInboundHandlerAdapter {
    private static final ByteBuf HEARTBEAT =
            Unpooled.unreleasableBuffer(Unpooled.copiedBuffer("PING".getBytes()));

    @Override
    public void channelActive(ChannelHandlerContext ctx) {
        // 启动定时心跳
        ctx.executor().scheduleAtFixedRate(() -> {
            if (ctx.channel().isActive()) {
                ctx.writeAndFlush(HEARTBEAT.retainedDuplicate());
            }
        }, 0, 10, TimeUnit.SECONDS);
    }
}
```

### 6.5 文件传输（零拷贝）

```java
// Netty 默认支持零拷贝大文件发送
pipeline.addLast(new ChunkedWriteHandler());

// 发送文件
public class FileServerHandler extends ChannelInboundHandlerAdapter {
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        String path = (String) msg;
        File file = new File(path);

        // DefaultFileRegion 使用 FileChannel.transferTo（零拷贝）
        DefaultFileRegion region = new DefaultFileRegion(
                file, 0, file.length());
        ctx.writeAndFlush(region);
    }
}
```

### 6.6 UDP

```java
public class UdpServer {
    public void start(int port) throws InterruptedException {
        EventLoopGroup group = new NioEventLoopGroup();
        try {
            Bootstrap b = new Bootstrap();
            b.group(group)
             .channel(NioDatagramChannel.class)
             .option(ChannelOption.SO_BROADCAST, true)
             .handler(new ChannelInitializer<DatagramChannel>() {
                 @Override
                 protected void initChannel(DatagramChannel ch) {
                     ch.pipeline().addLast(new UdpServerHandler());
                 }
             });

            b.bind(port).sync().channel().closeFuture().await();
        } finally {
            group.shutdownGracefully();
        }
    }
}

public class UdpServerHandler extends SimpleChannelInboundHandler<DatagramPacket> {
    @Override
    protected void channelRead0(ChannelHandlerContext ctx, DatagramPacket packet) {
        String msg = packet.content().toString(Charset.defaultCharset());
        System.out.println("UDP received: " + msg);

        // 回复
        ctx.writeAndFlush(new DatagramPacket(
                Unpooled.copiedBuffer("ACK", Charset.defaultCharset()),
                packet.sender()));
    }
}
```

### 6.7 Unix Domain Socket

```java
// 需要 netty-native 支持
EventLoopGroup group = new EpollEventLoopGroup();
try {
    Bootstrap b = new Bootstrap();
    b.group(group)
     .channel(EpollDomainSocketChannel.class)
     .handler(new ChannelInitializer<EpollDomainSocketChannel>() {
         @Override
         protected void initChannel(EpollDomainSocketChannel ch) {
             ch.pipeline().addLast(new EchoServerHandler());
         }
     });

    // 绑定 Unix Socket 文件
    b.bind(new DomainSocketAddress("/tmp/netty.sock")).sync();
} finally {
    group.shutdownGracefully();
}
```

---

## 7. 零拷贝与 ByteBuf

### 7.1 ByteBuf 结构

```
  +-------------------+------------------+------------------+
  | discardable bytes |  readable bytes  |  writable bytes  |
  |   (已读/已丢弃)     |   (可读取)       |   (可写入)       |
  +-------------------+------------------+------------------+
  |                   |                  |                  |
  0      <=      readerIndex    <=    writerIndex    <=    capacity
```

```java
ByteBuf buf = Unpooled.buffer(256);

// 写
buf.writeByte(1);
buf.writeInt(100);
buf.writeBytes("hello".getBytes());

// 读
byte b = buf.readByte();
int  i = buf.readInt();

// 指针操作
buf.readerIndex(0);     // 重置读指针
buf.writerIndex(100);   // 设置写指针
buf.markReaderIndex();  // 标记当前位置
buf.resetReaderIndex(); // 回到标记位置

// 随机访问（不移动指针）
byte b = buf.getByte(0);
int  i = buf.getInt(4);

// 容量
buf.capacity();          // 总容量
buf.readableBytes();     // 可读字节数 = writerIndex - readerIndex
buf.writableBytes();     // 可写字节数 = capacity - writerIndex

// 释放
buf.release();           // 引用计数 -1, 为 0 时回收
```

### 7.2 ByteBuf 类型

| 类型 | 分配位置 | 说明 |
|------|----------|------|
| `HeapByteBuf` | JVM 堆 | 分配/释放快，需拷贝到 Socket |
| `DirectByteBuf` | 堆外内存（Direct Memory） | 零拷贝到 Socket，分配/释放慢 |
| `CompositeByteBuf` | 组合 | 组合多个 ByteBuf，避免拷贝 |
| `PooledByteBuf` | 池化 | 从内存池分配，减少 GC |

```java
// 推荐：使用 Channel 的分配器（类型由配置决定）
ByteBuf buf = ctx.alloc().buffer();
ByteBuf buf = ctx.alloc().directBuffer();  // 显式分配直存

// Unpooled 工具类
ByteBuf heap    = Unpooled.buffer(256);            // Heap
ByteBuf direct  = Unpooled.directBuffer(256);      // Direct
ByteBuf copied  = Unpooled.copiedBuffer("hello", Charset.defaultCharset()); // 拷贝
ByteBuf wrapped = Unpooled.wrappedBuffer(bytes);   // 包装已有数组（避免拷贝）
```

### 7.3 零拷贝技术

Netty 的零拷贝（不是操作系统零拷贝，而是 Netty 层面的数据搬运优化）：

```java
// 1. CompositeByteBuf：合并多个 ByteBuf 时避免拷贝
CompositeByteBuf comp = Unpooled.compositeBuffer();
comp.addComponents(true, header, body); // true = 自动增加 writerIndex
// 相当于虚拟合并，不复制数据

// 2. wrap：包装已有字节数组，不拷贝
byte[] bytes = new byte[1024];
ByteBuf buf = Unpooled.wrappedBuffer(bytes);

// 3. slice：切片，共享同一段内存
ByteBuf whole = Unpooled.copiedBuffer("HelloWorld".getBytes());
ByteBuf slice = whole.slice(0, 5);  // "Hello"
// slice 和 whole 共享底层内存

// 4. FileRegion：文件传输零拷贝（transferTo）
FileRegion region = new DefaultFileRegion(channel, pos, count);
ctx.writeAndFlush(region);  // 直接通过 OS 发送文件

// 5. ByteBuf. duplicate
ByteBuf dup = whole.duplicate();  // 共享全部数据，独立指针
```

### 7.4 引用计数

```java
ByteBuf buf = ctx.alloc().buffer();
buf.retain();   // refCnt++
buf.release();  // refCnt--, 为 0 时回收
buf.refCnt();   // 当前引用计数

// 自动释放的三种方式
// 1. SimpleChannelInboundHandler (自动 release)
// 2. 调用 ctx.fireChannelRead(msg) 之后
// 3. ReferenceCountUtil.release(msg)

// 手动释放
@Override
public void channelRead(ChannelHandlerContext ctx, Object msg) {
    try {
        // 处理 msg
    } finally {
        ReferenceCountUtil.release(msg);
    }
}
```

---

## 8. 内存管理

### 8.1 Netty 内存池（PooledByteBufAllocator）

```java
// 默认使用 PooledByteBufAllocator（4.1+）
// 可通过系统属性切换
// -Dio.netty.allocator.type=unpooled
// -Dio.netty.allocator.type=pooled

// PooledByteBufAllocator 内部结构
// ┌──────────┐
// │ Arena    │ ← PoolArena（类似 jemalloc）
// ├──────────┤
// │ tiny     │ 0-512B      ← 缓存多个固定大小的块
// │ small    │ 512B-8KB
// │ normal   │ 8KB-16MB
// │ huge     │ >16MB       ← 直接分配，不池化
// └──────────┘
```

### 8.2 内存泄漏检测

```java
// 启动参数（推荐用 advanced 或 paranoid）
// -Dio.netty.leakDetectionLevel=simple
// -Dio.netty.leakDetectionLevel=advanced
// -Dio.netty.leakDetectionLevel=paranoid (最严格)

// 代码中检测
public class LeakDetectHandler extends ChannelDuplexHandler {
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        // 上报泄漏报告
        if (ResourceLeakDetector.isEnabled()) {
            System.err.println("Leak report: " +
                    ResourceLeakDetector.getLevel());
        }
        ctx.fireChannelRead(msg);
    }
}

// 日志示例:
// LEAK: ByteBuf.release() was not called before it's garbage-collected.
// See https://netty.io/wiki/reference-counted-objects.html
// Recent access records:
// #1: io.netty.buffer.AdvancedLeakAwareByteBuf.getBytes(...)
```

### 8.3 分配器配置

```java
// 启用池化（默认）
Bootstrap b = new Bootstrap()
    .option(ChannelOption.ALLOCATOR, PooledByteBufAllocator.DEFAULT);

// 启用堆内存分配（默认直存）
b.option(ChannelOption.ALLOCATOR, UnpooledByteBufAllocator.DEFAULT);
b.option(ChannelOption.RCVBUF_ALLOCATOR,
    AdaptiveRecvByteBufAllocator.DEFAULT);

// 自定义 Buffer 大小
b.option(ChannelOption.RCVBUF_ALLOCATOR,
    new AdaptiveRecvByteBufAllocator(512, 2048, 65536));
```

---

## 9. 线程模型与 EventLoop

### 9.1 Netty 线程模型

```
Reactor 模型演变:

1. 单线程 Reactor（早期 Netty）
   ┌──────────┐
   │ EventLoop│ ← Selector （1 个线程处理所有 accept + read + write）
   └──────────┘

2. 多线程 Reactor（主流）
   ┌──────────┐     ┌──────────┐
   │ Boss     │ ──> │ Worker 1 │  (Boss 只负责 accept)
   │ EventLoop│     ├──────────┤
   │ (1 个)   │     │ Worker 2 │  (Worker 负责 read/write)
   └──────────┘     ├──────────┤
                     │ Worker N │
                     └──────────┘

3. 主从 Reactor（Netty 默认）
   ┌───────────┐    ┌───────────┐
   │ BossGroup │ ──> │ WorkerGroup│
   │ (1+N 个)  │    │ (N 个)    │
   └───────────┘    └───────────┘
```

### 9.2 EventLoop 职责

```java
// 每个 EventLoop 的职责：
// 1. 监听 Selector 上的 I/O 事件
// 2. 执行 ChannelHandler
// 3. 执行提交的任务（execute / schedule）
// 4. 所有操作在同一线程完成（无锁）

EventLoop eventLoop = ctx.channel().eventLoop();

// 提交任务到 EventLoop（线程安全）
eventLoop.execute(() -> {
    // 在 I/O 线程执行
});

// 定时任务
eventLoop.schedule(() -> {
    // 延迟执行
}, 5, TimeUnit.SECONDS);

eventLoop.scheduleAtFixedRate(() -> {
    // 周期性执行
}, 0, 10, TimeUnit.SECONDS);

// ⚠️ 不要在 EventLoop 中执行阻塞操作
// ❌ eventLoop.execute(() -> {
//        Thread.sleep(5000); // 阻塞了整个 EventLoop!
//    });
```

### 9.3 多线程并发访问

```java
// 跨线程写入是安全的（Netty 内部做了串行化）
// 但需要注意以下场景:

// ✅ 安全：在 EventLoop 中直接 write
public void channelRead(ChannelHandlerContext ctx, Object msg) {
    ctx.writeAndFlush(msg);
}

// ✅ 安全：从其他线程写入（Netty 会任务化）
public void onBusinessEvent(String data) {
    Channel ch = getChannel();
    ch.writeAndFlush(Unpooled.copiedBuffer(data, Charset.defaultCharset()));
    // 内部会被包装为 WriteTask，提交到 EventLoop 执行
}

// ✅ 推荐：显式提交到 EventLoop
public void onBusinessEvent(String data) {
    Channel ch = getChannel();
    ch.eventLoop().execute(() -> {
        ch.writeAndFlush(Unpooled.copiedBuffer(data, Charset.defaultCharset()));
    });
}
```

### 9.4 非 Channel 任务使用 DefaultEventLoop

```java
// 用于执行 Channel 无关的任务
DefaultEventLoopGroup bg = new DefaultEventLoopGroup(4);
bg.execute(() -> {
    // 一些计算任务
});
bg.scheduleAtFixedRate(() -> {
    // 定时任务
}, 0, 1, TimeUnit.SECONDS);
```

---

## 10. 高级特性

### 10.1 ChannelGroup（群组管理）

```java
// 用于管理一组 Channel（广播消息）
public class ChannelManager {
    private static final ChannelGroup channels =
            new DefaultChannelGroup(GlobalEventExecutor.INSTANCE);

    public void addChannel(Channel ch) {
        channels.add(ch);
    }

    public void removeChannel(Channel ch) {
        channels.remove(ch);
    }

    public void broadcast(Object msg) {
        channels.writeAndFlush(msg);
    }

    public void broadcastExcept(Object msg, Channel exclude) {
        channels.writeAndFlush(msg, ch -> !ch.equals(exclude));
    }
}
```

### 10.2 连接管理（连接池 / 断线重连）

```java
// 断线重连客户端
public class ReconnectClient {
    private Bootstrap bootstrap;
    private EventLoopGroup group;

    public void start() {
        group = new NioEventLoopGroup();
        bootstrap = new Bootstrap()
            .group(group)
            .channel(NioSocketChannel.class)
            .handler(new ChannelInitializer<SocketChannel>() {
                @Override
                protected void initChannel(SocketChannel ch) {
                    ch.pipeline().addLast(new ReconnectHandler());
                }
            });
        connect();
    }

    private void connect() {
        bootstrap.connect("localhost", 8080).addListener((ChannelFutureListener) future -> {
            if (!future.isSuccess()) {
                System.err.println("Connect failed, retrying...");
                future.channel().eventLoop().schedule(this::connect, 3, TimeUnit.SECONDS);
            }
        });
    }

    // 断线自动重连
    public class ReconnectHandler extends ChannelInboundHandlerAdapter {
        @Override
        public void channelInactive(ChannelHandlerContext ctx) {
            System.out.println("Disconnected, reconnecting...");
            ctx.channel().eventLoop().schedule(() -> connect(), 3, TimeUnit.SECONDS);
        }
    }
}
```

### 10.3 流量控制

```java
// 写缓冲水位控制
b.childOption(ChannelOption.WRITE_BUFFER_WATER_MARK,
        new WriteBufferWaterMark(32 * 1024, 64 * 1024));

// 监控写缓冲
public class FlowControlHandler extends ChannelDuplexHandler {
    @Override
    public void write(ChannelHandlerContext ctx, Object msg, ChannelPromise promise) {
        Channel ch = ctx.channel();
        long pending = ch.bytesBeforeUnwritable();

        if (pending > 0) {
            // 写缓冲堆积，通知对端放慢
            System.out.println("Back pressure: " + pending);
        }

        // 水位变化通知
        ch.config().setAutoRead(false); // 暂停读
        // ... 等水位下降再恢复
        ch.config().setAutoRead(true);
        ctx.write(msg, promise);
    }
}

// 使用 Channel.isWritable() 判断
if (ctx.channel().isWritable()) {
    ctx.writeAndFlush(msg);
} else {
    // 缓冲太满，缓存或丢弃
    pendingQueue.add(msg);
}
```

### 10.4 自定义 EventLoop 任务调度

```java
public class RateLimiterHandler extends ChannelInboundHandlerAdapter {
    private final RateLimiter limiter = RateLimiter.create(1000);

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        if (limiter.tryAcquire()) {
            ctx.fireChannelRead(msg);
        } else {
            ReferenceCountUtil.release(msg);
            // 返回限流响应
            ctx.writeAndFlush(Unpooled.copiedBuffer("TOO FAST", Charset.defaultCharset()));
        }
    }
}
```

### 10.5 自定义 EventLoopGroup (替代 NIO)

```java
// 使用虚拟线程 (JDK 21+)
public class VirtualThreadServer {
    public void start() throws InterruptedException {
        // 每个 Channel 用一个虚拟线程（适合少量连接）
        EventLoopGroup group = new DefaultEventLoopGroup(
                0, Thread.ofVirtual().factory());

        ServerBootstrap b = new ServerBootstrap();
        b.group(group)
         .channel(NioServerSocketChannel.class)
         .childHandler(new ChannelInitializer<SocketChannel>() {
             @Override
             protected void initChannel(SocketChannel ch) {
                 ch.pipeline().addLast(new EchoServerHandler());
             }
         });

        b.bind(8080).sync().channel().closeFuture().await();
    }
}
```

---

## 11. 最佳实践与常见陷阱

### 11.1 内存泄漏预防

```java
// 1. 读到的 msg 必须释放
//   - SimpleChannelInboundHandler 自动释放
//   - ChannelInboundHandlerAdapter 必须手动释放

// ✅ 正确
@Override
public void channelRead(ChannelHandlerContext ctx, Object msg) {
    try {
        // 使用 msg
    } finally {
        ReferenceCountUtil.release(msg);
    }
}

// 或
@Override
public void channelRead(ChannelHandlerContext ctx, Object msg) {
    // 传给下一个 Handler 时，由最后一个 Handler 释放
    ctx.fireChannelRead(msg);
}

// 2. 发送出去的数据 Netty 自动释放（writeAndFlush 后）

// 3. 需要保留引用时 retain
ByteBuf retained = msg.retain(); // +1
ctx.fireChannelRead(msg);       // 后续 Handler 会 release
// 用完后 release retained

// 4. 不要直接释放从 Allocator 申请的 ByteBuf（由 Netty 管理）
```

### 11.2 阻塞操作禁止

```java
// ❌ 错误：在 EventLoop 中执行阻塞操作
pipeline.addLast(new ChannelInboundHandlerAdapter() {
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        // 阻塞了当前 EventLoop 的所有 Channel!
        String result = jdbcTemplate.queryForObject(sql, String.class);
        ctx.writeAndFlush(result);
    }
});

// ✅ 正确：丢到业务线程池
pipeline.addLast(new ChannelInboundHandlerAdapter() {
    private final ExecutorService bizPool = Executors.newFixedThreadPool(16);

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        ByteBuf buf = (ByteBuf) msg;
        bizPool.execute(() -> {
            String result = jdbcTemplate.queryForObject(sql, String.class);
            ctx.writeAndFlush(Unpooled.copiedBuffer(result, Charset.defaultCharset()));
        });
        ReferenceCountUtil.release(msg);  // 注意释放
    }
});

// ✅ 更推荐：添加一个 Handler 在业务线程池执行
pipeline.addLast(new DefaultEventExecutorGroup(16)) // 业务线程池
        .addLast(new BizHandler());                  // BizHandler 在业务线程中执行
```

### 11.3 @Sharable 误用

```java
// ❌ 错误：有状态的 Handler 加了 @Sharable
@ChannelHandler.Sharable
public class StatefulHandler extends ChannelInboundHandlerAdapter {
    private int count;           // 所有 Channel 共享这个变量!
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        count++;                 // 线程不安全!
    }
}

// ✅ 正确：无状态 Handler 才用 @Sharable
@ChannelHandler.Sharable
public class StatelessHandler extends ChannelInboundHandlerAdapter {
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        // 不使用实例变量
        ctx.writeAndFlush(msg);
    }
}
```

### 11.4 Channel 关闭后写入

```java
// 可能抛出 ClosedChannelException
// 写入前检查
if (ctx.channel().isActive()) {
    ctx.writeAndFlush(msg);
} else {
    ReferenceCountUtil.release(msg);
    // 或缓存消息，等待重连后发送
}

// 监听 Future
ctx.writeAndFlush(msg).addListener(f -> {
    if (!f.isSuccess()) {
        // 写入失败处理
    }
});
```

### 11.5 Encoder / Decoder 顺序

```java
// Outbound 执行顺序：从尾到头
// 所以 encoder 应该在 handler 之前（从 handler 发送出去后先经过 encoder）

ch.pipeline()
  .addLast("decoder",  new StringDecoder())      // Inbound:  1 ←
  .addLast("encoder",  new StringEncoder())      // Outbound: 2 ←
  .addLast("handler",  new StringHandler());     // Inbound:  2
                                                 // Outbound: 1 (最后经过)
// 数据流:
// 读: decoder(1) → handler(2)
// 写: handler → encoder(2) → (写网络)
```

### 11.6 性能调优

```java
public class NettyTuning {

    // 1. 选择合适的 EventLoopGroup 线程数
    //    服务端: Boss=1, Worker=CPU*2
    //    客户端: 根据连接数（通常 1~4 即可）
    EventLoopGroup boss   = new NioEventLoopGroup(1);
    EventLoopGroup worker = new NioEventLoopGroup(
            Runtime.getRuntime().availableProcessors() * 2);

    // 2. 使用 Epoll（Linux）或 KQueue（macOS）
    //    Linux:  -Dio.netty.transport.epoll.native=true
    //    代码:   EpollSocketChannel.class / KQueueSocketChannel.class

    // 3. 开启池化分配器（默认开启）
    //    -Dio.netty.allocator.type=pooled

    // 4. 调整 RecvByteBufAllocator
    bootstrap.option(ChannelOption.RCVBUF_ALLOCATOR,
            new AdaptiveRecvByteBufAllocator(512, 2048, 65536));

    // 5. 减少 Handler 数量（合并简单 Handler）
    //    使用 ChannelDuplexHandler 替代 Inbound+Outbound 两个

    // 6. 使用 SimpleChannelInboundHandler 代替 Adapter（自动 release）
}
```

### 11.7 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|------|----------|----------|
| 内存泄漏 | ByteBuf 未 release | `-Dio.netty.leakDetectionLevel=advanced` |
| OutOfMemory | Buffer 堆积/泄漏 | 检查 write 缓冲 + 水位控制 |
| 连接断开 | Idle 触发 | 心跳检测 + 合理超时 |
| 粘包/半包 | 未做拆包 | 添加 FrameDecoder |
| 性能低 | 阻塞 EventLoop | 业务操作移到线程池 |
| 死锁 | EventLoop 等待自身 | 确保不向 EventLoop 提交阻塞任务 |
| 解码异常 | 协议不匹配 | 检查魔数/版本/长度字段 |

### 11.8 常用系统属性

```bash
# 内存检测
-Dio.netty.leakDetectionLevel=advanced

# 内存池
-Dio.netty.allocator.type=pooled
-Dio.netty.allocator.maxOrder=11     # PooledByteBufAllocator 管理大小
-Dio.netty.allocator.numHeapArenas=4
-Dio.netty.allocator.numDirectArenas=4

# I/O 模型（Linux 强制 epoll）
-Dio.netty.transport.epoll.native=true

# 是否启用 native transport
-Dio.netty.noPreferEpoll=false

# Buffer 调优
-Dio.netty.buffer.byteBuf.checkAccessible=false  # 关闭检查提性能

# 是否打印泄漏报告
-Dio.netty.leakDetection.maxRecords=4
```

---

## 附录

### A. 完整服务端模板

```java
public class NettyServerTemplate {
    private final int port;
    private EventLoopGroup bossGroup;
    private EventLoopGroup workerGroup;

    public NettyServerTemplate(int port) {
        this.port = port;
    }

    public void start() throws InterruptedException {
        bossGroup   = new NioEventLoopGroup(1);
        workerGroup = new NioEventLoopGroup();

        try {
            ServerBootstrap b = new ServerBootstrap();
            b.group(bossGroup, workerGroup)
             .channel(NioServerSocketChannel.class)
             .option(ChannelOption.SO_BACKLOG, 128)
             .childOption(ChannelOption.SO_KEEPALIVE, true)
             .childOption(ChannelOption.TCP_NODELAY, true)
             .childHandler(new ChannelInitializer<SocketChannel>() {
                 @Override
                 protected void initChannel(SocketChannel ch) {
                     ch.pipeline()
                       .addLast("idle",       new IdleStateHandler(30, 0, 0))
                       .addLast("frame",      new LengthFieldBasedFrameDecoder(
                               1024 * 1024, 0, 4, 0, 4))
                       .addLast("decoder",    new CustomPacketDecoder())
                       .addLast("encoder",    new CustomPacketEncoder())
                       .addLast("biz-group",  new DefaultEventExecutorGroup(16))
                       .addLast("handler",    new BizHandler());
                 }
             });

            ChannelFuture f = b.bind(port).sync();
            f.channel().closeFuture().sync();
        } finally {
            shutdown();
        }
    }

    public void shutdown() {
        if (workerGroup != null) workerGroup.shutdownGracefully();
        if (bossGroup   != null) bossGroup.shutdownGracefully();
    }

    public static void main(String[] args) throws InterruptedException {
        new NettyServerTemplate(8080).start();
    }
}
```

### B. 完整客户端模板

```java
public class NettyClientTemplate {
    private EventLoopGroup group;
    private Bootstrap bootstrap;
    private Channel channel;

    public void start(String host, int port) throws InterruptedException {
        group = new NioEventLoopGroup();
        bootstrap = new Bootstrap()
            .group(group)
            .channel(NioSocketChannel.class)
            .option(ChannelOption.TCP_NODELAY, true)
            .option(ChannelOption.CONNECT_TIMEOUT_MILLIS, 5000)
            .handler(new ChannelInitializer<SocketChannel>() {
                @Override
                protected void initChannel(SocketChannel ch) {
                    ch.pipeline()
                      .addLast("frame",   new LengthFieldBasedFrameDecoder(
                              1024 * 1024, 0, 4, 0, 4))
                      .addLast("decoder", new CustomPacketDecoder())
                      .addLast("encoder", new CustomPacketEncoder())
                      .addLast("handler", new BizClientHandler());
                }
            });

        channel = bootstrap.connect(host, port).sync().channel();
    }

    public void send(Object msg) {
        if (channel != null && channel.isActive()) {
            channel.writeAndFlush(msg);
        }
    }

    public void shutdown() {
        if (channel  != null) channel.close();
        if (group    != null) group.shutdownGracefully();
    }
}
```

### C. Maven 依赖（推荐版本）

```xml
<properties>
    <netty.version>4.1.100.Final</netty.version>
</properties>

<dependencies>
    <!-- 核心 -->
    <dependency>
        <groupId>io.netty</groupId>
        <artifactId>netty-all</artifactId>
        <version>${netty.version}</version>
    </dependency>

    <!-- 按需引入（替代 netty-all，更轻量） -->
    <!--
    <dependency>
        <groupId>io.netty</groupId>
        <artifactId>netty-buffer</artifactId>
        <version>${netty.version}</version>
    </dependency>
    <dependency>
        <groupId>io.netty</groupId>
        <artifactId>netty-codec</artifactId>
        <version>${netty.version}</version>
    </dependency>
    <dependency>
        <groupId>io.netty</groupId>
        <artifactId>netty-codec-http</artifactId>
        <version>${netty.version}</version>
    </dependency>
    <dependency>
        <groupId>io.netty</groupId>
        <artifactId>netty-handler</artifactId>
        <version>${netty.version}</version>
    </dependency>
    <dependency>
        <groupId>io.netty</groupId>
        <artifactId>netty-transport-native-epoll</artifactId>
        <version>${netty.version}</version>
        <classifier>linux-x86_64</classifier>
    </dependency>
    -->
</dependencies>
```

---

> **参考**: [Netty Official Doc](https://netty.io/wiki/user-guide.html)、[Netty in Action](https://www.manning.com/books/netty-in-action)、源码分析
