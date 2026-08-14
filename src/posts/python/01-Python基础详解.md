# Python 基础详解

> 本文档面向 Python 初学者和进阶者，系统性地讲解 Python 基础语法、常用数据结构与内置库。所有示例均配有详细注释。

---

## 目录

1. [基础语法](#1-基础语法)
2. [常用数据结构](#2-常用数据结构)
3. [流程控制](#3-流程控制)
4. [函数详解](#4-函数详解)
5. [常用内置库](#5-常用内置库)
6. [综合示例](#6-综合示例)

---

## 1. 基础语法

### 1.1 变量与基本类型

Python 是**动态类型**语言，变量无需声明类型即可使用。

```python
# ---------- 数值类型 ----------
# int: 整数（任意精度，不会溢出）
a = 42          # 十进制整数
b = 0b1010      # 二进制，以 0b 开头 → 10
c = 0o77        # 八进制，以 0o 开头 → 63
d = 0xFF        # 十六进制，以 0x 开头 → 255
e = 1_000_000   # 数字中可以加下划线提高可读性

# float: 浮点数（双精度，遵循 IEEE 754）
f = 3.14159
g = 1.23e-4     # 科学记数法 → 0.000123

# complex: 复数
z = 3 + 4j      # j 表示虚数单位
print(z.real, z.imag)  # 获取实部和虚部 → 3.0 4.0

# ---------- 布尔类型 ----------
# bool 是 int 的子类：True == 1, False == 0
is_valid = True
is_empty = False
print(True + True)   # → 2（因为 True 就是 1）

# ---------- 字符串 ----------
# str: 不可变的 Unicode 字符序列
s1 = '单引号'
s2 = "双引号"
s3 = '''三引号支持
换行字符串'''
s4 = r'原始字符串\n不会转义'  # raw string，\n 就是两个字符

# f-string（Python 3.6+）：最推荐的格式化方式
name = "Python"
version = 3.12
msg = f"{name} 版本是 {version}，明年是 {version + 1}"
print(msg)  # → Python 版本是 3.12，明年是 4.12

# ---------- None 类型 ----------
# None 表示"没有值"或"空"，类似于其他语言的 null
result = None
if result is None:      # 判断 None 必须用 is，不要用 ==
    print("没有结果")
```

### 1.2 类型转换

```python
# 显式类型转换（铸造）
x = 3.14159
print(int(x))           # → 3（直接截断，不四舍五入）
print(round(x))         # → 3（四舍五入取整）
print(round(x, 2))      # → 3.14（保留两位小数）

print(float("3.14"))    # → 3.14（字符串 → 浮点数）
print(str(42))          # → "42"（任意类型 → 字符串）
print(bool(0))          # → False（0、空字符串、空容器 → False）
print(bool(42))         # → True（非零数值、非空容器 → True）

# ---------- 类型判断 ----------
print(type(42))         # → <class 'int'>
print(isinstance(42, int))       # → True
print(isinstance(42, (int, float)))  # → True（只要属于元组中的任一类型）
```

### 1.3 运算符

```python
# ---------- 算术运算符 ----------
a, b = 10, 3
print(a + b)   # → 13  加
print(a - b)   # → 7   减
print(a * b)   # → 30  乘
print(a / b)   # → 3.3333333333333335  真除法（总是返回 float）
print(a // b)  # → 3   整除（向下取整）
print(a % b)   # → 1   取余
print(a ** b)  # → 1000  幂运算

# ---------- 比较运算符 ----------
# 链式比较是 Python 的特色语法
x = 5
print(3 < x < 10)       # → True，等价于 3 < x and x < 10
print(3 < x < 4)        # → False
print("ABC" == "abc")   # → False（区分大小写）

# ---------- 逻辑运算符 ----------
# and / or / not（注意是英文单词，不是 && / || / !）
print(True and False)   # → False
print(True or False)    # → True
print(not True)         # → False

# 短路求值：and 遇到 False 立即停止，or 遇到 True 立即停止
def side_effect():
    print("函数被调用了！")
    return True

# False and side_effect()  → side_effect() 不会被调用（短路）
result = False and side_effect()
print("结果是:", result)  # → 结果是: False

# ---------- 身份运算符 ----------
# is / is not：判断两个变量是否引用同一个对象
a = [1, 2, 3]
b = [1, 2, 3]
c = a
print(a == b)   # → True（值相等）
print(a is b)   # → False（不同对象）
print(a is c)   # → True（同一对象）

# 小整数缓存：Python 会缓存 -5~256 的整数
x = 256
y = 256
print(x is y)   # → True（在缓存范围内）

x = 257
y = 257
print(x is y)   # → False（超出缓存范围，但是交互式环境/文件级可能不同）

# ---------- 成员运算符 ----------
# in / not in：检查元素是否在容器中
print(3 in [1, 2, 3])      # → True
print("he" in "hello")     # → True
print("x" not in "hello")  # → True

# ---------- 位运算符 ----------
# &（与）、|（或）、^（异或）、~（取反）、<<（左移）、>>（右移）
a, b = 0b1100, 0b1010
print(bin(a & b))   # → 0b1000
print(bin(a | b))   # → 0b1110
print(bin(a ^ b))   # → 0b0110
print(bin(~a))      # → -0b1101（补码表示）
print(bin(a << 1))  # → 0b11000（左移一位，等价于乘以 2）
print(bin(a >> 1))  # → 0b0110（右移一位，等价于整除 2）
```

---

## 2. 常用数据结构

### 2.1 列表（list）

> 有序、可变、可重复的容器。Python 中最常用的数据结构，底层是动态数组。

```python
# ---------- 创建 ----------
empty = []                     # 空列表
nums = [1, 2, 3, 4, 5]
mixed = [1, "hello", 3.14, True]  # 可以混合不同类型
nested = [[1, 2], [3, 4]]     # 嵌套列表（二维数组）

# 列表推导式（list comprehension）：创建列表的优雅方式
squares = [x ** 2 for x in range(10)]            # [0, 1, 4, 9, ..., 81]
evens = [x for x in range(20) if x % 2 == 0]     # [0, 2, 4, ..., 18]
matrix = [[x * y for x in range(3)] for y in range(3)]
# → [[0, 0, 0], [0, 1, 2], [0, 2, 4]]

# ---------- 索引与切片 ----------
# 索引从 0 开始，负索引从末尾开始
nums = [10, 20, 30, 40, 50]
print(nums[0])    # → 10
print(nums[-1])   # → 50（最后一个元素）
print(nums[-2])   # → 40（倒数第二个元素）

# 切片 [start:stop:step]（左闭右开）
print(nums[1:3])      # → [20, 30]（索引 1 到 2）
print(nums[:3])       # → [10, 20, 30]（从头到索引 2）
print(nums[2:])       # → [30, 40, 50]（从索引 2 到尾）
print(nums[::-1])     # → [50, 40, 30, 20, 10]（反转）
print(nums[::2])      # → [10, 30, 50]（步长为 2）

# ---------- 常用方法 ----------
nums = [3, 1, 4, 1, 5, 9]

nums.append(2)          # 末尾追加 → [3, 1, 4, 1, 5, 9, 2]
nums.insert(0, 99)      # 在指定位置插入 → [99, 3, 1, 4, 1, 5, 9, 2]
nums.extend([7, 8])     # 合并另一个列表 → [99, 3, 1, 4, 1, 5, 9, 2, 7, 8]

nums.pop()              # 移除并返回末尾元素 → 8
nums.pop(0)             # 移除并返回指定索引元素 → 99
nums.remove(1)          # 移除第一个匹配的元素（不返回）
del nums[0]             # 删除指定索引（不返回）

# 查找
nums = [3, 1, 4, 1, 5]
print(nums.index(4))    # → 2（第一个出现 4 的索引）
print(nums.count(1))    # → 2（1 出现的次数）

# 排序
nums.sort()             # 原地升序排序
nums.sort(reverse=True) # 原地降序排序
sorted_nums = sorted(nums)  # 返回新列表，原列表不变

# ---------- 列表作为栈和队列 ----------
# 栈（LIFO）：append() + pop()
stack = []
stack.append(1)
stack.append(2)
stack.append(3)
print(stack.pop())      # → 3

# 队列（FIFO）：使用 collections.deque（高效双向队列）
from collections import deque
queue = deque(["A", "B", "C"])
queue.append("D")           # 右端入队
queue.appendleft("Z")       # 左端入队
print(queue.popleft())      # → Z（左端出队）
print(queue.pop())          # → D（右端出队）
```

### 2.2 元组（tuple）

> 有序、**不可变**的序列。创建后不能修改，可用作字典的键。

```python
# ---------- 创建 ----------
empty = ()                  # 空元组
single = (42,)              # 单元素元组，**逗号不能省略**，否则就是 int
single2 = 42,               # 也可以省略括号
nested = (1, (2, 3), 4)     # 嵌套元组

# 元组拆包（packing / unpacking）
point = (3, 4)              # 打包
x, y = point                # 解包 → x=3, y=4
print(x, y)

# 交换变量（元组解包的经典用法）
a, b = 10, 20
a, b = b, a                 # 一步交换
print(a, b)                 # → 20 10

# 星号解包（Python 3+）
first, *rest, last = [1, 2, 3, 4, 5]
print(first)    # → 1
print(rest)     # → [2, 3, 4]
print(last)     # → 5

# ---------- 不可变性说明 ----------
# 元组本身不可变，但如果元组包含可变对象，该对象内部可以改变
t = ([1, 2], 3)
t[0].append(99)             # 可以修改列表的内容
print(t)                    # → ([1, 2, 99], 3)
# t[1] = 4                  # ❌ 错误！不能修改元组元素

# ---------- 命名元组 ----------
from collections import namedtuple

# 创建一个 "Point" 类型，包含 x, y 两个字段
Point = namedtuple("Point", ["x", "y"])
p = Point(10, 20)
print(p.x, p.y)             # → 10 20（通过属性名访问）
print(p[0], p[1])           # → 10 20（也支持索引访问）
x, y = p                    # 也支持解包
print(p._asdict())          # → {'x': 10, 'y': 20}（转为字典）
```

### 2.3 字典（dict）

> 无序（Python 3.7+ 变成有序）、可变、键值对容器。底层是哈希表，**键必须是不可变类型**。

```python
# ---------- 创建 ----------
empty = {}                  # 空字典
d = {"name": "Alice", "age": 25}

# 使用 dict() 构造函数
d = dict(name="Bob", age=30)            # 键自动转为字符串
d = dict([("a", 1), ("b", 2)])          # 从键值对列表创建
d = dict(zip(["x", "y"], [10, 20]))     # 从两个序列压缩

# 字典推导式
squares = {x: x**2 for x in range(5)}  # {0: 0, 1: 1, 2: 4, 3: 9, 4: 16}

# ---------- 基本操作 ----------
d = {"name": "Alice", "age": 25, "city": "Beijing"}

# 访问
print(d["name"])              # → Alice
print(d.get("name"))          # → Alice（推荐，键不存在返回 None）
print(d.get("salary", 0))     # → 0（键不存在时返回默认值）

# 修改
d["age"] = 26                 # 更新已有键
d["salary"] = 10000           # 添加新键值对

# 安全访问：setdefault() 和 defaultdict
# setdefault：键存在则返回值，不存在则设置默认值
d.setdefault("phone", "N/A")  # 因为 phone 不存在，现在 d["phone"] = "N/A"

from collections import defaultdict
# defaultdict：访问不存在的键时自动生成默认值
dd = defaultdict(list)        # 默认值是空列表
dd["students"].append("Alice")  # 不需要先初始化列表
print(dd["students"])         # → ['Alice']

# ---------- 删除 ----------
del d["phone"]                # 删除指定键
d.pop("age")                  # 删除并返回值
d.popitem()                   # 删除并返回最后插入的键值对（Python 3.7+）
d.clear()                     # 清空所有内容

# ---------- 遍历 ----------
d = {"A": 1, "B": 2, "C": 3}

# 遍历键
for key in d:
    print(key, end=" ")       # → A B C

# 遍历值
for value in d.values():
    print(value, end=" ")     # → 1 2 3

# 遍历键值对
for key, value in d.items():
    print(f"{key}: {value}")

# ---------- 合并字典 ----------
d1 = {"a": 1, "b": 2}
d2 = {"b": 3, "c": 4}

merged = {**d1, **d2}                    # 解包合并（Python 3.5+）
merged = d1 | d2                         # 管道合并（Python 3.9+）
d1.update(d2)                            # 原地合并，d1 被修改

# ---------- Counter 计数器 ----------
from collections import Counter

# Counter 是 dict 的子类，用于计数
text = "abracadabra"
counter = Counter(text)
print(counter)               # → Counter({'a': 5, 'b': 2, 'r': 2, 'c': 1, 'd': 1})
print(counter.most_common(3))  # → [('a', 5), ('b', 2), ('r', 2)]
```

### 2.4 集合（set）

> 无序、**可变**、元素不重复的容器。底层也是哈希表，适合去重和集合运算。

```python
# ---------- 创建 ----------
empty = set()                    # 空集合（{} 是空字典！）
nums = {1, 2, 3, 1, 2}          # → {1, 2, 3}（重复自动去除）
chars = set("hello")            # → {'h', 'e', 'l', 'o'}

# 集合推导式
even_set = {x for x in range(10) if x % 2 == 0}  # {0, 2, 4, 6, 8}

# ---------- 基本操作 ----------
s = {1, 2, 3, 4, 5}

s.add(6)                        # 添加元素
s.update([7, 8, 9])             # 批量添加（可以传入任何可迭代对象）
s.remove(9)                     # 删除元素，不存在则抛出 KeyError
s.discard(99)                   # 安全删除，元素不存在不会报错
s.pop()                         # 删除并返回一个任意元素

# ---------- 集合运算 ----------
a = {1, 2, 3, 4, 5}
b = {4, 5, 6, 7, 8}

print(a | b)      # 并集 → {1, 2, 3, 4, 5, 6, 7, 8}
print(a & b)      # 交集 → {4, 5}
print(a - b)      # 差集（在 a 不在 b）→ {1, 2, 3}
print(b - a)      # 差集（在 b 不在 a）→ {6, 7, 8}
print(a ^ b)      # 对称差集（去掉交集部分）→ {1, 2, 3, 6, 7, 8}

# 子集 / 超集判断
print({1, 2}.issubset(a))    # → True（{1,2} 是 a 的子集）
print(a.issuperset({1, 2}))  # → True（a 是 {1,2} 的超集）
print({1, 2}.isdisjoint({3, 4}))  # → True（没有交集）

# ---------- 不可变集合 frozenset ----------
# 可以作为字典的键或其他集合的元素
fs = frozenset([1, 2, 3])
d = {fs: "不可变集合作为键"}
```

### 2.5 字符串（str）

> **不可变**的 Unicode 字符序列。每次"修改"都会创建新字符串。

```python
# ---------- 创建与拼接 ----------
s = "Hello Python"

# 拼接
print("Hello" + " " + "World")    # → Hello World
print("Hello", "World")           # → Hello World（print 的多个参数，逗号变空格）
print("-".join(["a", "b", "c"]))  # → a-b-c（推荐的拼接方式）

# 重复
print("Ha" * 3)                   # → HaHaHa

# ---------- 常用方法 ----------
text = "  Hello, Python World!  "

# 去除空白
print(text.strip())               # → "Hello, Python World!"（首尾去空白）
print(text.lstrip())              # → "Hello, Python World!  "（左去空白）
print(text.rstrip())              # → "  Hello, Python World!"（右去空白）

# 分割与合并
csv = "a,b,c,d"
print(csv.split(","))             # → ['a', 'b', 'c', 'd']
print("one\ntwo\nthree".splitlines())  # → ['one', 'two', 'three']

# 查找与替换
s = "hello world hello"
print(s.find("world"))            # → 6（首次出现索引，找不到返回 -1）
print(s.index("world"))           # → 6（找不到抛出 ValueError）
print(s.count("hello"))           # → 2
print(s.replace("hello", "hi"))   # → "hi world hi"（替换所有）

# 大小写
print("hello".upper())            # → HELLO
print("HELLO".lower())            # → hello
print("hello world".title())      # → Hello World（每个单词首字母大写）
print("hello world".capitalize()) # → Hello world（首字母大写）

# 判断
print("123".isdigit())            # → True（是否全是数字）
print("abc".isalpha())            # → True（是否全是字母）
print("abc123".isalnum())         # → True（是否全是字母或数字）
print("   ".isspace())            # → True（是否全是空白字符）
print("Hello".startswith("He"))   # → True
print("Hello".endswith("lo"))     # → True

# ---------- 格式化 ----------
name, age = "Alice", 25

# % 格式化（老式）
print("Name: %s, Age: %d" % (name, age))

# str.format()（Python 2.6+）
print("Name: {}, Age: {}".format(name, age))
print("Name: {0}, Age: {1}, Again: {0}".format(name, age))  # 使用索引
print("Name: {n}, Age: {a}".format(n=name, a=age))          # 使用名称

# f-string（Python 3.6+，最推荐）
print(f"Name: {name}, Age: {age}")
print(f"圆周率: {3.14159:.2f}")       # → 3.14（格式说明符）
print(f"对齐: {'hello':>10}")         # → "     hello"（右对齐 10 位）
print(f"对齐: {'hello':^10}")         # → "  hello   "（居中对齐 10 位）
print(f"二进制: {42:b}")              # → 101010
print(f"十六进制: {255:x}")           # → ff
print(f"百分比: {0.1234:.2%}")        # → 12.34%
```

### 2.6 数据结构的性能对比

| 数据结构 | 操作 | 平均时间复杂度 | 说明 |
|---------|------|---------------|------|
| list | 索引/赋值 | O(1) | 底层是动态数组 |
| list | append/pop | O(1) | 末尾操作 |
| list | 插入/删除 | O(n) | 需要移动元素 |
| list | 查找 in | O(n) | 线性搜索 |
| set/dict | 查找/插入/删除 | O(1)** | 哈希表，最坏 O(n) |
| tuple | 索引 | O(1) | 不可变 |
| str | 索引 | O(1) | 不可变 |

---

## 3. 流程控制

### 3.1 if-elif-else 条件判断

```python
score = 85

# Python 没有 switch/case，使用 if/elif/else
if score >= 90:
    grade = "A"
elif score >= 80:
    grade = "B"
elif score >= 70:
    grade = "C"
elif score >= 60:
    grade = "D"
else:
    grade = "E"

print(f"成绩等级: {grade}")

# ---------- 三元表达式（条件表达式） ----------
age = 20
status = "成年" if age >= 18 else "未成年"
print(status)  # → "成年"

# ---------- 真值判断（Pythonic 风格） ----------
# 推荐写法：直接判断对象本身
items = []

# ✅ 推荐的 Pythonic 写法
if items:
    print("列表非空")

# ❌ 不推荐
if len(items) > 0:
    print("列表非空")

# 哪些是 False？→ False, None, 0, 0.0, "", [], (), {}, set(), range(0)
```

### 3.2 for 循环

```python
# ---------- 遍历可迭代对象 ----------
for i in range(5):              # range(5) → 0,1,2,3,4
    print(i, end=" ")           # → 0 1 2 3 4

for i in range(2, 10, 3):      # range(start, stop, step)
    print(i, end=" ")           # → 2 5 8

# 遍历序列时同时获取索引
fruits = ["apple", "banana", "cherry"]
for index, fruit in enumerate(fruits):
    print(f"{index}: {fruit}")
# 输出:
# 0: apple
# 1: banana
# 2: cherry

# 遍历两个序列时配对
names = ["Alice", "Bob", "Charlie"]
scores = [85, 92, 78]
for name, score in zip(names, scores):
    print(f"{name}: {score}")

# 反向遍历
for i in reversed(range(5)):
    print(i, end=" ")           # → 4 3 2 1 0

# ---------- break / continue / else ----------
# for...else：循环正常结束（没有 break）时执行 else 块
for n in range(2, 10):
    for divisor in range(2, int(n**0.5) + 1):
        if n % divisor == 0:
            print(f"{n} = {divisor} × {n // divisor}")
            break  # 找到因子，跳出内层循环
    else:
        # 如果没有被 break，说明是质数
        print(f"{n} 是质数")
```

### 3.3 while 循环

```python
# while 循环用于不确定次数的循环
import random

# 猜数字游戏
target = random.randint(1, 100)
guess = None
attempts = 0

while guess != target:
    guess = int(input("猜一个 1-100 的数字: "))
    attempts += 1
    if guess < target:
        print("太小了！")
    elif guess > target:
        print("太大了！")

print(f"恭喜！猜了 {attempts} 次猜中了！")

# 无限循环 + break
while True:
    cmd = input("输入命令 (quit 退出): ")
    if cmd == "quit":
        break
    print(f"执行命令: {cmd}")
```

---

## 4. 函数详解

### 4.1 函数定义与参数

```python
# ---------- 基本函数 ----------
def greet(name):
    """向指定的人打招呼。"""       # 文档字符串（docstring）
    return f"Hello, {name}!"

print(greet("Python"))            # → Hello, Python!

# ---------- 参数类型 ----------
def func(a, b, c=10, *args, d=20, **kwargs):
    """
    演示所有参数类型：
    a       — 位置参数（必填）
    b       — 位置参数（必填）
    c       — 默认参数（可选，默认 10）
    *args   — 可变位置参数（接收额外的位置参数，打包为元组）
    d       — 命名关键字参数（必须通过关键字传参）
    **kwargs — 可变关键字参数（接收额外的关键字参数，打包为字典）
    """
    print(f"a={a}, b={b}, c={c}")
    print(f"args={args}")       # 额外的位置参数
    print(f"d={d}")
    print(f"kwargs={kwargs}")   # 额外的关键字参数

func(1, 2, 3, 4, 5, d=6, e=7, f=8)
# 输出:
# a=1, b=2, c=3
# args=(4, 5)
# d=6
# kwargs={'e': 7, 'f': 8}

# ---------- 参数传递机制 ----------
# Python 的参数传递是"对象引用传递"
# 不可变对象（int, str, tuple）：函数内修改不影响外部
# 可变对象（list, dict, set）：函数内修改会影响外部

def modify_list(lst):
    lst.append(99)             # 会改变原列表
    lst = [100, 200]           # 这创建了新对象，不影响外部

data = [1, 2, 3]
modify_list(data)
print(data)                    # → [1, 2, 3, 99]

# ---------- 默认参数的陷阱 ----------
# ❌ 危险的默认参数（可变对象作为默认值）
def add_student(name, students=[]):
    """多次调用会累积学生列表！"""
    students.append(name)
    return students

print(add_student("Alice"))   # → ['Alice']
print(add_student("Bob"))     # → ['Alice', 'Bob']  ← 意外！


# ✅ 正确的做法
def add_student(name, students=None):
    """使用 None 作为默认值，函数内再创建新列表"""
    if students is None:
        students = []
    students.append(name)
    return students
```

### 4.2 匿名函数（lambda）

```python
# lambda 用于定义简单的单行函数
add = lambda x, y: x + y
print(add(3, 4))              # → 7

# 常用场景：排序时的 key 函数
students = [
    {"name": "Alice", "grade": 88},
    {"name": "Bob", "grade": 95},
    {"name": "Charlie", "grade": 72},
]

# 按成绩排序
students.sort(key=lambda s: s["grade"], reverse=True)
print(students)
# → [{'name': 'Bob', 'grade': 95}, {'name': 'Alice', 'grade': 88}, ...]

# 与 map / filter / reduce 配合
nums = [1, 2, 3, 4, 5]
doubled = list(map(lambda x: x * 2, nums))      # [2, 4, 6, 8, 10]
evens = list(filter(lambda x: x % 2 == 0, nums)) # [2, 4]

# 注意：列表推导式通常比 map/filter 更 Pythonic
doubled2 = [x * 2 for x in nums]                # 更推荐
```

### 4.3 装饰器（decorator）

```python
from functools import wraps
import time

# ---------- 最简单的装饰器 ----------
def my_decorator(func):
    """在函数执行前后打印日志"""
    def wrapper(*args, **kwargs):
        print("函数开始执行...")
        result = func(*args, **kwargs)
        print("函数执行结束！")
        return result
    return wrapper

@my_decorator
def say_hello(name):
    print(f"Hello, {name}!")

say_hello("Python")  # 会额外打印开始/结束消息

# ---------- 带参数的装饰器 ----------
def timer(unit="s"):
    """计时装饰器，可以指定时间单位"""
    def decorator(func):
        @wraps(func)          # 保留原函数的元信息（函数名、文档等）
        def wrapper(*args, **kwargs):
            start = time.time()
            result = func(*args, **kwargs)
            elapsed = time.time() - start
            print(f"{func.__name__} 耗时 {elapsed:.4f}{unit}")
            return result
        return wrapper
    return decorator

@timer(unit="秒")
def slow_function():
    time.sleep(0.5)
    return "完成"

slow_function()

# ---------- 多个装饰器叠加 ----------
def bold(func):
    def wrapper():
        return f"<b>{func()}</b>"
    return wrapper

def italic(func):
    def wrapper():
        return f"<i>{func()}</i>"
    return wrapper

# 装饰器从下往上应用，从上往下执行
@bold
@italic
def hello():
    return "Hello"

print(hello())  # → <b><i>Hello</i></b>
```

### 4.4 生成器（generator）

```python
# ---------- yield 关键字 ----------
def countdown(n):
    """生成器函数：从 n 倒数到 0"""
    while n >= 0:
        yield n
        n -= 1

# 生成器不会一次性生成所有值，而是惰性求值
for num in countdown(5):
    print(num, end=" ")       # → 5 4 3 2 1 0

# ---------- 生成器表达式 ----------
# 类似列表推导式，但使用 () 而不是 []
squares_gen = (x ** 2 for x in range(10))
print(type(squares_gen))      # → <class 'generator'>
print(sum(squares_gen))       # → 285（一次性使用，求完就耗尽）

# ---------- 生成器的优势 ----------
# 节省内存：处理大文件或无限序列时优势明显
def read_large_file(file_path):
    """逐行读取大文件，一次只加载一行到内存"""
    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            yield line.strip()

# ---------- yield from ----------
def flatten(nested):
    """展平嵌套的可迭代对象"""
    for item in nested:
        if hasattr(item, "__iter__") and not isinstance(item, str):
            # 如果 item 是可迭代的且不是字符串，递归展开
            yield from flatten(item)
        else:
            yield item

nested = [1, [2, [3, 4], 5], 6]
print(list(flatten(nested)))  # → [1, 2, 3, 4, 5, 6]
```

---

## 5. 常用内置库

### 5.1 math — 数学函数

```python
import math

# 常量
print(math.pi)       # π: 3.141592653589793
print(math.e)        # 自然常数: 2.718281828459045
print(math.inf)      # 正无穷大
print(math.nan)      # NaN（Not a Number）

# 数值运算
print(math.sqrt(16))      # → 4.0（平方根）
print(math.pow(2, 10))    # → 1024.0（幂运算，返回 float）
print(math.factorial(5))  # → 120（阶乘）
print(math.gcd(12, 18))   # → 6（最大公约数）

# 对数与指数
print(math.log(100, 10))  # → 2.0（以 10 为底的对数）
print(math.log(2.71828))  # → 1.0（自然对数）
print(math.exp(1))        # → e^1 ≈ 2.71828

# 三角函数
print(math.sin(math.pi/2))  # → 1.0
print(math.cos(0))          # → 1.0
print(math.degrees(math.pi)) # → 180.0（弧度转角度）
print(math.radians(180))     # → π（角度转弧度）

# 取整
print(math.ceil(3.14))    # → 4（向上取整）
print(math.floor(3.14))   # → 3（向下取整）
print(math.trunc(3.14))   # → 3（截断取整，等价于 int()）
```

### 5.2 random — 随机数

```python
import random

# 基础随机数
print(random.random())            # [0.0, 1.0) 之间的浮点数
print(random.uniform(10, 20))     # [10, 20] 之间的浮点数

# 整数随机数
print(random.randint(1, 6))       # [1, 6] 之间的整数（包含两端）
print(random.randrange(0, 10, 2)) # 从 range(0,10,2) 中随机选一个

# 序列操作
items = ["A", "B", "C", "D", "E"]
print(random.choice(items))       # 随机选一个元素
print(random.choices(items, k=3)) # 有放回地选 k 个
print(random.sample(items, k=2))  # 无放回地选 k 个（不会重复）
random.shuffle(items)             # 原地洗牌
print(items)

# 设置种子（可重现结果）
random.seed(42)
print(random.randint(1, 100))     # 每次运行结果相同
```

### 5.3 datetime — 日期时间

```python
from datetime import datetime, date, time, timedelta

# ---------- 获取当前时间 ----------
now = datetime.now()              # 当前本地时间
utc_now = datetime.utcnow()       # 当前 UTC 时间
today = date.today()              # 当前日期

print(f"现在: {now}")
print(f"日期: {today}")

# ---------- 创建特定日期 ----------
d = datetime(2024, 3, 15, 14, 30, 0)
print(d)                          # → 2024-03-15 14:30:00

# ---------- 格式化 ----------
# strftime：datetime → 字符串
formatted = now.strftime("%Y-%m-%d %H:%M:%S")
print(formatted)                  # → 2024-03-15 14:30:00

# strptime：字符串 → datetime
parsed = datetime.strptime("2024-03-15 14:30:00", "%Y-%m-%d %H:%M:%S")
print(parsed)

# ---------- 时间运算 ----------
# 使用 timedelta 进行日期加减
today = date.today()
one_week_later = today + timedelta(weeks=1)
one_month_ago = today - timedelta(days=30)
print(f"一周后: {one_week_later}")

# 两个日期之差
diff = one_week_later - today
print(diff.days)                  # → 7

# ---------- 时区处理 ----------
from datetime import timezone, timedelta

# 创建带时区的时间
utc_8 = timezone(timedelta(hours=8))  # UTC+8
dt_cn = datetime.now(utc_8)
print(f"北京时间: {dt_cn}")

# ---------- 常用格式字符串 ----------
"""
%Y — 四位数年份（2024）
%y — 两位数年份（24）
%m — 月份（01-12）
%d — 日期（01-31）
%H — 小时（00-23）
%I — 小时（01-12）
%M — 分钟（00-59）
%S — 秒（00-59）
%f — 微秒（000000-999999）
%p — AM 或 PM
%a — 简写星期（Mon）
%A — 全写星期（Monday）
%b — 简写月份（Jan）
%B — 全写月份（January）
"""
```

### 5.4 json — JSON 处理

```python
import json

# ---------- Python ↔ JSON 类型映射 ----------
"""
Python          JSON
dict            object
list, tuple     array
str             string
int, float      number
True            true
False           false
None            null
"""

# ---------- 序列化（Python → JSON） ----------
data = {
    "name": "Alice",
    "age": 25,
    "is_student": False,
    "courses": ["Python", "Java"],
    "address": None,
    "scores": {"math": 95, "english": 88}
}

# 转换为 JSON 字符串
json_str = json.dumps(data, ensure_ascii=False, indent=2)
print(json_str)

# 写入文件
with open("data.json", "w", encoding="utf-8") as f:
    json.dump(data, f, ensure_ascii=False, indent=2)

# ---------- 反序列化（JSON → Python） ----------
# 从字符串解析
parsed = json.loads(json_str)
print(parsed["name"])           # → Alice

# 从文件读取
with open("data.json", "r", encoding="utf-8") as f:
    loaded = json.load(f)

# ---------- 自定义序列化 ----------
from datetime import datetime

class CustomEncoder(json.JSONEncoder):
    """处理 datetime 等特殊类型"""
    def default(self, obj):
        if isinstance(obj, datetime):
            return obj.isoformat()
        return super().default(obj)

data_with_date = {
    "event": "Meeting",
    "time": datetime.now()
}

result = json.dumps(data_with_date, cls=CustomEncoder)
print(result)
```

### 5.5 re — 正则表达式

```python
import re

# ---------- 基本函数 ----------
text = "我的邮箱是 alice@example.com，备用邮箱是 bob@test.org"

# search：搜索第一个匹配
match = re.search(r"\w+@\w+\.\w+", text)
if match:
    print(match.group())          # → alice@example.com

# findall：查找所有匹配
emails = re.findall(r"\w+@\w+\.\w+", text)
print(emails)                     # → ['alice@example.com', 'bob@test.org']

# finditer：返回迭代器（适合大量匹配）
for match in re.finditer(r"\w+@\w+\.\w+", text):
    print(f"找到: {match.group()} 在位置 {match.start()}-{match.end()}")

# ---------- 分组匹配 ----------
phone = "我的电话是 010-88888888"
pattern = r"(\d{3,4})-(\d{7,8})"
match = re.search(pattern, phone)
if match:
    print(f"完整号码: {match.group(0)}")     # → 010-88888888
    print(f"区号: {match.group(1)}")         # → 010
    print(f"号码: {match.group(2)}")         # → 88888888

# ---------- 替换 ----------
text = "今天是 2024-03-15，明天是 2024-03-16"
# 将日期格式从 YYYY-MM-DD 改为 MM/DD/YYYY
result = re.sub(r"(\d{4})-(\d{2})-(\d{2})", r"\2/\3/\1", text)
print(result)                      # → 今天是 03/15/2024，明天是 03/16/2024

# ---------- 编译正则 ----------
# 如果需要多次使用同一个正则，编译可以提高性能
pattern = re.compile(r"\d+", re.IGNORECASE | re.MULTILINE)
result = pattern.findall("abc123def456")

# ---------- 常用正则表达式 ----------
"""
\d      — 数字
\w      — 字母、数字、下划线
\s      — 空白字符（空格、制表符、换行等）
.       — 任意字符（除了换行符）
*       — 前一个模式出现 0 次或多次
+       — 前一个模式出现 1 次或多次
?       — 前一个模式出现 0 次或 1 次（非贪婪模式）
{n,m}   — 前一个模式出现 n 到 m 次
^       — 字符串开头
$       — 字符串结尾
[]      — 字符集
()      — 分组
|       — 或
"""
```

### 5.6 os / sys / shutil — 系统与文件操作

```python
import os
import sys
import shutil
from pathlib import Path  # Python 3.4+ 推荐的路径操作方式

# ---------- os.path — 路径操作 ----------
# 推荐使用 pathlib，但 os.path 在很多旧代码中常见
print(os.path.join("dir", "sub", "file.txt"))  # → dir/sub/file.txt
print(os.path.expanduser("~/test.txt"))         # → /Users/username/test.txt
print(os.path.abspath("."))                     # 当前目录的绝对路径
print(os.path.basename("/a/b/c.py"))            # → c.py
print(os.path.dirname("/a/b/c.py"))             # → /a/b
print(os.path.splitext("file.txt"))             # → ('file', '.txt')

# ---------- pathlib（推荐） ----------
p = Path("/a/b/c.txt")
print(p.parent)            # → /a/b
print(p.name)              # → c.txt
print(p.stem)              # → c（无后缀的文件名）
print(p.suffix)            # → .txt
print(p.exists())          # 是否存在
print(p.is_file())         # 是否是文件
print(p.is_dir())          # 是否是目录

# ---------- 目录操作 ----------
# 创建目录
Path("new_folder").mkdir(exist_ok=True)           # 创建单层
Path("a/b/c").mkdir(parents=True, exist_ok=True)  # 创建多层

# 遍历目录
for f in Path(".").iterdir():                     # 当前目录下的条目
    print(f.name)

for f in Path(".").rglob("*.py"):                 # 递归查找所有 .py 文件
    print(f)

# ---------- 文件操作 ----------
# 复制
shutil.copy("source.txt", "dest.txt")         # 复制文件
shutil.copytree("src_dir", "dst_dir")         # 复制整个目录树

# 移动/重命名
shutil.move("old.txt", "new.txt")

# 删除
Path("temp.txt").unlink(missing_ok=True)       # 删除文件
shutil.rmtree("temp_dir")                      # 删除整个目录（慎用！）

# ---------- sys — Python 运行时 ----------
print(sys.version)              # Python 版本号
print(sys.platform)             # 操作系统名 → "darwin"（macOS）
print(sys.argv)                 # 命令行参数列表
sys.path.append("/my/custom/path")  # 添加模块搜索路径

# ---------- 环境变量 ----------
print(os.environ.get("HOME"))       # 获取环境变量
print(os.environ.get("MY_VAR", "default_value"))

# ---------- 执行系统命令 ----------
os.system("ls -la")                # 同步执行（不推荐，输出不可控）
# 推荐使用 subprocess 模块
import subprocess
result = subprocess.run(["ls", "-la"], capture_output=True, text=True)
print(result.stdout)
```

### 5.7 collections — 高级容器

```python
from collections import Counter, defaultdict, OrderedDict, deque, namedtuple

# Counter：计数（前文已介绍）
# defaultdict：自动初始化默认值（前文已介绍）
# deque：双向队列（前文已介绍）
# namedtuple：命名元组（前文已介绍）

# ---------- OrderedDict ----------
# 保持插入顺序的字典（Python 3.7+ 的普通 dict 也保持顺序）
# OrderedDict 额外提供了 popitem(last) 等方法
od = OrderedDict()
od["z"] = 1
od["a"] = 2
od["b"] = 3
for key in od:
    print(key, end=" ")           # → z a b

# ---------- ChainMap ----------
# 将多个字典逻辑上合并为一个（不实际合并）
from collections import ChainMap

defaults = {"theme": "dark", "lang": "en"}
user_settings = {"theme": "light"}

# 优先使用 user_settings 中的值，没有则从 defaults 查找
settings = ChainMap(user_settings, defaults)
print(settings["theme"])          # → light
print(settings["lang"])           # → en
```

### 5.8 itertools — 迭代器工具

```python
import itertools

# ---------- 无限迭代器 ----------
# count(start, step)：从 start 开始无限计数
for i in itertools.count(10, 2):
    if i > 20:
        break
    print(i, end=" ")           # → 10 12 14 16 18 20

# cycle(iterable)：无限循环迭代
# repeat(element, times)：重复元素
for x in itertools.repeat("A", 3):
    print(x, end=" ")           # → A A A

# ---------- 组合迭代器 ----------
items = ["A", "B", "C"]

# product：笛卡尔积
print(list(itertools.product(items, repeat=2)))
# → [('A','A'), ('A','B'), ('A','C'), ('B','A'), ...]

# permutations：排列（考虑顺序，不重复选取）
print(list(itertools.permutations(items, 2)))
# → [('A','B'), ('A','C'), ('B','A'), ('B','C'), ('C','A'), ('C','B')]

# combinations：组合（不考虑顺序，不重复选取）
print(list(itertools.combinations(items, 2)))
# → [('A','B'), ('A','C'), ('B','C')]

# combinations_with_replacement：组合（可重复选取）
print(list(itertools.combinations_with_replacement(items, 2)))
# → [('A','A'), ('A','B'), ('A','C'), ('B','B'), ('B','C'), ('C','C')]

# ---------- 分组 ----------
data = [("A", 1), ("A", 2), ("B", 3), ("B", 4), ("C", 5)]
# 需要先排序
data.sort(key=lambda x: x[0])
for key, group in itertools.groupby(data, key=lambda x: x[0]):
    print(f"{key}: {list(group)}")
# 输出:
# A: [('A', 1), ('A', 2)]
# B: [('B', 3), ('B', 4)]
# C: [('C', 5)]

# ---------- 切片与链式 ----------
# islice：对迭代器切片
print(list(itertools.islice(range(10), 2, 8, 2)))  # → [2, 4, 6]

# chain：将多个迭代器串联
print(list(itertools.chain([1, 2], [3, 4], [5, 6])))
# → [1, 2, 3, 4, 5, 6]
```

### 5.9 functools — 函数式工具

```python
import functools

# ---------- partial —— 偏函数 ----------
# 固定某个函数的某些参数

def power(base, exponent):
    return base ** exponent

# 创建平方和立方的专用函数
square = functools.partial(power, exponent=2)
cube = functools.partial(power, exponent=3)

print(square(5))        # → 25
print(cube(5))          # → 125

# ---------- lru_cache —— 缓存 ----------
# 缓存函数的结果，避免重复计算

@functools.lru_cache(maxsize=128)
def fibonacci(n):
    """带缓存的斐波那契数列"""
    if n < 2:
        return n
    return fibonacci(n-1) + fibonacci(n-2)

# 第一次调用会计算，后续直接从缓存返回
print(fibonacci(100))   # 瞬间返回 354224848179261915075

# 查看缓存信息
print(fibonacci.cache_info())
# → CacheInfo(hits=98, misses=101, maxsize=128, currsize=101)

# ---------- wraps —— 保留函数元信息 ----------
# 在装饰器中保留原函数的 __name__ 和 __doc__
# 前文装饰器部分已介绍，这里再强调一下

def decorator(func):
    @functools.wraps(func)    # 没有这一行，wrapper 的元信息会覆盖原函数
    def wrapper(*args, **kwargs):
        """这是 wrapper 的文档"""
        return func(*args, **kwargs)
    return wrapper

@decorator
def my_func():
    """这是原函数的文档"""
    pass

print(my_func.__name__)  # 有 @wraps: my_func，没有: wrapper
print(my_func.__doc__)   # 有 @wraps: 这是原函数的文档，没有: 这是 wrapper 的文档
```

### 5.10 logging — 日志记录

```python
import logging

# ---------- 基础配置 ----------
logging.basicConfig(
    level=logging.DEBUG,                    # 日志级别：DEBUG < INFO < WARNING < ERROR < CRITICAL
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    filename="app.log",                     # 输出到文件（不设置则输出到控制台）
    filemode="a",                           # 追加模式
)

# 如果不输出到文件，想输出到控制台，用 stream
# logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)

logging.debug("这是调试信息")
logging.info("这是一般信息")
logging.warning("这是警告信息")
logging.error("这是错误信息")
logging.critical("这是严重错误")

# ---------- 模块化日志 ----------
# 每个模块使用自己的 logger
logger = logging.getLogger(__name__)        # logger 名称为当前模块名

# ---------- 异常栈追踪 ----------
try:
    1 / 0
except ZeroDivisionError:
    logger.exception("除零异常")            # 自动记录异常栈信息

# ---------- 配置日志格式 ----------
# 创建自定义 logger
custom_logger = logging.getLogger("my_app")
custom_logger.setLevel(logging.DEBUG)

# 控制台 Handler
console_handler = logging.StreamHandler()
console_handler.setLevel(logging.INFO)

# 文件 Handler
file_handler = logging.FileHandler("app.log")
file_handler.setLevel(logging.DEBUG)

# 定义格式
formatter = logging.Formatter(
    "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
)
console_handler.setFormatter(formatter)
file_handler.setFormatter(formatter)

# 给 logger 添加 handler
custom_logger.addHandler(console_handler)
custom_logger.addHandler(file_handler)
```

---

## 6. 综合示例

```python
"""
综合示例：简易学生成绩管理系统
演示：函数、数据结构、文件操作、异常处理、列表推导式等
"""

import json
from pathlib import Path
from typing import Optional


class GradeManager:
    """成绩管理器"""

    def __init__(self, data_file: str = "grades.json"):
        self.data_file = Path(data_file)
        self.students: dict[str, dict] = {}  # {name: {"scores": {...}, "avg": float}}
        self._load_data()

    def _load_data(self) -> None:
        """从文件加载数据"""
        if self.data_file.exists():
            with open(self.data_file, "r", encoding="utf-8") as f:
                self.students = json.load(f)
            print(f"已加载 {len(self.students)} 名学生的数据")

    def _save_data(self) -> None:
        """保存数据到文件"""
        with open(self.data_file, "w", encoding="utf-8") as f:
            json.dump(self.students, f, ensure_ascii=False, indent=2)

    def add_student(self, name: str) -> bool:
        """添加学生

        Args:
            name: 学生姓名

        Returns:
            True 表示添加成功，False 表示已存在
        """
        if name in self.students:
            print(f"学生 '{name}' 已存在")
            return False
        self.students[name] = {"scores": {}, "avg": 0.0}
        self._save_data()
        print(f"学生 '{name}' 添加成功")
        return True

    def add_score(self, name: str, subject: str, score: float) -> bool:
        """添加或更新学生成绩

        Args:
            name: 学生姓名
            subject: 科目
            score: 成绩

        Returns:
            True 表示成功，False 表示学生不存在
        """
        if name not in self.students:
            print(f"学生 '{name}' 不存在")
            return False

        if not 0 <= score <= 100:
            print("成绩必须在 0-100 之间")
            return False

        self.students[name]["scores"][subject] = score

        # 更新平均分
        scores = self.students[name]["scores"].values()
        self.students[name]["avg"] = sum(scores) / len(scores)

        self._save_data()
        print(f"已更新 {name} 的 {subject} 成绩: {score}")
        return True

    def get_student_info(self, name: str) -> Optional[dict]:
        """获取学生信息

        Args:
            name: 学生姓名

        Returns:
            学生信息字典，不存在返回 None
        """
        return self.students.get(name)

    def get_ranking(self) -> list[tuple[str, float]]:
        """按平均分排名

        Returns:
            [(姓名, 平均分), ...] 按平均分降序
        """
        ranking = [
            (name, info["avg"])
            for name, info in self.students.items()
            if info["scores"]  # 只包括有成绩的学生
        ]
        ranking.sort(key=lambda x: x[1], reverse=True)
        return ranking

    def show_all(self) -> None:
        """显示所有学生信息"""
        if not self.students:
            print("暂无学生数据")
            return

        print("\n" + "=" * 60)
        print(f"{'姓名':<10} {'科目数':<8} {'平均分':<8} {'最高分':<8}")
        print("-" * 60)

        for name, info in sorted(self.students.items()):
            scores = info["scores"]
            subject_count = len(scores)
            avg = info["avg"]
            max_score = max(scores.values()) if scores else 0
            print(f"{name:<10} {subject_count:<8} {avg:<8.1f} {max_score:<8}")

        print("=" * 60)

    def stats(self) -> dict:
        """统计整体情况

        Returns:
            统计信息字典
        """
        if not self.students:
            return {"total": 0}

        all_scores = [
            score
            for info in self.students.values()
            for score in info["scores"].values()
        ]

        avgs = [info["avg"] for info in self.students.values() if info["scores"]]

        return {
            "total_students": len(self.students),
            "total_scores": len(all_scores),
            "class_avg": sum(avgs) / len(avgs) if avgs else 0,
            "best_avg": max(avgs) if avgs else 0,
            "worst_avg": min(avgs) if avgs else 0,
        }


def main():
    """主函数：命令行交互"""
    manager = GradeManager()

    while True:
        print("\n" + "=" * 40)
        print("学生成绩管理系统")
        print("=" * 40)
        print("1. 添加学生")
        print("2. 录入成绩")
        print("3. 查询学生")
        print("4. 查看排名")
        print("5. 显示所有")
        print("6. 统计信息")
        print("0. 退出")
        print("=" * 40)

        choice = input("请选择: ").strip()

        if choice == "1":
            name = input("学生姓名: ").strip()
            manager.add_student(name)

        elif choice == "2":
            name = input("学生姓名: ").strip()
            subject = input("科目: ").strip()
            try:
                score = float(input("成绩: ").strip())
                manager.add_score(name, subject, score)
            except ValueError:
                print("成绩格式错误")

        elif choice == "3":
            name = input("学生姓名: ").strip()
            info = manager.get_student_info(name)
            if info:
                print(f"\n姓名: {name}")
                print(f"成绩: {info['scores']}")
                print(f"平均分: {info['avg']:.1f}")
            else:
                print("学生不存在")

        elif choice == "4":
            ranking = manager.get_ranking()
            if ranking:
                print("\n" + "=" * 30)
                print(f"{'排名':<4} {'姓名':<10} {'平均分':<8}")
                print("-" * 30)
                for i, (name, avg) in enumerate(ranking, 1):
                    print(f"{i:<4} {name:<10} {avg:<8.1f}")
            else:
                print("暂无排名数据")

        elif choice == "5":
            manager.show_all()

        elif choice == "6":
            stats = manager.stats()
            if stats["total_students"] == 0:
                print("暂无数据")
            else:
                print(f"\n学生总数: {stats['total_students']}")
                print(f"成绩总数: {stats['total_scores']}")
                print(f"班级平均分: {stats['class_avg']:.1f}")
                print(f"最高平均分: {stats['best_avg']:.1f}")
                print(f"最低平均分: {stats['worst_avg']:.1f}")

        elif choice == "0":
            print("再见！")
            break

        else:
            print("无效选择")


if __name__ == "__main__":
    main()
```

---

> 本文档涵盖了 Python 基础语法、常用数据结构、流程控制、函数、以及常用内置库。
> 下一阶段将深入讲解 [类和对象、文件 I/O 等内容](02-Python进阶详解.md)。
