
---
cover: /assets/images/cover2.jpg
icon: pen-to-square
date: 2026-05-20
category:
  - C++
tag:
  - 新特性
  - C++11-17
star: true
sticky: true
---


# 现代 C++ 新特性详解

## 1. 文档范围

本文按工程学习视角整理 C++11 到 C++22 阶段常用新特性。

严格 ISO 版本通常写作：

- C++11
- C++14
- C++17
- C++20
- C++23

如果项目内部使用 “C++22” 说法，建议在文档中明确它是“工程阶段”而非正式标准名称。

---

## 2. C++11

### 2.1 `auto`

```cpp
#include <iostream>
#include <vector>

int main() {
    auto value = 10;
    std::vector<int> nums{1, 2, 3};
    auto it = nums.begin();
    std::cout << value << ", " << *it << std::endl;
    return 0;
}
```

适用场景：

- 迭代器类型很长
- 初始化表达式已经清晰表达类型

### 2.2 范围 `for`

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> nums{1, 2, 3, 4};
    for (int n : nums) {
        std::cout << n << std::endl;
    }
    return 0;
}
```

### 2.3 `nullptr`

```cpp
#include <iostream>

void print(int* p) {
    if (p == nullptr) {
        std::cout << "null" << std::endl;
    }
}
```

### 2.4 Lambda

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::vector<int> nums{1, 2, 3, 4, 5};
    int count = std::count_if(nums.begin(), nums.end(), [](int x) {
        return x % 2 == 0;
    });
    std::cout << count << std::endl;
    return 0;
}
```

### 2.5 右值引用与移动语义

```cpp
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::vector<std::string> values;
    std::string name = "packet";
    values.push_back(name);
    values.push_back(std::move(name));
    std::cout << values.size() << std::endl;
    return 0;
}
```

### 2.6 智能指针

```cpp
#include <iostream>
#include <memory>

class Session {
public:
    ~Session() {
        std::cout << "destroy session" << std::endl;
    }
};

int main() {
    std::unique_ptr<Session> session(new Session());
    return 0;
}
```

### 2.7 `enum class`

```cpp
#include <iostream>

enum class PacketType {
    Ping = 1,
    Pong = 2
};

int main() {
    PacketType type = PacketType::Ping;
    std::cout << static_cast<int>(type) << std::endl;
    return 0;
}
```

### 2.8 `constexpr`

```cpp
constexpr int square(int x) {
    return x * x;
}
```

### 2.9 多线程基础

```cpp
#include <iostream>
#include <thread>

void worker() {
    std::cout << "worker running" << std::endl;
}

int main() {
    std::thread t(worker);
    t.join();
    return 0;
}
```

---

## 3. C++14

### 3.1 泛型 Lambda

```cpp
#include <iostream>

int main() {
    auto add = [](auto a, auto b) {
        return a + b;
    };
    std::cout << add(3, 4) << std::endl;
    std::cout << add(1.5, 2.5) << std::endl;
    return 0;
}
```

### 3.2 `make_unique`

```cpp
#include <memory>

class Config {};

int main() {
    auto cfg = std::make_unique<Config>();
    return 0;
}
```

---

## 4. C++17

### 4.1 结构化绑定

```cpp
#include <iostream>
#include <map>

int main() {
    std::map<std::string, int> ports{{"http", 80}, {"https", 443}};
    for (const auto& [name, port] : ports) {
        std::cout << name << " -> " << port << std::endl;
    }
    return 0;
}
```

### 4.2 `if constexpr`

```cpp
#include <iostream>
#include <type_traits>

template <typename T>
void printType() {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "integral" << std::endl;
    } else {
        std::cout << "other" << std::endl;
    }
}
```

### 4.3 `std::optional`

```cpp
#include <iostream>
#include <optional>

std::optional<int> parsePort(bool ok) {
    if (!ok) {
        return std::nullopt;
    }
    return 8080;
}
```

### 4.4 `std::variant`

```cpp
#include <iostream>
#include <string>
#include <variant>

int main() {
    std::variant<int, std::string> value = "hello";
    std::cout << std::get<std::string>(value) << std::endl;
    return 0;
}
```

### 4.5 `std::filesystem`

```cpp
#include <filesystem>
#include <iostream>

int main() {
    std::filesystem::path p = "logs";
    std::cout << p.string() << std::endl;
    return 0;
}
```

---

## 5. C++20

### 5.1 Concepts

```cpp
#include <concepts>

template <std::integral T>
T add(T a, T b) {
    return a + b;
}
```

### 5.2 `std::span`

```cpp
#include <iostream>
#include <span>
#include <vector>

void printData(std::span<const int> values) {
    for (int v : values) {
        std::cout << v << ' ';
    }
    std::cout << std::endl;
}
```

### 5.3 三路比较

```cpp
#include <compare>

struct Version {
    int major;
    int minor;

    auto operator<=>(const Version&) const = default;
};
```

### 5.4 协程

```cpp
// 协程通常需要额外封装与运行时支持。
// 这里先给出概念占位，实际项目建议单独专题展开。
```

### 5.5 Ranges

```cpp
#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

int main() {
    std::vector<int> nums{1, 2, 3, 4, 5, 6};
    auto even = nums | std::views::filter([](int v) { return v % 2 == 0; });
    for (int v : even) {
        std::cout << v << std::endl;
    }
    return 0;
}
```

---

## 6. C++22 / 工程阶段能力

这一节按工程实践归纳，适合承接 “C++20 之后实际项目开始常用的能力”。

### 6.1 `std::format`

```cpp
#include <format>
#include <iostream>

int main() {
    std::string text = std::format("host={}, port={}", "127.0.0.1", 9000);
    std::cout << text << std::endl;
    return 0;
}
```

### 6.2 标准库补强趋势

重点关注：

- 更强的格式化能力
- 更强的范围算法
- 更适合系统编程的视图和字节工具
- 更清晰的错误返回模型

---

## 7. 从旧代码迁移到现代 C++

### 7.1 用 `nullptr` 替代 `NULL`

### 7.2 用范围 `for` 替代手写迭代器循环

### 7.3 用智能指针替代裸资源所有权

### 7.4 用 `enum class` 替代普通枚举

### 7.5 用 `constexpr` 和模板替代危险宏

---

## 8. 新特性使用建议

优先级建议：

1. 先掌握 `auto`、Lambda、智能指针、范围 `for`
2. 再掌握 `optional`、`variant`、`filesystem`
3. 然后学习 `span`、Concepts、Ranges
4. 最后再进入协程、模块等更大主题

