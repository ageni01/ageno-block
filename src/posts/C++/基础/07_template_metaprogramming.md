# C++ 模板与元编程详解

## 1. 文档目标

本文专门展开 C++ 里最容易“看着会、写起来难”的部分：

- 函数模板
- 类模板
- 模板特化
- 类型萃取
- SFINAE
- `enable_if`
- 编译期计算
- `decltype`、`decltype(auto)`
- 可变参数模板
- 折叠表达式
- Concepts

这部分既是现代 C++ 泛型能力的核心，也是很多库设计的基础。

---

## 2. 为什么需要元编程

元编程的本质是：

- 在编译期根据类型或常量做决策
- 生成更通用的代码
- 降低运行期开销
- 提前暴露类型错误

常见应用场景：

- 容器与算法库
- 序列化框架
- RPC 框架
- 反射模拟
- 接口约束与静态检查

---

## 3. 函数模板

### 3.1 最基础形式

```cpp
#include <iostream>

template <typename T>
T add(T a, T b) {
    return a + b;
}

int main() {
    std::cout << add(1, 2) << std::endl;
    std::cout << add(1.5, 2.5) << std::endl;
    return 0;
}
```

### 3.2 多模板参数

```cpp
#include <iostream>

template <typename T, typename U>
T castAdd(T a, U b) {
    return a + static_cast<T>(b);
}
```

---

## 4. 类模板

```cpp
#include <iostream>

template <typename T>
class Box {
public:
    explicit Box(const T& value) : value_(value) {}

    T get() const {
        return value_;
    }

private:
    T value_;
};

int main() {
    Box<int> box(10);
    std::cout << box.get() << std::endl;
    return 0;
}
```

---

## 5. 模板特化

### 5.1 全特化

```cpp
#include <iostream>

template <typename T>
struct TypeName {
    static const char* get() {
        return "unknown";
    }
};

template <>
struct TypeName<int> {
    static const char* get() {
        return "int";
    }
};
```

### 5.2 偏特化

```cpp
#include <iostream>

template <typename T>
struct IsPointer {
    enum { value = 0 };
};

template <typename T>
struct IsPointer<T*> {
    enum { value = 1 };
};
```

---

## 6. 模板参数推导

### 6.1 自动推导

```cpp
template <typename T>
void printValue(T value) {
}

int main() {
    printValue(42);
}
```

### 6.2 推导失败场景

- 重载冲突
- 类型不匹配
- 无法从上下文确定模板参数

---

## 7. `typename` 与 `class`

在模板参数列表中：

```cpp
template <typename T>
```

和

```cpp
template <class T>
```

基本等价。

但在依赖名场景中，`typename` 有额外含义：

```cpp
template <typename T>
void foo() {
    typename T::value_type x;
}
```

这里必须用 `typename` 告诉编译器：`T::value_type` 是一个类型。

---

## 8. 编译期常量与递归模板

### 8.1 传统模板元编程例子

```cpp
#include <iostream>

template <int N>
struct Factorial {
    enum { value = N * Factorial<N - 1>::value };
};

template <>
struct Factorial<0> {
    enum { value = 1 };
};

int main() {
    std::cout << Factorial<5>::value << std::endl;
    return 0;
}
```

说明：

- 这是传统模板元编程的代表写法
- 现代 C++ 通常更倾向 `constexpr`

### 8.2 `constexpr` 版本

```cpp
#include <iostream>

constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

int main() {
    std::cout << factorial(5) << std::endl;
    return 0;
}
```

---

## 9. `type_traits`

`<type_traits>` 是模板元编程非常高频的标准库头文件。

常见工具：

- `std::is_same`
- `std::is_integral`
- `std::is_pointer`
- `std::remove_reference`
- `std::remove_cv`
- `std::enable_if`

### 9.1 判断类型

```cpp
#include <iostream>
#include <type_traits>

int main() {
    std::cout << std::is_integral<int>::value << std::endl;
    std::cout << std::is_pointer<int*>::value << std::endl;
    return 0;
}
```

### 9.2 `if constexpr` 结合类型萃取

```cpp
#include <iostream>
#include <type_traits>

template <typename T>
void printCategory(const T&) {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "integral" << std::endl;
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "floating" << std::endl;
    } else {
        std::cout << "other" << std::endl;
    }
}
```

---

## 10. SFINAE

SFINAE 的核心意思是：

- 模板替换失败时，不把它当成编译错误
- 只是把该模板候选从重载集合中移除

### 10.1 `enable_if` 示例

```cpp
#include <iostream>
#include <type_traits>

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
safeAdd(T a, T b) {
    return a + b;
}

int main() {
    std::cout << safeAdd(1, 2) << std::endl;
    return 0;
}
```

### 10.2 分类型重载

```cpp
#include <iostream>
#include <type_traits>

template <typename T>
typename std::enable_if<std::is_integral<T>::value>::type
printType(const T&) {
    std::cout << "integral" << std::endl;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value>::type
printType(const T&) {
    std::cout << "floating" << std::endl;
}
```

---

## 11. `decltype`

### 11.1 推导表达式类型

```cpp
#include <iostream>

int main() {
    int x = 10;
    decltype(x) y = 20;
    std::cout << y << std::endl;
    return 0;
}
```

### 11.2 返回值推导

```cpp
template <typename T, typename U>
auto add(T a, U b) -> decltype(a + b) {
    return a + b;
}
```

---

## 12. 可变参数模板

### 12.1 递归展开

```cpp
#include <iostream>

void printAll() {
    std::cout << std::endl;
}

template <typename T, typename... Args>
void printAll(const T& first, const Args&... rest) {
    std::cout << first << ' ';
    printAll(rest...);
}

int main() {
    printAll(1, "hello", 3.14);
    return 0;
}
```

### 12.2 参数包大小

```cpp
template <typename... Args>
void checkCount(Args... args) {
    static_assert(sizeof...(args) > 0, "args must not be empty");
}
```

---

## 13. 折叠表达式

```cpp
#include <iostream>

template <typename... Args>
auto sum(Args... args) {
    return (... + args);
}

int main() {
    std::cout << sum(1, 2, 3, 4) << std::endl;
    return 0;
}
```

---

## 14. 完美转发与转发引用

这部分和元编程关系很深，通常一起出现。

```cpp
#include <iostream>
#include <utility>

void target(int& value) {
    std::cout << "lvalue: " << value << std::endl;
}

void target(int&& value) {
    std::cout << "rvalue: " << value << std::endl;
}

template <typename T>
void relay(T&& value) {
    target(std::forward<T>(value));
}

int main() {
    int x = 10;
    relay(x);
    relay(20);
    return 0;
}
```

说明：

- `T&&` 在模板里可能是转发引用
- `std::forward<T>` 保留值类别

---

## 15. Concepts

Concepts 是对 SFINAE 的现代替代和增强。

### 15.1 基础写法

```cpp
#include <concepts>

template <std::integral T>
T multiply(T a, T b) {
    return a * b;
}
```

### 15.2 自定义 Concept

```cpp
#include <concepts>

template <typename T>
concept Printable = requires(T a) {
    a.print();
};

template <Printable T>
void output(const T& value) {
    value.print();
}
```

---

## 16. 实战示例：只允许整数类型的包装器

```cpp
#include <iostream>
#include <type_traits>

template <typename T>
class IntOnlyBox {
    static_assert(std::is_integral<T>::value, "T must be integral");

public:
    explicit IntOnlyBox(T value) : value_(value) {}

    T get() const {
        return value_;
    }

private:
    T value_;
};

int main() {
    IntOnlyBox<int> box(100);
    std::cout << box.get() << std::endl;
    return 0;
}
```

---

## 17. 元编程最佳实践

1. 先用普通函数和模板
2. 确实需要编译期分发时再上元编程
3. 优先 `constexpr`、`if constexpr`、Concepts
4. 非必要不要把代码写成“只有编译器能看懂”
5. 把类型约束写清楚，错误信息才会好读

---

## 18. 推荐掌握顺序

1. 函数模板、类模板
2. 特化与偏特化
3. `type_traits`
4. `decltype` 与返回值推导
5. 可变参数模板
6. SFINAE 与 `enable_if`
7. 完美转发
8. Concepts

