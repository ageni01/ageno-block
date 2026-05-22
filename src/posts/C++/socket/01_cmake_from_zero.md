# 01. CMake 从 0 开始

## 1. CMake 是什么

写 C++ 程序时，最终要调用编译器，比如：

```bash
c++ src/01_hello_cmake.cpp -o 01_hello_cmake
```

但项目一大，就会有很多问题：

- 源文件很多，命令很长。
- 头文件目录要手动写。
- 不同系统编译器不同。
- Debug/Release 配置不同。
- 多个程序、多个库不好管理。

CMake 的作用是：你写 `CMakeLists.txt` 描述项目结构，CMake 帮你生成真正的构建文件。

## 2. 最小 CMakeLists.txt

看本项目的 [CMakeLists.txt](../CMakeLists.txt)。

最关键的是这几句：

```cmake
cmake_minimum_required(VERSION 3.16)
project(cmake_socket_from_zero LANGUAGES CXX)

add_executable(01_hello_cmake src/01_hello_cmake.cpp)
```

含义：

- `cmake_minimum_required`：要求最低 CMake 版本。
- `project`：定义项目名和语言。
- `add_executable`：生成一个可执行程序。

`add_executable(01_hello_cmake src/01_hello_cmake.cpp)` 的意思是：

- 目标名字叫 `01_hello_cmake`。
- 它由 `src/01_hello_cmake.cpp` 编译而来。
- 编译成功后会得到 `build/01_hello_cmake`。

## 3. 为什么要用 build 目录

推荐这样构建：

```bash
cmake -S . -B build
cmake --build build
```

含义：

- `-S .`：源码目录是当前目录。
- `-B build`：构建产物放到 `build/`。

这样源码和编译生成的文件不会混在一起。

## 4. 多个程序怎么写

本项目有多个可执行程序：

```cmake
add_executable(02_tcp_server src/02_tcp_server.cpp)
add_executable(02_tcp_client src/02_tcp_client.cpp)
```

每写一个 `add_executable`，就多生成一个程序。

## 5. include 目录怎么用

本项目有：

```text
include/common.hpp
```

所以 CMake 写了：

```cmake
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
```

意思是告诉编译器：以后 `#include "common.hpp"` 时，去 `include/` 目录找。

初学阶段先这样写就够用。项目变复杂后，可以学习更现代的：

```cmake
target_include_directories(目标名 PRIVATE include)
```

## 6. 你应该记住的 CMake 模板

初学时先背这个：

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(my_app src/main.cpp)
```

能跑起来后，再慢慢理解每一行。
