# 协议对接、字节处理与数据帧解析详解

## 1. 文档目标

本文专门解决协议对接时最容易出问题的部分：

- 字节序
- 整数字段解析
- 文本与二进制协议
- 粘包与拆包
- 数据帧设计
- 流式解码
- 封包与解包

---

## 2. TCP 为什么必须设计帧

TCP 是字节流协议，不保留应用层消息边界。

这意味着：

- 发送两次，接收端可能一次收到
- 发送一次，接收端可能分多次收到

所以必须在应用层定义帧格式。

---

## 3. 一个典型帧格式

| 字段 | 长度 | 说明 |
|---|---:|---|
| Magic | 2 字节 | 固定头 `0xAA55` |
| Version | 1 字节 | 协议版本 |
| Type | 1 字节 | 消息类型 |
| Length | 4 字节 | Payload 长度，大端 |
| Payload | N 字节 | 业务数据 |
| Checksum | 2 字节 | 校验 |

最短帧长度：

```text
2 + 1 + 1 + 4 + 0 + 2 = 10 字节
```

---

## 4. 常用基础类型

```cpp
#include <cstdint>
#include <vector>

struct Frame {
    std::uint8_t version = 0;
    std::uint8_t type = 0;
    std::vector<std::uint8_t> payload;
    std::uint16_t checksum = 0;
};
```

---

## 5. 字节序与字段读取

### 5.1 读取大端 16 位整数

```cpp
#include <cstdint>
#include <vector>

std::uint16_t readUint16BE(const std::vector<std::uint8_t>& data, std::size_t pos) {
    return (static_cast<std::uint16_t>(data[pos]) << 8) |
           static_cast<std::uint16_t>(data[pos + 1]);
}
```

### 5.2 读取大端 32 位整数

```cpp
#include <cstdint>
#include <vector>

std::uint32_t readUint32BE(const std::vector<std::uint8_t>& data, std::size_t pos) {
    return (static_cast<std::uint32_t>(data[pos]) << 24) |
           (static_cast<std::uint32_t>(data[pos + 1]) << 16) |
           (static_cast<std::uint32_t>(data[pos + 2]) << 8) |
           static_cast<std::uint32_t>(data[pos + 3]);
}
```

### 5.3 写入大端整数

```cpp
#include <cstdint>
#include <vector>

void writeUint32BE(std::vector<std::uint8_t>& out, std::uint32_t value) {
    out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFF));
    out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<std::uint8_t>(value & 0xFF));
}
```

---

## 6. 校验和示例

```cpp
#include <cstdint>
#include <vector>

std::uint16_t calcChecksum(const std::vector<std::uint8_t>& data) {
    std::uint32_t sum = 0;
    for (std::uint8_t byte : data) {
        sum += byte;
    }
    return static_cast<std::uint16_t>(sum & 0xFFFF);
}
```

说明：

- 这里只是示例校验
- 实际项目可能使用 CRC16、CRC32、异或校验或 MAC

---

## 7. 封包

```cpp
#include <cstdint>
#include <vector>

std::vector<std::uint8_t> buildFrame(std::uint8_t version,
                                     std::uint8_t type,
                                     const std::vector<std::uint8_t>& payload) {
    std::vector<std::uint8_t> out;
    out.push_back(0xAA);
    out.push_back(0x55);
    out.push_back(version);
    out.push_back(type);
    writeUint32BE(out, static_cast<std::uint32_t>(payload.size()));
    out.insert(out.end(), payload.begin(), payload.end());

    std::uint16_t checksum = calcChecksum(payload);
    out.push_back(static_cast<std::uint8_t>((checksum >> 8) & 0xFF));
    out.push_back(static_cast<std::uint8_t>(checksum & 0xFF));
    return out;
}
```

### 7.1 封包示例

```cpp
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string text = "PING";
    std::vector<std::uint8_t> payload(text.begin(), text.end());
    std::vector<std::uint8_t> frame = buildFrame(1, 1, payload);

    for (std::size_t i = 0; i < frame.size(); ++i) {
        std::cout << std::hex << static_cast<int>(frame[i]) << ' ';
    }
    std::cout << std::endl;
    return 0;
}
```

---

## 8. 单帧解析

```cpp
#include <cstdint>
#include <optional>
#include <vector>

std::optional<Frame> parseFrame(const std::vector<std::uint8_t>& buf) {
    const std::size_t kMinSize = 10;
    if (buf.size() < kMinSize) {
        return std::nullopt;
    }

    if (readUint16BE(buf, 0) != 0xAA55) {
        return std::nullopt;
    }

    std::uint8_t version = buf[2];
    std::uint8_t type = buf[3];
    std::uint32_t payload_len = readUint32BE(buf, 4);

    std::size_t total = 2 + 1 + 1 + 4 + payload_len + 2;
    if (buf.size() < total) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> payload(buf.begin() + 8, buf.begin() + 8 + payload_len);
    std::uint16_t checksum = readUint16BE(buf, 8 + payload_len);

    if (calcChecksum(payload) != checksum) {
        return std::nullopt;
    }

    return Frame{version, type, payload, checksum};
}
```

---

## 9. 流式解码器

网络程序中通常不会一次收到完整帧，因此需要缓存并增量解析。

```cpp
#include <cstdint>
#include <optional>
#include <vector>

class StreamFrameDecoder {
public:
    void append(const std::uint8_t* data, std::size_t len) {
        buffer_.insert(buffer_.end(), data, data + len);
    }

    std::optional<Frame> nextFrame() {
        while (buffer_.size() >= 2) {
            if (readUint16BE(buffer_, 0) == 0xAA55) {
                break;
            }
            buffer_.erase(buffer_.begin());
        }

        if (buffer_.size() < 10) {
            return std::nullopt;
        }

        std::uint32_t payload_len = readUint32BE(buffer_, 4);
        std::size_t total = 2 + 1 + 1 + 4 + payload_len + 2;
        if (buffer_.size() < total) {
            return std::nullopt;
        }

        std::vector<std::uint8_t> one(buffer_.begin(), buffer_.begin() + total);
        buffer_.erase(buffer_.begin(), buffer_.begin() + total);
        return parseFrame(one);
    }

private:
    std::vector<std::uint8_t> buffer_;
};
```

---

## 10. 接收循环中的使用方式

```cpp
#include <cstdint>
#include <sys/socket.h>
#include <unistd.h>

int processSocket(int fd) {
    StreamFrameDecoder decoder;
    std::uint8_t buf[1024];

    while (true) {
        ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) {
            break;
        }

        decoder.append(buf, static_cast<std::size_t>(n));
        while (true) {
            std::optional<Frame> frame = decoder.nextFrame();
            if (!frame.has_value()) {
                break;
            }
            // 处理 frame.value()
        }
    }
    return 0;
}
```

---

## 11. 文本协议与二进制协议

### 11.1 文本协议

优点：

- 易读
- 抓包调试方便

缺点：

- 体积更大
- 编码和转义复杂

### 11.2 二进制协议

优点：

- 紧凑
- 性能更高
- 字段控制更明确

缺点：

- 调试难度更高
- 必须严格处理字节序和长度

---

## 12. 常见坑

### 12.1 直接把结构体映射到网络缓冲区

不推荐：

```cpp
// 风险示例
// MyHeader* h = reinterpret_cast<MyHeader*>(buffer);
```

原因：

- 字节序问题
- 对齐问题
- 编译器填充问题
- 可移植性差

### 12.2 没有做长度校验

任何协议字段解析前都必须检查：

- 起始偏移是否合法
- 长度字段是否越界
- 整帧长度是否超上限

### 12.3 没有做同步恢复

当接收到脏数据时，应尝试重新找帧头，而不是直接假设缓冲区起点正确。

---

## 13. 实战建议

1. 先画出协议字段表
2. 明确每个字段的字节序
3. 明确最大帧长
4. 明确校验方式
5. 收包与解包分层
6. 对异常包、半包、粘包分别测试

