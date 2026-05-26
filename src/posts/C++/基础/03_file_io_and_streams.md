---
cover: /assets/images/cover2.jpg
icon: pen-to-square
date: 2026-01-2
category:
  - 文件io
  - C++
tag:
  - IO
star: true
sticky: true
---


# C++ 文件操作、IO 与流详解

## 1. 文档目标

本文整理 C++ 常用输入输出方式，包括：

- 标准输入输出
- 格式化输出
- 文件读写
- 二进制文件处理
- 字符串流
- 文件系统基础

---

## 2. 常用头文件

- `<iostream>`：标准输入输出
- `<iomanip>`：格式化输出
- `<fstream>`：文件流
- `<sstream>`：字符串流
- `<string>`：字符串
- `<vector>`：缓冲区容器
- `<filesystem>`：文件系统操作
- `<cstdio>`：C 风格文件操作

---

## 3. 标准输入输出

### 3.1 `std::cout`

```cpp
#include <iostream>

int main() {
    std::cout << "hello" << std::endl;
    return 0;
}
```

### 3.2 `std::cin`

```cpp
#include <iostream>

int main() {
    int age = 0;
    std::cin >> age;
    std::cout << "age = " << age << std::endl;
    return 0;
}
```

### 3.3 `std::getline`

```cpp
#include <iostream>
#include <string>

int main() {
    std::string line;
    std::getline(std::cin, line);
    std::cout << line << std::endl;
    return 0;
}
```

---

## 4. 格式化输出

### 4.1 小数位数控制

```cpp
#include <iomanip>
#include <iostream>

int main() {
    double pi = 3.1415926;
    std::cout << std::fixed << std::setprecision(3) << pi << std::endl;
    return 0;
}
```

### 4.2 宽度与填充

```cpp
#include <iomanip>
#include <iostream>

int main() {
    std::cout << std::setw(8) << std::setfill('0') << 42 << std::endl;
    return 0;
}
```

---

## 5. 文本文件读写

### 5.1 写文件

```cpp
#include <fstream>

int main() {
    std::ofstream out("app.log");
    out << "server started" << '\n';
    out << "port=9000" << '\n';
    return 0;
}
```

### 5.2 读文件

```cpp
#include <fstream>
#include <iostream>
#include <string>

int main() {
    std::ifstream in("app.log");
    std::string line;
    while (std::getline(in, line)) {
        std::cout << line << std::endl;
    }
    return 0;
}
```

### 5.3 逐词读取

```cpp
#include <fstream>
#include <iostream>
#include <string>

int main() {
    std::ifstream in("config.txt");
    std::string token;
    while (in >> token) {
        std::cout << token << std::endl;
    }
    return 0;
}
```

---

## 6. 二进制文件操作

### 6.1 写二进制数据

```cpp
#include <cstdint>
#include <fstream>

int main() {
    std::ofstream out("data.bin", std::ios::binary);
    std::uint32_t value = 0x12345678;
    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
    return 0;
}
```

### 6.2 读二进制数据

```cpp
#include <cstdint>
#include <fstream>
#include <iostream>

int main() {
    std::ifstream in("data.bin", std::ios::binary);
    std::uint32_t value = 0;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    std::cout << std::hex << value << std::endl;
    return 0;
}
```

注意：

- 直接写内存块只适合同平台或已知布局的场景
- 对协议和跨平台文件格式，应逐字段序列化

---

## 7. 使用字符串流解析文本

### 7.1 解析一行配置

```cpp
#include <iostream>
#include <sstream>
#include <string>

int main() {
    std::string line = "127.0.0.1 9000";
    std::stringstream ss(line);
    std::string host;
    int port = 0;
    ss >> host >> port;
    std::cout << host << ", " << port << std::endl;
    return 0;
}
```

### 7.2 拼装字符串

```cpp
#include <iostream>
#include <sstream>

int main() {
    std::stringstream ss;
    ss << "id=" << 1001 << ", ok=" << true;
    std::cout << ss.str() << std::endl;
    return 0;
}
```

---

## 8. 文件定位与大小

### 8.1 获取文件大小

```cpp
#include <fstream>
#include <iostream>

int main() {
    std::ifstream in("data.bin", std::ios::binary);
    in.seekg(0, std::ios::end);
    std::streampos size = in.tellg();
    std::cout << size << std::endl;
    return 0;
}
```

### 8.2 随机访问读取

```cpp
#include <fstream>
#include <iostream>

int main() {
    std::ifstream in("data.bin", std::ios::binary);
    in.seekg(2, std::ios::beg);
    char ch = 0;
    in.read(&ch, 1);
    std::cout << static_cast<int>(static_cast<unsigned char>(ch)) << std::endl;
    return 0;
}
```

---

## 9. 一次性读取整个文件

```cpp
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main() {
    std::ifstream in("app.log");
    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
    std::cout << content << std::endl;
    return 0;
}
```

---

## 10. 使用 `vector` 读取整个二进制文件

```cpp
#include <fstream>
#include <iostream>
#include <vector>

int main() {
    std::ifstream in("data.bin", std::ios::binary);
    in.seekg(0, std::ios::end);
    std::size_t size = static_cast<std::size_t>(in.tellg());
    in.seekg(0, std::ios::beg);

    std::vector<char> buf(size);
    in.read(&buf[0], size);
    std::cout << "size=" << buf.size() << std::endl;
    return 0;
}
```

---

## 11. 错误检查

### 11.1 判断文件是否打开成功

```cpp
#include <fstream>
#include <iostream>

int main() {
    std::ifstream in("not_exist.txt");
    if (!in) {
        std::cout << "open failed" << std::endl;
        return 1;
    }
    return 0;
}
```

### 11.2 流状态位

- `good()`
- `fail()`
- `eof()`
- `bad()`

---

## 12. C 风格文件操作

### 12.1 `fopen` / `fread` / `fwrite`

```cpp
#include <cstdio>

int main() {
    FILE* fp = std::fopen("raw.bin", "wb");
    if (!fp) {
        return 1;
    }

    char buf[4] = {1, 2, 3, 4};
    std::fwrite(buf, 1, sizeof(buf), fp);
    std::fclose(fp);
    return 0;
}
```

适用场景：

- 兼容 C 库
- 需要和旧代码混合
- 某些低层接口习惯使用 `FILE*`

---

## 13. 文件系统操作

### 13.1 创建目录

```cpp
#include <filesystem>

int main() {
    std::filesystem::create_directories("logs/2026");
    return 0;
}
```

### 13.2 判断文件是否存在

```cpp
#include <filesystem>
#include <iostream>

int main() {
    bool ok = std::filesystem::exists("app.log");
    std::cout << ok << std::endl;
    return 0;
}
```

### 13.3 遍历目录

```cpp
#include <filesystem>
#include <iostream>

int main() {
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        std::cout << entry.path().string() << std::endl;
    }
    return 0;
}
```

---

## 14. 实战示例：配置文件读取

假设配置文件 `server.conf` 内容如下：

```text
host=127.0.0.1
port=9000
worker=4
```

示例解析代码：

```cpp
#include <fstream>
#include <iostream>
#include <map>
#include <string>

int main() {
    std::ifstream in("server.conf");
    std::string line;
    std::map<std::string, std::string> config;

    while (std::getline(in, line)) {
        std::size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        config[key] = value;
    }

    std::cout << config["host"] << std::endl;
    std::cout << config["port"] << std::endl;
    std::cout << config["worker"] << std::endl;
    return 0;
}
```

---

## 15. 文件 IO 最佳实践

1. 文本与二进制模式要分清
2. 对外部输入必须做打开失败检查
3. 不要默认文件内容合法
4. 二进制协议不要依赖结构体直接落盘
5. 日志、配置、协议数据建议拆清格式边界

