---
title: "Java从入门到精通 - 完整教程"

category:
  - Java
tag:
  - Java
---
# Java从入门到精通 - 完整教程

> **文档版本**: 2.0  
> **适用Java版本**: Java 8 - Java 21  
> **创建日期**: 2026-06-18  
> **难度级别**: 入门 → 进阶 → 精通

---

## 📚 目录

- [第一章：Java基础入门](#第一章java基础入门)
- [第二章：面向对象编程](#第二章面向对象编程)
- [第三章：Java核心类库](#第三章java核心类库)
- [第四章：包装类详解](#第四章包装类详解)
- [第五章：集合框架与数据结构](#第五章集合框架与数据结构)
- [第六章：异常处理机制](#第六章异常处理机制)
- [第七章：Java多线程编程](#第七章java多线程编程)
- [第八章：文件IO操作](#第八章文件io操作)
- [第九章：网络编程Socket](#第九章网络编程socket)
- [第十章：高级特性](#第十章高级特性)

---

## 第一章：Java基础入门

### 1.1 Java简介

#### 1.1.1 Java的历史
```
1995年 - Sun公司发布Java 1.0
2004年 - Java 5.0（重大更新：泛型、枚举、注解）
2011年 - Oracle收购Sun
2014年 - Java 8（Lambda表达式、Stream API）
2017年 - Java 9（模块化系统）
2018年 - 开始每6个月发布一个新版本
2021年 - Java 17 LTS（长期支持版本）
2023年 - Java 21 LTS（虚拟线程、模式匹配）
```

#### 1.1.2 Java的特点
- **跨平台性**: "一次编写，到处运行"（Write Once, Run Anywhere）
- **面向对象**: 完全的面向对象语言
- **安全性**: 字节码验证、安全管理器
- **自动内存管理**: 垃圾回收机制（GC）
- **多线程**: 内置多线程支持
- **健壮性**: 强类型检查、异常处理

#### 1.1.3 JDK、JRE、JVM的区别
```
┌─────────────────────────────────┐
│           JDK (开发工具包)        │
│  ┌───────────────────────────┐  │
│  │      JRE (运行环境)         │  │
│  │  ┌─────────────────────┐  │  │
│  │  │    JVM (虚拟机)      │  │  │
│  │  │  - 字节码解释执行    │  │  │
│  │  │  - JIT编译器         │  │  │
│  │  │  - 垃圾回收器        │  │  │
│  │  └─────────────────────┘  │  │
│  │  + Java核心类库           │  │
│  └───────────────────────────┘  │
│  + 开发工具(javac, jar等)       │
└─────────────────────────────────┘
```

### 1.2 第一个Java程序

```java
/**
 * HelloWorld - Java的第一个程序
 * 
 * 文件名必须与类名一致: HelloWorld.java
 * 编译: javac HelloWorld.java
 * 运行: java HelloWorld
 */
public class HelloWorld {
    /**
     * main方法 - 程序的入口点
     * public: 公共访问权限
     * static: 静态方法，不需要创建对象就能调用
     * void: 无返回值
     * String[] args: 命令行参数数组
     */
    public static void main(String[] args) {
        // 输出到控制台
        System.out.println("Hello, World!");
        
        // 不换行输出
        System.out.print("Hello, ");
        System.out.print("Java!");
        
        // 格式化输出
        String name = "张三";
        int age = 25;
        System.out.printf("姓名: %s, 年龄: %d%n", name, age);
    }
}
```

### 1.3 Java基本数据类型

#### 1.3.1 八大基本数据类型

```java
/**
 * Java的8种基本数据类型详解
 */
public class PrimitiveTypes {
    public static void main(String[] args) {
        // 1. 整数类型
        byte b = 127;              // 8位，范围: -128 ~ 127
        short s = 32767;           // 16位，范围: -32768 ~ 32767
        int i = 2147483647;        // 32位，范围: -2^31 ~ 2^31-1（默认）
        long l = 9223372036854775807L; // 64位，后缀L或l
        
        // 2. 浮点类型
        float f = 3.14f;           // 32位，后缀F或f，精度约6-7位
        double d = 3.141592653589793; // 64位，默认浮点类型，精度约15位
        
        // 3. 字符类型
        char c = 'A';              // 16位，Unicode字符
        char c2 = '\u4E2D';        // Unicode: 中
        char c3 = 65;              // ASCII码: A
        
        // 4. 布尔类型
        boolean flag = true;       // true或false，大小不确定
        
        // 数据类型大小对比
        System.out.println("byte:    " + Byte.SIZE + " bits");      // 8
        System.out.println("short:   " + Short.SIZE + " bits");     // 16
        System.out.println("int:     " + Integer.SIZE + " bits");   // 32
        System.out.println("long:    " + Long.SIZE + " bits");      // 64
        System.out.println("float:   " + Float.SIZE + " bits");     // 32
        System.out.println("double:  " + Double.SIZE + " bits");    // 64
        System.out.println("char:    " + Character.SIZE + " bits"); // 16
        
        // 数值范围
        System.out.println("\nbyte范围:   " + Byte.MIN_VALUE + " ~ " + Byte.MAX_VALUE);
        System.out.println("short范围:  " + Short.MIN_VALUE + " ~ " + Short.MAX_VALUE);
        System.out.println("int范围:    " + Integer.MIN_VALUE + " ~ " + Integer.MAX_VALUE);
        System.out.println("long范围:   " + Long.MIN_VALUE + " ~ " + Long.MAX_VALUE);
    }
}
```

#### 1.3.2 类型转换

```java
/**
 * 类型转换详解
 */
public class TypeConversion {
    public static void main(String[] args) {
        // 1. 自动类型转换（隐式转换）
        // 转换规则: byte → short → int → long → float → double
        //                    char → int
        byte b = 10;
        int i = b;           // byte自动转int
        long l = i;          // int自动转long
        float f = l;         // long自动转float
        double d = f;        // float自动转double
        
        System.out.println("自动转换: " + d);
        
        // 2. 强制类型转换（显式转换）
        double d2 = 3.14;
        int i2 = (int) d2;   // 强制转换，会丢失小数部分
        System.out.println("强制转换: " + i2);  // 输出: 3
        
        // 3. 数值溢出
        int max = Integer.MAX_VALUE;
        int overflow = max + 1;
        System.out.println("溢出: " + overflow);  // 输出负数
        
        // 4. 表达式中的自动提升
        byte b1 = 10;
        byte b2 = 20;
        // byte b3 = b1 + b2;  // 编译错误！表达式结果自动提升为int
        int b3 = b1 + b2;      // 正确
        
        // 5. 字符与数字的转换
        char ch = 'A';
        int ascii = ch;        // 自动转换为ASCII码
        System.out.println("'A'的ASCII码: " + ascii);  // 65
        
        char ch2 = (char) (ascii + 1);
        System.out.println("下一个字符: " + ch2);  // B
    }
}
```

### 1.4 运算符

```java
/**
 * Java运算符完整演示
 */
public class Operators {
    public static void main(String[] args) {
        // 1. 算术运算符: + - * / %
        int a = 10, b = 3;
        System.out.println("=== 算术运算符 ===");
        System.out.println("a + b = " + (a + b));   // 13
        System.out.println("a - b = " + (a - b));   // 7
        System.out.println("a * b = " + (a * b));   // 30
        System.out.println("a / b = " + (a / b));   // 3（整除）
        System.out.println("a % b = " + (a % b));   // 1（取余）
        
        // 2. 自增自减运算符: ++ --
        System.out.println("\n=== 自增自减 ===");
        int x = 5;
        System.out.println("x = " + x);          // 5
        System.out.println("x++ = " + (x++));    // 5（先使用后加）
        System.out.println("x = " + x);          // 6
        System.out.println("++x = " + (++x));    // 7（先加后使用）
        
        // 3. 关系运算符: == != > < >= <=
        System.out.println("\n=== 关系运算符 ===");
        System.out.println("10 == 10: " + (10 == 10));   // true
        System.out.println("10 != 5: " + (10 != 5));     // true
        System.out.println("10 > 5: " + (10 > 5));       // true
        System.out.println("10 < 5: " + (10 < 5));       // false
        
        // 4. 逻辑运算符: && || !
        System.out.println("\n=== 逻辑运算符 ===");
        boolean t = true, f = false;
        System.out.println("true && false: " + (t && f));   // false
        System.out.println("true || false: " + (t || f));   // true
        System.out.println("!true: " + (!t));               // false
        
        // 短路特性
        int n = 10;
        if (false && (++n > 10)) {  // &&短路，右边不执行
            System.out.println("不会执行");
        }
        System.out.println("n = " + n);  // 10（没有自增）
        
        // 5. 位运算符: & | ^ ~ << >> >>>
        System.out.println("\n=== 位运算符 ===");
        int m = 0b1010;  // 二进制: 10
        int n2 = 0b1100; // 二进制: 12
        System.out.println("m & n:  " + (m & n2));   // 1000 = 8（按位与）
        System.out.println("m | n:  " + (m | n2));   // 1110 = 14（按位或）
        System.out.println("m ^ n:  " + (m ^ n2));   // 0110 = 6（按位异或）
        System.out.println("~m:     " + (~m));       // 按位取反
        System.out.println("m << 1: " + (m << 1));   // 10100 = 20（左移）
        System.out.println("m >> 1: " + (m >> 1));   // 0101 = 5（右移）
        
        // 6. 赋值运算符: = += -= *= /= %= &= |= ^= <<= >>= >>>=
        System.out.println("\n=== 赋值运算符 ===");
        int num = 10;
        num += 5;  // num = num + 5
        System.out.println("num += 5: " + num);  // 15
        num *= 2;  // num = num * 2
        System.out.println("num *= 2: " + num);  // 30
        
        // 7. 三元运算符: ? :
        System.out.println("\n=== 三元运算符 ===");
        int max = (a > b) ? a : b;
        System.out.println("max(10, 3) = " + max);  // 10
        
        // 8. instanceof运算符
        String str = "Hello";
        System.out.println("\n=== instanceof ===");
        System.out.println("str instanceof String: " + (str instanceof String));  // true
    }
}
```

### 1.5 流程控制

```java
/**
 * 流程控制语句详解
 */
public class ControlFlow {
    public static void main(String[] args) {
        // 1. if-else语句
        System.out.println("=== if-else ===");
        int score = 85;
        
        if (score >= 90) {
            System.out.println("优秀");
        } else if (score >= 80) {
            System.out.println("良好");
        } else if (score >= 60) {
            System.out.println("及格");
        } else {
            System.out.println("不及格");
        }
        
        // 2. switch语句
        System.out.println("\n=== switch ===");
        int dayOfWeek = 3;
        
        switch (dayOfWeek) {
            case 1:
                System.out.println("星期一");
                break;  // break防止穿透
            case 2:
                System.out.println("星期二");
                break;
            case 3:
                System.out.println("星期三");
                break;
            default:
                System.out.println("其他");
                break;
        }
        
        // Java 12+: switch表达式（增强版）
        String day = switch (dayOfWeek) {
            case 1 -> "星期一";
            case 2 -> "星期二";
            case 3 -> "星期三";
            default -> "其他";
        };
        System.out.println("增强switch: " + day);
        
        // 3. while循环
        System.out.println("\n=== while循环 ===");
        int i = 1;
        while (i <= 5) {
            System.out.print(i + " ");
            i++;
        }
        System.out.println();
        
        // 4. do-while循环（至少执行一次）
        System.out.println("\n=== do-while循环 ===");
        int j = 1;
        do {
            System.out.print(j + " ");
            j++;
        } while (j <= 5);
        System.out.println();
        
        // 5. for循环
        System.out.println("\n=== for循环 ===");
        for (int k = 1; k <= 5; k++) {
            System.out.print(k + " ");
        }
        System.out.println();
        
        // 6. 增强for循环（for-each）
        System.out.println("\n=== 增强for循环 ===");
        int[] arr = {1, 2, 3, 4, 5};
        for (int num : arr) {
            System.out.print(num + " ");
        }
        System.out.println();
        
        // 7. break和continue
        System.out.println("\n=== break和continue ===");
        for (int m = 1; m <= 10; m++) {
            if (m == 3) {
                continue;  // 跳过当前迭代
            }
            if (m == 8) {
                break;  // 跳出循环
            }
            System.out.print(m + " ");
        }
        System.out.println();
        
        // 8. 标签（label）
        System.out.println("\n=== 标签 ===");
        outer: for (int x = 1; x <= 3; x++) {
            for (int y = 1; y <= 3; y++) {
                if (x == 2 && y == 2) {
                    break outer;  // 跳出外层循环
                }
                System.out.println("x=" + x + ", y=" + y);
            }
        }
    }
}
```



---

## 第二章：面向对象编程

### 2.1 类和对象

#### 2.1.1 类的定义

```java
/**
 * Person类 - 演示面向对象的基本概念
 * 
 * 类是对象的模板/蓝图
 * 对象是类的实例
 */
public class Person {
    // 成员变量（实例变量/字段）
    private String name;      // 私有变量，外部不可直接访问
    private int age;
    private String gender;
    
    // 静态变量（类变量）- 所有对象共享
    private static int count = 0;
    
    // 常量
    public static final String SPECIES = "Homo sapiens";
    
    // 构造方法1：无参构造
    public Person() {
        this("未知", 0, "未知");  // 调用其他构造方法
    }
    
    // 构造方法2：带参构造
    public Person(String name, int age, String gender) {
        this.name = name;      // this表示当前对象
        this.age = age;
        this.gender = gender;
        count++;               // 每创建一个对象，计数器加1
    }
    
    // Getter方法
    public String getName() {
        return name;
    }
    
    // Setter方法
    public void setName(String name) {
        this.name = name;
    }
    
    public int getAge() {
        return age;
    }
    
    public void setAge(int age) {
        if (age < 0 || age > 150) {
            throw new IllegalArgumentException("年龄不合法");
        }
        this.age = age;
    }
    
    public String getGender() {
        return gender;
    }
    
    public void setGender(String gender) {
        this.gender = gender;
    }
    
    // 静态方法
    public static int getCount() {
        return count;
    }
    
    // 实例方法
    public void sayHello() {
        System.out.println("你好，我是" + name + "，今年" + age + "岁。");
    }
    
    // 实例方法
    public void celebrateBirthday() {
        age++;
        System.out.println(name + "生日快乐！现在" + age + "岁了。");
    }
    
    // 重写toString方法
    @Override
    public String toString() {
        return "Person{" +
                "name='" + name + '\'' +
                ", age=" + age +
                ", gender='" + gender + '\'' +
                '}';
    }
    
    // 重写equals方法
    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        Person person = (Person) obj;
        return age == person.age &&
               name.equals(person.name) &&
               gender.equals(person.gender);
    }
    
    // 重写hashCode方法
    @Override
    public int hashCode() {
        int result = name.hashCode();
        result = 31 * result + age;
        result = 31 * result + gender.hashCode();
        return result;
    }
}

/**
 * 测试类
 */
class PersonTest {
    public static void main(String[] args) {
        // 创建对象
        Person p1 = new Person("张三", 25, "男");
        Person p2 = new Person("李四", 30, "女");
        Person p3 = new Person();
        
        // 调用方法
        p1.sayHello();
        p2.celebrateBirthday();
        
        // 访问属性（通过getter/setter）
        System.out.println("p1的名字: " + p1.getName());
        p3.setName("王五");
        p3.setAge(28);
        
        // 静态方法调用
        System.out.println("创建的Person对象数量: " + Person.getCount());
        
        // toString
        System.out.println(p1);
        
        // equals
        Person p4 = new Person("张三", 25, "男");
        System.out.println("p1 equals p4: " + p1.equals(p4));
    }
}
```

### 2.2 封装（Encapsulation）

```java
/**
 * 封装：隐藏对象的内部实现细节，只暴露必要的接口
 * 优点：
 * 1. 保护数据安全
 * 2. 隐藏实现细节
 * 3. 提高代码的可维护性
 * 4. 便于修改和扩展
 */
public class BankAccount {
    // 私有属性，外部无法直接访问
    private String accountNumber;
    private String accountHolder;
    private double balance;
    private String password;
    
    public BankAccount(String accountNumber, String accountHolder, String password) {
        this.accountNumber = accountNumber;
        this.accountHolder = accountHolder;
        this.password = password;
        this.balance = 0.0;
    }
    
    // 存款方法
    public void deposit(double amount) {
        if (amount <= 0) {
            System.out.println("存款金额必须大于0");
            return;
        }
        balance += amount;
        System.out.println("存款成功，当前余额: " + balance);
    }
    
    // 取款方法（需要密码验证）
    public boolean withdraw(double amount, String password) {
        if (!this.password.equals(password)) {
            System.out.println("密码错误");
            return false;
        }
        if (amount <= 0) {
            System.out.println("取款金额必须大于0");
            return false;
        }
        if (amount > balance) {
            System.out.println("余额不足");
            return false;
        }
        balance -= amount;
        System.out.println("取款成功，当前余额: " + balance);
        return true;
    }
    
    // 查询余额（需要密码验证）
    public double getBalance(String password) {
        if (!this.password.equals(password)) {
            System.out.println("密码错误");
            return -1;
        }
        return balance;
    }
    
    // 修改密码
    public boolean changePassword(String oldPassword, String newPassword) {
        if (!this.password.equals(oldPassword)) {
            System.out.println("原密码错误");
            return false;
        }
        this.password = newPassword;
        System.out.println("密码修改成功");
        return true;
    }
    
    public String getAccountNumber() {
        return accountNumber;
    }
    
    public String getAccountHolder() {
        return accountHolder;
    }
}
```

### 2.3 继承（Inheritance）

```java
/**
 * 继承：子类继承父类的属性和方法
 * 关键字：extends
 * Java只支持单继承，但支持多层继承
 */

// 父类（基类/超类）
class Animal {
    protected String name;
    protected int age;
    
    public Animal(String name, int age) {
        this.name = name;
        this.age = age;
        System.out.println("Animal构造方法被调用");
    }
    
    public void eat() {
        System.out.println(name + "正在吃东西");
    }
    
    public void sleep() {
        System.out.println(name + "正在睡觉");
    }
    
    public void makeSound() {
        System.out.println(name + "发出声音");
    }
}

// 子类1：狗
class Dog extends Animal {
    private String breed;  // 品种
    
    public Dog(String name, int age, String breed) {
        super(name, age);  // 调用父类构造方法，必须是第一行
        this.breed = breed;
        System.out.println("Dog构造方法被调用");
    }
    
    // 重写父类方法
    @Override
    public void makeSound() {
        System.out.println(name + "汪汪汪~");
    }
    
    // 子类特有方法
    public void wagTail() {
        System.out.println(name + "摇尾巴");
    }
    
    public void fetchBall() {
        System.out.println(name + "去捡球");
    }
}

// 子类2：猫
class Cat extends Animal {
    private String color;
    
    public Cat(String name, int age, String color) {
        super(name, age);
        this.color = color;
        System.out.println("Cat构造方法被调用");
    }
    
    @Override
    public void makeSound() {
        System.out.println(name + "喵喵喵~");
    }
    
    public void climb() {
        System.out.println(name + "爬树");
    }
}

// 测试继承
class InheritanceTest {
    public static void main(String[] args) {
        // 创建子类对象
        Dog dog = new Dog("旺财", 3, "哈士奇");
        dog.eat();        // 继承自父类
        dog.sleep();      // 继承自父类
        dog.makeSound();  // 重写的方法
        dog.wagTail();    // 子类特有方法
        
        System.out.println();
        
        Cat cat = new Cat("咪咪", 2, "橘色");
        cat.eat();
        cat.makeSound();
        cat.climb();
        
        System.out.println();
        
        // 多态：父类引用指向子类对象
        Animal animal1 = new Dog("小黑", 1, "泰迪");
        Animal animal2 = new Cat("小白", 1, "白色");
        
        animal1.makeSound();  // 输出：小黑汪汪汪~
        animal2.makeSound();  // 输出：小白喵喵喵~
        
        // 向下转型
        if (animal1 instanceof Dog) {
            Dog d = (Dog) animal1;
            d.wagTail();
        }
    }
}
```

### 2.4 多态（Polymorphism）

```java
/**
 * 多态：同一个引用类型，使用不同的实例而执行不同的操作
 * 多态的三个必要条件：
 * 1. 继承
 * 2. 重写
 * 3. 父类引用指向子类对象
 */

// 形状基类
abstract class Shape {
    protected String color;
    
    public Shape(String color) {
        this.color = color;
    }
    
    // 抽象方法：计算面积
    public abstract double getArea();
    
    // 抽象方法：计算周长
    public abstract double getPerimeter();
    
    // 普通方法
    public void display() {
        System.out.println("这是一个" + color + "的图形");
        System.out.println("面积: " + getArea());
        System.out.println("周长: " + getPerimeter());
    }
}

// 圆形
class Circle extends Shape {
    private double radius;
    
    public Circle(String color, double radius) {
        super(color);
        this.radius = radius;
    }
    
    @Override
    public double getArea() {
        return Math.PI * radius * radius;
    }
    
    @Override
    public double getPerimeter() {
        return 2 * Math.PI * radius;
    }
}

// 矩形
class Rectangle extends Shape {
    private double width;
    private double height;
    
    public Rectangle(String color, double width, double height) {
        super(color);
        this.width = width;
        this.height = height;
    }
    
    @Override
    public double getArea() {
        return width * height;
    }
    
    @Override
    public double getPerimeter() {
        return 2 * (width + height);
    }
}

// 三角形
class Triangle extends Shape {
    private double a, b, c;
    
    public Triangle(String color, double a, double b, double c) {
        super(color);
        this.a = a;
        this.b = b;
        this.c = c;
    }
    
    @Override
    public double getArea() {
        // 海伦公式
        double s = (a + b + c) / 2;
        return Math.sqrt(s * (s - a) * (s - b) * (s - c));
    }
    
    @Override
    public double getPerimeter() {
        return a + b + c;
    }
}

// 多态测试
class PolymorphismTest {
    public static void main(String[] args) {
        // 多态数组
        Shape[] shapes = new Shape[3];
        shapes[0] = new Circle("红色", 5.0);
        shapes[1] = new Rectangle("蓝色", 4.0, 6.0);
        shapes[2] = new Triangle("绿色", 3.0, 4.0, 5.0);
        
        // 多态：同一个方法调用，不同的实现
        for (Shape shape : shapes) {
            shape.display();
            System.out.println("---");
        }
        
        // 方法参数多态
        printShapeInfo(new Circle("黄色", 3.0));
        printShapeInfo(new Rectangle("紫色", 5.0, 3.0));
    }
    
    // 多态参数：接受任何Shape子类
    public static void printShapeInfo(Shape shape) {
        System.out.println("图形信息:");
        shape.display();
    }
}
```

### 2.5 抽象类和接口

```java
/**
 * 抽象类 vs 接口
 */

// 1. 抽象类（abstract class）
abstract class Vehicle {
    protected String brand;
    protected int speed;
    
    // 抽象类可以有构造方法
    public Vehicle(String brand) {
        this.brand = brand;
        this.speed = 0;
    }
    
    // 抽象方法（没有方法体）
    public abstract void start();
    public abstract void stop();
    
    // 普通方法（有方法体）
    public void accelerate(int increment) {
        speed += increment;
        System.out.println(brand + "加速到" + speed + "km/h");
    }
}

// 2. 接口（interface）
interface Flyable {
    // 接口中的变量默认是 public static final
    int MAX_HEIGHT = 10000;
    
    // 接口中的方法默认是 public abstract
    void fly();
    void land();
    
    // Java 8+: 默认方法
    default void checkWeather() {
        System.out.println("检查天气状况");
    }
    
    // Java 8+: 静态方法
    static void displayInfo() {
        System.out.println("这是一个可飞行的接口");
    }
}

interface Swimmable {
    void swim();
}

// 类可以继承一个抽象类，实现多个接口
class Airplane extends Vehicle implements Flyable {
    public Airplane(String brand) {
        super(brand);
    }
    
    @Override
    public void start() {
        System.out.println(brand + "飞机启动引擎");
    }
    
    @Override
    public void stop() {
        System.out.println(brand + "飞机关闭引擎");
    }
    
    @Override
    public void fly() {
        System.out.println(brand + "飞机起飞，飞行高度: " + MAX_HEIGHT + "米");
    }
    
    @Override
    public void land() {
        System.out.println(brand + "飞机降落");
    }
}

// 水陆两用车：实现多个接口
class AmphibiousCar extends Vehicle implements Flyable, Swimmable {
    public AmphibiousCar(String brand) {
        super(brand);
    }
    
    @Override
    public void start() {
        System.out.println(brand + "水陆两用车启动");
    }
    
    @Override
    public void stop() {
        System.out.println(brand + "水陆两用车停止");
    }
    
    @Override
    public void fly() {
        System.out.println(brand + "在空中飞行");
    }
    
    @Override
    public void land() {
        System.out.println(brand + "降落到地面");
    }
    
    @Override
    public void swim() {
        System.out.println(brand + "在水中游泳");
    }
}

/**
 * 抽象类 vs 接口 对比：
 * 
 * 抽象类：
 * - 可以有构造方法
 * - 可以有成员变量
 * - 方法可以是抽象的或具体的
 * - 只能单继承
 * - 用于表示"is-a"关系
 * 
 * 接口：
 * - 不能有构造方法
 * - 只能有常量（public static final）
 * - 方法默认是抽象的（Java 8+支持default和static方法）
 * - 可以多实现
 * - 用于表示"can-do"关系（能力）
 */
```



---

## 第三章：Java核心类库

### 3.1 String类详解

```java
/**
 * String类 - Java中最常用的类
 * 特点：
 * 1. 不可变性（immutable）
 * 2. 字符串常量池
 * 3. final类，不能被继承
 */
public class StringDemo {
    public static void main(String[] args) {
        // 1. 创建字符串的方式
        String s1 = "Hello";              // 字面量，存储在常量池
        String s2 = "Hello";              // 指向同一个常量池对象
        String s3 = new String("Hello");  // new创建，存储在堆中
        String s4 = new String("Hello");
        
        System.out.println("s1 == s2: " + (s1 == s2));  // true（同一对象）
        System.out.println("s3 == s4: " + (s3 == s4));  // false（不同对象）
        System.out.println("s1.equals(s3): " + s1.equals(s3));  // true（内容相同）
        
        // 2. 字符串不可变性
        String str = "Hello";
        str.concat(" World");  // 返回新字符串，原字符串不变
        System.out.println("str: " + str);  // 输出: Hello
        
        str = str.concat(" World");  // 需要重新赋值
        System.out.println("str: " + str);  // 输出: Hello World
        
        // 3. 常用方法
        String text = "  Hello World  ";
        
        // 长度
        System.out.println("长度: " + text.length());
        
        // 字符访问
        System.out.println("第一个字符: " + text.charAt(2));
        
        // 子字符串
        System.out.println("子串: " + text.substring(2, 7));
        
        // 查找
        System.out.println("indexOf('W'): " + text.indexOf('W'));
        System.out.println("lastIndexOf('l'): " + text.lastIndexOf('l'));
        
        // 判断
        System.out.println("startsWith: " + text.startsWith("  He"));
        System.out.println("endsWith: " + text.endsWith("ld  "));
        System.out.println("contains: " + text.contains("World"));
        System.out.println("isEmpty: " + text.isEmpty());
        System.out.println("isBlank: " + text.isBlank());  // Java 11+
        
        // 转换
        System.out.println("大写: " + text.toUpperCase());
        System.out.println("小写: " + text.toLowerCase());
        System.out.println("去空格: [" + text.trim() + "]");
        System.out.println("strip: [" + text.strip() + "]");  // Java 11+，更强大
        
        // 替换
        System.out.println("replace: " + text.replace("World", "Java"));
        System.out.println("replaceAll: " + "a1b2c3".replaceAll("\\d", "X"));
        
        // 分割
        String csv = "apple,banana,orange";
        String[] fruits = csv.split(",");
        for (String fruit : fruits) {
            System.out.println("水果: " + fruit);
        }
        
        // 拼接
        String joined = String.join(" | ", fruits);
        System.out.println("拼接: " + joined);
        
        // 格式化
        String formatted = String.format("姓名: %s, 年龄: %d, 成绩: %.2f", 
                                         "张三", 25, 89.5);
        System.out.println(formatted);
        
        // 比较
        String a = "abc";
        String b = "abd";
        System.out.println("compareTo: " + a.compareTo(b));  // 负数
        System.out.println("compareToIgnoreCase: " + 
                          "ABC".compareToIgnoreCase("abc"));  // 0
        
        // 4. 字符串拼接性能
        // 方式1：+ 运算符（简单场景）
        String result1 = "Hello" + " " + "World";
        
        // 方式2：StringBuilder（单线程推荐）
        StringBuilder sb = new StringBuilder();
        sb.append("Hello");
        sb.append(" ");
        sb.append("World");
        String result2 = sb.toString();
        
        // 方式3：StringBuffer（线程安全）
        StringBuffer sbf = new StringBuffer();
        sbf.append("Hello");
        sbf.append(" ");
        sbf.append("World");
        String result3 = sbf.toString();
        
        // 性能测试
        long start = System.currentTimeMillis();
        String s = "";
        for (int i = 0; i < 10000; i++) {
            s += i;  // 每次都创建新对象，效率低
        }
        long end = System.currentTimeMillis();
        System.out.println("+ 运算符耗时: " + (end - start) + "ms");
        
        start = System.currentTimeMillis();
        StringBuilder sb2 = new StringBuilder();
        for (int i = 0; i < 10000; i++) {
            sb2.append(i);
        }
        String s2 = sb2.toString();
        end = System.currentTimeMillis();
        System.out.println("StringBuilder耗时: " + (end - start) + "ms");
    }
}
```

### 3.2 Math类

```java
/**
 * Math类 - 数学运算工具类
 * 所有方法都是静态的
 */
public class MathDemo {
    public static void main(String[] args) {
        // 常量
        System.out.println("π = " + Math.PI);
        System.out.println("e = " + Math.E);
        
        // 绝对值
        System.out.println("abs(-10) = " + Math.abs(-10));
        
        // 最大最小值
        System.out.println("max(10, 20) = " + Math.max(10, 20));
        System.out.println("min(10, 20) = " + Math.min(10, 20));
        
        // 幂运算
        System.out.println("pow(2, 3) = " + Math.pow(2, 3));  // 2^3 = 8
        System.out.println("sqrt(16) = " + Math.sqrt(16));    // 平方根
        System.out.println("cbrt(27) = " + Math.cbrt(27));    // 立方根
        
        // 四舍五入
        System.out.println("round(3.4) = " + Math.round(3.4));    // 3
        System.out.println("round(3.5) = " + Math.round(3.5));    // 4
        System.out.println("ceil(3.1) = " + Math.ceil(3.1));      // 4.0（向上取整）
        System.out.println("floor(3.9) = " + Math.floor(3.9));    // 3.0（向下取整）
        
        // 随机数
        System.out.println("random() = " + Math.random());  // [0.0, 1.0)
        
        // 生成[min, max]范围的随机整数
        int min = 10, max = 20;
        int randomInt = (int)(Math.random() * (max - min + 1)) + min;
        System.out.println("随机整数[10, 20]: " + randomInt);
        
        // 三角函数
        double angle = 30;  // 度
        double radians = Math.toRadians(angle);  // 转换为弧度
        System.out.println("sin(30°) = " + Math.sin(radians));
        System.out.println("cos(30°) = " + Math.cos(radians));
        System.out.println("tan(30°) = " + Math.tan(radians));
        
        // 对数
        System.out.println("log(e) = " + Math.log(Math.E));      // 自然对数
        System.out.println("log10(100) = " + Math.log10(100));   // 以10为底
    }
}
```

### 3.3 日期时间API

```java
import java.time.*;
import java.time.format.DateTimeFormatter;
import java.time.temporal.ChronoUnit;
import java.util.Date;

/**
 * Java 8+ 新的日期时间API
 * 旧API（Date, Calendar）的问题：
 * 1. 不是线程安全的
 * 2. 设计混乱
 * 3. 月份从0开始
 * 
 * 新API的特点：
 * 1. 不可变性（线程安全）
 * 2. 清晰的API设计
 * 3. 链式调用
 */
public class DateTimeDemo {
    public static void main(String[] args) {
        // 1. LocalDate - 日期（年月日）
        LocalDate today = LocalDate.now();
        System.out.println("今天: " + today);
        
        LocalDate birthday = LocalDate.of(1990, 5, 20);
        System.out.println("生日: " + birthday);
        
        LocalDate tomorrow = today.plusDays(1);
        LocalDate nextMonth = today.plusMonths(1);
        LocalDate nextYear = today.plusYears(1);
        
        System.out.println("明天: " + tomorrow);
        System.out.println("下个月: " + nextMonth);
        System.out.println("明年: " + nextYear);
        
        // 获取日期信息
        System.out.println("年: " + today.getYear());
        System.out.println("月: " + today.getMonthValue());
        System.out.println("日: " + today.getDayOfMonth());
        System.out.println("星期: " + today.getDayOfWeek());
        
        // 2. LocalTime - 时间（时分秒）
        LocalTime now = LocalTime.now();
        System.out.println("现在: " + now);
        
        LocalTime time = LocalTime.of(10, 30, 45);
        System.out.println("时间: " + time);
        
        LocalTime later = now.plusHours(2).plusMinutes(30);
        System.out.println("2小时30分钟后: " + later);
        
        // 3. LocalDateTime - 日期时间
        LocalDateTime dateTime = LocalDateTime.now();
        System.out.println("当前日期时间: " + dateTime);
        
        LocalDateTime custom = LocalDateTime.of(2024, 12, 31, 23, 59, 59);
        System.out.println("自定义: " + custom);
        
        // 4. ZonedDateTime - 带时区的日期时间
        ZonedDateTime zonedNow = ZonedDateTime.now();
        System.out.println("当前时区时间: " + zonedNow);
        
        ZonedDateTime tokyo = ZonedDateTime.now(ZoneId.of("Asia/Tokyo"));
        System.out.println("东京时间: " + tokyo);
        
        ZonedDateTime newYork = ZonedDateTime.now(ZoneId.of("America/New_York"));
        System.out.println("纽约时间: " + newYork);
        
        // 5. Duration - 时间间隔（秒、纳秒）
        LocalTime start = LocalTime.of(9, 0);
        LocalTime end = LocalTime.of(17, 30);
        Duration duration = Duration.between(start, end);
        System.out.println("工作时长: " + duration.toHours() + "小时" + 
                          duration.toMinutesPart() + "分钟");
        
        // 6. Period - 日期间隔（年月日）
        LocalDate birth = LocalDate.of(1990, 5, 20);
        LocalDate now2 = LocalDate.now();
        Period period = Period.between(birth, now2);
        System.out.println("年龄: " + period.getYears() + "岁" + 
                          period.getMonths() + "个月");
        
        // 7. 格式化和解析
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
        String formatted = dateTime.format(formatter);
        System.out.println("格式化: " + formatted);
        
        String dateStr = "2024-12-31 23:59:59";
        LocalDateTime parsed = LocalDateTime.parse(dateStr, formatter);
        System.out.println("解析: " + parsed);
        
        // 8. 日期比较
        LocalDate date1 = LocalDate.of(2024, 1, 1);
        LocalDate date2 = LocalDate.of(2024, 12, 31);
        
        System.out.println("date1在date2之前: " + date1.isBefore(date2));
        System.out.println("date1在date2之后: " + date1.isAfter(date2));
        System.out.println("date1等于date2: " + date1.isEqual(date2));
        
        // 9. 计算两个日期之间的天数
        long days = ChronoUnit.DAYS.between(date1, date2);
        System.out.println("相差天数: " + days);
        
        // 10. Instant - 时间戳
        Instant instant = Instant.now();
        System.out.println("时间戳: " + instant);
        System.out.println("毫秒: " + instant.toEpochMilli());
    }
}
```

---

## 第四章：包装类详解

### 4.1 包装类概述

```java
/**
 * 包装类（Wrapper Classes）
 * 
 * 为什么需要包装类？
 * 1. 基本数据类型不是对象，不能调用方法
 * 2. 集合框架只能存储对象，不能存储基本类型
 * 3. 提供了基本类型与字符串之间的转换方法
 * 4. 提供了常量和工具方法
 * 
 * 8个包装类：
 * byte    → Byte
 * short   → Short
 * int     → Integer
 * long    → Long
 * float   → Float
 * double  → Double
 * char    → Character
 * boolean → Boolean
 */
public class WrapperClassDemo {
    public static void main(String[] args) {
        // 1. 装箱和拆箱
        
        // 手动装箱（基本类型 → 包装类）
        Integer obj1 = Integer.valueOf(10);
        
        // 手动拆箱（包装类 → 基本类型）
        int num1 = obj1.intValue();
        
        // 自动装箱（Java 5+）
        Integer obj2 = 10;  // 编译器自动调用Integer.valueOf(10)
        
        // 自动拆箱
        int num2 = obj2;    // 编译器自动调用obj2.intValue()
        
        // 2. Integer类详解
        System.out.println("=== Integer类 ===");
        
        // 常量
        System.out.println("最大值: " + Integer.MAX_VALUE);
        System.out.println("最小值: " + Integer.MIN_VALUE);
        System.out.println("字节数: " + Integer.BYTES);
        System.out.println("位数: " + Integer.SIZE);
        
        // 字符串转整数
        int a = Integer.parseInt("123");
        Integer b = Integer.valueOf("456");
        System.out.println("a = " + a + ", b = " + b);
        
        // 不同进制解析
        int bin = Integer.parseInt("1010", 2);   // 二进制
        int oct = Integer.parseInt("17", 8);     // 八进制
        int hex = Integer.parseInt("1F", 16);    // 十六进制
        System.out.println("二进制1010 = " + bin);
        System.out.println("八进制17 = " + oct);
        System.out.println("十六进制1F = " + hex);
        
        // 整数转字符串
        String s1 = Integer.toString(123);
        String s2 = String.valueOf(123);
        String s3 = 123 + "";
        
        // 进制转换
        String binary = Integer.toBinaryString(10);
        String octal = Integer.toOctalString(10);
        String hexadecimal = Integer.toHexString(10);
        System.out.println("10的二进制: " + binary);
        System.out.println("10的八进制: " + octal);
        System.out.println("10的十六进制: " + hexadecimal);
        
        // 比较
        System.out.println("compare(10, 20): " + Integer.compare(10, 20));  // -1
        
        // 位运算工具
        System.out.println("bitCount(7): " + Integer.bitCount(7));  // 二进制中1的个数
        System.out.println("highestOneBit(10): " + Integer.highestOneBit(10));
        System.out.println("reverse(1): " + Integer.reverse(1));
        
        // 3. Integer缓存机制
        System.out.println("\n=== Integer缓存 ===");
        Integer i1 = 127;
        Integer i2 = 127;
        System.out.println("i1 == i2: " + (i1 == i2));  // true（缓存范围-128~127）
        
        Integer i3 = 128;
        Integer i4 = 128;
        System.out.println("i3 == i4: " + (i3 == i4));  // false（超出缓存范围）
        
        // 正确的比较方式
        System.out.println("i3.equals(i4): " + i3.equals(i4));  // true
        
        // 4. Double类
        System.out.println("\n=== Double类 ===");
        Double d1 = 3.14;
        Double d2 = Double.valueOf("2.718");
        
        double d3 = Double.parseDouble("1.414");
        
        System.out.println("NaN: " + Double.NaN);
        System.out.println("正无穷: " + Double.POSITIVE_INFINITY);
        System.out.println("负无穷: " + Double.NEGATIVE_INFINITY);
        
        System.out.println("isNaN(0.0/0.0): " + Double.isNaN(0.0/0.0));
        System.out.println("isInfinite(1.0/0.0): " + Double.isInfinite(1.0/0.0));
        
        // 5. Boolean类
        System.out.println("\n=== Boolean类 ===");
        Boolean flag1 = true;
        Boolean flag2 = Boolean.valueOf("true");
        boolean flag3 = Boolean.parseBoolean("false");
        
        System.out.println("flag1: " + flag1);
        System.out.println("flag2: " + flag2);
        System.out.println("flag3: " + flag3);
        
        // 6. Character类
        System.out.println("\n=== Character类 ===");
        Character ch = 'A';
        
        System.out.println("isDigit('5'): " + Character.isDigit('5'));
        System.out.println("isLetter('A'): " + Character.isLetter('A'));
        System.out.println("isUpperCase('A'): " + Character.isUpperCase('A'));
        System.out.println("isLowerCase('a'): " + Character.isLowerCase('a'));
        System.out.println("isWhitespace(' '): " + Character.isWhitespace(' '));
        
        System.out.println("toUpperCase('a'): " + Character.toUpperCase('a'));
        System.out.println("toLowerCase('A'): " + Character.toLowerCase('A'));
        
        // 7. 包装类的注意事项
        System.out.println("\n=== 注意事项 ===");
        
        // 避免空指针异常
        Integer nullInt = null;
        try {
            int value = nullInt;  // 自动拆箱，抛出NullPointerException
        } catch (NullPointerException e) {
            System.out.println("空指针异常!");
        }
        
        // 性能考虑：频繁装箱拆箱影响性能
        long start = System.currentTimeMillis();
        Integer sum1 = 0;
        for (int i = 0; i < 1000000; i++) {
            sum1 += i;  // 每次都装箱拆箱
        }
        long end = System.currentTimeMillis();
        System.out.println("包装类耗时: " + (end - start) + "ms");
        
        start = System.currentTimeMillis();
        int sum2 = 0;
        for (int i = 0; i < 1000000; i++) {
            sum2 += i;  // 基本类型，更快
        }
        end = System.currentTimeMillis();
        System.out.println("基本类型耗时: " + (end - start) + "ms");
    }
}
```



---

## 第五章：集合框架与数据结构

### 5.1 集合框架概述

```
Java集合框架体系结构：

Collection（接口）
├── List（接口）- 有序，可重复
│   ├── ArrayList - 动态数组，查询快，增删慢
│   ├── LinkedList - 双向链表，增删快，查询慢
│   └── Vector - 线程安全的ArrayList（过时）
│       └── Stack - 栈
├── Set（接口）- 无序，不可重复
│   ├── HashSet - 基于HashMap实现
│   │   └── LinkedHashSet - 保持插入顺序
│   └── TreeSet - 基于红黑树，有序
└── Queue（接口）- 队列
    ├── LinkedList - 也实现了Queue接口
    ├── PriorityQueue - 优先队列
    └── Deque（接口）- 双端队列
        └── ArrayDeque - 基于数组的双端队列

Map（接口）- 键值对
├── HashMap - 哈希表，无序
│   └── LinkedHashMap - 保持插入顺序
├── TreeMap - 红黑树，有序
├── Hashtable - 线程安全（过时）
└── ConcurrentHashMap - 线程安全的HashMap
```

### 5.2 List集合

```java
import java.util.*;

/**
 * List集合详解
 */
public class ListDemo {
    public static void main(String[] args) {
        // ============ ArrayList ============
        System.out.println("=== ArrayList ===");
        
        // 创建
        ArrayList<String> list1 = new ArrayList<>();
        List<String> list2 = new ArrayList<>();  // 多态写法（推荐）
        
        // 添加元素
        list1.add("Apple");
        list1.add("Banana");
        list1.add("Cherry");
        list1.add(1, "Blueberry");  // 指定位置插入
        
        System.out.println("list1: " + list1);
        
        // 获取元素
        String first = list1.get(0);
        System.out.println("第一个元素: " + first);
        
        // 修改元素
        list1.set(0, "Apricot");
        System.out.println("修改后: " + list1);
        
        // 删除元素
        list1.remove(0);              // 按索引删除
        list1.remove("Banana");       // 按对象删除
        System.out.println("删除后: " + list1);
        
        // 大小
        System.out.println("大小: " + list1.size());
        
        // 判断
        System.out.println("是否为空: " + list1.isEmpty());
        System.out.println("是否包含Cherry: " + list1.contains("Cherry"));
        
        // 查找
        System.out.println("Cherry的索引: " + list1.indexOf("Cherry"));
        System.out.println("不存在的索引: " + list1.indexOf("Orange"));  // -1
        
        // 遍历
        System.out.println("\n遍历方式:");
        
        // 方式1: for循环
        for (int i = 0; i < list1.size(); i++) {
            System.out.print(list1.get(i) + " ");
        }
        System.out.println();
        
        // 方式2: 增强for
        for (String item : list1) {
            System.out.print(item + " ");
        }
        System.out.println();
        
        // 方式3: Iterator迭代器
        Iterator<String> it = list1.iterator();
        while (it.hasNext()) {
            String item = it.next();
            System.out.print(item + " ");
        }
        System.out.println();
        
        // 方式4: forEach + Lambda（Java 8+）
        list1.forEach(item -> System.out.print(item + " "));
        System.out.println();
        
        // 方式5: Stream（Java 8+）
        list1.stream().forEach(System.out::println);
        
        // 转数组
        String[] arr = list1.toArray(new String[0]);
        System.out.println("转数组: " + Arrays.toString(arr));
        
        // 数组转List
        List<String> list3 = Arrays.asList("A", "B", "C");
        System.out.println("数组转List: " + list3);
        
        // 子列表
        List<String> subList = list1.subList(0, 2);
        System.out.println("子列表: " + subList);
        
        // 排序
        List<Integer> numbers = new ArrayList<>(Arrays.asList(5, 2, 8, 1, 9));
        Collections.sort(numbers);  // 升序
        System.out.println("排序后: " + numbers);
        
        Collections.sort(numbers, Collections.reverseOrder());  // 降序
        System.out.println("降序: " + numbers);
        
        // 打乱
        Collections.shuffle(numbers);
        System.out.println("打乱: " + numbers);
        
        // 反转
        Collections.reverse(numbers);
        System.out.println("反转: " + numbers);
        
        // ============ LinkedList ============
        System.out.println("\n=== LinkedList ===");
        
        LinkedList<String> linkedList = new LinkedList<>();
        linkedList.add("A");
        linkedList.add("B");
        linkedList.add("C");
        
        // 头部操作
        linkedList.addFirst("Head");
        linkedList.addLast("Tail");
        System.out.println("linkedList: " + linkedList);
        
        System.out.println("getFirst: " + linkedList.getFirst());
        System.out.println("getLast: " + linkedList.getLast());
        
        linkedList.removeFirst();
        linkedList.removeLast();
        System.out.println("删除头尾: " + linkedList);
        
        // 作为队列使用
        linkedList.offer("D");      // 入队
        String polled = linkedList.poll();  // 出队
        System.out.println("出队: " + polled);
        
        // 作为栈使用
        linkedList.push("E");       // 入栈
        String popped = linkedList.pop();   // 出栈
        System.out.println("出栈: " + popped);
        
        // ============ ArrayList vs LinkedList ============
        System.out.println("\n=== 性能对比 ===");
        
        int count = 100000;
        
        // ArrayList尾部添加
        long start = System.currentTimeMillis();
        List<Integer> arrayList = new ArrayList<>();
        for (int i = 0; i < count; i++) {
            arrayList.add(i);
        }
        long end = System.currentTimeMillis();
        System.out.println("ArrayList尾部添加" + count + "个元素: " + (end - start) + "ms");
        
        // LinkedList尾部添加
        start = System.currentTimeMillis();
        List<Integer> linkedList2 = new LinkedList<>();
        for (int i = 0; i < count; i++) {
            linkedList2.add(i);
        }
        end = System.currentTimeMillis();
        System.out.println("LinkedList尾部添加" + count + "个元素: " + (end - start) + "ms");
        
        // ArrayList头部添加
        start = System.currentTimeMillis();
        List<Integer> arrayList2 = new ArrayList<>();
        for (int i = 0; i < count; i++) {
            arrayList2.add(0, i);
        }
        end = System.currentTimeMillis();
        System.out.println("ArrayList头部添加" + count + "个元素: " + (end - start) + "ms");
        
        // LinkedList头部添加
        start = System.currentTimeMillis();
        List<Integer> linkedList3 = new LinkedList<>();
        for (int i = 0; i < count; i++) {
            linkedList3.add(0, i);
        }
        end = System.currentTimeMillis();
        System.out.println("LinkedList头部添加" + count + "个元素: " + (end - start) + "ms");
    }
}
```

### 5.3 Set集合

```java
import java.util.*;

/**
 * Set集合详解
 * 特点：无序，不可重复
 */
public class SetDemo {
    public static void main(String[] args) {
        // ============ HashSet ============
        System.out.println("=== HashSet ===");
        
        Set<String> hashSet = new HashSet<>();
        
        // 添加元素
        hashSet.add("Apple");
        hashSet.add("Banana");
        hashSet.add("Cherry");
        hashSet.add("Apple");  // 重复元素不会被添加
        
        System.out.println("hashSet: " + hashSet);
        System.out.println("大小: " + hashSet.size());  // 3
        
        // 判断
        System.out.println("包含Apple: " + hashSet.contains("Apple"));
        
        // 删除
        hashSet.remove("Banana");
        System.out.println("删除后: " + hashSet);
        
        // 遍历
        for (String item : hashSet) {
            System.out.println(item);
        }
        
        // ============ LinkedHashSet ============
        System.out.println("\n=== LinkedHashSet ===");
        
        Set<String> linkedHashSet = new LinkedHashSet<>();
        linkedHashSet.add("C");
        linkedHashSet.add("A");
        linkedHashSet.add("B");
        
        // 保持插入顺序
        System.out.println("linkedHashSet: " + linkedHashSet);  // [C, A, B]
        
        // ============ TreeSet ============
        System.out.println("\n=== TreeSet ===");
        
        Set<String> treeSet = new TreeSet<>();
        treeSet.add("C");
        treeSet.add("A");
        treeSet.add("B");
        
        // 自动排序
        System.out.println("treeSet: " + treeSet);  // [A, B, C]
        
        // TreeSet特有方法
        TreeSet<Integer> numbers = new TreeSet<>();
        numbers.add(5);
        numbers.add(2);
        numbers.add(8);
        numbers.add(1);
        numbers.add(9);
        
        System.out.println("numbers: " + numbers);  // [1, 2, 5, 8, 9]
        System.out.println("first: " + numbers.first());  // 1
        System.out.println("last: " + numbers.last());    // 9
        System.out.println("lower(5): " + numbers.lower(5));  // 小于5的最大值: 2
        System.out.println("higher(5): " + numbers.higher(5));  // 大于5的最小值: 8
        
        // 子集
        SortedSet<Integer> subSet = numbers.subSet(2, 8);  // [2, 8)
        System.out.println("subSet[2, 8): " + subSet);
        
        // 头集和尾集
        System.out.println("headSet(5): " + numbers.headSet(5));  // 小于5
        System.out.println("tailSet(5): " + numbers.tailSet(5));  // 大于等于5
        
        // ============ 自定义对象去重 ============
        System.out.println("\n=== 自定义对象去重 ===");
        
        Set<Student> students = new HashSet<>();
        students.add(new Student("张三", 20));
        students.add(new Student("李四", 21));
        students.add(new Student("张三", 20));  // 重复（需要重写equals和hashCode）
        
        System.out.println("学生数量: " + students.size());
        for (Student s : students) {
            System.out.println(s);
        }
        
        // ============ 集合运算 ============
        System.out.println("\n=== 集合运算 ===");
        
        Set<Integer> set1 = new HashSet<>(Arrays.asList(1, 2, 3, 4, 5));
        Set<Integer> set2 = new HashSet<>(Arrays.asList(4, 5, 6, 7, 8));
        
        // 并集
        Set<Integer> union = new HashSet<>(set1);
        union.addAll(set2);
        System.out.println("并集: " + union);  // [1, 2, 3, 4, 5, 6, 7, 8]
        
        // 交集
        Set<Integer> intersection = new HashSet<>(set1);
        intersection.retainAll(set2);
        System.out.println("交集: " + intersection);  // [4, 5]
        
        // 差集
        Set<Integer> difference = new HashSet<>(set1);
        difference.removeAll(set2);
        System.out.println("差集: " + difference);  // [1, 2, 3]
    }
}

class Student {
    private String name;
    private int age;
    
    public Student(String name, int age) {
        this.name = name;
        this.age = age;
    }
    
    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        Student student = (Student) obj;
        return age == student.age && name.equals(student.name);
    }
    
    @Override
    public int hashCode() {
        return Objects.hash(name, age);
    }
    
    @Override
    public String toString() {
        return "Student{name='" + name + "', age=" + age + "}";
    }
}
```

### 5.4 Map集合

```java
import java.util.*;

/**
 * Map集合详解
 * 特点：键值对，键唯一
 */
public class MapDemo {
    public static void main(String[] args) {
        // ============ HashMap ============
        System.out.println("=== HashMap ===");
        
        Map<String, Integer> map = new HashMap<>();
        
        // 添加元素
        map.put("Apple", 5);
        map.put("Banana", 3);
        map.put("Cherry", 7);
        map.put("Apple", 10);  // 相同key会覆盖
        
        System.out.println("map: " + map);
        System.out.println("大小: " + map.size());
        
        // 获取元素
        int appleCount = map.get("Apple");
        System.out.println("Apple数量: " + appleCount);
        
        // getOrDefault（Java 8+）
        int orangeCount = map.getOrDefault("Orange", 0);
        System.out.println("Orange数量: " + orangeCount);
        
        // 判断
        System.out.println("包含Apple: " + map.containsKey("Apple"));
        System.out.println("包含值5: " + map.containsValue(5));
        
        // 删除
        map.remove("Banana");
        System.out.println("删除后: " + map);
        
        // 遍历方式
        System.out.println("\n遍历方式:");
        
        // 方式1: keySet
        for (String key : map.keySet()) {
            System.out.println(key + " = " + map.get(key));
        }
        
        // 方式2: values
        for (Integer value : map.values()) {
            System.out.println("值: " + value);
        }
        
        // 方式3: entrySet（推荐）
        for (Map.Entry<String, Integer> entry : map.entrySet()) {
            System.out.println(entry.getKey() + " = " + entry.getValue());
        }
        
        // 方式4: forEach + Lambda（Java 8+）
        map.forEach((key, value) -> System.out.println(key + " = " + value));
        
        // Java 8+新方法
        System.out.println("\nJava 8+新方法:");
        
        // putIfAbsent：如果key不存在才添加
        map.putIfAbsent("Date", 2);
        map.putIfAbsent("Apple", 20);  // Apple已存在，不会更新
        System.out.println("putIfAbsent后: " + map);
        
        // compute：计算新值
        map.compute("Apple", (key, value) -> value + 5);
        System.out.println("compute后: " + map);
        
        // merge：合并值
        map.merge("Apple", 1, (oldValue, newValue) -> oldValue + newValue);
        System.out.println("merge后: " + map);
        
        // ============ LinkedHashMap ============
        System.out.println("\n=== LinkedHashMap ===");
        
        Map<String, Integer> linkedMap = new LinkedHashMap<>();
        linkedMap.put("C", 3);
        linkedMap.put("A", 1);
        linkedMap.put("B", 2);
        
        // 保持插入顺序
        System.out.println("linkedMap: " + linkedMap);  // {C=3, A=1, B=2}
        
        // ============ TreeMap ============
        System.out.println("\n=== TreeMap ===");
        
        Map<String, Integer> treeMap = new TreeMap<>();
        treeMap.put("C", 3);
        treeMap.put("A", 1);
        treeMap.put("B", 2);
        
        // 按key自动排序
        System.out.println("treeMap: " + treeMap);  // {A=1, B=2, C=3}
        
        // TreeMap特有方法
        TreeMap<Integer, String> tm = new TreeMap<>();
        tm.put(3, "C");
        tm.put(1, "A");
        tm.put(2, "B");
        tm.put(5, "E");
        tm.put(4, "D");
        
        System.out.println("firstKey: " + tm.firstKey());
        System.out.println("lastKey: " + tm.lastKey());
        System.out.println("lowerKey(3): " + tm.lowerKey(3));
        System.out.println("higherKey(3): " + tm.higherKey(3));
        
        // 子Map
        SortedMap<Integer, String> subMap = tm.subMap(2, 5);
        System.out.println("subMap[2, 5): " + subMap);
        
        // ============ HashMap原理 ============
        System.out.println("\n=== HashMap原理 ===");
        /*
         * HashMap底层实现：
         * 1. JDK 1.7: 数组 + 链表
         * 2. JDK 1.8+: 数组 + 链表 + 红黑树
         * 
         * 当链表长度 >= 8且数组长度 >= 64时，链表转红黑树
         * 当红黑树节点 <= 6时，红黑树退化为链表
         * 
         * 初始容量：16
         * 负载因子：0.75
         * 扩容：容量翻倍
         * 
         * 计算索引：index = (n - 1) & hash
         */
        
        // ============ 统计单词频率示例 ============
        System.out.println("\n=== 单词频率统计 ===");
        
        String text = "apple banana apple cherry banana apple";
        String[] words = text.split(" ");
        
        Map<String, Integer> wordCount = new HashMap<>();
        for (String word : words) {
            wordCount.put(word, wordCount.getOrDefault(word, 0) + 1);
        }
        
        System.out.println("单词频率: " + wordCount);
        
        // 排序输出
        wordCount.entrySet().stream()
            .sorted(Map.Entry.<String, Integer>comparingByValue().reversed())
            .forEach(entry -> System.out.println(entry.getKey() + ": " + entry.getValue()));
    }
}
```



---

## 第六章：异常处理机制

### 6.1 异常体系

```
Throwable（所有异常的根类）
├── Error（错误）- 程序无法处理
│   ├── OutOfMemoryError - 内存溢出
│   ├── StackOverflowError - 栈溢出
│   └── VirtualMachineError - 虚拟机错误
└── Exception（异常）- 程序可以处理
    ├── RuntimeException（运行时异常/非受检异常）
    │   ├── NullPointerException - 空指针
    │   ├── ArrayIndexOutOfBoundsException - 数组越界
    │   ├── ArithmeticException - 算术异常
    │   ├── ClassCastException - 类型转换异常
    │   └── IllegalArgumentException - 非法参数
    └── 其他Exception（编译时异常/受检异常）
        ├── IOException - IO异常
        ├── SQLException - SQL异常
        ├── ClassNotFoundException - 类找不到
        └── ParseException - 解析异常
```

```java
/**
 * 异常处理详解
 */
public class ExceptionDemo {
    public static void main(String[] args) {
        // 1. try-catch
        System.out.println("=== try-catch ===");
        try {
            int result = 10 / 0;  // 抛出ArithmeticException
        } catch (ArithmeticException e) {
            System.out.println("捕获算术异常: " + e.getMessage());
        }
        
        // 2. 多重catch
        System.out.println("\n=== 多重catch ===");
        try {
            int[] arr = {1, 2, 3};
            System.out.println(arr[5]);  // 数组越界
        } catch (ArrayIndexOutOfBoundsException e) {
            System.out.println("数组越界: " + e.getMessage());
        } catch (Exception e) {
            System.out.println("其他异常: " + e.getMessage());
        }
        
        // 3. try-catch-finally
        System.out.println("\n=== try-catch-finally ===");
        try {
            System.out.println("try块执行");
            int result = 10 / 0;
        } catch (Exception e) {
            System.out.println("catch块执行");
        } finally {
            System.out.println("finally块总是执行（清理资源）");
        }
        
        // 4. try-with-resources（Java 7+）
        System.out.println("\n=== try-with-resources ===");
        // 自动关闭资源，资源必须实现AutoCloseable接口
        try (MyResource resource = new MyResource()) {
            resource.doSomething();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // 5. 抛出异常
        System.out.println("\n=== 抛出异常 ===");
        try {
            checkAge(-5);
        } catch (IllegalArgumentException e) {
            System.out.println("捕获异常: " + e.getMessage());
        }
        
        // 6. 自定义异常
        System.out.println("\n=== 自定义异常 ===");
        try {
            withdraw(1000, 1500);
        } catch (InsufficientFundsException e) {
            System.out.println("余额不足: 需要" + e.getAmount());
        }
        
        // 7. 异常链
        System.out.println("\n=== 异常链 ===");
        try {
            methodA();
        } catch (Exception e) {
            System.out.println("最终异常: " + e.getMessage());
            System.out.println("原因: " + e.getCause().getMessage());
        }
    }
    
    // 声明抛出异常
    public static void checkAge(int age) throws IllegalArgumentException {
        if (age < 0) {
            throw new IllegalArgumentException("年龄不能为负数");
        }
    }
    
    // 自定义异常使用
    public static void withdraw(double balance, double amount) 
            throws InsufficientFundsException {
        if (amount > balance) {
            throw new InsufficientFundsException(amount - balance);
        }
    }
    
    // 异常链
    public static void methodA() throws Exception {
        try {
            methodB();
        } catch (Exception e) {
            throw new Exception("方法A中的异常", e);
        }
    }
    
    public static void methodB() throws Exception {
        throw new Exception("方法B中的原始异常");
    }
}

// 自动关闭资源示例
class MyResource implements AutoCloseable {
    public void doSomething() {
        System.out.println("使用资源");
    }
    
    @Override
    public void close() throws Exception {
        System.out.println("自动关闭资源");
    }
}

// 自定义异常
class InsufficientFundsException extends Exception {
    private double amount;
    
    public InsufficientFundsException(double amount) {
        super("余额不足");
        this.amount = amount;
    }
    
    public double getAmount() {
        return amount;
    }
}
```

---

## 第七章：Java多线程编程

### 7.1 线程的创建

```java
/**
 * 创建线程的4种方式
 */

// 方式1：继承Thread类
class MyThread extends Thread {
    @Override
    public void run() {
        for (int i = 0; i < 5; i++) {
            System.out.println(Thread.currentThread().getName() + ": " + i);
            try {
                Thread.sleep(100);  // 休眠100ms
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}

// 方式2：实现Runnable接口（推荐）
class MyRunnable implements Runnable {
    @Override
    public void run() {
        for (int i = 0; i < 5; i++) {
            System.out.println(Thread.currentThread().getName() + ": " + i);
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}

// 方式3：实现Callable接口（可以有返回值）
import java.util.concurrent.*;

class MyCallable implements Callable<Integer> {
    @Override
    public Integer call() throws Exception {
        int sum = 0;
        for (int i = 1; i <= 100; i++) {
            sum += i;
        }
        return sum;
    }
}

// 测试类
public class ThreadCreation {
    public static void main(String[] args) throws Exception {
        System.out.println("=== 方式1：继承Thread ===");
        MyThread t1 = new MyThread();
        t1.start();  // 启动线程，调用run方法
        
        System.out.println("\n=== 方式2：实现Runnable ===");
        MyRunnable r = new MyRunnable();
        Thread t2 = new Thread(r);
        t2.start();
        
        // Lambda简化（Java 8+）
        Thread t3 = new Thread(() -> {
            for (int i = 0; i < 5; i++) {
                System.out.println(Thread.currentThread().getName() + ": " + i);
            }
        });
        t3.start();
        
        System.out.println("\n=== 方式3：实现Callable ===");
        MyCallable callable = new MyCallable();
        FutureTask<Integer> future = new FutureTask<>(callable);
        Thread t4 = new Thread(future);
        t4.start();
        
        // 获取返回值（会阻塞）
        Integer result = future.get();
        System.out.println("计算结果: " + result);
        
        System.out.println("\n=== 方式4：线程池（推荐）===");
        ExecutorService executor = Executors.newFixedThreadPool(3);
        
        // 提交任务
        executor.submit(() -> {
            System.out.println("线程池任务1");
        });
        
        executor.submit(() -> {
            System.out.println("线程池任务2");
        });
        
        // 关闭线程池
        executor.shutdown();
    }
}
```

### 7.2 线程的状态和方法

```java
/**
 * 线程状态和常用方法
 */
public class ThreadStateDemo {
    public static void main(String[] args) throws InterruptedException {
        System.out.println("=== 线程状态 ===");
        /*
         * 线程的6种状态（Thread.State枚举）：
         * 1. NEW - 新建状态，线程被创建但未启动
         * 2. RUNNABLE - 可运行状态，包括就绪和运行中
         * 3. BLOCKED - 阻塞状态，等待监视器锁
         * 4. WAITING - 等待状态，无限期等待
         * 5. TIMED_WAITING - 定时等待状态
         * 6. TERMINATED - 终止状态，线程执行完毕
         */
        
        Thread thread = new Thread(() -> {
            System.out.println("线程运行中");
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        });
        
        System.out.println("创建后状态: " + thread.getState());  // NEW
        thread.start();
        System.out.println("启动后状态: " + thread.getState());  // RUNNABLE
        
        Thread.sleep(100);
        System.out.println("运行时状态: " + thread.getState());  // TIMED_WAITING
        
        thread.join();  // 等待线程执行完毕
        System.out.println("结束后状态: " + thread.getState());  // TERMINATED
        
        System.out.println("\n=== 线程常用方法 ===");
        
        // 1. sleep - 休眠
        System.out.println("开始休眠");
        Thread.sleep(1000);
        System.out.println("休眠结束");
        
        // 2. join - 等待线程结束
        Thread t1 = new Thread(() -> {
            for (int i = 0; i < 3; i++) {
                System.out.println("t1: " + i);
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        });
        
        t1.start();
        t1.join();  // 主线程等待t1执行完毕
        System.out.println("t1执行完毕");
        
        // 3. yield - 让出CPU
        Thread t2 = new Thread(() -> {
            for (int i = 0; i < 5; i++) {
                System.out.println("t2: " + i);
                Thread.yield();  // 提示调度器可以让出CPU
            }
        });
        t2.start();
        
        // 4. interrupt - 中断线程
        Thread t3 = new Thread(() -> {
            while (!Thread.currentThread().isInterrupted()) {
                System.out.println("t3运行中...");
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    System.out.println("t3被中断");
                    break;  // 退出循环
                }
            }
        });
        
        t3.start();
        Thread.sleep(1500);
        t3.interrupt();  // 发送中断信号
        
        // 5. 线程优先级
        Thread t4 = new Thread(() -> System.out.println("t4执行"));
        t4.setPriority(Thread.MAX_PRIORITY);  // 1-10，默认5
        System.out.println("t4优先级: " + t4.getPriority());
        
        // 6. 守护线程
        Thread daemon = new Thread(() -> {
            while (true) {
                System.out.println("守护线程运行");
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    break;
                }
            }
        });
        daemon.setDaemon(true);  // 设置为守护线程
        daemon.start();
        
        Thread.sleep(2000);
        System.out.println("主线程结束，守护线程也会结束");
    }
}
```

### 7.3 线程同步

```java
/**
 * 线程同步：解决多线程访问共享资源的安全问题
 */

// 模拟售票系统
class TicketDemo {
    public static void main(String[] args) {
        // 不安全的售票
        UnsafeTicket ticket1 = new UnsafeTicket();
        new Thread(ticket1, "窗口1").start();
        new Thread(ticket1, "窗口2").start();
        new Thread(ticket1, "窗口3").start();
        
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        
        System.out.println("\n=== 使用synchronized同步 ===");
        SafeTicket ticket2 = new SafeTicket();
        new Thread(ticket2, "窗口1").start();
        new Thread(ticket2, "窗口2").start();
        new Thread(ticket2, "窗口3").start();
    }
}

// 不安全的售票（会出现重复售票、负数票）
class UnsafeTicket implements Runnable {
    private int ticketCount = 10;
    
    @Override
    public void run() {
        while (ticketCount > 0) {
            try {
                Thread.sleep(100);  // 模拟售票耗时
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            System.out.println(Thread.currentThread().getName() + 
                             "售出第" + ticketCount-- + "张票");
        }
    }
}

// 安全的售票（synchronized同步）
class SafeTicket implements Runnable {
    private int ticketCount = 10;
    
    @Override
    public void run() {
        while (true) {
            if (!sellTicket()) {
                break;
            }
        }
    }
    
    // 同步方法
    private synchronized boolean sellTicket() {
        if (ticketCount <= 0) {
            return false;
        }
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        System.out.println(Thread.currentThread().getName() + 
                         "售出第" + ticketCount-- + "张票");
        return true;
    }
}

/**
 * synchronized的3种用法
 */
class SynchronizedDemo {
    private int count = 0;
    private Object lock = new Object();
    
    // 1. 同步方法（锁是this）
    public synchronized void method1() {
        count++;
    }
    
    // 2. 同步代码块（锁是this）
    public void method2() {
        synchronized (this) {
            count++;
        }
    }
    
    // 3. 同步代码块（锁是指定对象）
    public void method3() {
        synchronized (lock) {
            count++;
        }
    }
    
    // 4. 同步静态方法（锁是类对象）
    public static synchronized void method4() {
        System.out.println("静态同步方法");
    }
}

/**
 * Lock接口（更灵活的锁）
 */
import java.util.concurrent.locks.*;

class LockDemo {
    private int count = 0;
    private Lock lock = new ReentrantLock();
    
    public void increment() {
        lock.lock();  // 获取锁
        try {
            count++;
        } finally {
            lock.unlock();  // 释放锁（必须在finally中）
        }
    }
    
    // tryLock：尝试获取锁
    public boolean tryIncrement() {
        if (lock.tryLock()) {
            try {
                count++;
                return true;
            } finally {
                lock.unlock();
            }
        }
        return false;
    }
    
    // 读写锁：读读不互斥，读写、写写互斥
    private ReadWriteLock rwLock = new ReentrantReadWriteLock();
    private int value = 0;
    
    public int read() {
        rwLock.readLock().lock();
        try {
            Thread.sleep(100);
            return value;
        } catch (InterruptedException e) {
            return -1;
        } finally {
            rwLock.readLock().unlock();
        }
    }
    
    public void write(int value) {
        rwLock.writeLock().lock();
        try {
            Thread.sleep(100);
            this.value = value;
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            rwLock.writeLock().unlock();
        }
    }
}
```

### 7.4 线程通信

```java
/**
 * 线程通信：生产者-消费者模式
 */
class ProducerConsumerDemo {
    public static void main(String[] args) {
        Warehouse warehouse = new Warehouse();
        
        // 生产者线程
        new Thread(() -> {
            for (int i = 1; i <= 10; i++) {
                warehouse.produce(i);
            }
        }, "生产者").start();
        
        // 消费者线程
        new Thread(() -> {
            for (int i = 1; i <= 10; i++) {
                warehouse.consume();
            }
        }, "消费者").start();
    }
}

// 仓库类
class Warehouse {
    private int product = 0;
    private int capacity = 5;
    
    // 生产
    public synchronized void produce(int num) {
        while (product >= capacity) {
            try {
                System.out.println("仓库已满，生产者等待");
                wait();  // 等待，释放锁
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        product++;
        System.out.println("生产者生产了产品" + num + "，当前库存: " + product);
        notifyAll();  // 唤醒所有等待的线程
    }
    
    // 消费
    public synchronized void consume() {
        while (product <= 0) {
            try {
                System.out.println("仓库为空，消费者等待");
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        System.out.println("消费者消费了产品，当前库存: " + --product);
        notifyAll();
    }
}
```



### 7.5 线程池

```java
import java.util.concurrent.*;

/**
 * 线程池详解
 * 优点：
 * 1. 降低资源消耗（复用线程）
 * 2. 提高响应速度（不需要创建线程）
 * 3. 提高线程的可管理性
 */
public class ThreadPoolDemo {
    public static void main(String[] args) {
        // 1. 固定大小线程池
        ExecutorService fixedPool = Executors.newFixedThreadPool(3);
        for (int i = 0; i < 5; i++) {
            final int taskId = i;
            fixedPool.submit(() -> {
                System.out.println("Fixed Pool - Task" + taskId + 
                                 " by " + Thread.currentThread().getName());
            });
        }
        
        // 2. 单线程池
        ExecutorService singlePool = Executors.newSingleThreadExecutor();
        
        // 3. 缓存线程池（动态调整）
        ExecutorService cachedPool = Executors.newCachedThreadPool();
        
        // 4. 定时线程池
        ScheduledExecutorService scheduledPool = Executors.newScheduledThreadPool(2);
        
        // 延迟3秒执行
        scheduledPool.schedule(() -> {
            System.out.println("延迟任务执行");
        }, 3, TimeUnit.SECONDS);
        
        // 周期性执行（固定频率）
        scheduledPool.scheduleAtFixedRate(() -> {
            System.out.println("周期任务: " + System.currentTimeMillis());
        }, 0, 1, TimeUnit.SECONDS);
        
        // 5. 自定义线程池（推荐）
        ThreadPoolExecutor customPool = new ThreadPoolExecutor(
            2,                      // 核心线程数
            5,                      // 最大线程数
            60L,                    // 空闲线程存活时间
            TimeUnit.SECONDS,       // 时间单位
            new LinkedBlockingQueue<>(10),  // 任务队列
            Executors.defaultThreadFactory(),  // 线程工厂
            new ThreadPoolExecutor.AbortPolicy()  // 拒绝策略
        );
        
        // 关闭线程池
        fixedPool.shutdown();  // 等待任务完成后关闭
        // fixedPool.shutdownNow();  // 立即关闭
    }
}
```

---

## 第八章：文件IO操作

### 8.1 File类

```java
import java.io.File;
import java.io.IOException;
import java.util.Date;

/**
 * File类：文件和目录的抽象表示
 */
public class FileDemo {
    public static void main(String[] args) throws IOException {
        // 创建File对象
        File file = new File("/Users/test/example.txt");
        File dir = new File("/Users/test");
        
        // 文件信息
        System.out.println("文件名: " + file.getName());
        System.out.println("父路径: " + file.getParent());
        System.out.println("绝对路径: " + file.getAbsolutePath());
        System.out.println("文件大小: " + file.length() + " bytes");
        System.out.println("最后修改时间: " + new Date(file.lastModified()));
        
        // 判断
        System.out.println("是否存在: " + file.exists());
        System.out.println("是否是文件: " + file.isFile());
        System.out.println("是否是目录: " + file.isDirectory());
        System.out.println("是否可读: " + file.canRead());
        System.out.println("是否可写: " + file.canWrite());
        System.out.println("是否隐藏: " + file.isHidden());
        
        // 创建文件
        File newFile = new File("test.txt");
        if (newFile.createNewFile()) {
            System.out.println("文件创建成功");
        }
        
        // 创建目录
        File newDir = new File("testDir");
        if (newDir.mkdir()) {
            System.out.println("目录创建成功");
        }
        
        // 创建多级目录
        File multiDir = new File("a/b/c");
        if (multiDir.mkdirs()) {
            System.out.println("多级目录创建成功");
        }
        
        // 删除
        // file.delete();
        
        // 列出目录内容
        File currentDir = new File(".");
        String[] files = currentDir.list();
        for (String name : files) {
            System.out.println(name);
        }
        
        // 列出文件对象
        File[] fileObjects = currentDir.listFiles();
        for (File f : fileObjects) {
            if (f.isFile()) {
                System.out.println("文件: " + f.getName());
            } else {
                System.out.println("目录: " + f.getName());
            }
        }
        
        // 文件过滤
        File[] txtFiles = currentDir.listFiles((dir2, name) -> name.endsWith(".txt"));
        
        // 重命名
        // file.renameTo(new File("newName.txt"));
    }
}
```

### 8.2 字节流

```java
import java.io.*;

/**
 * 字节流：以字节为单位读写数据
 * InputStream/OutputStream
 */
public class ByteStreamDemo {
    public static void main(String[] args) {
        // 1. FileInputStream - 文件字节输入流
        System.out.println("=== 读取文件 ===");
        try (FileInputStream fis = new FileInputStream("input.txt")) {
            // 方式1：一次读一个字节
            int b;
            while ((b = fis.read()) != -1) {
                System.out.print((char) b);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // 方式2：一次读多个字节
        try (FileInputStream fis = new FileInputStream("input.txt")) {
            byte[] buffer = new byte[1024];
            int len;
            while ((len = fis.read(buffer)) != -1) {
                String str = new String(buffer, 0, len);
                System.out.print(str);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // 2. FileOutputStream - 文件字节输出流
        System.out.println("\n=== 写入文件 ===");
        try (FileOutputStream fos = new FileOutputStream("output.txt")) {
            String data = "Hello, Java IO!";
            fos.write(data.getBytes());
            System.out.println("写入成功");
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // 追加模式
        try (FileOutputStream fos = new FileOutputStream("output.txt", true)) {
            fos.write("\n追加内容".getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // 3. 文件复制
        System.out.println("\n=== 文件复制 ===");
        copyFile("source.txt", "dest.txt");
        
        // 4. BufferedInputStream/BufferedOutputStream - 缓冲流
        System.out.println("\n=== 缓冲流 ===");
        try (BufferedInputStream bis = new BufferedInputStream(
                 new FileInputStream("input.txt"));
             BufferedOutputStream bos = new BufferedOutputStream(
                 new FileOutputStream("output.txt"))) {
            
            byte[] buffer = new byte[1024];
            int len;
            while ((len = bis.read(buffer)) != -1) {
                bos.write(buffer, 0, len);
            }
            bos.flush();  // 刷新缓冲区
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    // 文件复制方法
    public static void copyFile(String src, String dest) {
        try (FileInputStream fis = new FileInputStream(src);
             FileOutputStream fos = new FileOutputStream(dest)) {
            
            byte[] buffer = new byte[1024];
            int len;
            while ((len = fis.read(buffer)) != -1) {
                fos.write(buffer, 0, len);
            }
            System.out.println("复制成功");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
```

### 8.3 字符流

```java
import java.io.*;

/**
 * 字符流：以字符为单位读写数据（自动处理编码）
 * Reader/Writer
 */
public class CharStreamDemo {
    public static void main(String[] args) {
        // 1. FileReader - 文件字符输入流
        System.out.println("=== 读取文件 ===");
        try (FileReader fr = new FileReader("input.txt")) {
            int c;
            while ((c = fr.read()) != -1) {
                System.out.print((char) c);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // 2. FileWriter - 文件字符输出流
        System.out.println("\n=== 写入文件 ===");
        try (FileWriter fw = new FileWriter("output.txt")) {
            fw.write("Hello, 世界!");
            System.out.println("写入成功");
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // 3. BufferedReader/BufferedWriter - 缓冲字符流
        System.out.println("\n=== 缓冲字符流 ===");
        try (BufferedReader br = new BufferedReader(
                 new FileReader("input.txt"));
             BufferedWriter bw = new BufferedWriter(
                 new FileWriter("output.txt"))) {
            
            String line;
            while ((line = br.readLine()) != null) {  // 按行读取
                bw.write(line);
                bw.newLine();  // 写入换行符
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // 4. 指定编码
        System.out.println("\n=== 指定编码 ===");
        try (InputStreamReader isr = new InputStreamReader(
                 new FileInputStream("input.txt"), "UTF-8");
             OutputStreamWriter osw = new OutputStreamWriter(
                 new FileOutputStream("output.txt"), "UTF-8")) {
            
            int c;
            while ((c = isr.read()) != -1) {
                osw.write(c);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
```

### 8.4 对象序列化

```java
import java.io.*;

/**
 * 对象序列化：将对象转换为字节序列
 * 对象反序列化：将字节序列恢复为对象
 */
public class SerializationDemo {
    public static void main(String[] args) {
        // 序列化
        Person person = new Person("张三", 25);
        serializeObject(person, "person.ser");
        
        // 反序列化
        Person p = (Person) deserializeObject("person.ser");
        System.out.println("反序列化: " + p);
    }
    
    // 序列化对象
    public static void serializeObject(Object obj, String filename) {
        try (ObjectOutputStream oos = new ObjectOutputStream(
                 new FileOutputStream(filename))) {
            oos.writeObject(obj);
            System.out.println("对象已序列化: " + filename);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    // 反序列化对象
    public static Object deserializeObject(String filename) {
        try (ObjectInputStream ois = new ObjectInputStream(
                 new FileInputStream(filename))) {
            return ois.readObject();
        } catch (IOException | ClassNotFoundException e) {
            e.printStackTrace();
            return null;
        }
    }
}

// 可序列化的类（必须实现Serializable接口）
class Person implements Serializable {
    private static final long serialVersionUID = 1L;  // 版本号
    
    private String name;
    private int age;
    private transient String password;  // transient：不序列化
    
    public Person(String name, int age) {
        this.name = name;
        this.age = age;
    }
    
    @Override
    public String toString() {
        return "Person{name='" + name + "', age=" + age + "}";
    }
}
```



---

## 第九章：网络编程Socket

### 9.1 网络编程基础

```
网络通信协议：
- TCP/IP协议族
  - 应用层：HTTP, FTP, SMTP等
  - 传输层：TCP（可靠）, UDP（不可靠但快）
  - 网络层：IP
  - 链路层

IP地址：网络中设备的唯一标识
端口号：0-65535，标识进程
  - 公认端口：0-1023（HTTP:80, HTTPS:443, FTP:21）
  - 注册端口：1024-49151
  - 动态端口：49152-65535
```

### 9.2 TCP编程

```java
import java.io.*;
import java.net.*;

/**
 * TCP客户端
 * 特点：面向连接，可靠传输
 */
public class TCPClient {
    public static void main(String[] args) {
        try {
            // 1. 创建Socket连接到服务器
            Socket socket = new Socket("localhost", 8888);
            System.out.println("连接到服务器: " + socket.getInetAddress());
            
            // 2. 获取输出流，向服务器发送数据
            OutputStream os = socket.getOutputStream();
            PrintWriter pw = new PrintWriter(os);
            pw.println("Hello, Server!");
            pw.flush();
            
            // 3. 获取输入流，接收服务器响应
            InputStream is = socket.getInputStream();
            BufferedReader br = new BufferedReader(new InputStreamReader(is));
            String response = br.readLine();
            System.out.println("服务器响应: " + response);
            
            // 4. 关闭连接
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

/**
 * TCP服务器
 */
public class TCPServer {
    public static void main(String[] args) {
        try {
            // 1. 创建ServerSocket，监听端口
            ServerSocket serverSocket = new ServerSocket(8888);
            System.out.println("服务器启动，监听端口8888...");
            
            // 2. 等待客户端连接
            while (true) {
                Socket clientSocket = serverSocket.accept();  // 阻塞等待
                System.out.println("客户端连接: " + clientSocket.getInetAddress());
                
                // 3. 处理客户端请求（开启新线程）
                new Thread(new ClientHandler(clientSocket)).start();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

// 客户端处理器
class ClientHandler implements Runnable {
    private Socket socket;
    
    public ClientHandler(Socket socket) {
        this.socket = socket;
    }
    
    @Override
    public void run() {
        try {
            // 接收客户端数据
            InputStream is = socket.getInputStream();
            BufferedReader br = new BufferedReader(new InputStreamReader(is));
            String message = br.readLine();
            System.out.println("收到消息: " + message);
            
            // 发送响应
            OutputStream os = socket.getOutputStream();
            PrintWriter pw = new PrintWriter(os);
            pw.println("Hello, Client! 收到你的消息: " + message);
            pw.flush();
            
            // 关闭连接
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
```

### 9.3 简单聊天室实现

```java
import java.io.*;
import java.net.*;
import java.util.*;

/**
 * 聊天室服务器
 */
public class ChatServer {
    private static Set<ClientThread> clients = Collections.synchronizedSet(new HashSet<>());
    
    public static void main(String[] args) {
        try (ServerSocket serverSocket = new ServerSocket(8888)) {
            System.out.println("聊天室服务器启动...");
            
            while (true) {
                Socket socket = serverSocket.accept();
                ClientThread client = new ClientThread(socket);
                clients.add(client);
                client.start();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    // 广播消息给所有客户端
    public static void broadcast(String message, ClientThread sender) {
        synchronized (clients) {
            for (ClientThread client : clients) {
                if (client != sender) {
                    client.sendMessage(message);
                }
            }
        }
    }
    
    // 移除客户端
    public static void removeClient(ClientThread client) {
        clients.remove(client);
        broadcast(client.getClientName() + " 离开了聊天室", client);
    }
}

/**
 * 客户端线程
 */
class ClientThread extends Thread {
    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;
    private String clientName;
    
    public ClientThread(Socket socket) {
        this.socket = socket;
    }
    
    @Override
    public void run() {
        try {
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);
            
            // 获取客户端昵称
            out.println("请输入你的昵称:");
            clientName = in.readLine();
            ChatServer.broadcast(clientName + " 加入了聊天室", this);
            
            // 接收并广播消息
            String message;
            while ((message = in.readLine()) != null) {
                if (message.equals("exit")) {
                    break;
                }
                ChatServer.broadcast(clientName + ": " + message, this);
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                socket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            ChatServer.removeClient(this);
        }
    }
    
    public void sendMessage(String message) {
        out.println(message);
    }
    
    public String getClientName() {
        return clientName;
    }
}

/**
 * 聊天室客户端
 */
public class ChatClient {
    public static void main(String[] args) {
        try {
            Socket socket = new Socket("localhost", 8888);
            
            // 接收消息线程
            new Thread(() -> {
                try {
                    BufferedReader in = new BufferedReader(
                        new InputStreamReader(socket.getInputStream()));
                    String message;
                    while ((message = in.readLine()) != null) {
                        System.out.println(message);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }).start();
            
            // 发送消息线程
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader console = new BufferedReader(
                new InputStreamReader(System.in));
            
            String message;
            while ((message = console.readLine()) != null) {
                out.println(message);
                if (message.equals("exit")) {
                    break;
                }
            }
            
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
```

### 9.4 UDP编程

```java
import java.net.*;
import java.io.*;

/**
 * UDP发送端
 * 特点：无连接，不可靠但快速
 */
public class UDPSender {
    public static void main(String[] args) {
        try {
            // 1. 创建DatagramSocket
            DatagramSocket socket = new DatagramSocket();
            
            // 2. 准备数据
            String message = "Hello, UDP!";
            byte[] data = message.getBytes();
            
            // 3. 创建数据包，指定目标地址和端口
            InetAddress address = InetAddress.getByName("localhost");
            DatagramPacket packet = new DatagramPacket(data, data.length, address, 9999);
            
            // 4. 发送数据包
            socket.send(packet);
            System.out.println("消息已发送: " + message);
            
            // 5. 关闭socket
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

/**
 * UDP接收端
 */
public class UDPReceiver {
    public static void main(String[] args) {
        try {
            // 1. 创建DatagramSocket，绑定端口
            DatagramSocket socket = new DatagramSocket(9999);
            System.out.println("UDP接收端启动，监听端口9999...");
            
            // 2. 准备接收缓冲区
            byte[] buffer = new byte[1024];
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
            
            // 3. 接收数据包
            socket.receive(packet);  // 阻塞等待
            
            // 4. 解析数据
            String message = new String(packet.getData(), 0, packet.getLength());
            InetAddress address = packet.getAddress();
            int port = packet.getPort();
            
            System.out.println("收到来自 " + address + ":" + port + " 的消息: " + message);
            
            // 5. 关闭socket
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
```

### 9.5 URL编程

```java
import java.net.*;
import java.io.*;

/**
 * URL编程：访问网络资源
 */
public class URLDemo {
    public static void main(String[] args) {
        try {
            // 1. 创建URL对象
            URL url = new URL("https://www.example.com/index.html?name=test#section1");
            
            // 2. 获取URL各个部分
            System.out.println("协议: " + url.getProtocol());  // https
            System.out.println("主机: " + url.getHost());      // www.example.com
            System.out.println("端口: " + url.getPort());      // -1（默认端口）
            System.out.println("路径: " + url.getPath());      // /index.html
            System.out.println("文件: " + url.getFile());      // /index.html?name=test
            System.out.println("查询: " + url.getQuery());     // name=test
            System.out.println("锚点: " + url.getRef());       // section1
            
            // 3. 读取网页内容
            System.out.println("\n=== 读取网页内容 ===");
            readURL("https://www.baidu.com");
            
            // 4. 下载文件
            System.out.println("\n=== 下载文件 ===");
            downloadFile("https://www.example.com/file.txt", "downloaded.txt");
            
        } catch (MalformedURLException e) {
            System.out.println("URL格式错误");
        }
    }
    
    // 读取URL内容
    public static void readURL(String urlStr) {
        try {
            URL url = new URL(urlStr);
            URLConnection conn = url.openConnection();
            
            // 读取内容
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(conn.getInputStream(), "UTF-8"));
            
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    // 下载文件
    public static void downloadFile(String urlStr, String savePath) {
        try {
            URL url = new URL(urlStr);
            InputStream is = url.openStream();
            FileOutputStream fos = new FileOutputStream(savePath);
            
            byte[] buffer = new byte[1024];
            int len;
            while ((len = is.read(buffer)) != -1) {
                fos.write(buffer, 0, len);
            }
            
            fos.close();
            is.close();
            System.out.println("文件下载完成: " + savePath);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
```

---

## 第十章：高级特性

### 10.1 泛型

```java
/**
 * 泛型：参数化类型，提供编译时类型检查
 */

// 1. 泛型类
class Box<T> {
    private T content;
    
    public void set(T content) {
        this.content = content;
    }
    
    public T get() {
        return content;
    }
}

// 2. 泛型接口
interface Generator<T> {
    T generate();
}

// 3. 泛型方法
class GenericMethod {
    // 泛型方法
    public static <T> void printArray(T[] array) {
        for (T element : array) {
            System.out.print(element + " ");
        }
        System.out.println();
    }
    
    // 多个类型参数
    public static <K, V> void printKeyValue(K key, V value) {
        System.out.println(key + " = " + value);
    }
}

// 4. 泛型通配符
class WildcardDemo {
    // ? 表示任意类型
    public static void printList(List<?> list) {
        for (Object obj : list) {
            System.out.println(obj);
        }
    }
    
    // ? extends E 上界通配符（只能读取）
    public static double sumOfList(List<? extends Number> list) {
        double sum = 0;
        for (Number num : list) {
            sum += num.doubleValue();
        }
        return sum;
    }
    
    // ? super E 下界通配符（只能写入）
    public static void addNumbers(List<? super Integer> list) {
        list.add(1);
        list.add(2);
        list.add(3);
    }
}

// 测试类
public class GenericDemo {
    public static void main(String[] args) {
        // 泛型类使用
        Box<String> stringBox = new Box<>();
        stringBox.set("Hello");
        String str = stringBox.get();  // 不需要类型转换
        
        Box<Integer> intBox = new Box<>();
        intBox.set(123);
        Integer num = intBox.get();
        
        // 泛型方法使用
        Integer[] intArray = {1, 2, 3, 4, 5};
        String[] strArray = {"A", "B", "C"};
        
        GenericMethod.printArray(intArray);
        GenericMethod.printArray(strArray);
        
        GenericMethod.printKeyValue("name", "张三");
        GenericMethod.printKeyValue("age", 25);
        
        // 泛型的好处：
        // 1. 类型安全（编译时检查）
        // 2. 消除类型转换
        // 3. 代码重用
    }
}
```

### 10.2 反射

```java
import java.lang.reflect.*;

/**
 * 反射：在运行时动态获取类的信息和操作类
 * 应用：框架开发、动态代理、注解处理
 */
public class ReflectionDemo {
    public static void main(String[] args) throws Exception {
        // 1. 获取Class对象的三种方式
        Class<?> clazz1 = Class.forName("java.lang.String");
        Class<?> clazz2 = String.class;
        Class<?> clazz3 = "hello".getClass();
        
        System.out.println(clazz1 == clazz2);  // true（同一个对象）
        
        // 2. 创建对象
        Class<?> personClass = Person2.class;
        Person2 person = (Person2) personClass.newInstance();  // 调用无参构造
        
        // 3. 获取构造方法
        Constructor<?>[] constructors = personClass.getConstructors();
        for (Constructor<?> c : constructors) {
            System.out.println("构造方法: " + c);
        }
        
        Constructor<?> constructor = personClass.getConstructor(String.class, int.class);
        Person2 p2 = (Person2) constructor.newInstance("张三", 25);
        
        // 4. 获取字段
        Field[] fields = personClass.getDeclaredFields();
        for (Field field : fields) {
            System.out.println("字段: " + field.getName() + ", 类型: " + field.getType());
        }
        
        // 访问私有字段
        Field nameField = personClass.getDeclaredField("name");
        nameField.setAccessible(true);  // 跳过访问权限检查
        nameField.set(p2, "李四");
        System.out.println("修改后: " + p2);
        
        // 5. 获取方法
        Method[] methods = personClass.getDeclaredMethods();
        for (Method method : methods) {
            System.out.println("方法: " + method.getName());
        }
        
        // 调用方法
        Method sayHello = personClass.getMethod("sayHello");
        sayHello.invoke(p2);
        
        // 6. 获取类信息
        System.out.println("类名: " + personClass.getName());
        System.out.println("简单类名: " + personClass.getSimpleName());
        System.out.println("包名: " + personClass.getPackage());
        System.out.println("父类: " + personClass.getSuperclass());
        
        Class<?>[] interfaces = personClass.getInterfaces();
        System.out.println("实现的接口: " + Arrays.toString(interfaces));
    }
}

class Person2 {
    private String name;
    private int age;
    
    public Person2() {}
    
    public Person2(String name, int age) {
        this.name = name;
        this.age = age;
    }
    
    public void sayHello() {
        System.out.println("Hello, I'm " + name);
    }
    
    @Override
    public String toString() {
        return "Person{name='" + name + "', age=" + age + "}";
    }
}
```

### 10.3 Lambda表达式和Stream API

```java
import java.util.*;
import java.util.stream.*;

/**
 * Lambda表达式（Java 8+）
 * 函数式编程
 */
public class LambdaDemo {
    public static void main(String[] args) {
        // 1. Lambda表达式基础
        List<String> list = Arrays.asList("Apple", "Banana", "Cherry");
        
        // 传统方式
        list.forEach(new Consumer<String>() {
            @Override
            public void accept(String s) {
                System.out.println(s);
            }
        });
        
        // Lambda方式
        list.forEach(s -> System.out.println(s));
        
        // 方法引用
        list.forEach(System.out::println);
        
        // 2. Stream API
        List<Integer> numbers = Arrays.asList(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        
        // filter - 过滤
        List<Integer> evens = numbers.stream()
            .filter(n -> n % 2 == 0)
            .collect(Collectors.toList());
        System.out.println("偶数: " + evens);
        
        // map - 映射转换
        List<Integer> squares = numbers.stream()
            .map(n -> n * n)
            .collect(Collectors.toList());
        System.out.println("平方: " + squares);
        
        // reduce - 聚合
        int sum = numbers.stream()
            .reduce(0, (a, b) -> a + b);
        System.out.println("求和: " + sum);
        
        // sorted - 排序
        List<String> sorted = list.stream()
            .sorted()
            .collect(Collectors.toList());
        
        // distinct - 去重
        List<Integer> distinct = Arrays.asList(1, 2, 2, 3, 3, 3, 4)
            .stream()
            .distinct()
            .collect(Collectors.toList());
        
        // limit - 限制数量
        List<Integer> limited = numbers.stream()
            .limit(5)
            .collect(Collectors.toList());
        
        // skip - 跳过
        List<Integer> skipped = numbers.stream()
            .skip(5)
            .collect(Collectors.toList());
        
        // 链式调用
        double avg = numbers.stream()
            .filter(n -> n % 2 == 0)
            .mapToInt(Integer::intValue)
            .average()
            .orElse(0.0);
        System.out.println("偶数平均值: " + avg);
        
        // 3. 常用函数式接口
        // Predicate<T> - 断言
        Predicate<Integer> isEven = n -> n % 2 == 0;
        System.out.println("4是偶数: " + isEven.test(4));
        
        // Function<T, R> - 函数
        Function<String, Integer> strLength = s -> s.length();
        System.out.println("长度: " + strLength.apply("Hello"));
        
        // Consumer<T> - 消费者
        Consumer<String> printer = s -> System.out.println(s);
        printer.accept("Hello");
        
        // Supplier<T> - 供给者
        Supplier<Double> random = () -> Math.random();
        System.out.println("随机数: " + random.get());
    }
}
```

---

## 总结

这份教程涵盖了Java从入门到精通的核心内容：
1. ✅ **Java基础** - 数据类型、运算符、流程控制
2. ✅ **面向对象** - 封装、继承、多态、抽象类、接口
3. ✅ **核心类库** - String、Math、日期时间
4. ✅ **包装类** - 自动装箱拆箱、缓存机制
5. ✅ **集合框架** - List、Set、Map及其实现类
6. ✅ **异常处理** - try-catch-finally、自定义异常
7. ✅ **多线程** - 线程创建、同步、通信、线程池
8. ✅ **文件IO** - 字节流、字符流、序列化
9. ✅ **网络编程** - TCP、UDP、Socket通信
10. ✅ **高级特性** - 泛型、反射、Lambda、Stream

建议学习路径：
- **初学者**: 第1-4章 → 第6章 → 第5章
- **进阶**: 第7章 → 第8章 → 第9章
- **高级**: 第10章 → 深入JVM、设计模式、框架学习

---

*持续更新中...*


---

## 第十一章：Java注解与反射深入

### 11.1 注解基础

```java
import java.lang.annotation.*;

/**
 * 注解（Annotation）
 * 元数据，为代码提供额外信息
 */

// 自定义注解
@Target(ElementType.METHOD)  // 作用目标
@Retention(RetentionPolicy.RUNTIME)  // 保留策略
@Documented  // 生成文档
public @interface MyAnnotation {
    String value() default "";
    int priority() default 0;
    String[] tags() default {};
}

// 使用注解
class AnnotationDemo {
    @MyAnnotation(value = "test", priority = 1, tags = {"tag1", "tag2"})
    public void testMethod() {
        System.out.println("测试方法");
    }
}

/**
 * 元注解
 */
// @Target：指定注解的作用目标
// ElementType.TYPE - 类、接口、枚举
// ElementType.FIELD - 字段
// ElementType.METHOD - 方法
// ElementType.PARAMETER - 参数
// ElementType.CONSTRUCTOR - 构造器
// ElementType.LOCAL_VARIABLE - 局部变量
// ElementType.ANNOTATION_TYPE - 注解
// ElementType.PACKAGE - 包

// @Retention：指定注解的保留策略
// RetentionPolicy.SOURCE - 源码阶段，编译后丢弃
// RetentionPolicy.CLASS - 类文件阶段，运行时不可用（默认）
// RetentionPolicy.RUNTIME - 运行时可用，可通过反射获取

// @Inherited：注解可被继承
// @Repeatable：可重复注解（Java 8+）
```

### 11.2 常用内置注解

```java
/**
 * Java内置注解
 */

// @Override：重写父类方法
class Parent {
    public void method() {}
}

class Child extends Parent {
    @Override
    public void method() {
        // 重写父类方法
    }
}

// @Deprecated：标记过时
@Deprecated
public void oldMethod() {
    System.out.println("这个方法已过时");
}

// @SuppressWarnings：抑制警告
@SuppressWarnings("unchecked")
public void unsafeMethod() {
    List list = new ArrayList();  // 原始类型
    list.add("item");
}

// @SafeVarargs：抑制可变参数警告（Java 7+）
@SafeVarargs
public final <T> void printAll(T... items) {
    for (T item : items) {
        System.out.println(item);
    }
}

// @FunctionalInterface：函数式接口（Java 8+）
@FunctionalInterface
interface Calculator {
    int calculate(int a, int b);
}
```

### 11.3 注解处理器

```java
import java.lang.reflect.*;

/**
 * 注解处理器
 * 运行时读取和处理注解
 */
public class AnnotationProcessor {
    public static void main(String[] args) throws Exception {
        Class<?> clazz = AnnotationDemo.class;
        
        // 获取类上的注解
        if (clazz.isAnnotationPresent(MyAnnotation.class)) {
            MyAnnotation annotation = clazz.getAnnotation(MyAnnotation.class);
            System.out.println("Value: " + annotation.value());
        }
        
        // 获取方法上的注解
        Method method = clazz.getMethod("testMethod");
        if (method.isAnnotationPresent(MyAnnotation.class)) {
            MyAnnotation annotation = method.getAnnotation(MyAnnotation.class);
            System.out.println("Value: " + annotation.value());
            System.out.println("Priority: " + annotation.priority());
            System.out.println("Tags: " + Arrays.toString(annotation.tags()));
        }
        
        // 获取所有注解
        Annotation[] annotations = method.getAnnotations();
        for (Annotation ann : annotations) {
            System.out.println("Annotation: " + ann.annotationType().getName());
        }
    }
}
```

### 11.4 实战：简单的依赖注入框架

```java
/**
 * 依赖注入示例
 */

// 自动注入注解
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD)
@interface Autowired {
}

// 服务类
class UserService {
    public void saveUser(String name) {
        System.out.println("保存用户: " + name);
    }
}

// 控制器类
class UserController {
    @Autowired
    private UserService userService;
    
    public void createUser(String name) {
        userService.saveUser(name);
    }
}

// 简单的IOC容器
class SimpleIOC {
    public static void inject(Object target) throws Exception {
        Class<?> clazz = target.getClass();
        
        // 遍历所有字段
        for (Field field : clazz.getDeclaredFields()) {
            if (field.isAnnotationPresent(Autowired.class)) {
                // 创建依赖对象
                Object dependency = field.getType().newInstance();
                
                // 注入依赖
                field.setAccessible(true);
                field.set(target, dependency);
                
                System.out.println("注入依赖: " + field.getName());
            }
        }
    }
    
    public static void main(String[] args) throws Exception {
        UserController controller = new UserController();
        inject(controller);
        controller.createUser("张三");
    }
}
```

---

## 第十二章：Java 8+ Stream API详解

### 12.1 Stream基础

```java
import java.util.stream.*;

/**
 * Stream API
 * 函数式编程风格处理集合
 */
public class StreamBasics {
    public static void main(String[] args) {
        List<Integer> numbers = Arrays.asList(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        
        // 创建Stream
        Stream<Integer> stream1 = numbers.stream();
        Stream<Integer> stream2 = Stream.of(1, 2, 3, 4, 5);
        Stream<Integer> stream3 = Stream.iterate(0, n -> n + 2).limit(10);
        Stream<Double> stream4 = Stream.generate(Math::random).limit(5);
        
        // 过滤
        List<Integer> evens = numbers.stream()
            .filter(n -> n % 2 == 0)
            .collect(Collectors.toList());
        System.out.println("偶数: " + evens);
        
        // 映射
        List<Integer> squares = numbers.stream()
            .map(n -> n * n)
            .collect(Collectors.toList());
        System.out.println("平方: " + squares);
        
        // 去重
        List<Integer> distinct = Arrays.asList(1, 1, 2, 2, 3, 3).stream()
            .distinct()
            .collect(Collectors.toList());
        
        // 排序
        List<Integer> sorted = numbers.stream()
            .sorted((a, b) -> b - a)  // 降序
            .collect(Collectors.toList());
        
        // 限制数量
        List<Integer> limited = numbers.stream()
            .limit(5)
            .collect(Collectors.toList());
        
        // 跳过
        List<Integer> skipped = numbers.stream()
            .skip(5)
            .collect(Collectors.toList());
    }
}
```

### 12.2 Stream高级操作

```java
/**
 * Stream高级操作
 */
public class StreamAdvanced {
    public static void main(String[] args) {
        List<String> words = Arrays.asList("Hello", "World", "Java", "Stream");
        
        // flatMap：扁平化
        List<String> letters = words.stream()
            .flatMap(word -> Arrays.stream(word.split("")))
            .distinct()
            .collect(Collectors.toList());
        System.out.println("所有字母: " + letters);
        
        // peek：调试
        List<Integer> result = Stream.of(1, 2, 3, 4, 5)
            .peek(n -> System.out.println("处理前: " + n))
            .map(n -> n * 2)
            .peek(n -> System.out.println("处理后: " + n))
            .collect(Collectors.toList());
        
        // reduce：聚合
        int sum = Stream.of(1, 2, 3, 4, 5)
            .reduce(0, (a, b) -> a + b);
        System.out.println("求和: " + sum);
        
        int product = Stream.of(1, 2, 3, 4, 5)
            .reduce(1, (a, b) -> a * b);
        System.out.println("求积: " + product);
        
        // 查找
        Optional<Integer> first = Stream.of(1, 2, 3, 4, 5)
            .filter(n -> n > 3)
            .findFirst();
        
        Optional<Integer> any = Stream.of(1, 2, 3, 4, 5)
            .filter(n -> n > 3)
            .findAny();
        
        // 匹配
        boolean allMatch = Stream.of(1, 2, 3, 4, 5)
            .allMatch(n -> n > 0);  // 所有元素都满足
        
        boolean anyMatch = Stream.of(1, 2, 3, 4, 5)
            .anyMatch(n -> n > 3);  // 至少一个元素满足
        
        boolean noneMatch = Stream.of(1, 2, 3, 4, 5)
            .noneMatch(n -> n < 0);  // 没有元素满足
        
        // 计数
        long count = Stream.of(1, 2, 3, 4, 5)
            .filter(n -> n % 2 == 0)
            .count();
        
        // 最大最小值
        Optional<Integer> max = Stream.of(1, 2, 3, 4, 5)
            .max(Integer::compareTo);
        
        Optional<Integer> min = Stream.of(1, 2, 3, 4, 5)
            .min(Integer::compareTo);
    }
}
```

### 12.3 Collectors收集器

```java
import java.util.stream.Collectors;

/**
 * Collectors收集器
 */
public class CollectorsDemo {
    public static void main(String[] args) {
        List<String> names = Arrays.asList("Alice", "Bob", "Charlie", "David", "Eve");
        
        // toList
        List<String> list = names.stream().collect(Collectors.toList());
        
        // toSet
        Set<String> set = names.stream().collect(Collectors.toSet());
        
        // toMap
        Map<String, Integer> map = names.stream()
            .collect(Collectors.toMap(
                name -> name,
                name -> name.length()
            ));
        
        // joining：字符串拼接
        String joined = names.stream()
            .collect(Collectors.joining(", "));
        System.out.println("拼接: " + joined);
        
        String withPrefix = names.stream()
            .collect(Collectors.joining(", ", "[", "]"));
        System.out.println("带前后缀: " + withPrefix);
        
        // counting：计数
        long count = names.stream()
            .collect(Collectors.counting());
        
        // summingInt：求和
        int totalLength = names.stream()
            .collect(Collectors.summingInt(String::length));
        
        // averagingInt：平均值
        double avgLength = names.stream()
            .collect(Collectors.averagingInt(String::length));
        
        // summarizingInt：统计信息
        IntSummaryStatistics stats = names.stream()
            .collect(Collectors.summarizingInt(String::length));
        System.out.println("统计: " + stats);
        
        // groupingBy：分组
        Map<Integer, List<String>> groupByLength = names.stream()
            .collect(Collectors.groupingBy(String::length));
        System.out.println("按长度分组: " + groupByLength);
        
        // partitioningBy：分区（二分）
        Map<Boolean, List<String>> partition = names.stream()
            .collect(Collectors.partitioningBy(name -> name.length() > 4));
        System.out.println("分区: " + partition);
        
        // 多级分组
        Map<Integer, Map<Character, List<String>>> multiGroup = names.stream()
            .collect(Collectors.groupingBy(
                String::length,
                Collectors.groupingBy(name -> name.charAt(0))
            ));
    }
}
```

### 12.4 并行Stream

```java
/**
 * 并行Stream
 * 利用多核处理器提升性能
 */
public class ParallelStreamDemo {
    public static void main(String[] args) {
        List<Integer> numbers = new ArrayList<>();
        for (int i = 1; i <= 1000000; i++) {
            numbers.add(i);
        }
        
        // 串行Stream
        long start = System.currentTimeMillis();
        long sum1 = numbers.stream()
            .map(n -> n * n)
            .reduce(0L, Long::sum);
        long end = System.currentTimeMillis();
        System.out.println("串行耗时: " + (end - start) + "ms");
        
        // 并行Stream
        start = System.currentTimeMillis();
        long sum2 = numbers.parallelStream()
            .map(n -> n * n)
            .reduce(0L, Long::sum);
        end = System.currentTimeMillis();
        System.out.println("并行耗时: " + (end - start) + "ms");
        
        // 注意：并行Stream的线程安全问题
        List<Integer> result = new ArrayList<>();
        numbers.parallelStream()
            .forEach(result::add);  // 不安全！
        
        // 正确做法
        List<Integer> safeResult = numbers.parallelStream()
            .collect(Collectors.toList());
        
        // 自定义并行度
        System.setProperty("java.util.concurrent.ForkJoinPool.common.parallelism", "4");
    }
}
```

---

## 第十三章：JVM虚拟机基础

### 13.1 JVM内存结构

```java
/**
 * JVM内存结构
 * 
 * 1. 程序计数器（Program Counter Register）
 *    - 当前线程执行的字节码行号指示器
 *    - 线程私有
 *    - 唯一不会OOM的区域
 * 
 * 2. Java虚拟机栈（VM Stack）
 *    - 存储局部变量、操作数栈、方法出口等
 *    - 线程私有
 *    - StackOverflowError / OutOfMemoryError
 * 
 * 3. 本地方法栈（Native Method Stack）
 *    - 为Native方法服务
 *    - 线程私有
 * 
 * 4. Java堆（Heap）
 *    - 存储对象实例
 *    - 线程共享
 *    - 垃圾回收的主要区域
 *    - OutOfMemoryError: Java heap space
 * 
 * 5. 方法区（Method Area）/ 元空间（Metaspace）
 *    - 存储类信息、常量、静态变量、JIT编译后的代码
 *    - 线程共享
 *    - OutOfMemoryError: Metaspace
 * 
 * 6. 运行时常量池（Runtime Constant Pool）
 *    - 方法区的一部分
 *    - 存储编译期生成的各种字面量和符号引用
 * 
 * 7. 直接内存（Direct Memory）
 *    - 不是JVM运行时数据区的一部分
 *    - NIO使用
 */

public class JVMMemoryDemo {
    // 类变量（方法区）
    private static int classVar = 100;
    
    // 实例变量（堆）
    private int instanceVar = 200;
    
    public void method() {
        // 局部变量（栈）
        int localVar = 300;
        
        // 对象（堆）
        String str = new String("Hello");
        
        // 常量（常量池）
        String constant = "World";
    }
    
    public static void main(String[] args) {
        // 查看JVM内存信息
        Runtime runtime = Runtime.getRuntime();
        
        long maxMemory = runtime.maxMemory();      // 最大内存
        long totalMemory = runtime.totalMemory();  // 总内存
        long freeMemory = runtime.freeMemory();    // 空闲内存
        
        System.out.println("最大内存: " + maxMemory / 1024 / 1024 + "MB");
        System.out.println("总内存: " + totalMemory / 1024 / 1024 + "MB");
        System.out.println("空闲内存: " + freeMemory / 1024 / 1024 + "MB");
        System.out.println("已用内存: " + (totalMemory - freeMemory) / 1024 / 1024 + "MB");
    }
}
```

### 13.2 垃圾回收

```java
/**
 * 垃圾回收（Garbage Collection）
 * 
 * 判断对象是否可回收：
 * 1. 引用计数法（Reference Counting）
 *    - 简单，但无法解决循环引用
 * 
 * 2. 可达性分析（Reachability Analysis）
 *    - 从GC Roots开始搜索
 *    - GC Roots包括：
 *      * 虚拟机栈中引用的对象
 *      * 方法区中静态属性引用的对象
 *      * 方法区中常量引用的对象
 *      * 本地方法栈中引用的对象
 * 
 * 垃圾回收算法：
 * 1. 标记-清除（Mark-Sweep）
 *    - 标记所有存活对象，清除未标记对象
 *    - 缺点：产生内存碎片
 * 
 * 2. 复制（Copying）
 *    - 将内存分为两块，每次只使用一块
 *    - 优点：不产生碎片
 *    - 缺点：浪费一半内存
 * 
 * 3. 标记-整理（Mark-Compact）
 *    - 标记后移动存活对象，消除碎片
 * 
 * 4. 分代收集（Generational Collection）
 *    - 新生代：复制算法
 *    - 老年代：标记-清除或标记-整理
 * 
 * 垃圾收集器：
 * - Serial：单线程
 * - ParNew：Serial的多线程版本
 * - Parallel Scavenge：吞吐量优先
 * - CMS（Concurrent Mark Sweep）：低停顿
 * - G1（Garbage First）：可预测停顿
 * - ZGC：超低停顿（Java 11+）
 */

public class GCDemo {
    public static void main(String[] args) {
        // 手动触发GC（不保证立即执行）
        System.gc();
        
        // finalize方法（不推荐使用）
        // 对象被回收前调用，但不保证一定执行
        
        // 查看GC信息
        // JVM参数：
        // -XX:+PrintGC              // 打印GC简要信息
        // -XX:+PrintGCDetails       // 打印GC详细信息
        // -XX:+PrintGCDateStamps    // 打印GC时间戳
        // -Xloggc:gc.log            // GC日志文件
        
        // 内存泄漏示例
        List<byte[]> list = new ArrayList<>();
        while (true) {
            list.add(new byte[1024 * 1024]);  // 1MB
            // OutOfMemoryError
        }
    }
}
```

### 13.3 JVM参数调优

```java
/**
 * JVM常用参数
 * 
 * 内存设置：
 * -Xms512m              // 初始堆大小
 * -Xmx1024m             // 最大堆大小
 * -Xmn256m              // 新生代大小
 * -Xss256k              // 栈大小
 * -XX:MetaspaceSize=128m    // 元空间初始大小
 * -XX:MaxMetaspaceSize=256m // 元空间最大大小
 * 
 * 垃圾收集器：
 * -XX:+UseSerialGC          // 使用Serial GC
 * -XX:+UseParallelGC        // 使用Parallel GC
 * -XX:+UseConcMarkSweepGC   // 使用CMS GC
 * -XX:+UseG1GC              // 使用G1 GC
 * -XX:+UseZGC               // 使用ZGC（Java 11+）
 * 
 * GC日志：
 * -XX:+PrintGCDetails       // 打印GC详情
 * -XX:+PrintGCDateStamps    // 打印GC时间戳
 * -Xloggc:gc.log            // GC日志文件
 * 
 * 性能监控：
 * -XX:+HeapDumpOnOutOfMemoryError  // OOM时生成堆转储
 * -XX:HeapDumpPath=/path/to/dump   // 堆转储路径
 * 
 * 调优建议：
 * 1. 新生代设置为堆的1/3到1/2
 * 2. Xms和Xmx设置相同，避免动态扩展
 * 3. 根据应用类型选择合适的GC
 * 4. 定期分析GC日志
 */

public class JVMTuning {
    public static void main(String[] args) {
        // 运行时信息
        RuntimeMXBean runtimeMXBean = ManagementFactory.getRuntimeMXBean();
        List<String> arguments = runtimeMXBean.getInputArguments();
        System.out.println("JVM参数:");
        for (String arg : arguments) {
            System.out.println(arg);
        }
        
        // 内存信息
        MemoryMXBean memoryMXBean = ManagementFactory.getMemoryMXBean();
        MemoryUsage heapMemory = memoryMXBean.getHeapMemoryUsage();
        System.out.println("\n堆内存:");
        System.out.println("初始: " + heapMemory.getInit() / 1024 / 1024 + "MB");
        System.out.println("已用: " + heapMemory.getUsed() / 1024 / 1024 + "MB");
        System.out.println("提交: " + heapMemory.getCommitted() / 1024 / 1024 + "MB");
        System.out.println("最大: " + heapMemory.getMax() / 1024 / 1024 + "MB");
    }
}
```



---

## 第十四章：设计模式

### 14.1 单例模式

```java
/**
 * 单例模式（Singleton Pattern）
 * 确保一个类只有一个实例
 */

// 1. 饿汉式（线程安全）
class Singleton1 {
    private static final Singleton1 INSTANCE = new Singleton1();
    
    private Singleton1() {}
    
    public static Singleton1 getInstance() {
        return INSTANCE;
    }
}

// 2. 懒汉式（线程不安全）
class Singleton2 {
    private static Singleton2 instance;
    
    private Singleton2() {}
    
    public static Singleton2 getInstance() {
        if (instance == null) {
            instance = new Singleton2();
        }
        return instance;
    }
}

// 3. 懒汉式（线程安全，但效率低）
class Singleton3 {
    private static Singleton3 instance;
    
    private Singleton3() {}
    
    public static synchronized Singleton3 getInstance() {
        if (instance == null) {
            instance = new Singleton3();
        }
        return instance;
    }
}

// 4. 双重检查锁定（推荐）
class Singleton4 {
    private static volatile Singleton4 instance;
    
    private Singleton4() {}
    
    public static Singleton4 getInstance() {
        if (instance == null) {
            synchronized (Singleton4.class) {
                if (instance == null) {
                    instance = new Singleton4();
                }
            }
        }
        return instance;
    }
}

// 5. 静态内部类（推荐）
class Singleton5 {
    private Singleton5() {}
    
    private static class Holder {
        private static final Singleton5 INSTANCE = new Singleton5();
    }
    
    public static Singleton5 getInstance() {
        return Holder.INSTANCE;
    }
}

// 6. 枚举（最佳实践）
enum Singleton6 {
    INSTANCE;
    
    public void doSomething() {
        System.out.println("Singleton method");
    }
}
```

### 14.2 工厂模式

```java
/**
 * 工厂模式（Factory Pattern）
 */

// 产品接口
interface Product {
    void use();
}

// 具体产品
class ConcreteProductA implements Product {
    @Override
    public void use() {
        System.out.println("使用产品A");
    }
}

class ConcreteProductB implements Product {
    @Override
    public void use() {
        System.out.println("使用产品B");
    }
}

// 简单工厂
class SimpleFactory {
    public static Product createProduct(String type) {
        switch (type) {
            case "A": return new ConcreteProductA();
            case "B": return new ConcreteProductB();
            default: throw new IllegalArgumentException("Unknown type");
        }
    }
}

// 工厂方法模式
interface Factory {
    Product createProduct();
}

class FactoryA implements Factory {
    @Override
    public Product createProduct() {
        return new ConcreteProductA();
    }
}

class FactoryB implements Factory {
    @Override
    public Product createProduct() {
        return new ConcreteProductB();
    }
}

// 抽象工厂模式
interface AbstractFactory {
    ProductA createProductA();
    ProductB createProductB();
}

class ConcreteFactory1 implements AbstractFactory {
    @Override
    public ProductA createProductA() {
        return new ProductA1();
    }
    
    @Override
    public ProductB createProductB() {
        return new ProductB1();
    }
}
```

### 14.3 观察者模式

```java
/**
 * 观察者模式（Observer Pattern）
 * 定义对象间一对多的依赖关系
 */

// 观察者接口
interface Observer {
    void update(String message);
}

// 主题接口
interface Subject {
    void attach(Observer observer);
    void detach(Observer observer);
    void notifyObservers();
}

// 具体主题
class ConcreteSubject implements Subject {
    private List<Observer> observers = new ArrayList<>();
    private String state;
    
    @Override
    public void attach(Observer observer) {
        observers.add(observer);
    }
    
    @Override
    public void detach(Observer observer) {
        observers.remove(observer);
    }
    
    @Override
    public void notifyObservers() {
        for (Observer observer : observers) {
            observer.update(state);
        }
    }
    
    public void setState(String state) {
        this.state = state;
        notifyObservers();
    }
}

// 具体观察者
class ConcreteObserver implements Observer {
    private String name;
    
    public ConcreteObserver(String name) {
        this.name = name;
    }
    
    @Override
    public void update(String message) {
        System.out.println(name + " received: " + message);
    }
}

// 使用示例
class ObserverDemo {
    public static void main(String[] args) {
        ConcreteSubject subject = new ConcreteSubject();
        
        Observer obs1 = new ConcreteObserver("Observer 1");
        Observer obs2 = new ConcreteObserver("Observer 2");
        
        subject.attach(obs1);
        subject.attach(obs2);
        
        subject.setState("状态改变了");
    }
}
```

### 14.4 策略模式

```java
/**
 * 策略模式（Strategy Pattern）
 * 定义一系列算法，封装每个算法，使它们可以互换
 */

// 策略接口
interface PaymentStrategy {
    void pay(int amount);
}

// 具体策略
class CreditCardStrategy implements PaymentStrategy {
    private String cardNumber;
    
    public CreditCardStrategy(String cardNumber) {
        this.cardNumber = cardNumber;
    }
    
    @Override
    public void pay(int amount) {
        System.out.println("使用信用卡 " + cardNumber + " 支付 " + amount + " 元");
    }
}

class AlipayStrategy implements PaymentStrategy {
    private String account;
    
    public AlipayStrategy(String account) {
        this.account = account;
    }
    
    @Override
    public void pay(int amount) {
        System.out.println("使用支付宝账号 " + account + " 支付 " + amount + " 元");
    }
}

// 上下文
class ShoppingCart {
    private PaymentStrategy paymentStrategy;
    
    public void setPaymentStrategy(PaymentStrategy strategy) {
        this.paymentStrategy = strategy;
    }
    
    public void checkout(int amount) {
        paymentStrategy.pay(amount);
    }
}

// 使用示例
class StrategyDemo {
    public static void main(String[] args) {
        ShoppingCart cart = new ShoppingCart();
        
        // 使用信用卡支付
        cart.setPaymentStrategy(new CreditCardStrategy("1234-5678-9012-3456"));
        cart.checkout(100);
        
        // 切换为支付宝支付
        cart.setPaymentStrategy(new AlipayStrategy("user@example.com"));
        cart.checkout(200);
    }
}
```

### 14.5 装饰器模式

```java
/**
 * 装饰器模式（Decorator Pattern）
 * 动态地给对象添加额外的职责
 */

// 组件接口
interface Coffee {
    String getDescription();
    double getCost();
}

// 具体组件
class SimpleCoffee implements Coffee {
    @Override
    public String getDescription() {
        return "简单咖啡";
    }
    
    @Override
    public double getCost() {
        return 10.0;
    }
}

// 装饰器基类
abstract class CoffeeDecorator implements Coffee {
    protected Coffee coffee;
    
    public CoffeeDecorator(Coffee coffee) {
        this.coffee = coffee;
    }
    
    @Override
    public String getDescription() {
        return coffee.getDescription();
    }
    
    @Override
    public double getCost() {
        return coffee.getCost();
    }
}

// 具体装饰器
class MilkDecorator extends CoffeeDecorator {
    public MilkDecorator(Coffee coffee) {
        super(coffee);
    }
    
    @Override
    public String getDescription() {
        return coffee.getDescription() + ", 加牛奶";
    }
    
    @Override
    public double getCost() {
        return coffee.getCost() + 2.0;
    }
}

class SugarDecorator extends CoffeeDecorator {
    public SugarDecorator(Coffee coffee) {
        super(coffee);
    }
    
    @Override
    public String getDescription() {
        return coffee.getDescription() + ", 加糖";
    }
    
    @Override
    public double getCost() {
        return coffee.getCost() + 1.0;
    }
}

// 使用示例
class DecoratorDemo {
    public static void main(String[] args) {
        Coffee coffee = new SimpleCoffee();
        System.out.println(coffee.getDescription() + " - " + coffee.getCost() + "元");
        
        coffee = new MilkDecorator(coffee);
        System.out.println(coffee.getDescription() + " - " + coffee.getCost() + "元");
        
        coffee = new SugarDecorator(coffee);
        System.out.println(coffee.getDescription() + " - " + coffee.getCost() + "元");
    }
}
```

---

## 第十五章：性能优化与最佳实践

### 15.1 代码优化技巧

```java
/**
 * Java性能优化技巧
 */
public class PerformanceOptimization {
    
    // 1. 使用StringBuilder而不是字符串拼接
    public String badStringConcat(List<String> list) {
        String result = "";
        for (String s : list) {
            result += s;  // 每次都创建新String对象
        }
        return result;
    }
    
    public String goodStringConcat(List<String> list) {
        StringBuilder sb = new StringBuilder();
        for (String s : list) {
            sb.append(s);
        }
        return sb.toString();
    }
    
    // 2. 避免在循环中创建对象
    public void badLoop() {
        for (int i = 0; i < 1000; i++) {
            Object obj = new Object();  // 创建1000个对象
        }
    }
    
    public void goodLoop() {
        Object obj = new Object();  // 只创建1个对象
        for (int i = 0; i < 1000; i++) {
            // 重用obj
        }
    }
    
    // 3. 使用局部变量
    private int instanceVar = 100;
    
    public void badMethod() {
        for (int i = 0; i < 1000; i++) {
            System.out.println(instanceVar);  // 每次访问实例变量
        }
    }
    
    public void goodMethod() {
        int local = instanceVar;  // 复制到局部变量
        for (int i = 0; i < 1000; i++) {
            System.out.println(local);
        }
    }
    
    // 4. 延迟初始化
    private ExpensiveObject expensive;
    
    public ExpensiveObject getExpensive() {
        if (expensive == null) {
            expensive = new ExpensiveObject();  // 只在需要时创建
        }
        return expensive;
    }
    
    // 5. 使用枚举代替常量
    // 不好的方式
    public static final int TYPE_A = 1;
    public static final int TYPE_B = 2;
    
    // 好的方式
    public enum Type {
        TYPE_A, TYPE_B
    }
    
    // 6. 合理使用缓存
    private Map<String, Result> cache = new ConcurrentHashMap<>();
    
    public Result getResult(String key) {
        return cache.computeIfAbsent(key, k -> computeExpensiveResult(k));
    }
    
    private Result computeExpensiveResult(String key) {
        // 耗时操作
        return new Result();
    }
    
    // 7. 避免过度同步
    // 不好的方式
    public synchronized void badSync() {
        // 整个方法都同步
        doSomething();
        doAnotherThing();
    }
    
    // 好的方式
    public void goodSync() {
        doSomething();  // 不需要同步
        synchronized (this) {
            doAnotherThing();  // 只同步必要的部分
        }
    }
    
    // 8. 使用适当的集合类型
    public void collectionChoice() {
        // 频繁随机访问：ArrayList
        List<String> list = new ArrayList<>();
        
        // 频繁插入删除：LinkedList
        List<String> linkedList = new LinkedList<>();
        
        // 不需要重复：HashSet
        Set<String> set = new HashSet<>();
        
        // 需要排序：TreeSet
        Set<String> treeSet = new TreeSet<>();
        
        // 键值对，无序：HashMap
        Map<String, String> map = new HashMap<>();
        
        // 键值对，有序：TreeMap
        Map<String, String> treeMap = new TreeMap<>();
    }
}
```

### 15.2 内存优化

```java
/**
 * 内存优化技巧
 */
public class MemoryOptimization {
    
    // 1. 及时释放资源
    public void releaseResources() {
        Connection conn = null;
        try {
            conn = getConnection();
            // 使用连接
        } finally {
            if (conn != null) {
                conn.close();  // 确保释放
            }
        }
        
        // 更好的方式：try-with-resources
        try (Connection connection = getConnection()) {
            // 自动关闭
        }
    }
    
    // 2. 避免内存泄漏
    // 问题：监听器未移除
    class LeakExample {
        private List<Listener> listeners = new ArrayList<>();
        
        public void addListener(Listener listener) {
            listeners.add(listener);
        }
        
        public void removeListener(Listener listener) {
            listeners.remove(listener);  // 记得移除！
        }
    }
    
    // 3. 使用弱引用
    class CacheWithWeakReference {
        private Map<String, WeakReference<Object>> cache = new HashMap<>();
        
        public void put(String key, Object value) {
            cache.put(key, new WeakReference<>(value));
        }
        
        public Object get(String key) {
            WeakReference<Object> ref = cache.get(key);
            return ref != null ? ref.get() : null;
        }
    }
    
    // 4. 对象池
    class ObjectPool<T> {
        private Queue<T> pool = new ConcurrentLinkedQueue<>();
        private int maxSize;
        
        public T acquire() {
            T obj = pool.poll();
            return obj != null ? obj : createNew();
        }
        
        public void release(T obj) {
            if (pool.size() < maxSize) {
                reset(obj);
                pool.offer(obj);
            }
        }
        
        private T createNew() { return null; }
        private void reset(T obj) {}
    }
    
    // 5. 使用原始类型而不是包装类
    public void primitiveVsWrapper() {
        // 占用更多内存
        List<Integer> wrapperList = new ArrayList<>();
        
        // 更节省内存（需要时）
        int[] primitiveArray = new int[1000];
    }
}
```

### 15.3 最佳实践

```java
/**
 * Java开发最佳实践
 */
public class BestPractices {
    
    // 1. 始终重写equals时也重写hashCode
    class Person {
        private String name;
        private int age;
        
        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null || getClass() != obj.getClass()) return false;
            Person person = (Person) obj;
            return age == person.age && Objects.equals(name, person.name);
        }
        
        @Override
        public int hashCode() {
            return Objects.hash(name, age);
        }
    }
    
    // 2. 使用Optional避免空指针
    public Optional<String> findName(int id) {
        String name = database.get(id);
        return Optional.ofNullable(name);
    }
    
    public void useOptional() {
        findName(1).ifPresent(name -> System.out.println(name));
        String name = findName(1).orElse("Unknown");
    }
    
    // 3. 不要捕获Exception或Throwable
    public void badExceptionHandling() {
        try {
            // 代码
        } catch (Exception e) {  // 太宽泛
            e.printStackTrace();
        }
    }
    
    public void goodExceptionHandling() {
        try {
            // 代码
        } catch (IOException e) {  // 具体的异常
            logger.error("IO error", e);
        }
    }
    
    // 4. 使用日志框架而不是System.out
    // 不好
    public void badLogging() {
        System.out.println("Debug info");
    }
    
    // 好
    private static final Logger logger = LoggerFactory.getLogger(BestPractices.class);
    
    public void goodLogging() {
        logger.debug("Debug info");
        logger.info("Info message");
        logger.error("Error occurred", exception);
    }
    
    // 5. 使用依赖注入
    // 不好：紧耦合
    class BadService {
        private UserRepository repository = new UserRepositoryImpl();
    }
    
    // 好：松耦合
    class GoodService {
        private final UserRepository repository;
        
        public GoodService(UserRepository repository) {
            this.repository = repository;
        }
    }
    
    // 6. 不可变对象
    public final class ImmutablePerson {
        private final String name;
        private final int age;
        
        public ImmutablePerson(String name, int age) {
            this.name = name;
            this.age = age;
        }
        
        public String getName() { return name; }
        public int getAge() { return age; }
    }
    
    // 7. 使用枚举实现单例
    public enum Singleton {
        INSTANCE;
        
        public void doSomething() {
            // 线程安全，防止反序列化
        }
    }
    
    // 8. 合理使用注释
    // 不好：无意义的注释
    // 设置名字
    public void setName(String name) {
        this.name = name;
    }
    
    // 好：解释为什么
    /**
     * 重试3次，因为网络可能不稳定
     */
    public void retry() {
        for (int i = 0; i < 3; i++) {
            try {
                doSomething();
                break;
            } catch (Exception e) {
                if (i == 2) throw e;
            }
        }
    }
}
```

---

## 总结与学习建议

### Java学习路线图

```
基础阶段（1-2个月）：
├── Java语法基础
├── 面向对象
├── 常用类库
└── 集合框架

进阶阶段（2-3个月）：
├── 异常处理
├── IO流
├── 多线程
├── 网络编程
└── 反射与注解

高级阶段（3-6个月）：
├── JVM虚拟机
├── 并发编程深入
├── 设计模式
├── 性能优化
└── 框架学习（Spring、MyBatis等）

专家阶段（持续学习）：
├── 源码阅读
├── 分布式系统
├── 微服务架构
└── 性能调优
```

### 推荐资源

**书籍：**
- 《Java核心技术》- 入门经典
- 《Effective Java》- 最佳实践
- 《Java并发编程实战》- 并发专题
- 《深入理解Java虚拟机》- JVM深入
- 《Head First设计模式》- 设计模式

**在线资源：**
- Oracle官方文档
- GitHub优秀开源项目
- Stack Overflow
- LeetCode刷题

**工具：**
- IDE: IntelliJ IDEA
- 构建工具: Maven / Gradle
- 版本控制: Git
- 调试工具: JVisualVM, JProfiler

---

*教程完结 - 持续更新中*  
*最后更新：2026-06-18*

祝您学习愉快，成为Java专家！🚀
