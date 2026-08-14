# Python 核心机制详解

> 本文档深入讲解 Python 中必须要掌握的核心编程机制：魔法方法（`__dunder__`）、类型提示（Type Hints）、类型注解、以及元编程（元类与反射）。

---

## 目录

1. [魔法方法（__dunder__）](#1-魔法方法dunder)
2. [类型提示（Type Hints）](#2-类型提示type-hints)
3. [类型注解详解](#3-类型注解详解)
4. [元类（Metaclass）](#4-元类metaclass)
5. [反射与内省](#5-反射与内省)
6. [综合示例](#6-综合示例)

---

## 1. 魔法方法（__dunder__）

> "dunder" = Double UNDERscore（双下划线）。Python 的特殊方法，赋予类各种内置行为。

### 1.1 对象创建与销毁

```python
class LifecycleDemo:
    """
    演示对象的生命周期钩子
    执行顺序：__new__ → __init__ → ...使用... → __del__
    """

    # ---------- __new__：真正的"构造方法" ----------
    def __new__(cls, *args, **kwargs):
        """
        创建并返回一个新实例
        在 __init__ 之前调用

        典型用途：
        1. 实现单例模式
        2. 返回不可变类型的子类（如 int、str 的子类必须用 __new__）
        3. 对象池 / 缓存

        注意：__new__ 的第一个参数是 cls（类），不是 self（实例）
        """
        print(f"1. __new__ 被调用: cls={cls.__name__}")
        # 必须调用父类的 __new__ 来分配内存
        instance = super().__new__(cls)
        print(f"   创建实例: {id(instance)}")
        return instance

    # ---------- __init__：初始化方法 ----------
    def __init__(self, name: str):
        """
        初始化实例（设置属性等）
        __new__ 返回实例后，Python 自动调用 __init__
        """
        print(f"2. __init__ 被调用: name={name}")
        self.name = name

    # ---------- __del__：析构方法 ----------
    def __del__(self):
        """
        实例被垃圾回收前调用
        注意：**不保证**会被调用（解释器退出时不一定会调用）

        不要过度依赖 __del__，资源释放应该用上下文管理器
        """
        print(f"3. __del__ 被调用: {self.name} 被销毁了")


# ---------- 单例模式（使用 __new__） ----------
class Singleton:
    """单例：整个进程只有一个实例"""

    _instance = None

    def __new__(cls, *args, **kwargs):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self, value=None):
        # 注意：__init__ 每次都会调用，需要防止重复初始化
        if not hasattr(self, "_initialized"):
            self.value = value
            self._initialized = True


s1 = Singleton("第一次")
s2 = Singleton("第二次")
print(s1 is s2)              # → True（同一个实例）
print(s1.value)              # → "第一次"（第二次的初始化被跳过）


# ---------- 不可变类型子类 ----------
class PositiveInteger(int):
    """只能存储正整数的 int 子类"""

    def __new__(cls, value):
        if value < 0:
            raise ValueError("不能为负数")
        return super().__new__(cls, value)

    # int 是不可变的，所以必须在 __new__ 中处理
    # __init__ 对不可变类型无效


# ---------- 对象池模式 ----------
class ConnectionPool:
    """数据库连接池（简化版）"""

    _pool = []

    def __new__(cls):
        if cls._pool:
            conn = cls._pool.pop()
            print(f"复用连接: {id(conn)}")
            return conn
        instance = super().__new__(cls)
        print(f"新建连接: {id(instance)}")
        return instance

    def close(self):
        """放回连接池"""
        ConnectionPool._pool.append(self)
```

### 1.2 字符串表示

```python
class Book:
    """书籍类 —— 演示字符串相关的魔法方法"""

    def __init__(self, title: str, author: str, pages: int):
        self.title = title
        self.author = author
        self.pages = pages

    # ---------- __repr__：调试用的"官方"表示 ----------
    def __repr__(self) -> str:
        """
        由 repr() 和内省工具调用
        应返回能够重建对象的表达式字符串（如果可能）
        如果没有 __str__，print() 也会回到 __repr__
        """
        return (f"Book(title={self.title!r}, "
                f"author={self.author!r}, "
                f"pages={self.pages})")

    # ---------- __str__：用户友好的字符串 ----------
    def __str__(self) -> str:
        """
        由 str()、print()、f-string 调用
        返回对用户友好的表示
        """
        return f"《{self.title}》- {self.author}（{self.pages}页）"

    # ---------- __format__：自定义格式说明符 ----------
    def __format__(self, format_spec: str) -> str:
        """
        由 f-string 和 format() 调用
        format_spec 是冒号后面的内容
        例如 f"{book:short}" 中 format_spec = "short"
        """
        if format_spec == "short":
            return self.title
        elif format_spec == "bib":
            return f"{self.author}：《{self.title}》"
        return str(self)

    # ---------- __bytes__：字节表示 ----------
    def __bytes__(self) -> bytes:
        """由 bytes() 调用"""
        return str(self).encode("utf-8")

    # ---------- __bool__：真值判断 ----------
    def __bool__(self) -> bool:
        """由 bool() 调用，也用于 if 判断"""
        return self.pages > 0


# 使用
book = Book("Python编程", "张三", 400)

print(repr(book))           # → Book(title='Python编程', author='张三', pages=400)
print(str(book))            # → 《Python编程》- 张三（400页）
print(book)                 # → 《Python编程》- 张三（400页）
print(f"{book:short}")      # → Python编程
print(f"{book:bib}")        # → 张三：《Python编程》
print(bool(Book("空", "无", 0)))  # → False
```

### 1.3 容器相关方法

```python
class RangeCollection:
    """
    自定义容器：模拟范围集合
    支持 in、len、切片、迭代等操作
    """

    def __init__(self, start: int, end: int, step: int = 1):
        """存储一个数值范围"""
        self._start = start
        self._end = end
        self._step = step

    # ---------- 长度 ----------
    def __len__(self) -> int:
        """
        len() 调用
        必须返回非负整数
        """
        return max(0, (self._end - self._start + self._step - 1) // self._step)

    # ---------- 成员检测 ----------
    def __contains__(self, item) -> bool:
        """
        in 运算符调用
        没有 __contains__ 时，Python 会回退到 __iter__ 进行线性搜索
        """
        if not isinstance(item, int):
            return False
        if item < self._start or item >= self._end:
            return False
        return (item - self._start) % self._step == 0

    # ---------- 索引访问 ----------
    def __getitem__(self, index):
        """
        self[index] 调用

        index 可以是:
        - 整数
        - slice 对象（切片）
        """
        if isinstance(index, slice):
            # 处理切片
            start = index.start or 0
            stop = index.stop or len(self)
            step = index.step or 1
            return [
                self._start + i * self._step
                for i in range(start, stop, step)
                if i < len(self)
            ]
        # 处理整数索引
        if index < 0:
            index += len(self)
        if index < 0 or index >= len(self):
            raise IndexError("索引超出范围")
        return self._start + index * self._step

    # ---------- 迭代器 ----------
    def __iter__(self):
        """
        iter() 和 for 循环调用
        返回一个迭代器对象
        """
        return RangeIterator(self._start, self._end, self._step)

    # ---------- 反向迭代 ----------
    def __reversed__(self):
        """
        reversed() 调用
        没有此方法时，reversed() 会使用 __len__ + __getitem__
        """
        return RangeIterator(
            self._end - 1,
            self._start - 1,
            -self._step
        )

    def __str__(self):
        return f"RangeCollection({self._start}, {self._end}, {self._step})"


class RangeIterator:
    """范围迭代器"""

    def __init__(self, start, end, step):
        self.current = start
        self.end = end
        self.step = step

    def __iter__(self):
        return self

    def __next__(self):
        if self.current >= self.end:
            raise StopIteration
        value = self.current
        self.current += self.step
        return value


# 使用
rc = RangeCollection(0, 10, 2)
print(len(rc))               # → 5
print(6 in rc)               # → True
print(5 in rc)               # → False
print(rc[2])                 # → 4
print(rc[1:4])              # → [2, 4, 6]
print(list(rc))              # → [0, 2, 4, 6, 8]
print(list(reversed(rc)))   # → [8, 6, 4, 2, 0]
```

### 1.4 算术与比较运算符（完整列表）

```python
class CustomNumber:
    """
    自定义数字类型 —— 演示所有算术/比较魔法方法

    Python 的运算符实际上是在调用这些魔法方法：
    a + b  →  a.__add__(b) 或 b.__radd__(a)
    """

    def __init__(self, value):
        self.value = value

    # ---------- 一元运算符 ----------
    def __neg__(self):           return CustomNumber(-self.value)      # -obj
    def __pos__(self):           return CustomNumber(+self.value)      # +obj
    def __abs__(self):           return CustomNumber(abs(self.value))  # abs(obj)
    def __invert__(self):        return CustomNumber(~self.value)      # ~obj

    # ---------- 二元算术运算符 ----------
    def __add__(self, other):    return CustomNumber(self.value + other)       # obj + x
    def __sub__(self, other):    return CustomNumber(self.value - other)       # obj - x
    def __mul__(self, other):    return CustomNumber(self.value * other)       # obj * x
    def __truediv__(self, other): return CustomNumber(self.value / other)      # obj / x
    def __floordiv__(self, other): return CustomNumber(self.value // other)    # obj // x
    def __mod__(self, other):    return CustomNumber(self.value % other)       # obj % x
    def __pow__(self, other):    return CustomNumber(self.value ** other)      # obj ** x
    def __matmul__(self, other): return CustomNumber(self.value @ other)       # obj @ x

    # ---------- 反向算术运算符 ----------
    # 当左操作数不支持该运算时，Python 会调用右操作数的反向方法
    # 例如 10 + obj  →  obj.__radd__(10)
    def __radd__(self, other):   return CustomNumber(other + self.value)       # x + obj
    def __rsub__(self, other):   return CustomNumber(other - self.value)       # x - obj
    def __rmul__(self, other):   return CustomNumber(other * self.value)       # x * obj
    def __rtruediv__(self, other): return CustomNumber(other / self.value)     # x / obj

    # ---------- 复合赋值运算符 ----------
    # 这些方法应该就地修改 self 并返回 self（或返回修改后的副本）
    def __iadd__(self, other):   self.value += other; return self              # obj += x
    def __isub__(self, other):   self.value -= other; return self              # obj -= x
    def __imul__(self, other):   self.value *= other; return self              # obj *= x

    # ---------- 比较运算符 ----------
    def __eq__(self, other):     return self.value == other    # obj == x
    def __ne__(self, other):     return self.value != other    # obj != x
    def __lt__(self, other):     return self.value < other     # obj < x
    def __le__(self, other):     return self.value <= other    # obj <= x
    def __gt__(self, other):     return self.value > other     # obj > x
    def __ge__(self, other):     return self.value >= other    # obj >= x

    # 注意：Python 3 中，__ne__ 默认会反向 __eq__ 的结果
    # 但显式定义总是更安全

    # ---------- 类型转换 ----------
    def __int__(self):           return int(self.value)        # int(obj)
    def __float__(self):         return float(self.value)      # float(obj)
    def __bool__(self):          return bool(self.value)       # bool(obj)
    def __complex__(self):       return complex(self.value)    # complex(obj)
    def __str__(self):           return str(self.value)        # str(obj)
    def __repr__(self):          return f"CustomNumber({self.value})"
    def __hash__(self):          return hash(self.value)       # hash(obj) 和字典键

    # ---------- 合理处理 NotImplemented ----------
    def __eq__(self, other):
        if isinstance(other, CustomNumber):
            return self.value == other.value
        if isinstance(other, (int, float)):
            return self.value == other
        return NotImplemented  # 告诉 Python 尝试 other.__eq__(self)


# ---------- 快捷方式 ----------
"""
x < y 的完整调用流程：
1. 调用 x.__lt__(y)
2. 如果返回 NotImplemented，尝试 y.__gt__(x)
3. 如果也返回 NotImplemented，抛出 TypeError

x == y 的完整调用流程：
1. 调用 x.__eq__(y)
2. 如果返回 NotImplemented，尝试 y.__eq__(x)
3. 如果也返回 NotImplemented，返回 False（不会异常）
"""
```

### 1.5 属性访问方法

```python
class AttributeLogger:
    """
    属性访问日志 —— 演示属性访问的魔法方法

    这些方法在属性访问时被调用：
    obj.attr  →  __getattribute__(obj, 'attr')
    obj.attr  →  __getattr__(obj, 'attr')（仅当属性不存在时）
    obj.attr = x  →  __setattr__(obj, 'attr', x)
    del obj.attr  →  __delattr__(obj, 'attr')
    """

    def __init__(self):
        # 注意：在 __init__ 中直接赋值会调用 __setattr__
        # 所以要用 object.__setattr__ 绕过
        object.__setattr__(self, "_data", {})
        object.__setattr__(self, "_log", [])

    # ---------- __getattribute__：所有属性访问（优先） ----------
    def __getattribute__(self, name):
        """
        每次访问属性时都会调用
        优先级高于 __getattr__

        危险：容易导致无限递归！比如 self._data 也会触发 __getattribute__
        一定要用 super().__getattribute__() 或 object.__getattribute__()
        """
        # 记录所有属性访问
        log = object.__getattribute__(self, "_log")
        log.append(f"GET {name}")

        # 使用 super() 调用默认行为
        return super().__getattribute__(name)

    # ---------- __getattr__：属性不存在时调用 ----------
    def __getattr__(self, name):
        """
        仅当 __getattribute__ 抛出 AttributeError 时调用

        典型用途：动态属性、默认值
        """
        self._log.append(f"MISS {name}")
        return f"动态属性: {name}"

    # ---------- __setattr__：属性赋值 ----------
    def __setattr__(self, name, value):
        """
        每次属性赋值时调用
        容易导致无限递归！
        """
        self._log.append(f"SET {name} = {value!r}")
        super().__setattr__(name, value)

    # ---------- __delattr__：删除属性 ----------
    def __delattr__(self, name):
        """
        del obj.attr 时调用
        """
        self._log.append(f"DEL {name}")
        super().__delattr__(name)

    def show_log(self):
        """显示访问日志"""
        for entry in self._log:
            print(entry)


# ---------- 实用示例：惰性计算属性 ----------
class LazyProperty:
    """
    惰性计算属性：只在首次访问时计算，之后缓存结果
    类似 @property，但结果会被缓存
    """

    def __init__(self, func):
        self.func = func
        self.name = func.__name__

    def __get__(self, instance, owner):
        """
        描述符协议：当属性在类上定义，通过实例访问时调用
        """
        if instance is None:
            return self
        # 计算并缓存
        value = self.func(instance)
        instance.__dict__[self.name] = value
        return value


class DataAnalyzer:
    """数据分析器 —— 使用惰性计算属性"""

    def __init__(self, data):
        self.data = data

    @LazyProperty
    def expensive_computation(self):
        """模拟耗时的计算"""
        print("执行耗时计算...")
        import time
        time.sleep(1)
        return sum(self.data) / len(self.data)


# 使用：第一次访问会计算，第二次直接返回缓存值
da = DataAnalyzer([1, 2, 3, 4, 5])
print(da.expensive_computation)  # → 先打印"执行耗时计算..."，再打印 3.0
print(da.expensive_computation)  # → 直接返回 3.0（不再计算）
```

### 1.6 可调用对象（`__call__`）

```python
from typing import Callable


# ---------- 使对象可调用 ----------
class Adder:
    """
    加法器 —— 实现了 __call__ 的对象称为"可调用对象"

    obj()  →  obj.__call__()
    可调用对象可以作为函数使用
    """

    def __init__(self, base: float = 0):
        self.base = base

    def __call__(self, *args) -> float:
        """对象被"调用"时执行"""
        return self.base + sum(args)

    def __repr__(self):
        return f"Adder(base={self.base})"


add_five = Adder(5)
print(add_five(10))             # → 15（调用 __call__）
print(add_five(1, 2, 3))        # → 11
print(callable(add_five))       # → True（检查是否可调用）


# ---------- 带状态的函数（闭包的替代方案） ----------
class Counter:
    """
    计数器 —— 通过 __call__ 维护状态
    比闭包更易读
    """

    def __init__(self, start: int = 0):
        self.count = start

    def __call__(self) -> int:
        self.count += 1
        return self.count

    def reset(self):
        self.count = 0


counter = Counter()
print(counter())    # → 1
print(counter())    # → 2
print(counter())    # → 3


# ---------- 函数装饰器也可以写作类 ----------
class Timer:
    """
    计时装饰器 —— 类的 __call__ 实现
    比函数装饰器的优势：可记录调用次数、总时间等状态
    """

    def __init__(self, func: Callable):
        self.func = func
        self.call_count = 0
        self.total_time = 0.0

    def __call__(self, *args, **kwargs):
        import time
        start = time.perf_counter()
        result = self.func(*args, **kwargs)
        elapsed = time.perf_counter() - start

        self.call_count += 1
        self.total_time += elapsed

        print(f"{self.func.__name__} 调用 #{self.call_count}: {elapsed:.4f}s")
        return result


@Timer
def slow_function(n: int) -> int:
    return sum(range(n))

slow_function(1000000)
slow_function(2000000)
```

### 1.7 上下文管理器相关

```python
class Transaction:
    """
    事务上下文管理器
    在 with 块中，自动处理开始和回滚/提交
    """

    def __init__(self, name: str):
        self.name = name

    def __enter__(self):
        print(f"[开始事务] {self.name}")
        return self  # as 变量即为 self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """
        Args:
            exc_type: 异常类型（没有异常则为 None）
            exc_val: 异常实例
            exc_tb: traceback 对象

        Returns:
            True = 抑制异常（不向外传播）
            False/None = 不抑制异常
        """
        if exc_type:
            print(f"[回滚事务] {self.name}（原因: {exc_val}）")
            return False  # 不抑制异常
        else:
            print(f"[提交事务] {self.name}")
        return False


# ---------- __enter__ 返回值 ----------
class Connection:
    """数据库连接"""

    def __enter__(self):
        print("打开连接")
        return self  # 返回 self，让 as 变量使用

    def __exit__(self, *args):
        print("关闭连接")

    def query(self, sql):
        print(f"执行: {sql}")


# 使用
with Connection() as conn:
    conn.query("SELECT * FROM users")

# with 还可以同时管理多个上下文（Python 3.1+）
with open("a.txt") as f1, open("b.txt") as f2:
    pass
```

### 1.8 魔法方法速查表

| 类别 | 方法 | 触发场景 |
|------|------|----------|
| **创建/销毁** | `__new__` | `cls()` 创建实例时（先于 __init__） |
| | `__init__` | 实例创建后初始化 |
| | `__del__` | 实例被垃圾回收前 |
| **字符串** | `__str__` | `str(obj)`、`print(obj)`、f-string |
| | `__repr__` | `repr(obj)`、交互式命令行显示 |
| | `__format__` | `format(obj, spec)`、f-string 格式说明 |
| | `__bytes__` | `bytes(obj)` |
| **容器** | `__len__` | `len(obj)` |
| | `__getitem__` | `obj[key]`、`for x in obj`（回退） |
| | `__setitem__` | `obj[key] = value` |
| | `__delitem__` | `del obj[key]` |
| | `__contains__` | `x in obj` |
| | `__iter__` | `iter(obj)`、for 循环 |
| | `__next__` | `next(iterator)` |
| | `__reversed__` | `reversed(obj)` |
| **属性** | `__getattr__` | `obj.x` 属性不存在时（兜底） |
| | `__getattribute__` | 所有属性访问（优先） |
| | `__setattr__` | 所有属性赋值 |
| | `__delattr__` | `del obj.x` |
| | `__dir__` | `dir(obj)` |
| **可调用** | `__call__` | `obj()` |
| **比较** | `__eq__`/`__ne__` | `==` / `!=` |
| | `__lt__`/`__le__` | `<` / `<=` |
| | `__gt__`/`__ge__` | `>` / `>=` |
| **算术** | `__add__`/`__radd__` | `+`（右操作数版本） |
| | `__sub__`/`__rsub__` | `-` |
| | `__mul__`/`__rmul__` | `*` |
| | `__truediv__`/`__rtruediv__` | `/` |
| **类型转换** | `__int__`/`__float__`/`__bool__` | `int(obj)` / `float(obj)` / `bool(obj)` |
| | `__hash__` | `hash(obj)`、字典键 |
| | `__index__` | 用于索引的整数转换 |
| **上下文** | `__enter__`/`__exit__` | `with obj:` |
| **描述符** | `__get__`/`__set__`/`__delete__` | 属性访问协议 |
| **序列化** | `__getstate__`/`__setstate__` | `pickle` 序列化 |
| **协程** | `__await__`/`__aiter__`/`__anext__` | 异步编程 |

---

## 2. 类型提示（Type Hints）

### 2.1 为什么需要类型提示

```python
"""
类型提示（Type Hints）是 Python 3.5+ 引入的可选特性：
1. 提高代码可读性和可维护性
2. 让 IDE 和编辑器提供更好的自动补全和错误检查
3. 配合 mypy、pyright 等工具进行静态类型检查
4. 在运行时**不会被强制检查**（Python 仍是动态语言）

主要工具：
- mypy：最流行的静态类型检查器
- pyright / Pylance：VS Code 使用
"""
```

### 2.2 基础类型提示

```python
# ---------- 变量注解 ----------
name: str = "Alice"               # 基本类型
age: int = 25
height: float = 1.68
is_student: bool = True

# 不需要初始值（但必须有类型注解）
nickname: str                     # 未初始化变量

# ---------- 函数注解 ----------
def greet(name: str, times: int = 1) -> str:
    """
    参数注解：
    name: str     → 参数 name 应该是 str 类型
    times: int    → 参数 times 应该是 int 类型（默认值为 1）

    返回类型：
    -> str        → 返回值应该是 str 类型
    """
    return f"Hello, {name}!" * times

# 没有返回值的函数
def log_message(msg: str) -> None:
    """返回 None 表示不返回值"""
    print(msg)

# 可能不返回值的函数
def find_user(user_id: int) -> str | None:
    """Python 3.10+：联合类型"""
    if user_id == 1:
        return "Alice"
    return None
```

### 2.3 容器类型提示

```python
from typing import List, Dict, Tuple, Set, Optional

# ---------- Python 3.9+（推荐） ----------
# Python 3.9+ 可以直接使用内置容器类型

# 列表
numbers: list[int] = [1, 2, 3]                  # 元素为 int 的列表

# 字典
scores: dict[str, int] = {"math": 95, "eng": 88}  # str→int 的映射

# 元组
point: tuple[float, float] = (3.5, 4.2)          # 固定长度的元组
variadic: tuple[int, ...] = (1, 2, 3)             # 可变长度的元组

# 集合
unique_ids: set[int] = {1, 2, 3}

# ---------- Python 3.8 及以下（使用 typing） ----------
numbers_old: List[int] = [1, 2, 3]
scores_old: Dict[str, int] = {"math": 95}
point_old: Tuple[float, float] = (3.5, 4.2)
set_old: Set[int] = {1, 2, 3}

# ---------- Optional ----------
# Optional[X] 等价于 X | None
maybe_name: Optional[str] = None     # 可以是 str 或 None
maybe_age: str | None = None          # Python 3.10+ 语法
```

### 2.4 复杂类型组合

```python
from typing import Any, Union, Optional, Callable, TypeVar, Generic

# ---------- Union —— 联合类型 ----------
# 一个值可以是几种类型之一
def parse_int(value: str | int) -> int:
    """Python 3.10+ 语法"""
    if isinstance(value, str):
        return int(value)
    return value

# 旧语法
def parse_int_old(value: Union[str, int]) -> int:
    return int(value)

# ---------- Any —— 任意类型 ----------
# 关闭类型检查（用于和动态代码交互）
def process_anything(data: Any) -> Any:
    """Any 类型表示不进行类型检查"""
    return data.some_method()

# ---------- Callable —— 可调用类型 ----------
# Callable[[参数类型], 返回类型]

def apply_twice(func: Callable[[int], int], x: int) -> int:
    """接受一个接收 int 返回 int 的函数"""
    return func(func(x))

print(apply_twice(lambda n: n * 2, 5))  # → 20

# 没有任何参数的函数
def run_later(func: Callable[[], None]) -> None:
    """接受一个没有参数且不返回值的函数"""
    func()

# ---------- TypeVar —— 泛型变量 ----------
T = TypeVar("T", int, float)  # T 只能是 int 或 float

def first(seq: list[T]) -> T:
    """泛型函数：返回列表的第一个元素"""
    return seq[0]

result = first([1, 2, 3])      # 类型推断为 int
# result = first(["a", "b"])   # ❌ 类型错误：str 不是 int 或 float

# 更通用的泛型
U = TypeVar("U")               # 可以是任何类型

def identity(x: U) -> U:
    """恒等函数：输入和输出类型相同"""
    return x

# ---------- Generic —— 泛型类 ----------
class Box(Generic[T]):
    """泛型容器"""

    def __init__(self, value: T):
        self.value = value

    def get(self) -> T:
        return self.value

    def set(self, value: T) -> None:
        self.value = value


int_box = Box[int](42)          # Box 中装的是 int
int_val = int_box.get()          # int_val 被推断为 int

str_box = Box[str]("hello")
str_val = str_box.get()          # str_val 被推断为 str
```

### 2.5 类型别名

```python
# ---------- 类型别名 ----------
from typing import TypeAlias

# 定义复杂的类型别名
UserID: TypeAlias = int
UserData: TypeAlias = dict[str, Any]
UserList: TypeAlias = list[UserData]

# 更复杂的别名
JSON = dict[str, "JSON"] | list["JSON"] | str | int | float | bool | None

def parse_json(data: str) -> JSON:
    """处理 JSON 数据"""
    import json
    return json.loads(data)


# ---------- NewType —— 创建新类型 ----------
from typing import NewType

# 创建运行时不会改变类型的新类型
# 用于区分不同类型的数据，即使它们的底层类型相同
UserID = NewType("UserID", int)
ProductID = NewType("ProductID", int)

def get_user(user_id: UserID) -> dict:
    """只能接受 UserID 类型"""
    return {"id": user_id, "name": "Alice"}

def get_product(product_id: ProductID) -> dict:
    """只能接受 ProductID 类型"""
    return {"id": product_id, "name": "Widget"}

uid = UserID(42)
pid = ProductID(42)

get_user(uid)           # ✅ 正确
get_user(pid)           # ❌ 类型错误（mypy 会报错）
# 注意：NewType 只在静态类型检查时生效，运行时 uid 就是 int
```

### 2.6 TypedDict —— 精确的字典类型

```python
from typing import TypedDict, NotRequired


# ---------- 定义字典的结构 ----------
class Person(TypedDict):
    """Person 字典必须包含 name 和 age，可包含 email"""
    name: str
    age: int
    email: NotRequired[str]     # Python 3.11+: 可选字段


# 旧语法（Python 3.8+）
# Person = TypedDict("Person", {"name": str, "age": int, "email": NotRequired[str]})


# ---------- 使用 ----------
def create_person(name: str, age: int) -> Person:
    return {"name": name, "age": age}

person = create_person("Alice", 25)
print(person["name"])           # → Alice
# person["extra"] = "test"      # ❌ 类型错误：extra 不在 Person 中


# ---------- 基于类的 TypedDict（Python 3.11+） ----------
class Movie(TypedDict, total=False):
    """total=False 表示所有字段都可选"""
    title: str
    year: int
    rating: float


movie: Movie = {}               # 空字典也是合法的
movie["title"] = "Inception"
```

### 2.7 Protocols —— 结构化子类型

```python
from typing import Protocol, runtime_checkable


# ---------- 定义协议 ----------
# 类似 Go 的接口或 TypeScript 的 structural typing
# 只要对象有正确的方法/属性，就自动满足协议（不需要显式继承）

@runtime_checkable  # 允许使用 isinstance() 在运行时检查
class Drawable(Protocol):
    """可绘制对象的协议"""

    def draw(self) -> str:
        """绘制自身"""
        ...

    @property
    def name(self) -> str:
        """名称"""
        ...


# ---------- 不需要继承 Drawable ----------
class Circle:
    """圆形 —— 自动满足 Drawable 协议"""

    def __init__(self, radius: float):
        self.radius = radius

    @property
    def name(self) -> str:
        return f"圆形(r={self.radius})"

    def draw(self) -> str:
        return f"绘制半径 {self.radius} 的圆"


class Square:
    """正方形 —— 也自动满足 Drawable 协议"""

    def __init__(self, side: float):
        self.side = side

    @property
    def name(self) -> str:
        return f"正方形(s={self.side})"

    def draw(self) -> str:
        return f"绘制边长 {self.side} 的正方形"


def render(thing: Drawable) -> None:
    """只依赖协议，不依赖具体类型"""
    print(f"渲染: {thing.name}")
    print(f"结果: {thing.draw()}")


# 鸭子类型（duck typing）：不需要继承
render(Circle(5.0))      # ✅
render(Square(3.0))      # ✅
print(isinstance(Circle(1), Drawable))  # → True（因为有 @runtime_checkable）
```

---

## 3. 类型注解详解

### 3.1 运行时访问类型注解

```python
# ---------- __annotations__ 属性 ----------
class User:
    name: str
    age: int
    email: str | None = None

    def greet(self) -> str:
        return f"Hello, {self.name}"


# 访问类注解
print(User.__annotations__)
# → {'name': <class 'str'>, 'age': <class 'int'>, 'email': typing.Optional[str]}

# 访问函数注解
def process(name: str, age: int = 0) -> bool:
    return True

print(process.__annotations__)
# → {'name': <class 'str'>, 'age': <class 'int'>, 'return': <class 'bool'>}


# ---------- get_type_hints() —— 解析前向引用 ----------
from typing import get_type_hints

class Node:
    """前向引用：类在定义中引用自身"""
    def link(self, other: "Node") -> None:
        """"Node" 是字符串，因为 Node 此时还未完全定义"""
        pass

    def get_children(self) -> list["Node"]:
        return []


# __annotations__ 中还是字符串
print(Node.link.__annotations__)  # → {'other': 'Node', 'return': None}

# get_type_hints 会解析字符串为实际类型
resolved = get_type_hints(Node.link)
print(resolved)                   # → {'other': <class '__main__.Node'>, 'return': <class 'NoneType'>}


# ---------- TYPE_CHECKING —— 仅在类型检查时导入 ----------
from typing import TYPE_CHECKING

# TYPE_CHECKING 在运行时为 False，但在 mypy 等工具中为 True
# 用于避免循环导入

if TYPE_CHECKING:
    # 这些导入只在类型检查时发生，不会在运行时执行
    from some_module import SomeType

def func(x: "SomeType") -> None:
    """SomeType 在运行时实际上未定义，所以要用字符串"""
    pass
```

### 3.2 运行时使用注解

```python
# ---------- 数据验证（简单示例） ----------
import json
from typing import get_type_hints


class FieldValidator:
    """利用类型注解做简单的数据验证"""

    @classmethod
    def validate(cls, data: dict) -> dict:
        """
        根据类的类型注解验证数据

        Args:
            data: 要验证的原始数据字典

        Returns:
            验证后的数据
        """
        hints = get_type_hints(cls)
        errors = []

        for field_name, field_type in hints.items():
            if field_name.startswith("_"):
                continue

            # 检查字段是否存在
            if field_name not in data:
                errors.append(f"缺少字段: {field_name}")
                continue

            value = data[field_name]

            # 检查类型（简单版）
            origin = getattr(field_type, "__origin__", None)  # Python 3.8+
            if origin is None:
                # 简单类型
                if not isinstance(value, field_type):
                    errors.append(
                        f"字段 '{field_name}' 类型错误: "
                        f"期望 {field_type.__name__}，实际 {type(value).__name__}"
                    )

        if errors:
            raise ValueError("\n".join(errors))

        return data


class CreateUserRequest(FieldValidator):
    """创建用户的请求格式"""
    name: str
    age: int
    email: str


# 使用
valid_data = CreateUserRequest.validate({
    "name": "Alice",
    "age": 25,
    "email": "alice@example.com"
})
print("验证通过:", valid_data)

try:
    CreateUserRequest.validate({
        "name": "Bob",
        "age": "二十五",  # 类型错误
    })
except ValueError as e:
    print("验证失败:", e)
```

### 3.3 `@dataclass` — 数据类

```python
from dataclasses import dataclass, field, asdict, astuple
from typing import ClassVar


# ---------- 基本使用 ----------
@dataclass
class Point:
    """
    数据类：自动生成 __init__、__repr__、__eq__ 等
    等价于写一个只有属性的简单类
    """
    x: float                          # 必填字段，无默认值
    y: float                          # 必填字段，无默认值
    z: float = 0.0                    # 可选字段，有默认值


p1 = Point(1.0, 2.0, 3.0)
p2 = Point(1.0, 2.0, 3.0)
print(p1)                     # → Point(x=1.0, y=2.0, z=3.0)
print(p1 == p2)               # → True（自动生成 __eq__）


# ---------- 高级配置 ----------
@dataclass(order=True)  # 生成比较运算符（排序时按字段顺序比较）
class Person:
    # sort_index: int = field(init=False)  # 用于排序的字段，不参与 __init__

    name: str
    age: int

    # field 的配置
    email: str = field(default="unknown@example.com", repr=False)
    # repr=False 表示 email 不参与 __repr__

    # ClassVar：类变量（不是实例字段，不参与 __init__）
    species: ClassVar[str] = "Homo sapiens"

    # 冻结实例（不可修改）
    # @dataclass(frozen=True) 会让实例不可变


# ---------- __post_init__ —— 初始化后处理 ----------
@dataclass
class Rectangle:
    """矩形 —— 使用 __post_init__ 做验证和计算"""
    width: float
    height: float
    area: float = field(init=False)  # 不参与 __init__

    def __post_init__(self):
        """__init__ 之后自动调用"""
        if self.width <= 0 or self.height <= 0:
            raise ValueError("宽高必须为正数")
        self.area = self.width * self.height


r = Rectangle(3.0, 4.0)
print(r.area)                 # → 12.0


# ---------- 不可变数据类 ----------
from dataclasses import dataclass

@dataclass(frozen=True)
class Config:
    """配置类 —— 不可变"""
    host: str = "localhost"
    port: int = 8080
    debug: bool = False

config = Config()
# config.port = 9090          # ❌ FrozenInstanceError: 不能修改


# ---------- dataclass 常用选项 ----------
"""
@dataclass 参数：
    init=True       → 生成 __init__
    repr=True       → 生成 __repr__
    eq=True         → 生成 __eq__
    order=False     → 生成比较运算符
    frozen=False    → 不可变
    slots=False     → Python 3.10+: 使用 __slots__

field() 参数：
    default         → 默认值
    default_factory → 默认值工厂（用于可变类型）
    init=True       → 是否参与 __init__
    repr=True       → 是否在 __repr__ 中包含
    hash=True       → 是否参与 __hash__
    compare=True    → 是否参与比较
    metadata={}     → 元数据（用于其他框架）
"""
```

### 3.4 `@dataclass` 与 `field` 高级用法

```python
from dataclasses import dataclass, field
from typing import List, Dict
from uuid import uuid4


# ---------- default_factory —— 处理可变默认值 ----------
@dataclass
class Student:
    name: str
    grades: List[int] = field(default_factory=list)
    # ❌ 错误的做法：grades: List[int] = []（所有实例共享同一个列表）
    tags: Dict[str, str] = field(default_factory=dict)


s1 = Student("Alice")
s2 = Student("Bob")
s1.grades.append(95)
print(s2.grades)              # → []（正确，没有共享）


# ---------- init=False —— 自动生成的字段 ----------
@dataclass
class Order:
    """订单 —— 自动生成 ID 和创建时间"""
    product: str
    quantity: int

    # 自动生成，不参与 __init__
    order_id: str = field(init=False, default_factory=lambda: f"ORD-{uuid4().hex[:8]}")
    total: float = field(init=False)

    # 默认值由 __post_init__ 设置
    def __post_init__(self):
        self.total = self.quantity * 10.0  # 假设单价 10


order = Order("Widget", 5)
print(order.order_id)         # → ORD-3f8a2c1b（自动生成）
print(order.total)            # → 50.0


# ---------- metadata —— 元数据 ----------
@dataclass
class User:
    name: str = field(metadata={"label": "用户名", "max_length": 50})
    age: int = field(metadata={"label": "年龄", "min": 0, "max": 150})

    def get_field_info(self, field_name: str) -> dict:
        """获取字段的元数据"""
        from dataclasses import fields
        for f in fields(self):
            if f.name == field_name:
                return f.metadata
        return {}


user = User("Alice", 25)
print(user.get_field_info("name"))  # → {'label': '用户名', 'max_length': 50}
```

---

## 4. 元类（Metaclass）

### 4.1 什么是元类

```python
"""
元类（Metaclass）是"创建类的类"。

Python 中一切都是对象：
- 实例是类的对象
- 类是元类的对象

默认的元类是 type：
type(实例) → 类
type(类)   → <class 'type'>

创建类的三种方式：
1. class 语句：class Foo: pass
2. type('名称', (基类,), {属性字典})
3. 自定义元类
"""

# ---------- 万物皆对象 ----------
class MyClass:
    pass

obj = MyClass()

print(type(obj))              # → <class '__main__.MyClass'>
print(type(MyClass))          # → <class 'type'>
print(type(type))             # → <class 'type'>（type 是自己的元类）


# ---------- 使用 type 动态创建类 ----------
# type('类名', (基类元组,), {属性字典})

def say_hello(self):
    return f"Hello, {self.name}!"

# 动态创建一个 Person 类
Person = type("Person", (object,), {
    "species": "Human",
    "__init__": lambda self, n: setattr(self, "name", n),
    "say_hello": say_hello,
})

p = Person("Alice")
print(p.say_hello())          # → Hello, Alice!
print(type(p))                # → <class '__main__.Person'>
print(type(Person))           # → <class 'type'>
```

### 4.2 自定义元类

```python
class MetaLogger(type):
    """
    元类：记录所有类的创建信息
    继承 type → 这就是一个元类

    __new__ 在类创建时调用
    参数：
        mcs     → 元类本身
        name    → 要创建的类名
        bases   → 基类元组
        namespace → 类的所有属性和方法
    """

    def __new__(mcs, name: str, bases: tuple, namespace: dict):
        """
        创建类时调用
        在 class 语句执行时触发
        """
        print(f"[元类] 创建类: {name}")
        print(f"  基类: {[b.__name__ for b in bases]}")
        print(f"  属性: {list(namespace.keys())}")

        # 可以在创建时修改类的定义
        if "created_by" not in namespace:
            namespace["created_by"] = "MetaLogger"

        # 添加一个类方法
        namespace["class_info"] = classmethod(
            lambda cls: f"{cls.__name__} (元类: {type(cls).__name__})"
        )

        # 调用父类的 __new__ 创建类
        return super().__new__(mcs, name, bases, namespace)

    # ---------- __call__：控制类实例化 ----------
    def __call__(cls, *args, **kwargs):
        """
        当类的实例被创建时调用
        即 cls(*args, **kwargs) 时触发
        """
        print(f"[元类] 实例化: {cls.__name__}({args}, {kwargs})")
        instance = super().__call__(*args, **kwargs)
        print(f"  实例: {instance}")
        return instance


# 使用元类
class MyService(metaclass=MetaLogger):
    """使用 MetaLogger 元类"""

    def __init__(self, name: str):
        self.name = name


# 输出：
# [元类] 创建类: MyService
#   基类: ['object']
#   属性: ['__module__', '__qualname__', '__init__']

s = MyService("数据库服务")
# 输出：
# [元类] 实例化: MyService(('数据库服务',), {})

print(MyService.created_by)   # → MetaLogger（元类添加的属性）
print(MyService.class_info()) # → MyService (元类: MetaLogger)
```

### 4.3 元类的实际应用

```python
# ---------- 单例元类 ----------
class SingletonMeta(type):
    """单例模式的元类实现"""

    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super().__call__(*args, **kwargs)
        return cls._instances[cls]


class Database(metaclass=SingletonMeta):
    """数据库连接（单例）"""

    def __init__(self, name: str):
        self.name = name

    def __repr__(self):
        return f"Database({self.name}, id={id(self)})"


db1 = Database("主库")
db2 = Database("从库")
print(db1 is db2)             # → True（都是同一个实例）
print(db1.name)               # → "主库"（第二次的构造参数被忽略）


# ---------- ORM 风格的元类 ----------
class ModelMeta(type):
    """
    ORM 元类：自动处理表名、字段验证等
    类似 Django ORM / SQLAlchemy 的实现思路
    """

    def __new__(mcs, name, bases, namespace):
        if name == "Model":
            return super().__new__(mcs, name, bases, namespace)

        # 收集字段定义
        fields = {}
        for key, value in list(namespace.items()):
            if isinstance(value, Field):
                fields[key] = value
                value.name = key
                namespace.pop(key)

        namespace["_fields"] = fields
        namespace["_table_name"] = name.lower() + "s"

        return super().__new__(mcs, name, bases, namespace)


class Field:
    """字段定义"""

    def __init__(self, field_type: type, required: bool = True, default=None):
        self.field_type = field_type
        self.required = required
        self.default = default
        self.name = ""

    def validate(self, value) -> bool:
        if value is None and not self.required:
            return True
        if not isinstance(value, self.field_type):
            raise TypeError(
                f"字段 '{self.name}' 期望 {self.field_type.__name__}, "
                f"实际 {type(value).__name__}"
            )
        return True


class Model(metaclass=ModelMeta):
    """模型基类"""

    def __init__(self, **kwargs):
        for name, field in self._fields.items():
            if name in kwargs:
                value = kwargs[name]
                field.validate(value)
                setattr(self, name, value)
            elif field.default is not None:
                setattr(self, name, field.default)
            elif not field.required:
                setattr(self, name, None)
            else:
                raise ValueError(f"缺少必填字段: {name}")

    def save(self):
        """模拟保存到数据库"""
        fields = []
        values = []
        for name, field in self._fields.items():
            value = getattr(self, name, field.default)
            fields.append(name)
            values.append(repr(value))

        sql = (f"INSERT INTO {self._table_name} "
               f"({', '.join(fields)}) VALUES ({', '.join(values)})")
        print(f"[SQL] {sql}")
        return sql

    def __repr__(self):
        fields = ", ".join(
            f"{k}={getattr(self, k, None)}"
            for k in self._fields
        )
        return f"{self.__class__.__name__}({fields})"


# 使用 ORM
class User(Model):
    """用户模型"""
    name = Field(str, required=True)
    age = Field(int, required=True)
    email = Field(str, required=False, default=None)


user = User(name="Alice", age=25)
print(user)
user.save()

# user2 = User(name="Bob", age="twenty")  # ❌ TypeError: 字段 'age' 期望 int
```

### 4.4 元类 vs 装饰器 vs 继承

```python
"""
什么时候用元类？

元类的适用场景（少，但一旦需要，没有更好的替代方案）：
1. 框架/库的底层实现（如 Django ORM、SQLAlchemy）
2. 注册模式（自动注册子类）
3. 修改类的创建行为（如单例、接口检查）
4. 阻止继承或修改继承行为

不需要用元类的情况（用更简单的方式替代）：
1. 修改方法：用装饰器
2. 修改属性：用描述符
3. 类注册：用 __init_subclass__
4. 简单配置：用 @dataclass
"""


# ---------- 用 __init_subclass__ 代替简单元类 ----------
class PluginBase:
    """插件基类 —— 自动注册所有子类"""

    _registry = {}

    def __init_subclass__(cls, **kwargs):
        """子类被创建时自动调用（Python 3.6+）"""
        super().__init_subclass__(**kwargs)
        PluginBase._registry[cls.__name__] = cls
        print(f"注册插件: {cls.__name__}")


class AudioPlugin(PluginBase):
    pass

class VideoPlugin(PluginBase):
    pass

print(PluginBase._registry)
# → {'AudioPlugin': <class 'AudioPlugin'>, 'VideoPlugin': <class 'VideoPlugin'>}
```

---

## 5. 反射与内省

### 5.1 内省基础

```python
"""
内省（Introspection）：程序运行时检查对象的类型、属性、方法等。
反射（Reflection）：程序运行时动态获取和修改对象的状态和行为。

Python 中一切皆对象，所有对象都可以被内省。
"""


class Sample:
    """示例类"""

    class_attr = "类属性"

    def __init__(self, value):
        self.instance_attr = value

    def method(self):
        return "实例方法"

    @classmethod
    def class_method(cls):
        return "类方法"

    @staticmethod
    def static_method():
        return "静态方法"


obj = Sample("测试值")

# ---------- 基本内省 ----------
print(type(obj))              # → <class '__main__.Sample'>
print(id(obj))                # → 对象的内存地址
print(hash(obj))              # → 哈希值（如果可哈希）
print(isinstance(obj, Sample))  # → True


# ---------- 检查属性/方法 ----------
print(hasattr(obj, "class_attr"))   # → True
print(hasattr(obj, "nonexistent"))  # → False

print(getattr(obj, "instance_attr"))  # → "测试值"
print(getattr(obj, "nonexistent", "默认值"))  # → "默认值"（不存在的属性返回默认值）

setattr(obj, "new_attr", "新属性")   # 动态设置属性
print(obj.new_attr)                  # → "新属性"

delattr(obj, "new_attr")             # 删除属性


# ---------- dir() —— 列出所有属性 ----------
print(dir(obj))
# → ['class_attr', 'class_method', 'instance_attr', 'method', 'static_method', ...]
# 包含继承自 object 的方法


# ---------- vars() —— 返回实例的 __dict__ ----------
print(vars(obj))
# → {'instance_attr': '测试值'}
print(vars(Sample))
# → {'__module__': '__main__', 'class_attr': '类属性', ...}
```

### 5.2 检查类的结构

```python
class MyClass:
    """这是我的类"""

    x: int = 10
    y: str = "hello"

    def __init__(self, z: float):
        self.z = z

    def method(self, a: int, b: str) -> bool:
        return True

    @classmethod
    def cm(cls):
        pass

    @staticmethod
    def sm():
        pass


obj = MyClass(3.14)

# ---------- 检查类 ----------
print(obj.__class__.__name__)       # → MyClass
print(MyClass.__name__)             # → MyClass
print(MyClass.__module__)           # → __main__
print(MyClass.__doc__)              # → 这是我的类
print(MyClass.__dict__)             # → 类的所有属性（映射代理）

# ---------- 检查方法 ----------
import inspect

print(inspect.isfunction(MyClass.method))      # → True（普通方法）
print(inspect.ismethod(obj.method))             # → True（绑定方法）
print(inspect.isclass(MyClass))                 # → True
print(inspect.ismodule(inspect))                # → True（模块也是对象）

# ---------- 检查源代码 ----------
print(inspect.getsource(MyClass.method))
# 打印 method 的源代码

print(inspect.getfile(MyClass))                 # 类定义的文件路径
print(inspect.getmembers(obj, inspect.ismethod))  # 获取所有方法

# ---------- 签名 ----------
sig = inspect.signature(MyClass.method)
print(sig)                          # → (self, a: int, b: str) -> bool

for name, param in sig.parameters.items():
    print(f"  参数 {name}:")
    print(f"    默认值: {param.default}")
    print(f"    注解: {param.annotation}")
    print(f"    种类: {param.kind.name}")

print(sig.return_annotation)        # → <class 'bool'>
```

### 5.3 动态导入与调用

```python
# ---------- 动态导入模块 ----------
import importlib

# 相当于 import math
math = importlib.import_module("math")
print(math.sqrt(16))               # → 4.0

# 动态导入子模块
# 相当于 from os import path
path = importlib.import_module("os.path")
print(path.join("a", "b"))         # → a/b


# ---------- 动态调用函数 ----------
def execute(func_name: str, *args, **kwargs):
    """根据函数名动态调用"""
    # 在当前模块查找函数
    import sys
    current_module = sys.modules[__name__]
    func = getattr(current_module, func_name, None)

    if func is None:
        raise ImportError(f"函数 {func_name} 不存在")

    if not callable(func):
        raise TypeError(f"{func_name} 不可调用")

    return func(*args, **kwargs)


def add(a: int, b: int) -> int:
    return a + b


print(execute("add", 3, 4))        # → 7


# ---------- 动态创建实例 ----------
class Factory:
    """工厂类 —— 根据字符串创建对象"""

    _classes = {}

    @classmethod
    def register(cls, name: str, klass: type):
        """注册类"""
        cls._classes[name] = klass

    @classmethod
    def create(cls, name: str, *args, **kwargs):
        """根据名称创建实例"""
        if name not in cls._classes:
            raise ValueError(f"未知类型: {name}")
        return cls._classes[name](*args, **kwargs)


class Cat:
    def __init__(self, name: str):
        self.name = name

    def speak(self):
        return "喵~"


class Dog:
    def __init__(self, name: str):
        self.name = name

    def speak(self):
        return "汪汪！"


# 注册类型
Factory.register("cat", Cat)
Factory.register("dog", Dog)

# 动态创建
animal = Factory.create("cat", "小花")
print(animal.speak())              # → 喵~
```

### 5.4 inspect 模块详解

```python
import inspect
import asyncio


class Inspector:
    """使用 inspect 模块进行全面的对象内省"""

    @staticmethod
    def analyze(obj, name: str = "obj"):
        """分析任意对象"""
        print(f"\n{'='*60}")
        print(f"分析对象: {name}")
        print(f"{'='*60}")

        # 基本类型
        print(f"类型: {type(obj).__name__}")
        print(f"ID: {id(obj)}")

        # 类别检查
        if inspect.isfunction(obj):
            print("类别: 函数")
            sig = inspect.signature(obj)
            print(f"签名: {sig}")
            print(f"源代码行: {inspect.getsourcelines(obj)[0]}")
        elif inspect.isclass(obj):
            print("类别: 类")
            print(f"MRO: {[c.__name__ for c in inspect.getmro(obj)]}")
            methods = inspect.getmembers(obj, inspect.isfunction)
            print(f"方法: {[m[0] for m in methods]}")
        elif inspect.ismodule(obj):
            print("类别: 模块")
            print(f"文件: {inspect.getfile(obj)}")
        elif inspect.iscoroutinefunction(obj):
            print("类别: 协程函数")
        elif inspect.isgeneratorfunction(obj):
            print("类别: 生成器函数")

        # 检查可调用性
        if callable(obj):
            try:
                sig = inspect.signature(obj)
                print(f"可调用 → 签名: {sig}")
                for name, param in sig.parameters.items():
                    if param.default is inspect.Parameter.empty:
                        print(f"  必填参数: {name}: {param.annotation}")
                    else:
                        print(f"  可选参数: {name}={param.default}: {param.annotation}")
                if sig.return_annotation is not inspect.Parameter.empty:
                    print(f"  返回类型: {sig.return_annotation}")
            except (ValueError, TypeError):
                print("可调用（无法获取签名）")


# 使用
Inspector.analyze(print, "print")
Inspector.analyze(Inspector, "Inspector")

import json
Inspector.analyze(json, "json")


# ---------- 获取调用栈 ----------
def func_a():
    return func_b()

def func_b():
    return func_c()

def func_c():
    """获取调用栈"""
    print("\n调用栈:")
    for frame_info in inspect.stack():
        print(f"  {frame_info.filename}:{frame_info.lineno} "
              f"in {frame_info.function}()")

func_a()
```

### 5.5 反射实战：通用的序列化器

```python
"""
反射实战：通用的对象序列化/反序列化工具
支持将任意 Python 对象转为字典，以及从字典恢复对象
"""

import inspect
from datetime import datetime
from typing import get_type_hints, Any


class ObjectSerializer:
    """
    通用的对象序列化器
    利用反射自动处理任意对象
    """

    @classmethod
    def to_dict(cls, obj: Any, depth: int = 3) -> dict:
        """
        将任意对象序列化为字典

        Args:
            obj: 要序列化的对象
            depth: 递归深度限制

        Returns:
            字典表示
        """
        if depth <= 0:
            return {"__type__": "truncated"}

        result = {}

        # 1. 包含类型信息
        result["__type__"] = obj.__class__.__name__
        result["__module__"] = obj.__class__.__module__

        # 2. 处理实例属性
        if hasattr(obj, "__dict__"):
            for key, value in obj.__dict__.items():
                # 跳过私有属性
                if key.startswith("_"):
                    continue
                result[key] = cls._serialize_value(value, depth - 1)

        # 3. 处理 @property
        for name, value in inspect.getmembers(obj, lambda x: not callable(x)):
            if name.startswith("_"):
                continue
            if name not in result:
                try:
                    result[name] = cls._serialize_value(value, depth - 1)
                except Exception:
                    result[name] = "<不可序列化>"

        return result

    @classmethod
    def _serialize_value(cls, value: Any, depth: int) -> Any:
        """递归序列化值"""
        if value is None:
            return None
        if isinstance(value, (str, int, float, bool)):
            return value
        if isinstance(value, datetime):
            return value.isoformat()
        if isinstance(value, (list, tuple)):
            return [cls._serialize_value(item, depth) for item in value]
        if isinstance(value, dict):
            return {k: cls._serialize_value(v, depth) for k, v in value.items()}
        if hasattr(value, "__dict__"):
            return cls.to_dict(value, depth)
        return str(value)

    @classmethod
    def from_dict(cls, data: dict) -> Any:
        """
        从字典反序列化为对象（简化版）

        注意：这只是一个简化实现，完整实现需要维护类型注册表
        """
        if "__type__" not in data:
            return data

        # 查找模块中的类
        import sys
        module_name = data.get("__module__", "__main__")
        class_name = data["__type__"]

        try:
            module = sys.modules.get(module_name)
            if module is None:
                import importlib
                module = importlib.import_module(module_name)

            klass = getattr(module, class_name)
        except (ImportError, AttributeError):
            return data  # 无法恢复，返回原始字典

        # 尝试构造实例
        try:
            # 收集构造函数参数
            sig = inspect.signature(klass.__init__)
            kwargs = {}
            for name, param in sig.parameters.items():
                if name == "self":
                    continue
                if name in data:
                    kwargs[name] = data[name]
                elif param.default is not inspect.Parameter.empty:
                    kwargs[name] = param.default

            obj = klass(**kwargs)

            # 设置其他属性
            for key, value in data.items():
                if key not in ("__type__", "__module__") and key not in kwargs:
                    setattr(obj, key, value)

            return obj
        except Exception as e:
            print(f"反序列化失败: {e}")
            return data


# ---------- 使用示例 ----------
@dataclass
class Product:
    """产品"""
    id: int
    name: str
    price: float
    created_at: datetime = field(default_factory=datetime.now)


class Order:
    """订单"""

    def __init__(self, order_id: str, items: list[Product]):
        self.order_id = order_id
        self.items = items
        self.created_at = datetime.now()

    @property
    def total(self) -> float:
        return sum(item.price for item in self.items)


# 创建对象
product = Product(1, "Python编程", 59.00)
order = Order("ORD-001", [product, Product(2, "算法导论", 89.00)])

# 序列化
order_dict = ObjectSerializer.to_dict(order)
import json
print(json.dumps(order_dict, ensure_ascii=False, indent=2))
# 输出包含所有属性和嵌套对象的字典

# 反序列化
# restored = ObjectSerializer.from_dict(order_dict)
```

---

## 6. 综合示例

```python
"""
综合示例：简易 ORM 框架
演示：元类、类型注解、反射、魔法方法的综合运用
"""

import inspect
import sqlite3
from dataclasses import dataclass, field
from typing import Any, ClassVar, Optional, get_type_hints


# ---------- 字段类型映射 ----------
TYPE_MAP = {
    int: "INTEGER",
    float: "REAL",
    str: "TEXT",
    bool: "INTEGER",    # SQLite 没有布尔类型
    bytes: "BLOB",
    Optional[int]: "INTEGER",
    Optional[str]: "TEXT",
    Optional[float]: "REAL",
}


def resolve_sqlite_type(py_type: type) -> str:
    """将 Python 类型映射到 SQLite 类型"""
    # 处理 Optional[X]
    origin = getattr(py_type, "__origin__", None)
    if origin is not None:
        args = getattr(py_type, "__args__", [])
        if origin is Union and type(None) in args:
            for arg in args:
                if arg is not type(None):
                    return resolve_sqlite_type(arg)

    return TYPE_MAP.get(py_type, "TEXT")


# ---------- 元类：自动生成 SQL ----------
class ModelMeta(type):
    """
    ORM 元类
    在创建 Model 子类时自动分析字段并生成表结构
    """

    def __new__(mcs, name: str, bases: tuple, namespace: dict):
        # 跳过 Model 基类本身
        if name == "Model":
            return super().__new__(mcs, name, bases, namespace)

        # 收集字段信息
        annotations = namespace.get("__annotations__", {})
        fields = []
        table_name = name.lower()

        for field_name, field_type in annotations.items():
            # 跳过 ClassVar
            origin = getattr(field_type, "__origin__", None)
            if origin is ClassVar:
                continue

            sql_type = resolve_sqlite_type(field_type)
            fields.append({
                "name": field_name,
                "type": field_type,
                "sql_type": sql_type,
            })

        namespace["_meta"] = {
            "table_name": table_name,
            "fields": fields,
            "field_names": [f["name"] for f in fields],
        }

        return super().__new__(mcs, name, bases, namespace)


# ---------- 字段描述符 ----------
class Field:
    """
    字段描述符
    用于自动处理属性访问和类型验证
    """

    def __set_name__(self, owner, name):
        self.name = name

    def __get__(self, instance, owner):
        if instance is None:
            return self
        return instance.__dict__.get(self.name)

    def __set__(self, instance, value):
        instance.__dict__[self.name] = value


# ---------- 基础 Model 类 ----------
class Model(metaclass=ModelMeta):
    """所有 ORM 模型的基类"""

    _connection: ClassVar[Optional[sqlite3.Connection]] = None
    _meta: ClassVar[dict] = {}

    @classmethod
    def connect(cls, db_path: str = ":memory:"):
        """连接数据库"""
        cls._connection = sqlite3.connect(db_path)
        cls._connection.row_factory = sqlite3.Row
        return cls._connection

    @classmethod
    def create_table(cls):
        """根据元信息创建表"""
        if cls._connection is None:
            raise RuntimeError("请先调用 connect()")

        meta = cls._meta
        columns = []
        for f in meta["fields"]:
            col = f"{f['name']} {f['sql_type']}"
            columns.append(col)

        # 默认添加 id 主键
        sql = (f"CREATE TABLE IF NOT EXISTS {meta['table_name']} "
               f"(id INTEGER PRIMARY KEY AUTOINCREMENT, {', '.join(columns)})")

        cls._connection.execute(sql)
        cls._connection.commit()
        return sql

    def save(self):
        """保存到数据库"""
        if self._connection is None:
            raise RuntimeError("请先调用 connect()")

        meta = self._meta
        values = [getattr(self, name) for name in meta["field_names"]]
        placeholders = ", ".join("?" for _ in values)
        columns = ", ".join(meta["field_names"])

        sql = (f"INSERT INTO {meta['table_name']} "
               f"({columns}) VALUES ({placeholders})")

        cursor = self._connection.execute(sql, values)
        self._connection.commit()
        self.id = cursor.lastrowid
        return self.id

    @classmethod
    def find(cls, **kwargs) -> list["Model"]:
        """根据条件查找"""
        if cls._connection is None:
            raise RuntimeError("请先连接数据库")

        if kwargs:
            conditions = " AND ".join(f"{k} = ?" for k in kwargs)
            values = list(kwargs.values())
            sql = f"SELECT * FROM {cls._meta['table_name']} WHERE {conditions}"
            cursor = cls._connection.execute(sql, values)
        else:
            sql = f"SELECT * FROM {cls._meta['table_name']}"
            cursor = cls._connection.execute(sql)

        results = []
        for row in cursor.fetchall():
            obj = cls.__new__(cls)
            for key in row.keys():
                object.__setattr__(obj, key, row[key])
            results.append(obj)

        return results

    def delete(self):
        """删除记录"""
        if not hasattr(self, "id"):
            raise ValueError("未保存的记录无法删除")

        sql = f"DELETE FROM {self._meta['table_name']} WHERE id = ?"
        self._connection.execute(sql, (self.id,))
        self._connection.commit()

    def __repr__(self):
        meta = self._meta
        fields = ", ".join(
            f"{name}={getattr(self, name, None)}"
            for name in meta["field_names"]
        )
        return f"{self.__class__.__name__}({fields})"


# ---------- 使用 ORM ----------
# 1. 定义模型
class User(Model):
    """用户表"""
    name: str
    email: str
    age: Optional[int] = None


class Article(Model):
    """文章表"""
    title: str
    content: str
    author_id: int
    published: bool = False  # 默认值


# 2. 连接数据库并创建表
User.connect("test.db")
User.create_table()
Article.create_table()

# 3. 插入数据
user = User()
user.name = "Alice"
user.email = "alice@example.com"
user.age = 25
user.save()
print(f"用户保存成功，ID: {user.id}")

article = Article()
article.title = "Python 元类详解"
article.content = "元类是 Python 的高级特性..."
article.author_id = user.id
article.published = True
article.save()

# 4. 查询
users = User.find()
print(f"所有用户: {users}")

alice = User.find(name="Alice")
print(f"找到 Alice: {alice}")

articles = Article.find(author_id=user.id)
print(f"用户文章: {articles}")

# 5. 删除
# article.delete()


# ---------- 关闭连接 ----------
if User._connection:
    User._connection.close()
    import os
    os.remove("test.db")
```

---

> 本文档涵盖了 Python 三大核心机制：
> - **魔法方法**：控制对象各种行为的钩子
> - **类型提示**：让 Python 代码更具可读性和可维护性
> - **元类与反射**：Python 元编程的基础
>
> 掌握这些内容后，你就能深入理解 Python 的设计哲学，
> 并能编写出更优雅、更灵活的代码。
