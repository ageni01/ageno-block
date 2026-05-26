---
cover: /assets/images/cover2.jpg
icon: pen-to-square
date: 2026-05-2
category:
  - C++
tag:
  - C++
star: true
sticky: true
---


# C++98 完整基础教程

## 1. 文档定位

本文件集中整理传统 C++98 阶段最核心、最常用的知识点，适合用于：

- 学习旧项目代码
- 理解现代 C++ 之前的设计方式
- 打牢类、对象、指针、模板、STL 的基础

说明：

- 本文以 `C++98` 为主
- 示例尽量保持旧标准可编译
- 某些现代写法不在本文件中使用

---

## 2. 第一个程序与编译方式

### 2.1 Hello World

```cpp
#include <iostream>

int main() {
    std::cout << "Hello, C++98" << std::endl;
    return 0;
}
```

### 2.2 编译命令

```bash
g++ main.cpp -std=c++98 -Wall -Wextra -o app
```

说明：

- `-std=c++98`：使用 C++98 标准
- `-Wall -Wextra`：开启常见警告
- `-o app`：输出可执行文件

---

## 3. 基本语法

### 3.1 变量与基本类型

常见内置类型：

- `char`
- `short`
- `int`
- `long`
- `float`
- `double`
- `bool`

示例：

```cpp
#include <iostream>

int main() {
    int age = 20;
    double score = 99.5;
    char grade = 'A';
    bool passed = true;

    std::cout << age << ", " << score << ", "
              << grade << ", " << passed << std::endl;
    return 0;
}
```

### 3.2 常量

```cpp
#include <iostream>

int main() {
    const int kMaxCount = 100;
    std::cout << kMaxCount << std::endl;
    return 0;
}
```

### 3.3 运算符

常见分类：

- 算术运算：`+ - * / %`
- 比较运算：`== != < > <= >=`
- 逻辑运算：`&& || !`
- 位运算：`& | ^ ~ << >>`

示例：

```cpp
#include <iostream>

int main() {
    int a = 6;
    int b = 3;
    std::cout << (a + b) << std::endl;
    std::cout << (a & b) << std::endl;
    std::cout << (a > b) << std::endl;
    return 0;
}
```

---

## 4. 控制流程

### 4.1 if / else

```cpp
#include <iostream>

int main() {
    int x = 10;
    if (x > 0) {
        std::cout << "positive" << std::endl;
    } else {
        std::cout << "non-positive" << std::endl;
    }
    return 0;
}
```

### 4.2 switch

```cpp
#include <iostream>

int main() {
    int cmd = 2;
    switch (cmd) {
        case 1:
            std::cout << "start" << std::endl;
            break;
        case 2:
            std::cout << "stop" << std::endl;
            break;
        default:
            std::cout << "unknown" << std::endl;
            break;
    }
    return 0;
}
```

### 4.3 循环

```cpp
#include <iostream>

int main() {
    for (int i = 0; i < 3; ++i) {
        std::cout << i << std::endl;
    }

    int n = 3;
    while (n > 0) {
        std::cout << n << std::endl;
        --n;
    }
    return 0;
}
```

---

## 5. 函数

### 5.1 函数定义

```cpp
#include <iostream>

int add(int a, int b) {
    return a + b;
}

int main() {
    std::cout << add(3, 4) << std::endl;
    return 0;
}
```

### 5.2 函数声明

```cpp
#include <iostream>

int multiply(int a, int b);

int main() {
    std::cout << multiply(2, 5) << std::endl;
    return 0;
}

int multiply(int a, int b) {
    return a * b;
}
```

### 5.3 引用传参

```cpp
#include <iostream>

void increment(int& value) {
    ++value;
}

int main() {
    int n = 10;
    increment(n);
    std::cout << n << std::endl;
    return 0;
}
```

### 5.4 `const` 引用

```cpp
#include <iostream>
#include <string>

void printName(const std::string& name) {
    std::cout << name << std::endl;
}

int main() {
    std::string name = "Alice";
    printName(name);
    return 0;
}
```

---

## 6. 数组、字符串、指针、引用

### 6.1 C 风格数组

```cpp
#include <iostream>

int main() {
    int nums[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; ++i) {
        std::cout << nums[i] << std::endl;
    }
    return 0;
}
```

### 6.2 `std::string`

```cpp
#include <iostream>
#include <string>

int main() {
    std::string text = "network_protocol";
    std::cout << text.size() << std::endl;
    std::cout << text.substr(0, 7) << std::endl;
    return 0;
}
```

### 6.3 指针

```cpp
#include <iostream>

int main() {
    int value = 42;
    int* p = &value;
    std::cout << *p << std::endl;
    *p = 100;
    std::cout << value << std::endl;
    return 0;
}
```

### 6.4 动态内存

```cpp
#include <iostream>

int main() {
    int* p = new int(10);
    std::cout << *p << std::endl;
    delete p;

    int* arr = new int[3];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    delete[] arr;
    return 0;
}
```

注意：

- `new` 对应 `delete`
- `new[]` 对应 `delete[]`
- 忘记释放会导致内存泄漏

### 6.5 引用

```cpp
#include <iostream>

int main() {
    int x = 5;
    int& ref = x;
    ref = 20;
    std::cout << x << std::endl;
    return 0;
}
```

---

## 7. 结构体与类

### 7.1 结构体

```cpp
#include <iostream>
#include <string>

struct Student {
    std::string name;
    int age;
};

int main() {
    Student s;
    s.name = "Tom";
    s.age = 18;
    std::cout << s.name << ", " << s.age << std::endl;
    return 0;
}
```

### 7.2 类

```cpp
#include <iostream>
#include <string>

class User {
public:
    User(const std::string& name, int age) : name_(name), age_(age) {}

    void print() const {
        std::cout << name_ << ", " << age_ << std::endl;
    }

private:
    std::string name_;
    int age_;
};

int main() {
    User user("Alice", 20);
    user.print();
    return 0;
}
```

### 7.3 构造函数与析构函数

```cpp
#include <iostream>

class Resource {
public:
    Resource() {
        std::cout << "construct" << std::endl;
    }

    ~Resource() {
        std::cout << "destruct" << std::endl;
    }
};

int main() {
    Resource r;
    return 0;
}
```

### 7.4 拷贝构造与赋值

```cpp
#include <iostream>
#include <string>

class Person {
public:
    Person(const std::string& name) : name_(name) {}

    Person(const Person& other) : name_(other.name_) {
        std::cout << "copy ctor" << std::endl;
    }

    Person& operator=(const Person& other) {
        if (this != &other) {
            name_ = other.name_;
        }
        return *this;
    }

private:
    std::string name_;
};
```

如果类管理资源，通常必须显式考虑：

- 析构函数
- 拷贝构造
- 拷贝赋值

这就是传统的“三法则”。

---

## 8. 面向对象

### 8.1 封装

通过访问控制隐藏实现细节。

```cpp
class Counter {
public:
    Counter() : value_(0) {}

    void increase() {
        ++value_;
    }

    int get() const {
        return value_;
    }

private:
    int value_;
};
```

### 8.2 继承

```cpp
#include <iostream>

class Animal {
public:
    void breathe() const {
        std::cout << "breathing" << std::endl;
    }
};

class Dog : public Animal {
public:
    void bark() const {
        std::cout << "wang" << std::endl;
    }
};
```

### 8.3 多态

```cpp
#include <iostream>

class Shape {
public:
    virtual ~Shape() {}
    virtual void draw() const = 0;
};

class Circle : public Shape {
public:
    virtual void draw() const {
        std::cout << "draw circle" << std::endl;
    }
};

void render(const Shape& shape) {
    shape.draw();
}
```

说明：

- 多态依赖虚函数
- 基类析构函数通常应为虚函数

### 8.4 纯虚函数与抽象类

```cpp
class Codec {
public:
    virtual ~Codec() {}
    virtual bool encode() = 0;
    virtual bool decode() = 0;
};
```

---

## 9. 运算符重载

```cpp
#include <iostream>

class Point {
public:
    Point(int x, int y) : x_(x), y_(y) {}

    Point operator+(const Point& other) const {
        return Point(x_ + other.x_, y_ + other.y_);
    }

    void print() const {
        std::cout << "(" << x_ << ", " << y_ << ")" << std::endl;
    }

private:
    int x_;
    int y_;
};
```

建议：

- 重载应符合直觉语义
- 不要为了“炫技”滥用重载

---

## 10. 模板

### 10.1 函数模板

```cpp
#include <iostream>

template <typename T>
T maxValue(T a, T b) {
    return a > b ? a : b;
}

int main() {
    std::cout << maxValue(3, 5) << std::endl;
    std::cout << maxValue(2.5, 4.8) << std::endl;
    return 0;
}
```

### 10.2 类模板

```cpp
#include <iostream>

template <typename T>
class Holder {
public:
    Holder(const T& value) : value_(value) {}

    T get() const {
        return value_;
    }

private:
    T value_;
};
```

### 10.3 模板特化

```cpp
#include <iostream>

template <typename T>
struct Printer {
    static void print(const T&) {
        std::cout << "generic" << std::endl;
    }
};

template <>
struct Printer<int> {
    static void print(const int& value) {
        std::cout << "int: " << value << std::endl;
    }
};
```

---

## 11. STL 容器

### 11.1 `vector`

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> nums;
    nums.push_back(10);
    nums.push_back(20);
    nums.push_back(30);

    for (std::size_t i = 0; i < nums.size(); ++i) {
        std::cout << nums[i] << std::endl;
    }
    return 0;
}
```

### 11.2 `list`

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> values;
    values.push_back(1);
    values.push_back(2);
    values.push_front(0);

    for (std::list<int>::iterator it = values.begin(); it != values.end(); ++it) {
        std::cout << *it << std::endl;
    }
    return 0;
}
```

### 11.3 `deque`

```cpp
#include <deque>
#include <iostream>

int main() {
    std::deque<int> q;
    q.push_back(1);
    q.push_front(0);
    std::cout << q.front() << ", " << q.back() << std::endl;
    return 0;
}
```

### 11.4 `map`

```cpp
#include <iostream>
#include <map>
#include <string>

int main() {
    std::map<std::string, int> ports;
    ports["http"] = 80;
    ports["https"] = 443;

    for (std::map<std::string, int>::iterator it = ports.begin();
         it != ports.end(); ++it) {
        std::cout << it->first << " = " << it->second << std::endl;
    }
    return 0;
}
```

### 11.5 `set`

```cpp
#include <iostream>
#include <set>

int main() {
    std::set<int> ids;
    ids.insert(3);
    ids.insert(1);
    ids.insert(3);

    for (std::set<int>::iterator it = ids.begin(); it != ids.end(); ++it) {
        std::cout << *it << std::endl;
    }
    return 0;
}
```

### 11.6 `stack` 与 `queue`

```cpp
#include <iostream>
#include <queue>
#include <stack>

int main() {
    std::stack<int> st;
    st.push(1);
    st.push(2);
    std::cout << st.top() << std::endl;

    std::queue<int> q;
    q.push(10);
    q.push(20);
    std::cout << q.front() << std::endl;
    return 0;
}
```

---

## 12. STL 算法与迭代器

### 12.1 常用算法

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::vector<int> nums;
    nums.push_back(4);
    nums.push_back(2);
    nums.push_back(5);
    nums.push_back(1);

    std::sort(nums.begin(), nums.end());
    for (std::size_t i = 0; i < nums.size(); ++i) {
        std::cout << nums[i] << std::endl;
    }
    return 0;
}
```

### 12.2 `find`

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::vector<int> nums;
    nums.push_back(7);
    nums.push_back(9);
    nums.push_back(11);

    std::vector<int>::iterator it = std::find(nums.begin(), nums.end(), 9);
    if (it != nums.end()) {
        std::cout << "found" << std::endl;
    }
    return 0;
}
```

### 12.3 迭代器失效注意点

重点：

- `vector` 扩容可能导致迭代器失效
- `erase` 后部分迭代器会失效
- 不同容器规则不同

---

## 13. 异常处理

### 13.1 try / catch / throw

```cpp
#include <iostream>
#include <stdexcept>

double divide(int a, int b) {
    if (b == 0) {
        throw std::runtime_error("divide by zero");
    }
    return static_cast<double>(a) / b;
}

int main() {
    try {
        std::cout << divide(10, 2) << std::endl;
        std::cout << divide(10, 0) << std::endl;
    } catch (const std::exception& ex) {
        std::cout << "error: " << ex.what() << std::endl;
    }
    return 0;
}
```

### 13.2 异常使用建议

- 正常业务分支不要滥用异常
- 构造失败、资源失败、前置条件破坏可使用异常
- 析构函数中避免抛异常

---

## 14. 预处理与宏

### 14.1 常量宏

```cpp
#define MAX_BUFFER_SIZE 1024
```

### 14.2 宏函数

```cpp
#define SQUARE(x) ((x) * (x))
```

风险示例：

```cpp
int i = 3;
int y = SQUARE(i++);
```

这里会有多次求值风险。

### 14.3 条件编译

```cpp
#include <iostream>

#ifdef _WIN32
    #define PLATFORM_NAME "Windows"
#else
    #define PLATFORM_NAME "POSIX"
#endif

int main() {
    std::cout << PLATFORM_NAME << std::endl;
    return 0;
}
```

### 14.4 include guard

```cpp
#ifndef USER_H
#define USER_H

class User {};

#endif
```

---

## 15. 常用头文件整理

### 输入输出

- `<iostream>`：标准输入输出
- `<iomanip>`：格式控制
- `<fstream>`：文件读写
- `<sstream>`：字符串流

### 字符串与工具

- `<string>`：`std::string`
- `<cstring>`：C 风格字符串函数
- `<cstdlib>`：转换、内存、工具函数
- `<cstdio>`：C 风格 IO

### 容器与算法

- `<vector>`
- `<list>`
- `<deque>`
- `<map>`
- `<set>`
- `<queue>`
- `<stack>`
- `<algorithm>`
- `<functional>`
- `<numeric>`

### 异常与时间

- `<stdexcept>`
- `<exception>`
- `<ctime>`

---

## 16. 开发规范

### 16.1 命名规范

建议统一：

- 类名：`PascalCase`
- 函数名：`camelCase`
- 成员变量：`name_`
- 常量：`kMaxLength`
- 宏：`MAX_LENGTH`

### 16.2 头文件规范

- 一个头文件负责一个清晰职责
- 使用 include guard
- 头文件中避免 `using namespace std;`

### 16.3 指针使用规范

- 优先对象语义
- 谨慎使用裸指针
- 动态分配后明确谁负责释放

### 16.4 注释规范

建议说明：

- 接口用途
- 输入输出约束
- 边界条件
- 异常行为

---

## 17. 典型综合示例：简单学生管理

```cpp
#include <iostream>
#include <string>
#include <vector>

class Student {
public:
    Student(const std::string& name, int score)
        : name_(name), score_(score) {}

    void print() const {
        std::cout << name_ << ": " << score_ << std::endl;
    }

private:
    std::string name_;
    int score_;
};

int main() {
    std::vector<Student> students;
    students.push_back(Student("Alice", 95));
    students.push_back(Student("Bob", 88));

    for (std::size_t i = 0; i < students.size(); ++i) {
        students[i].print();
    }
    return 0;
}
```

这个例子用到了：

- 类
- 构造函数
- `vector`
- `string`
- 循环遍历

---

## 18. C++98 学习重点总结

必须掌握：

1. 变量、流程控制、函数
2. 指针、引用、动态内存
3. 类、构造、析构、拷贝控制
4. 继承、多态、虚函数
5. STL 容器与算法
6. 异常、宏、头文件组织

在阅读老项目时，你最常遇到的就是：

- 裸指针
- 手动内存管理
- 头文件层层依赖
- C 风格数组与字符串
- 传统虚函数接口设计

