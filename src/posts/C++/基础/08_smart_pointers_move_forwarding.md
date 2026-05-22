# 智能指针、移动语义与完美转发详解

## 1. 文档目标

本文把现代 C++ 最重要的一组能力集中讲清楚：

- RAII
- 所有权模型
- `unique_ptr`
- `shared_ptr`
- `weak_ptr`
- 右值引用
- 移动语义
- `std::move`
- 转发引用
- `std::forward`
- 完美转发

这部分是写工程代码时最常决定代码质量的一组知识。

---

## 2. RAII

RAII 的核心思想：

- 资源获取即初始化
- 对象析构时自动释放资源

示例：

```cpp
#include <cstdio>
#include <stdexcept>

class FileHandle {
public:
    explicit FileHandle(const char* path) : fp_(std::fopen(path, "rb")) {
        if (!fp_) {
            throw std::runtime_error("open file failed");
        }
    }

    ~FileHandle() {
        if (fp_) {
            std::fclose(fp_);
        }
    }

private:
    FILE* fp_ = nullptr;
};
```

---

## 3. 为什么需要智能指针

裸指针的问题：

- 不表达所有权
- 易泄漏
- 易悬空
- 异常路径容易漏释放

智能指针解决的是：

- 资源生命周期
- 所有权表达
- 异常安全

---

## 4. `std::unique_ptr`

`unique_ptr` 表示独占所有权。

### 4.1 基本使用

```cpp
#include <iostream>
#include <memory>

class Connection {
public:
    ~Connection() {
        std::cout << "close connection" << std::endl;
    }
};

int main() {
    std::unique_ptr<Connection> conn = std::make_unique<Connection>();
    return 0;
}
```

### 4.2 转移所有权

```cpp
#include <memory>

struct Session {};

std::unique_ptr<Session> createSession() {
    return std::make_unique<Session>();
}

int main() {
    std::unique_ptr<Session> a = createSession();
    std::unique_ptr<Session> b = std::move(a);
    return 0;
}
```

说明：

- `unique_ptr` 不能拷贝
- 只能移动

### 4.3 作为函数参数

```cpp
#include <memory>

struct Worker {};

void useWorker(const Worker* worker) {
}

void takeOwnership(std::unique_ptr<Worker> worker) {
}
```

建议：

- 只使用对象但不接管所有权：传裸指针或引用
- 要接管所有权：传 `unique_ptr`

---

## 5. `std::shared_ptr`

`shared_ptr` 表示共享所有权。

### 5.1 基本使用

```cpp
#include <iostream>
#include <memory>

struct Config {
    ~Config() {
        std::cout << "destroy config" << std::endl;
    }
};

int main() {
    std::shared_ptr<Config> a = std::make_shared<Config>();
    std::shared_ptr<Config> b = a;
    std::cout << a.use_count() << std::endl;
    return 0;
}
```

### 5.2 使用场景

适合：

- 对象需要被多个模块共同持有
- 生命周期难以单点管理

不适合：

- 所有权本来就很清晰
- 性能敏感但又不需要共享

---

## 6. `std::weak_ptr`

`weak_ptr` 用来打破循环引用。

### 6.1 循环引用问题

```cpp
#include <memory>

struct B;

struct A {
    std::shared_ptr<B> b;
};

struct B {
    std::shared_ptr<A> a;
};
```

这里 `A` 和 `B` 互相引用，可能无法释放。

### 6.2 使用 `weak_ptr`

```cpp
#include <memory>

struct B;

struct A {
    std::shared_ptr<B> b;
};

struct B {
    std::weak_ptr<A> a;
};
```

### 6.3 `lock`

```cpp
#include <iostream>
#include <memory>

int main() {
    std::shared_ptr<int> p = std::make_shared<int>(42);
    std::weak_ptr<int> wp = p;

    if (std::shared_ptr<int> sp = wp.lock()) {
        std::cout << *sp << std::endl;
    }
    return 0;
}
```

---

## 7. 自定义删除器

```cpp
#include <cstdio>
#include <memory>

int main() {
    std::unique_ptr<FILE, decltype(&std::fclose)> fp(std::fopen("a.txt", "w"), &std::fclose);
    if (fp) {
        std::fputs("hello\n", fp.get());
    }
    return 0;
}
```

适合封装：

- `FILE*`
- socket fd 包装对象
- 句柄资源

---

## 8. 左值、右值与右值引用

### 8.1 左值与右值

- 左值：有名字、可取地址、可长期存在
- 右值：临时对象或将被销毁的值

### 8.2 右值引用

```cpp
int&& value = 10;
```

这允许我们区分：

- 可复用对象
- 即将销毁的临时对象

---

## 9. 移动语义

### 9.1 为什么需要移动

拷贝大对象成本高。移动语义允许“转移资源所有权”而不是深拷贝。

### 9.2 示例：自定义移动构造

```cpp
#include <cstring>
#include <iostream>

class Buffer {
public:
    explicit Buffer(std::size_t size)
        : size_(size), data_(new char[size]) {}

    ~Buffer() {
        delete[] data_;
    }

    Buffer(const Buffer& other)
        : size_(other.size_), data_(new char[other.size_]) {
        std::memcpy(data_, other.data_, size_);
    }

    Buffer(Buffer&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        other.size_ = 0;
        other.data_ = nullptr;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = other.data_;
            other.size_ = 0;
            other.data_ = nullptr;
        }
        return *this;
    }

private:
    std::size_t size_ = 0;
    char* data_ = nullptr;
};
```

### 9.3 `std::move`

```cpp
#include <string>
#include <utility>

int main() {
    std::string a = "hello";
    std::string b = std::move(a);
    return 0;
}
```

说明：

- `std::move` 不会真的移动资源
- 它只是把对象转换成右值，允许调用移动构造或移动赋值

---

## 10. 万能引用 / 转发引用

### 10.1 模板里的 `T&&`

```cpp
template <typename T>
void func(T&& value) {
}
```

当 `T` 参与推导时，这个 `T&&` 不是单纯右值引用，而是转发引用。

规则：

- 传左值时，`T` 推导成左值引用类型
- 传右值时，`T` 推导成普通类型

---

## 11. 完美转发

完美转发的目标是：

- 原样保留参数的左值/右值属性
- 原样转发给下一层函数

### 11.1 示例

```cpp
#include <iostream>
#include <utility>

void sink(int& x) {
    std::cout << "lvalue: " << x << std::endl;
}

void sink(int&& x) {
    std::cout << "rvalue: " << x << std::endl;
}

template <typename T>
void wrapper(T&& value) {
    sink(std::forward<T>(value));
}

int main() {
    int a = 5;
    wrapper(a);
    wrapper(10);
    return 0;
}
```

### 11.2 `std::forward`

作用：

- 如果原参数是左值，就转发成左值
- 如果原参数是右值，就转发成右值

---

## 12. `emplace_back` 与完美转发

`emplace_back` 典型依赖完美转发。

```cpp
#include <iostream>
#include <string>
#include <vector>

class User {
public:
    User(std::string name, int age) : name_(name), age_(age) {}

private:
    std::string name_;
    int age_;
};

int main() {
    std::vector<User> users;
    users.emplace_back("Alice", 18);
    return 0;
}
```

好处：

- 直接在容器内构造对象
- 减少临时对象

---

## 13. 常见误区

### 13.1 误把 `std::move` 当移动本身

它只是类型转换工具。

### 13.2 对已经 `move` 过的对象继续依赖原值

移动后的对象只保证“有效但未指定”状态。

### 13.3 滥用 `shared_ptr`

很多对象其实应该明确唯一所有权，用 `unique_ptr` 更清晰。

### 13.4 完美转发写错导致值类别丢失

错误示例：

```cpp
template <typename T>
void bad(T&& value) {
    sink(value);
}
```

这里 `value` 在函数体内是左值。

正确方式：

```cpp
template <typename T>
void good(T&& value) {
    sink(std::forward<T>(value));
}
```

---

## 14. 实战建议

1. 默认优先栈对象
2. 动态资源优先 `unique_ptr`
3. 真正共享时才用 `shared_ptr`
4. 需要观察但不拥有时用 `weak_ptr`
5. 写泛型构造封装时理解转发引用和 `std::forward`
6. 对性能敏感代码理解移动语义非常重要

