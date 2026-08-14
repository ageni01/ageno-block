# Python 进阶详解

> 本文档深入讲解 Python 的面向对象编程（OOP）和文件输入输出，涵盖类与对象的核心机制、继承、多态、异常处理以及各种文件操作。

---

## 目录

1. [面向对象基础](#1-面向对象基础)
2. [继承与多态](#2-继承与多态)
3. [封装与属性](#3-封装与属性)
4. [魔法方法（部分）](#4-魔法方法部分)
5. [异常处理](#5-异常处理)
6. [文件输入输出](#6-文件输入输出)
7. [上下文管理器](#7-上下文管理器)
8. [综合示例](#8-综合示例)

---

## 1. 面向对象基础

### 1.1 类与实例

```python
class Dog:
    """狗类 —— 演示类的基本定义"""

    # ---------- 类属性（所有实例共享） ----------
    species = "Canis familiaris"
    # 类属性的典型用途：常量、默认值、计数器
    dog_count = 0

    # ---------- __init__ —— 构造方法（实例化时自动调用） ----------
    def __init__(self, name: str, age: int):
        """
        实例初始化方法
        注意：__init__ 不是构造器（构造器是 __new__），而是初始化器

        Args:
            name: 狗的名字
            age: 狗的年龄（年）
        """
        # 实例属性（每个实例独有）
        self.name = name
        self.age = age
        self.energy = 100  # 默认值
        self.tricks = []   # 可变对象作为实例属性是安全的

        # 类属性可以通过 self 访问，但通过类名访问更清晰
        Dog.dog_count += 1  # 每创建一个实例，计数器加 1

    # ---------- 实例方法 ----------
    def bark(self, times: int = 1) -> str:
        """
        狗叫方法
        第一个参数永远是 self，指向实例本身

        Args:
            times: 叫的次数

        Returns:
            叫声字符串
        """
        return "Woof! " * times

    def learn_trick(self, trick: str) -> None:
        """学习新技能"""
        self.tricks.append(trick)
        # 注意：不 return 的方法返回 None

    def run(self, distance: int) -> None:
        """
        跑步消耗体力
        如果体力不足，抛出异常（演示异常处理）
        """
        cost = distance * 2
        if cost > self.energy:
            raise ValueError(f"{self.name} 太累了，无法跑 {distance} 米")
        self.energy -= cost
        print(f"{self.name} 跑了 {distance} 米，剩余体力 {self.energy}")

    # ---------- 类方法（@classmethod） ----------
    @classmethod
    def create_puppy(cls, name: str) -> "Dog":
        """
        类方法：创建一只幼犬（age 固定为 0）
        第一个参数是 cls，指向类本身（而不是实例）

        类方法的典型用途：
        1. 备选的构造函数
        2. 操作类属性
        """
        return cls(name, age=0)

    # ---------- 静态方法（@staticmethod） ----------
    @staticmethod
    def get_species_info() -> str:
        """
        静态方法：不需要 self 或 cls
        就是放在类命名空间中的普通函数
        典型用途：与类相关但不依赖实例/类状态的工具函数
        """
        return f"狗属于 {Dog.species}"

    # ---------- 字符串表示方法 ----------
    def __str__(self) -> str:
        """给用户看的字符串表示（print 时调用）"""
        return f"{self.name}（{self.age} 岁）"

    def __repr__(self) -> str:
        """给开发者看的字符串表示（repr、调试时调用）"""
        return f"Dog(name='{self.name}', age={self.age})"


# ---------- 使用示例 ----------
# 创建实例
buddy = Dog("Buddy", 3)
max_dog = Dog("Max", 5)

print(buddy)                    # → Buddy（3 岁）（调用 __str__）
print(repr(buddy))              # → Dog(name='Buddy', age=3)（调用 __repr__）

print(buddy.bark(3))            # → Woof! Woof! Woof!
buddy.learn_trick("坐下")
buddy.learn_trick("打滚")
print(buddy.tricks)             # → ['坐下', '打滚']

# 类方法的使用
puppy = Dog.create_puppy("小黄")
print(puppy)                    # → 小黄（0 岁）

# 类属性的访问
print(Dog.dog_count)            # → 3（总共创建了 3 个实例）

# 静态方法的调用
print(Dog.get_species_info())   # → 狗属于 Canis familiaris
```

### 1.2 self 与 __init__ 详解

```python
class Student:
    """
    self 详解：
    - self 是约定俗成的名字，不是关键字（用 this 也行，但千万别）
    - 当调用实例方法时，Python 自动将实例作为第一个参数传入
    - 所以实例方法必须至少有一个参数（通常是 self）
    """

    school = "第一中学"          # 类属性：所有学生共享

    def __init__(self, name, grade):
        """
        __init__ 的工作：
        1. Python 先调用 __new__ 创建实例
        2. 然后调用 __init__ 初始化实例
        3. self 就是 __new__ 创建的那个实例

        注意：__init__ 不能有 return（除 None 外）
        """
        self.name = name        # 实例属性
        self.grade = grade
        self._scores = []       # 约定：_ 开头的属性是"受保护的"

    def introduce(self):
        """
        实例方法：需要 self 参数
        Python 会将调用者作为 self 传入
        """
        return f"我叫{self.name}，在{self.school}读{self.grade}"

    # def introduce(self) 等价于 Student.introduce(s)
    # 下面演示手动传 self 的等效调用

s = Student("小明", "三年级")
print(s.introduce())            # 正常调用，Python 自动传 self
print(Student.introduce(s))     # 等效调用，手动传 self
```

### 1.3 可见性与命名约定

```python
class AccessDemo:
    """
    Python 没有真正的 private，全靠约定和 name mangling
    """

    def __init__(self):
        self.public = "公开的"            # 公开属性
        self._protected = "受保护的"       # 约定：_ 开头表示"不要直接访问"
        self.__private = "私有的"          # 名称改写（name mangling）

    def get_private(self):
        """公开的 getter 方法"""
        return self.__private

# 测试
obj = AccessDemo()
print(obj.public)               # → 公开的（可以访问）
print(obj._protected)           # → 受保护的（技术上可以访问，但不要）
# print(obj.__private)          # ❌ AttributeError（名称已被改写）

# 名称改写的原理：_类名__属性名
print(obj._AccessDemo__private) # → 私有的（通过改写后的名称可以访问，但不要这么做）

# 总结 Python 的可见性约定：
"""
名称            访问级别         说明
name            公开            可以随意访问
_name           受保护          不要从类外部直接访问（需要 getter/setter）
__name          私有（改名）     会被改写为 _ClassName__name
__name__        魔法方法         不要自定义这种命名
"""
```

---

## 2. 继承与多态

### 2.1 基本继承

```python
# ---------- 基类（父类） ----------
class Animal:
    """所有动物的基类"""

    def __init__(self, name: str):
        self.name = name

    def make_sound(self) -> str:
        """发出声音 —— 基类定义默认行为"""
        return "...（沉默）"

    def move(self) -> str:
        """移动"""
        return f"{self.name} 在移动"

    def __str__(self):
        return f"{self.name}（{self.__class__.__name__}）"


# ---------- 派生类（子类） ----------
class Cat(Animal):
    """猫 —— 继承自 Animal"""

    def __init__(self, name: str, color: str):
        """
        子类的 __init__ 必须调用父类的 __init__
        """
        # 方式 1：直接调用父类 __init__
        super().__init__(name)      # 推荐方式
        # 方式 2：Animal.__init__(self, name)  # 不推荐，硬编码类名

        self.color = color          # 子类自己的属性

    # 重写（override）父类方法
    def make_sound(self) -> str:
        return "喵~喵~"

    # 子类新增方法
    def purr(self) -> str:
        return "咕噜咕噜（表示舒服）"


class Dog(Animal):
    """狗 —— 继承自 Animal"""

    def __init__(self, name: str, breed: str):
        super().__init__(name)
        self.breed = breed

    def make_sound(self) -> str:
        return "汪汪！"

    def wag_tail(self) -> str:
        return f"{self.name} 摇尾巴了"


# ---------- 使用 ----------
animals = [
    Cat("小白", "白色"),
    Dog("旺财", "金毛"),
    Animal("未知生物"),
]

# 多态：同样的接口，不同的行为
for animal in animals:
    print(f"{animal}: {animal.make_sound()}")
# 输出:
# 小白（Cat）: 喵~喵~
# 旺财（Dog）: 汪汪！
# 未知生物（Animal）: ...（沉默）

# 类型检查
print(isinstance(Cat("M", "黑"), Animal))  # → True
print(isinstance(Cat("M", "黑"), Cat))     # → True
print(isinstance(Cat("M", "黑"), Dog))     # → False
print(issubclass(Cat, Animal))             # → True
```

### 2.2 super() 详解

```python
class A:
    def __init__(self):
        print("A.__init__")
        self.a_value = "A"


class B(A):
    def __init__(self):
        print("B.__init__ 开始")
        super().__init__()    # 调用 A.__init__
        print("B.__init__ 结束")
        self.b_value = "B"


class C(A):
    def __init__(self):
        print("C.__init__ 开始")
        super().__init__()    # 调用 A.__init__
        print("C.__init__ 结束")
        self.c_value = "C"


class D(B, C):
    """
    多继承 —— 使用 MRO（方法解析顺序）决定调用顺序
    MRO 遵循 C3 线性化算法
    """
    def __init__(self):
        print("D.__init__ 开始")
        # super() 遵循 MRO，不会重复调用父类
        super().__init__()
        print("D.__init__ 结束")
        self.d_value = "D"


print(D.__mro__)
# → (<class '__main__.D'>, <class '__main__.B'>,
#    <class '__main__.C'>, <class '__main__.A'>, <class 'object'>)

d = D()
# 输出:
# D.__init__ 开始
# B.__init__ 开始
# C.__init__ 开始
# A.__init__ 开始
# A.__init__ 结束
# C.__init__ 结束
# B.__init__ 结束
# D.__init__ 结束

# MRO（Method Resolution Order）保证了每个父类的 __init__ 只调用一次
# super() 不是"调用父类"，而是"按照 MRO 调用下一个类"
```

### 2.3 抽象基类（ABC）

```python
from abc import ABC, abstractmethod


class Shape(ABC):
    """
    抽象基类：不能实例化，只提供接口规范
    所有子类**必须**实现抽象方法
    """

    @abstractmethod
    def area(self) -> float:
        """计算面积（子类必须实现）"""
        pass

    @abstractmethod
    def perimeter(self) -> float:
        """计算周长（子类必须实现）"""
        pass

    # 非抽象方法：子类可以继承或重写
    def describe(self) -> str:
        return f"这是一个 {self.__class__.__name__}，面积 {self.area():.1f}"


class Circle(Shape):
    """圆形"""

    def __init__(self, radius: float):
        self.radius = radius

    def area(self) -> float:
        return 3.14159 * self.radius ** 2

    def perimeter(self) -> float:
        return 2 * 3.14159 * self.radius


class Rectangle(Shape):
    """矩形"""

    def __init__(self, width: float, height: float):
        self.width = width
        self.height = height

    def area(self) -> float:
        return self.width * self.height

    def perimeter(self) -> float:
        return 2 * (self.width + self.height)


# 使用
shapes: list[Shape] = [Circle(5), Rectangle(3, 4)]

for shape in shapes:
    print(shape.describe())
    print(f"  周长: {shape.perimeter():.1f}")

# s = Shape()  # ❌ TypeError: 不能实例化抽象类
```

### 2.4 Mixin 模式

```python
"""
Mixin：一种通过多重继承实现代码复用的设计模式
- Mixin 类通常只包含方法，不包含 __init__
- Mixin 类名通常以 ...Mixin 或 ...able 结尾
"""


class JsonMixin:
    """为类添加 JSON 序列化功能"""

    def to_json(self) -> str:
        import json
        return json.dumps(self.__dict__, ensure_ascii=False, indent=2)


class LogMixin:
    """为类添加日志功能"""

    def log(self, message: str) -> None:
        import logging
        logger = logging.getLogger(self.__class__.__name__)
        logger.info(f"{self}: {message}")


class TimestampMixin:
    """为类添加时间戳"""

    def __init__(self, *args, **kwargs):
        from datetime import datetime
        super().__init__(*args, **kwargs)
        self.created_at = datetime.now()
        self.updated_at = self.created_at

    def touch(self):
        """更新时间戳"""
        from datetime import datetime
        self.updated_at = datetime.now()


# 组合使用多个 Mixin
class User(JsonMixin, LogMixin, TimestampMixin):
    """用户类 —— 通过 Mixin 获得 JSON 序列化和日志功能"""

    def __init__(self, username: str, email: str):
        # 调用 Mixin 和父类的 __init__
        # 注意 MRO：User → LogMixin → TimestampMixin → object
        super().__init__()
        self.username = username
        self.email = email

    def __str__(self):
        return f"User({self.username})"


user = User("alice", "alice@example.com")
print(user.to_json())        # 来自 JsonMixin
user.log("用户登录")         # 来自 LogMixin
print(user.created_at)       # 来自 TimestampMixin
```

---

## 3. 封装与属性

### 3.1 @property 装饰器

```python
class Temperature:
    """
    温度类 —— 演示 @property 的用法
    内部存储开尔文，但对外提供摄氏度和华氏度接口
    """

    def __init__(self, celsius: float = 0):
        self._celsius = celsius  # 约定：_ 开头表示"内部实现"

    # ---------- Getter ----------
    @property
    def celsius(self) -> float:
        """
        摄氏度的 getter
        使用 @property 后，可以像访问属性一样调用：temp.celsius
        而不是 temp.celsius()
        """
        return self._celsius

    # ---------- Setter ----------
    @celsius.setter
    def celsius(self, value: float) -> None:
        """
        摄氏度的 setter
        允许验证或转换输入值
        """
        if value < -273.15:
            raise ValueError("温度不能低于绝对零度！")
        self._celsius = value

    # ---------- Deleter ----------
    @celsius.deleter
    def celsius(self) -> None:
        """删除属性时的处理（很少用）"""
        print("删除温度值")
        self._celsius = None

    # ---------- 计算属性（只有 getter，没有 setter） ----------
    @property
    def fahrenheit(self) -> float:
        """华氏度 —— 计算属性，只读"""
        return self._celsius * 9 / 5 + 32

    @property
    def kelvin(self) -> float:
        """开尔文 —— 只读"""
        return self._celsius + 273.15


# 使用
t = Temperature(25)

# 像访问属性一样使用（而不是方法调用）
print(t.celsius)               # → 25
print(t.fahrenheit)            # → 77.0

t.celsius = 30                 # 调用 setter
print(t.fahrenheit)            # → 86.0

# t.celsius = -300             # ❌ ValueError: 温度不能低于绝对零度！

# t.fahrenheit = 100           # ❌ AttributeError: 没有 setter


# ---------- 计算属性示例 ----------
class Rectangle:
    """矩形 —— 使用 @property 实现计算属性"""

    def __init__(self, width: float, height: float):
        self.width = width
        self.height = height

    @property
    def area(self) -> float:
        """面积 —— 每次访问都重新计算"""
        return self.width * self.height

    @property
    def perimeter(self) -> float:
        """周长"""
        return 2 * (self.width + self.height)


rect = Rectangle(3, 4)
print(rect.area)                # → 12
rect.width = 5
print(rect.area)                # → 20（自动更新，不需要额外操作）
```

### 3.2 属性访问控制对比

```python
class PropertyDemo:
    """对比不同的属性访问控制方式"""

    def __init__(self):
        # 直接暴露（最简，但无控制）
        self.public = "公开"

        # 受保护属性 + getter/setter 方法（Java 风格）
        self._value = 0

    # ---------- 方式 1：Java 风格的 getter/setter ----------
    def get_value(self):
        return self._value

    def set_value(self, val):
        if val < 0:
            raise ValueError("不能为负")
        self._value = val

    # ---------- 方式 2：Python 风格的 @property ----------
    @property
    def value(self):
        """使用 @property 可以让访问像属性一样自然"""
        return self._value

    @value.setter
    def value(self, val):
        if val < 0:
            raise ValueError("不能为负")
        self._value = val

    # ---------- 方式 3：使用 __slots__ 限制属性 ----------
    """
    __slots__ 告诉 Python 只允许这些属性名
    优点：节省内存（每个实例不用维护 __dict__）
    缺点：不能动态添加其他属性
    """


class SlottedClass:
    """使用 __slots__ 的类"""
    __slots__ = ["x", "y"]      # 只允许 x 和 y 两个属性

    def __init__(self, x, y):
        self.x = x
        self.y = y


s = SlottedClass(1, 2)
print(s.x, s.y)                 # → 1 2
# s.z = 3                       # ❌ AttributeError: 'SlottedClass' has no attribute 'z'
```

---

## 4. 魔法方法（部分）

> 完整的魔法方法列表见 [03-Python核心机制详解.md](03-Python核心机制详解.md)

```python
class Vector:
    """
    二维向量 —— 演示部分魔法方法
    完整的 __xxx__ 见第三篇文档
    """

    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

    # ---------- 字符串表示 ----------
    def __str__(self) -> str:
        """print() 和 str() 调用"""
        return f"({self.x}, {self.y})"

    def __repr__(self) -> str:
        """repr() 和调试时调用，应返回可重建对象的字符串"""
        return f"Vector({self.x!r}, {self.y!r})"

    # ---------- 算术运算 ----------
    def __add__(self, other: "Vector") -> "Vector":
        """实现向量加法：v1 + v2"""
        return Vector(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Vector") -> "Vector":
        """向量减法：v1 - v2"""
        return Vector(self.x - other.x, self.y - other.y)

    def __mul__(self, scalar: float) -> "Vector":
        """标量乘法：v * 3"""
        return Vector(self.x * scalar, self.y * scalar)

    def __truediv__(self, scalar: float) -> "Vector":
        """标量除法：v / 2"""
        return Vector(self.x / scalar, self.y / scalar)

    # ---------- 反向运算 ----------
    def __rmul__(self, scalar: float) -> "Vector":
        """当左操作数不支持乘法时调用：3 * v"""
        return Vector(self.x * scalar, self.y * scalar)

    # ---------- 比较运算 ----------
    def __eq__(self, other) -> bool:
        """v1 == v2（相等判断）"""
        if not isinstance(other, Vector):
            return NotImplemented
        return self.x == other.x and self.y == other.y

    def __lt__(self, other: "Vector") -> bool:
        """v1 < v2（按长度比较）"""
        return self.length < other.length

    # ---------- 其他 ----------
    def __abs__(self) -> float:
        """abs(v)：向量的长度"""
        return (self.x ** 2 + self.y ** 2) ** 0.5

    def __bool__(self) -> bool:
        """bool(v)：零向量为 False"""
        return self.x != 0 or self.y != 0

    def __neg__(self) -> "Vector":
        """-v：取反"""
        return Vector(-self.x, -self.y)

    # 辅助属性
    @property
    def length(self) -> float:
        return abs(self)


# 使用
v1 = Vector(3, 4)
v2 = Vector(1, 2)

print(v1 + v2)                  # → (4, 6)
print(v1 * 3)                   # → (9, 12)
print(3 * v1)                   # → (9, 12)（__rmul__）
print(abs(v1))                  # → 5.0
print(v1 == Vector(3, 4))       # → True
print(bool(Vector(0, 0)))       # → False
```

---

## 5. 异常处理

### 5.1 try / except / else / finally

```python
def divide(a: float, b: float) -> float:
    """
    安全的除法 —— 演示异常处理的完整结构

    Args:
        a: 被除数
        b: 除数

    Returns:
        商

    Raises:
        ValueError: 如果除数为 0
    """
    try:
        # 可能出错的代码
        result = a / b

    except ZeroDivisionError as e:
        # 捕获特定异常
        print(f"错误：不能除以零！({e})")
        raise ValueError("除数为零") from e
        # from e 保留了异常链（原始异常作为 cause）

    except (TypeError, OverflowError) as e:
        # 捕获多种异常
        print(f"类型或溢出错误: {e}")
        raise

    else:
        # 没有异常时执行（可选）
        print(f"除法运算成功！")
        return result

    finally:
        # 无论是否异常都会执行（可选）
        # 典型用途：释放资源、关闭文件
        print("finally 块总是执行")


# ---------- 调用 ----------
try:
    result = divide(10, 0)
except ValueError as e:
    print(f"捕获到: {e}")

# ---------- 使用 else 子句 ----------
# 一个更实际的例子
def process_file(filename: str):
    """处理文件 —— 演示 try/except/else 的合理使用"""
    file = None
    try:
        file = open(filename, "r", encoding="utf-8")
        content = file.read()
    except FileNotFoundError:
        print(f"文件 {filename} 不存在")
        return
    except PermissionError:
        print(f"没有权限读取 {filename}")
        return
    except UnicodeDecodeError:
        print(f"文件编码不是 UTF-8")
        return
    else:
        # 只有没有异常时才执行
        # 放在 else 中意味着不会意外捕获 open/read 之外的异常
        print(f"成功读取 {len(content)} 个字符")
        return content
    finally:
        if file:
            file.close()
            print("文件已关闭")
```

### 5.2 自定义异常

```python
class BankError(Exception):
    """银行操作的基类异常"""
    pass


class InsufficientFundsError(BankError):
    """余额不足异常"""

    def __init__(self, balance: float, amount: float):
        self.balance = balance
        self.amount = amount
        super().__init__(f"余额不足: 当前 {balance}，需要 {amount}")


class AccountNotFoundError(BankError):
    """账户不存在异常"""
    pass


class BankAccount:
    """银行账户 —— 演示自定义异常的使用"""

    def __init__(self, account_id: str, initial_balance: float = 0):
        self.account_id = account_id
        self._balance = initial_balance

    @property
    def balance(self) -> float:
        return self._balance

    def deposit(self, amount: float) -> float:
        """存款"""
        if amount <= 0:
            raise ValueError("存款金额必须为正数")
        self._balance += amount
        return self._balance

    def withdraw(self, amount: float) -> float:
        """取款"""
        if amount <= 0:
            raise ValueError("取款金额必须为正数")
        if amount > self._balance:
            raise InsufficientFundsError(self._balance, amount)
        self._balance -= amount
        return self._balance


# 使用
acc = BankAccount("ACC-001", 1000)

try:
    acc.withdraw(2000)
except InsufficientFundsError as e:
    print(f"交易失败: {e}")
    print(f"当前余额: {e.balance}，想取: {e.amount}")
```

### 5.3 异常链与上下文

```python
import traceback


def inner_func():
    """内部函数：抛出原始异常"""
    try:
        return 1 / 0
    except ZeroDivisionError as e:
        # 隐式异常链：新异常自动包含原异常
        raise ValueError("内部计算出错") from e


def outer_func():
    """外部函数：重新抛出"""
    try:
        inner_func()
    except ValueError as e:
        # 显式异常链
        raise RuntimeError("外层处理失败") from e


# 使用
try:
    outer_func()
except RuntimeError as e:
    print(f"异常: {e}")
    print(f"原始异常: {e.__cause__}")  # from e 设置的 cause
    # 打印完整异常链
    traceback.print_exc()

# 如果不想保留异常链: raise ... from None
# 这会切断异常链
```

---

## 6. 文件输入输出

### 6.1 打开文件的完整模式

```python
"""
文件打开模式（mode 参数）：

'r'     — 只读（默认）。文件必须存在
'w'     — 只写。文件存在则清空，不存在则创建
'a'     — 追加。文件存在则在末尾追加，不存在则创建
'x'     — 独占创建。文件存在则报错

读写组合：
'r+'    — 读写（文件必须存在）
'w+'    — 读写（文件存在则清空）
'a+'    — 读写（追加模式，读从开头，写从末尾）

文件类型：
't'     — 文本模式（默认）
'b'     — 二进制模式

例子：
'rb'    — 二进制只读
'wb'    — 二进制只写
'ab'    — 二进制追加

编码：
encoding='utf-8'  — 文本模式时指定编码（强烈推荐指定）
"""
```

### 6.2 文本文件操作

```python
# ---------- 方式 1：推荐 —— with 语句（自动关闭文件） ----------
# 写入
with open("example.txt", "w", encoding="utf-8") as f:
    f.write("第一行\n")
    f.write("第二行\n")
    f.writelines(["第三行\n", "第四行\n"])
    # write 不会自动添加换行符，需要手动加 \n

# 读取
with open("example.txt", "r", encoding="utf-8") as f:
    content = f.read()          # 读取整个文件为字符串
    print(content)

with open("example.txt", "r", encoding="utf-8") as f:
    lines = f.readlines()       # 读取所有行为列表
    for line in lines:
        print(line.strip())     # strip() 去掉末尾的换行符

# 逐行读取（适合大文件，不会一次性加载到内存）
with open("example.txt", "r", encoding="utf-8") as f:
    for line in f:              # 文件对象本身就是可迭代的
        print(line.strip())

# 追加
with open("example.txt", "a", encoding="utf-8") as f:
    f.write("追加的内容\n")

# ---------- 方式 2：不推荐 —— 手动管理 ----------
f = None
try:
    f = open("example.txt", "r", encoding="utf-8")
    data = f.read()
finally:
    if f:
        f.close()               # 必须手动关闭
```

### 6.3 二进制文件操作

```python
# ---------- 读写二进制文件 ----------
# 写入二进制数据
data = bytes(range(256))          # 创建 0~255 的字节序列
with open("binary.bin", "wb") as f:
    f.write(data)

# 读取二进制文件
with open("binary.bin", "rb") as f:
    content = f.read()
    print(f"读取了 {len(content)} 个字节")  # → 256

# 复制图片（二进制复制）
with open("source.jpg", "rb") as src, open("copy.jpg", "wb") as dst:
    # 分块复制，避免大文件占用太多内存
    while True:
        chunk = src.read(8192)    # 每次读取 8KB
        if not chunk:
            break
        dst.write(chunk)

# ---------- 使用 struct 处理结构化二进制数据 ----------
import struct

# 打包：Python 值 → 二进制
# 格式: 'i' = int(4字节), 'f' = float(4字节), 'd' = double(8字节)
packed = struct.pack("i f d", 42, 3.14, 2.71828)
print(f"打包后长度: {len(packed)} 字节")  # → 16 字节

# 解包：二进制 → Python 值
unpacked = struct.unpack("i f d", packed)
print(unpacked)                   # → (42, 3.140000104904175, 2.71828)
# 注意：float 精度问题，3.14 不能精确表示为二进制浮点数
```

### 6.4 文件的随机访问

```python
# ---------- seek() 和 tell() ----------
with open("example.txt", "w+", encoding="utf-8") as f:
    f.write("Hello, World!")
    f.write("\n第二行")

    # tell()：返回当前指针位置
    print(f.tell())              # → 22（写入后的位置在末尾）

    # seek(offset, whence)：移动指针
    # whence: 0=文件开头, 1=当前位置, 2=文件末尾
    f.seek(0, 0)                # 回到文件开头
    print(f.read())             # 读取全部

    f.seek(7)                   # 移动到第 7 个字符（0 开始）
    print(f.read())             # → World!\n第二行

    f.seek(0, 2)                # 移动到文件末尾
    f.write("\n第三行")          # 在末尾追加

# ---------- 读取文件的指定范围 ----------
def read_chunk(filename: str, start: int, size: int) -> bytes:
    """读取文件的指定字节范围"""
    with open(filename, "rb") as f:
        f.seek(start)
        return f.read(size)
```

### 6.5 临时文件

```python
import tempfile
import os

# ---------- 临时文件（自动删除） ----------
with tempfile.TemporaryFile(mode="w+t") as temp:
    temp.write("临时数据\n")
    temp.seek(0)
    print(temp.read())          # → 临时数据
    # 离开 with 块后文件自动删除

# ---------- 临时文件（指定位置和名称） ----------
with tempfile.NamedTemporaryFile(
    mode="w+t",
    prefix="prefix_",
    suffix=".tmp",
    delete=True                # 是否在关闭时自动删除
) as temp:
    print(f"临时文件路径: {temp.name}")
    temp.write("临时数据")

# ---------- 临时目录 ----------
with tempfile.TemporaryDirectory() as temp_dir:
    print(f"临时目录: {temp_dir}")
    # 在临时目录中创建文件
    test_file = os.path.join(temp_dir, "test.txt")
    with open(test_file, "w") as f:
        f.write("临时目录中的文件")
    # 离开 with 块后目录自动删除
```

### 6.6 文件元数据与路径操作

```python
from pathlib import Path
import os
import stat
import time

p = Path("example.txt")
if p.exists():
    # 文件信息
    print(f"大小: {p.stat().st_size} 字节")
    print(f"修改时间: {time.ctime(p.stat().st_mtime)}")
    print(f"权限: {oct(p.stat().st_mode)[-3:]}")  # → 0o644

    # 检查类型
    print(f"是文件: {p.is_file()}")
    print(f"是目录: {p.is_dir()}")
    print(f"是符号链接: {p.is_symlink()}")

    # 修改权限
    p.chmod(0o644)  # rw-r--r--

# 重命名
p.rename("renamed.txt")

# 符号链接
# p.symlink_to("target.txt")    # 创建符号链接
```

---

## 7. 上下文管理器

### 7.1 with 语句的原理

```python
"""
with 语句的原理：
1. 调用对象的 __enter__ 方法（返回值赋给 as 变量）
2. 执行 with 块中的代码
3. 无论是否异常，都调用 __exit__ 方法
"""

class ManagedResource:
    """
    自定义上下文管理器
    典型场景：管理数据库连接、锁、文件等需要清理的资源
    """

    def __enter__(self):
        """
        进入 with 块时调用
        返回值赋值给 as 后的变量
        """
        print(">>> 进入上下文（__enter__）")
        self.resource = "已打开的资源"
        return self  # 返回 self，as 变量就是这个对象

    def __exit__(self, exc_type, exc_val, exc_tb):
        """
        离开 with 块时调用
        无论是否发生异常都会调用

        Args:
            exc_type: 异常类型（没有异常则为 None）
            exc_val: 异常实例
            exc_tb: 异常追踪信息

        Returns:
            True 表示异常已处理，不会向外传播
            False（或 None）表示异常会继续向外传播
        """
        print("<<< 退出上下文（__exit__）")
        if exc_type:
            print(f"    发生了异常: {exc_type.__name__}: {exc_val}")
            # 返回 True 会抑制异常（通常不这么做）
        self.resource = None  # 清理资源
        return False          # 不抑制异常


# 使用
with ManagedResource() as r:
    print(f"    使用资源: {r.resource}")
    # raise ValueError("出错了")  # 取消注释看看 __exit__ 的行为

print("资源已释放:", r.resource)  # → None
```

### 7.2 contextlib 工具

```python
from contextlib import contextmanager, closing, suppress
import os


# ---------- @contextmanager 装饰器 ----------
# 用生成器的方式定义上下文管理器，比写 __enter__/__exit__ 更简洁

@contextmanager
def managed_file(filename: str, mode: str = "r"):
    """
    使用 @contextmanager 实现文件上下文管理器

    yield 之前的代码 = __enter__
    yield 的值 = as 变量
    yield 之后的代码 = __exit__
    """
    print(f"打开文件: {filename}")
    f = open(filename, mode, encoding="utf-8")
    try:
        yield f                 # ← 这是 __enter__ 的返回值
    finally:
        print(f"关闭文件: {filename}")
        f.close()               # ← 这是 __exit__ 的一部分


# 使用
with managed_file("example.txt", "w") as f:
    f.write("通过上下文管理器写入")


# ---------- @contextmanager 处理异常 ----------
@contextmanager
def transaction(name: str):
    """模拟事务：失败时回滚"""
    print(f"开始事务: {name}")
    try:
        yield
        print(f"事务提交: {name}")       # 没有异常时执行
    except Exception as e:
        print(f"事务回滚: {name}（原因: {e}）")
        raise                         # 重新抛出异常


# ---------- closing —— 自动调用 close() ----------
# 适用于有 close() 方法但不是上下文管理器的对象
class Connection:
    def close(self):
        print("关闭连接")

with closing(Connection()) as conn:
    print("使用连接")


# ---------- suppress —— 忽略指定异常 ----------
# Python 3.4+，等价于 try/except pass
with suppress(FileNotFoundError):
    os.remove("不存在的文件.txt")        # 不会报错

# 等价于:
# try:
#     os.remove("不存在的文件.txt")
# except FileNotFoundError:
#     pass
```

### 7.3 @contextmanager 高级用法

```python
import time
from contextlib import contextmanager


@contextmanager
def timer(label: str = "执行时间"):
    """
    计时器上下文管理器
    测量代码块的执行时间
    """
    start = time.perf_counter()
    try:
        yield
    finally:
        elapsed = time.perf_counter() - start
        print(f"{label}: {elapsed:.4f} 秒")


@contextmanager
def change_dir(path: str):
    """
    临时切换工作目录
    离开 with 块时自动恢复
    """
    old_dir = os.getcwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(old_dir)


# 嵌套使用
with timer("总时间"):
    with change_dir("/tmp"):
        print(f"当前目录: {os.getcwd()}")
        time.sleep(0.5)
    print(f"已恢复: {os.getcwd()}")


# @contextmanager 还可以实现可重入的上下文管理器
@contextmanager
def redirect_output(filepath: str):
    """
    临时重定向标准输出到文件
    """
    import sys
    old_stdout = sys.stdout
    sys.stdout = open(filepath, "w", encoding="utf-8")
    try:
        yield
    finally:
        sys.stdout.close()
        sys.stdout = old_stdout


print("这行输出到控制台")
with redirect_output("output.txt"):
    print("这行输出到文件")       # ← 输出重定向到文件
print("这行又输出到控制台")
```

---

## 8. 综合示例

```python
"""
综合示例：简易文件同步工具
演示：面向对象设计、文件I/O、异常处理、上下文管理器等
"""

import hashlib
import json
import os
import shutil
import time
from pathlib import Path
from typing import Optional


class FileSnapshot:
    """文件快照 —— 记录文件的状态信息"""

    def __init__(self, path: Path):
        self.path = path
        self.size = path.stat().st_size
        self.mtime = path.stat().st_mtime
        self.hash = self._compute_hash()

    def _compute_hash(self) -> str:
        """计算文件的 SHA-256 哈希值"""
        hasher = hashlib.sha256()
        # 分块读取，避免大文件占用过多内存
        with open(self.path, "rb") as f:
            while True:
                chunk = f.read(65536)  # 64KB
                if not chunk:
                    break
                hasher.update(chunk)
        return hasher.hexdigest()

    def is_identical_to(self, other: "FileSnapshot") -> bool:
        """判断两个快照是否相同（先比较大小和修改时间，再比哈希）"""
        # 优化：先比大小，再比 mtime，最后比哈希
        if self.size != other.size:
            return False
        if abs(self.mtime - other.mtime) < 0.01:  # 修改时间相同
            return True
        return self.hash == other.hash

    def __repr__(self):
        return f"FileSnapshot({self.path.name}, {self.size}b, {self.hash[:8]}...)"


class SyncState:
    """同步状态 —— 管理已同步的文件记录"""

    def __init__(self, state_file: Path):
        self.state_file = state_file
        self._records: dict[str, dict] = {}
        self._load()

    def _load(self) -> None:
        """从文件加载同步记录"""
        if self.state_file.exists():
            with open(self.state_file, "r", encoding="utf-8") as f:
                self._records = json.load(f)

    def _save(self) -> None:
        """保存同步记录到文件"""
        self.state_file.parent.mkdir(parents=True, exist_ok=True)
        # 使用临时文件 + 原子重命名避免写入损坏
        temp = self.state_file.with_suffix(".tmp")
        with open(temp, "w", encoding="utf-8") as f:
            json.dump(self._records, f, indent=2)
        temp.replace(self.state_file)

    def get_record(self, rel_path: str) -> Optional[dict]:
        """获取文件的同步记录"""
        return self._records.get(rel_path)

    def update_record(self, rel_path: str, snapshot: FileSnapshot) -> None:
        """更新文件的同步记录"""
        self._records[rel_path] = {
            "hash": snapshot.hash,
            "size": snapshot.size,
            "mtime": snapshot.mtime,
            "synced_at": time.time(),
        }
        self._save()

    def remove_record(self, rel_path: str) -> None:
        """删除文件的同步记录"""
        self._records.pop(rel_path, None)
        self._save()

    def needs_sync(self, rel_path: str, snapshot: FileSnapshot) -> bool:
        """判断文件是否需要同步"""
        record = self.get_record(rel_path)
        if record is None:
            return True
        return record["hash"] != snapshot.hash


class FileSync:
    """文件同步器 —— 核心同步逻辑"""

    def __init__(self, source_dir: Path, target_dir: Path, state_dir: Optional[Path] = None):
        """
        Args:
            source_dir: 源目录
            target_dir: 目标目录
            state_dir: 状态文件目录（默认在目标目录的 .sync_state 中）
        """
        self.source = Path(source_dir)
        self.target = Path(target_dir)

        if state_dir is None:
            state_dir = self.target / ".sync_state"
        self.state = SyncState(state_dir / "state.json")

        # 统计信息
        self.stats = {
            "copied": 0,
            "updated": 0,
            "deleted": 0,
            "skipped": 0,
            "errors": 0,
        }

    def sync(self, dry_run: bool = False) -> None:
        """
        执行同步

        Args:
            dry_run: 如果为 True，只显示将要执行的操作，不实际执行
        """
        print(f"同步: {self.source} → {self.target}")
        print(f"模拟运行: {'是' if dry_run else '否'}")
        print("-" * 60)

        # 收集源文件
        source_files: dict[str, Path] = {}
        for f in self.source.rglob("*"):
            if f.is_file():
                rel_path = str(f.relative_to(self.source))
                source_files[rel_path] = f

        # 复制/更新源文件到目标
        for rel_path, src_path in source_files.items():
            try:
                snapshot = FileSnapshot(src_path)
                dst_path = self.target / rel_path

                if dst_path.exists():
                    # 目标文件已存在，检查是否需要更新
                    if self.state.needs_sync(rel_path, snapshot):
                        self._copy_file(src_path, dst_path, snapshot, rel_path, dry_run)
                    else:
                        self.stats["skipped"] += 1
                        if dry_run:
                            print(f"  [跳过] {rel_path}（未变化）")
                else:
                    # 目标文件不存在，需要创建
                    self._copy_file(src_path, dst_path, snapshot, rel_path, dry_run)

            except Exception as e:
                print(f"  [错误] {rel_path}: {e}")
                self.stats["errors"] += 1

        # 删除目标中存在但源中不存在的文件
        for f in self.target.rglob("*"):
            if f.is_file():
                rel_path = str(f.relative_to(self.target))
                # 跳过状态目录
                if rel_path.startswith(".sync_state"):
                    continue
                if rel_path not in source_files:
                    if dry_run:
                        print(f"  [删除] {rel_path}")
                    else:
                        f.unlink()
                        self.state.remove_record(rel_path)
                        print(f"  [删除] {rel_path}")
                    self.stats["deleted"] += 1

        # 清理空目录
        self._cleanup_empty_dirs(self.target, dry_run)

        # 打印统计
        print("-" * 60)
        print("同步完成！")
        for key, value in self.stats.items():
            print(f"  {key}: {value}")

    def _copy_file(
        self,
        src: Path,
        dst: Path,
        snapshot: FileSnapshot,
        rel_path: str,
        dry_run: bool,
    ) -> None:
        """复制单个文件"""
        if dry_run:
            action = "更新" if dst.exists() else "新建"
            print(f"  [{action}] {rel_path}")
            self.stats["copied"] if not dst.exists() else self.stats["updated"]
            return

        # 确保目标目录存在
        dst.parent.mkdir(parents=True, exist_ok=True)
        # 执行复制
        shutil.copy2(src, dst)  # copy2 保留元数据
        # 更新状态
        self.state.update_record(rel_path, snapshot)

        action = "更新" if dst.exists() else "新建"
        print(f"  [{action}] {rel_path}（{snapshot.size:,} 字节）")
        self.stats["copied" if not dst.exists() else "updated"] += 1

    def _cleanup_empty_dirs(self, directory: Path, dry_run: bool) -> None:
        """递归删除空目录"""
        for d in sorted(directory.rglob("*"), key=lambda p: len(str(p)), reverse=True):
            if d.is_dir() and not any(d.iterdir()):
                if not dry_run:
                    d.rmdir()
                print(f"  [清理空目录] {d}")


def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(description="简易文件同步工具")
    parser.add_argument("source", help="源目录路径")
    parser.add_argument("target", help="目标目录路径")
    parser.add_argument("--dry-run", action="store_true", help="仅显示将要执行的操作")
    parser.add_argument("--state-dir", help="状态文件保存目录")

    args = parser.parse_args()

    sync = FileSync(
        source_dir=args.source,
        target_dir=args.target,
        state_dir=Path(args.state_dir) if args.state_dir else None,
    )
    sync.sync(dry_run=args.dry_run)


if __name__ == "__main__":
    main()
```

---

> 本文档涵盖了 Python 类与对象的核心机制、文件 I/O 操作、异常处理和上下文管理器。
> 下一篇将深入 [魔法方法、类型提示、元类与反射等核心机制](03-Python核心机制详解.md)。
