---
title: "JVM虚拟机详解：从入门到精通"

category:
  - Java
tag:
  - Java
---
# JVM虚拟机详解：从入门到精通

> 全面深入理解Java虚拟机的核心原理、运行机制、性能调优与故障诊断

---

## 目录

1. [JVM基础入门](#1-jvm基础入门)
2. [JVM内存结构详解](#2-jvm内存结构详解)
3. [类加载机制](#3-类加载机制)
4. [字节码执行引擎](#4-字节码执行引擎)
5. [垃圾回收机制](#5-垃圾回收机制)
6. [JVM性能监控与诊断](#6-jvm性能监控与诊断)
7. [JVM调优实战](#7-jvm调优实战)
8. [高级特性与底层原理](#8-高级特性与底层原理)

---

## 1. JVM基础入门

### 1.1 什么是JVM

**JVM（Java Virtual Machine）** 是Java虚拟机的简称，是Java程序运行的核心基础设施。

#### 核心特性

- **平台无关性**：Write Once, Run Anywhere（一次编写，到处运行）
- **自动内存管理**：垃圾回收机制，自动管理内存分配和回收
- **安全性**：字节码验证、沙箱机制
- **高性能**：JIT即时编译、热点代码优化

#### JVM、JRE、JDK的关系

```
┌─────────────────────────────────────┐
│            JDK (开发工具包)            │
│  ┌───────────────────────────────┐  │
│  │      JRE (运行时环境)          │  │
│  │  ┌─────────────────────────┐  │  │
│  │  │       JVM (虚拟机)       │  │  │
│  │  │  - 类加载器              │  │  │
│  │  │  - 执行引擎              │  │  │
│  │  │  - 运行时数据区          │  │  │
│  │  └─────────────────────────┘  │  │
│  │  + Java核心类库               │  │
│  └───────────────────────────────┘  │
│  + 开发工具(javac, jar, javadoc等)  │
└─────────────────────────────────────┘
```

### 1.2 JVM规范与实现

#### 主流JVM实现

| JVM实现 | 开发者 | 特点 |
|---------|--------|------|
| **HotSpot VM** | Oracle/Sun | 最广泛使用，JDK默认VM |
| **OpenJ9** | IBM/Eclipse | 低内存占用，快速启动 |
| **GraalVM** | Oracle | 多语言支持，AOT编译 |
| **Azul Zing** | Azul Systems | 低延迟，企业级 |
| **JRockit** | Oracle | 已合并到HotSpot |

### 1.3 Java程序执行流程

```
┌──────────────┐
│  .java源文件  │
└──────┬───────┘
       │ javac编译
       ▼
┌──────────────┐
│  .class字节码 │
└──────┬───────┘
       │ 类加载器加载
       ▼
┌──────────────────────────────┐
│         JVM运行时            │
│  ┌────────────────────────┐  │
│  │   字节码验证器          │  │
│  └───────────┬────────────┘  │
│              ▼                │
│  ┌────────────────────────┐  │
│  │   解释器 / JIT编译器    │  │
│  └───────────┬────────────┘  │
│              ▼                │
│  ┌────────────────────────┐  │
│  │     机器码执行          │  │
│  └────────────────────────┘  │
└──────────────────────────────┘
```

---

## 2. JVM内存结构详解

### 2.1 运行时数据区概览

JVM运行时数据区分为**线程共享**和**线程私有**两类：

```
┌─────────────────────────────────────────────────────┐
│                    JVM运行时数据区                    │
├─────────────────────────────────────────────────────┤
│                   线程共享区域                        │
│  ┌───────────────────────────────────────────────┐  │
│  │              方法区 (Method Area)              │  │
│  │  - 类信息、常量、静态变量、JIT编译后的代码      │  │
│  │  - 运行时常量池 (Runtime Constant Pool)       │  │
│  └───────────────────────────────────────────────┘  │
│  ┌───────────────────────────────────────────────┐  │
│  │              堆 (Heap)                         │  │
│  │  - 对象实例                                    │  │
│  │  - 数组                                        │  │
│  │  - 垃圾回收的主要区域                          │  │
│  └───────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────┤
│                   线程私有区域                        │
│  ┌───────────────┐ ┌───────────────┐ ┌──────────┐  │
│  │  程序计数器    │ │  虚拟机栈      │ │ 本地方法栈│  │
│  │ (PC Register) │ │ (VM Stack)    │ │(Native   │  │
│  │               │ │               │ │ Stack)   │  │
│  └───────────────┘ └───────────────┘ └──────────┘  │
└─────────────────────────────────────────────────────┘
```

### 2.2 程序计数器（Program Counter Register）

#### 特点
- **线程私有**：每个线程都有独立的程序计数器
- **最小内存区域**：占用内存很小
- **唯一不会OOM**：不会出现OutOfMemoryError

#### 作用
- 记录当前线程执行的字节码行号
- 线程切换后能恢复到正确的执行位置
- 执行Native方法时，计数器值为空（Undefined）

```java
public class PCRegisterDemo {
    public static void main(String[] args) {
        int a = 10;           // PC指向第1条指令
        int b = 20;           // PC指向第2条指令
        int c = a + b;        // PC指向第3条指令
        System.out.println(c);// PC指向第4条指令
    }
}
```

### 2.3 Java虚拟机栈（VM Stack）

#### 栈的结构

```
线程栈
┌─────────────────────┐
│   栈帧3 (method3)   │ ← 栈顶（当前方法）
├─────────────────────┤
│   栈帧2 (method2)   │
├─────────────────────┤
│   栈帧1 (method1)   │
└─────────────────────┘
```

#### 栈帧（Stack Frame）内部结构

```
┌───────────────────────────────────┐
│          栈帧 (Stack Frame)        │
├───────────────────────────────────┤
│  局部变量表 (Local Variable Table) │
│  - 方法参数                        │
│  - 局部变量                        │
│  - Slot槽位存储                    │
├───────────────────────────────────┤
│  操作数栈 (Operand Stack)          │
│  - 临时数据存储                    │
│  - 方法调用和返回值传递            │
├───────────────────────────────────┤
│  动态链接 (Dynamic Linking)        │
│  - 指向运行时常量池的方法引用      │
├───────────────────────────────────┤
│  返回地址 (Return Address)         │
│  - 方法正常/异常退出的返回位置     │
└───────────────────────────────────┘
```

#### 局部变量表详解

```java
public int calculate(int a, int b) {
    int c = a + b;
    return c;
}
```

对应的局部变量表：

| 索引 | 变量 | 类型 | 说明 |
|------|------|------|------|
| 0 | this | reference | 实例方法隐式参数 |
| 1 | a | int | 方法参数 |
| 2 | b | int | 方法参数 |
| 3 | c | int | 局部变量 |

#### Slot槽位复用

```java
public void slotReuse() {
    {
        int a = 10;  // 占用Slot 1
    }  // a的作用域结束
    
    {
        int b = 20;  // 复用Slot 1
    }
}
```

#### 栈相关异常

```java
// StackOverflowError示例
public class StackOverflowDemo {
    private int depth = 0;
    
    public void recursion() {
        depth++;
        recursion();  // 无限递归
    }
    
    public static void main(String[] args) {
        StackOverflowDemo demo = new StackOverflowDemo();
        try {
            demo.recursion();
        } catch (StackOverflowError e) {
            System.out.println("栈深度：" + demo.depth);
            // HotSpot默认栈大小1MB，大约递归10000次
        }
    }
}
```

**调整栈大小**：
```bash
# 设置栈大小为2MB
java -Xss2m StackOverflowDemo
```

### 2.4 本地方法栈（Native Method Stack）

#### 特点
- 为Native方法服务
- HotSpot VM将本地方法栈和虚拟机栈合二为一
- 也会抛出StackOverflowError和OutOfMemoryError

```java
public class NativeMethodDemo {
    // Native方法声明
    public native void nativeMethod();
    
    static {
        // 加载本地库
        System.loadLibrary("native-lib");
    }
}
```

### 2.5 堆（Heap）

#### 堆的作用
- **最大内存区域**：占JVM内存的主要部分
- **对象分配**：几乎所有对象实例都在此分配
- **GC主战场**：垃圾回收器管理的主要区域
- **线程共享**：所有线程共享堆内存

#### 堆的分代结构（Generational Heap）

```
┌─────────────────────────────────────────────────────────┐
│                      Java堆 (Heap)                       │
├──────────────────────────────────┬──────────────────────┤
│         新生代 (Young Gen)        │  老年代 (Old Gen)     │
│            约1/3堆空间             │     约2/3堆空间       │
├────────┬────────┬────────────────┤                      │
│  Eden  │   S0   │      S1        │    Tenured Space     │
│  8/10  │  1/10  │     1/10       │                      │
│        │(From)  │     (To)       │                      │
└────────┴────────┴────────────────┴──────────────────────┘
         ↑                                    ↑
      新对象分配                         长期存活对象
```

#### 对象分配流程

```
新对象创建
     ↓
  ┌──────┐
  │ Eden │ 区分配
  └──┬───┘
     │ Eden区满了
     ▼
  ┌────────────┐
  │ Minor GC   │ (Young GC)
  └──┬────┬────┘
     │    │
存活 │    │ 回收
     ▼    ↓
┌────────┐ ╳
│Survivor│
│区(S0/S1)│
└───┬────┘
    │ 年龄达到阈值(默认15)
    │ 或Survivor区放不下
    ▼
┌────────┐
│ 老年代  │
│ (Old)  │
└───┬────┘
    │ 老年代满了
    ▼
┌────────────┐
│ Major GC   │ (Full GC)
│ (Full GC)  │
└────────────┘
```



#### 对象年龄计数

```java
public class ObjectAgingDemo {
    // 对象头中的分代年龄（4位，最大15）
    private static class User {
        private String name;
        private int age;
    }
    
    public static void main(String[] args) {
        User user = new User();
        // 每次Minor GC存活，年龄+1
        // 年龄达到15（或动态年龄判定），晋升到老年代
    }
}
```

#### 大对象直接进入老年代

```java
// -XX:PretenureSizeThreshold=1000000 (1MB)
// 大于该值的对象直接进入老年代
public class LargeObjectDemo {
    public static void main(String[] args) {
        // 大数组直接分配到老年代
        byte[] largeArray = new byte[2 * 1024 * 1024]; // 2MB
    }
}
```

#### 堆内存配置参数

```bash
# 堆内存配置
-Xms2g                    # 初始堆大小2GB
-Xmx4g                    # 最大堆大小4GB
-Xmn1g                    # 新生代大小1GB
-XX:SurvivorRatio=8       # Eden:Survivor=8:1
-XX:NewRatio=2            # 老年代:新生代=2:1
-XX:MaxTenuringThreshold=15  # 对象晋升老年代的年龄阈值

# 示例
java -Xms2g -Xmx2g -Xmn800m -XX:SurvivorRatio=8 MyApp
```

### 2.6 方法区（Method Area）

#### 方法区的作用
存储已被虚拟机加载的类型信息、常量、静态变量、即时编译器编译后的代码缓存等。

#### 方法区的演进

| JDK版本 | 实现 | 位置 | 说明 |
|---------|------|------|------|
| **JDK 7之前** | 永久代 (PermGen) | 堆内存 | 容易OOM |
| **JDK 7** | 永久代（过渡） | 堆内存 | 字符串常量池移到堆 |
| **JDK 8+** | 元空间 (Metaspace) | 本地内存 | 不再OOM，受限于物理内存 |

```
JDK 7之前                        JDK 8+
┌──────────────┐                ┌──────────────┐
│     堆       │                │     堆       │
│  ┌────────┐  │                │  ┌────────┐  │
│  │ 对象   │  │                │  │ 对象   │  │
│  │        │  │                │  │        │  │
│  │ 字符串 │  │ ← JDK7移到这   │  │ 字符串 │  │
│  │ 常量池 │  │                │  │ 常量池 │  │
│  └────────┘  │                │  └────────┘  │
├──────────────┤                └──────────────┘
│   永久代     │                ┌──────────────┐
│ (PermGen)    │                │  元空间      │
│ - 类元数据   │                │ (Metaspace)  │
│ - 方法信息   │                │ (本地内存)   │
│ - 常量池     │                │ - 类元数据   │
└──────────────┘                │ - 方法信息   │
                                └──────────────┘
```

#### 方法区存储内容

```java
public class MethodAreaDemo {
    // 静态变量 → 方法区
    private static int staticVar = 100;
    
    // 常量 → 方法区（运行时常量池）
    private static final String CONSTANT = "Hello";
    
    // 类信息 → 方法区
    // - 类的全限定名
    // - 父类信息
    // - 接口信息
    // - 访问修饰符
    // - 字段信息
    // - 方法信息
    // - 方法字节码
    
    public void method() {
        // 方法字节码 → 方法区
    }
}
```

#### 运行时常量池（Runtime Constant Pool）

```java
public class ConstantPoolDemo {
    public static void main(String[] args) {
        // 字符串字面量 → 字符串常量池（堆中）
        String s1 = "hello";
        String s2 = "hello";
        System.out.println(s1 == s2);  // true，指向同一个对象
        
        // new创建 → 堆中新对象
        String s3 = new String("hello");
        System.out.println(s1 == s3);  // false，不同对象
        
        // intern() → 尝试放入常量池
        String s4 = s3.intern();
        System.out.println(s1 == s4);  // true，指向常量池
    }
}
```

#### 方法区配置参数

```bash
# JDK 7及之前（永久代）
-XX:PermSize=128m           # 永久代初始大小
-XX:MaxPermSize=256m        # 永久代最大大小

# JDK 8+（元空间）
-XX:MetaspaceSize=128m      # 元空间初始大小
-XX:MaxMetaspaceSize=256m   # 元空间最大大小（默认无限制）
-XX:MinMetaspaceFreeRatio=40  # GC后最小空闲比例
-XX:MaxMetaspaceFreeRatio=70  # GC后最大空闲比例
```

#### 方法区OOM示例

```java
import java.util.ArrayList;
import java.util.List;
import net.sf.cglib.proxy.Enhancer;
import net.sf.cglib.proxy.MethodInterceptor;

/**
 * JDK 7: -XX:PermSize=10m -XX:MaxPermSize=10m
 * JDK 8+: -XX:MetaspaceSize=10m -XX:MaxMetaspaceSize=10m
 */
public class MetaspaceOOMDemo {
    public static void main(String[] args) {
        List<Class<?>> classes = new ArrayList<>();
        
        while (true) {
            // 动态生成类，填满方法区
            Enhancer enhancer = new Enhancer();
            enhancer.setSuperclass(Object.class);
            enhancer.setUseCache(false);
            enhancer.setCallback((MethodInterceptor) 
                (obj, method, args1, proxy) -> proxy.invokeSuper(obj, args1));
            
            classes.add(enhancer.createClass());
        }
        // 最终抛出：java.lang.OutOfMemoryError: Metaspace
    }
}
```

### 2.7 直接内存（Direct Memory）

#### 特点
- **不是JVM运行时数据区的一部分**
- **NIO（New IO）使用**：DirectByteBuffer
- **避免Java堆和Native堆之间来回复制数据**
- **不受JVM GC直接管理**，但受限于物理内存

#### DirectByteBuffer示例

```java
import java.nio.ByteBuffer;

public class DirectMemoryDemo {
    public static void main(String[] args) {
        // 堆内存分配
        ByteBuffer heapBuffer = ByteBuffer.allocate(1024);
        
        // 直接内存分配
        ByteBuffer directBuffer = ByteBuffer.allocateDirect(1024);
        
        // 直接内存优势：减少数据拷贝
        // 堆内存：Java堆 → Native内存 → IO设备
        // 直接内存：Native内存 → IO设备
    }
}
```

#### 直接内存配置

```bash
# 限制直接内存大小
-XX:MaxDirectMemorySize=512m

# 直接内存不足时会触发Full GC
# 如果Full GC后仍不足，抛出OutOfMemoryError
```

#### 直接内存OOM示例

```java
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * VM参数：-XX:MaxDirectMemorySize=10m
 */
public class DirectMemoryOOMDemo {
    public static void main(String[] args) {
        List<ByteBuffer> buffers = new ArrayList<>();
        
        while (true) {
            // 每次分配1MB直接内存
            ByteBuffer buffer = ByteBuffer.allocateDirect(1024 * 1024);
            buffers.add(buffer);
            
            System.out.println("分配：" + buffers.size() + "MB");
        }
        // 抛出：java.lang.OutOfMemoryError: Direct buffer memory
    }
}
```

---

## 3. 类加载机制

### 3.1 类的生命周期

```
┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐
│  加载    │→ │  验证    │→ │  准备    │→ │  解析    │→ │  初始化  │
│ Loading  │   │Verification│  │Preparation│  │Resolution│  │Initialization│
└──────────┘   └──────────┘   └──────────┘   └──────────┘   └──────────┘
     ↓              ↓              ↓              ↓              ↓
     └──────────────┴──────────────┴──────────────┴──────────────┘
                          连接 (Linking)
                                 ↓
                         ┌──────────┐
                         │   使用    │
                         │  Using   │
                         └──────────┘
                                 ↓
                         ┌──────────┐
                         │   卸载    │
                         │Unloading │
                         └──────────┘
```

### 3.2 类加载过程详解

#### 1. 加载（Loading）

**做三件事**：
1. 通过类的全限定名获取定义此类的二进制字节流
2. 将字节流代表的静态存储结构转化为方法区的运行时数据结构
3. 在内存中生成代表这个类的java.lang.Class对象

**获取字节流的方式**：
```java
// 1. 从ZIP包读取（JAR、WAR、EAR）
// 2. 从网络获取（Applet）
// 3. 运行时计算生成（动态代理）
public class DynamicProxyDemo {
    public static void main(String[] args) {
        // 动态代理在运行时生成$Proxy类
        Subject proxy = (Subject) Proxy.newProxyInstance(
            Subject.class.getClassLoader(),
            new Class[]{Subject.class},
            new InvocationHandler() {
                @Override
                public Object invoke(Object proxy, Method method, Object[] args) {
                    return null;
                }
            }
        );
    }
}

// 4. 从数据库读取
// 5. 从加密文件读取（防止反编译）
```

#### 2. 验证（Verification）

确保Class文件的字节流符合JVM规范，不会危害虚拟机安全。

```
验证阶段
├── 文件格式验证
│   ├── 魔数（0xCAFEBABE）
│   ├── 版本号
│   ├── 常量池
│   └── ...
├── 元数据验证
│   ├── 是否有父类（除Object外）
│   ├── 继承了final类？
│   ├── 非抽象类实现了所有抽象方法？
│   └── ...
├── 字节码验证（最复杂）
│   ├── 数据流分析
│   ├── 控制流分析
│   ├── 类型检查
│   └── ...
└── 符号引用验证
    ├── 符号引用能否找到对应的类
    ├── 访问性检查（private、protected、public）
    └── ...
```

```bash
# 关闭验证（提升启动速度，生产环境慎用）
-Xverify:none
```

#### 3. 准备（Preparation）

为**类变量**（static变量）分配内存并设置初始值。

```java
public class PreparationDemo {
    // 准备阶段：value = 0（int的零值）
    // 初始化阶段：value = 123
    public static int value = 123;
    
    // 准备阶段：如果是final常量，直接赋值为123
    public static final int CONSTANT = 123;
    
    // 实例变量不在准备阶段分配内存
    private int instanceVar = 456;
}
```

**各类型的零值**：
| 数据类型 | 零值 |
|----------|------|
| int | 0 |
| long | 0L |
| short | (short)0 |
| char | '\u0000' |
| byte | (byte)0 |
| boolean | false |
| float | 0.0f |
| double | 0.0d |
| reference | null |

#### 4. 解析（Resolution）

将常量池内的**符号引用**替换为**直接引用**。

```java
public class ResolutionDemo {
    public void method() {
        // 编译时：符号引用（字符串描述）
        // "com/example/User"
        User user = new User();
        
        // 解析后：直接引用（内存地址指针）
        // → 0x00007f8a1c000000
    }
}
```

**符号引用 vs 直接引用**：
- **符号引用**：用字符串表示引用关系，与内存布局无关
- **直接引用**：直接指向目标的指针、偏移量或句柄

#### 5. 初始化（Initialization）

执行类构造器`<clinit>()`方法。

```java
public class InitializationDemo {
    // ① 静态变量赋值
    private static int a = 10;
    
    // ② 静态代码块
    static {
        a = 20;
        b = 20;  // 可以赋值
        // System.out.println(b);  // 编译错误：非法前向引用
    }
    
    // ③ 静态变量赋值
    private static int b = 10;
    
    // <clinit>()执行顺序：
    // 1. a = 10
    // 2. a = 20, b = 20
    // 3. b = 10
    // 最终：a = 20, b = 10
    
    public static void main(String[] args) {
        System.out.println("a=" + a + ", b=" + b);  // a=20, b=10
    }
}
```

#### 类初始化时机

**主动引用**（必然触发初始化）：
```java
public class ActiveReferenceDemo {
    public static void main(String[] args) {
        // 1. 遇到new、getstatic、putstatic、invokestatic指令
        MyClass obj = new MyClass();
        int value = MyClass.staticVar;
        MyClass.staticMethod();
        
        // 2. 反射调用
        Class.forName("com.example.MyClass");
        
        // 3. 初始化子类时，父类未初始化
        SubClass sub = new SubClass();
        
        // 4. 虚拟机启动时，主类（main方法所在类）
        // 当前类就是主类
        
        // 5. JDK 7+动态语言支持
        // MethodHandle解析结果为某些句柄
    }
}
```

**被动引用**（不会触发初始化）：
```java
public class PassiveReferenceDemo {
    public static void main(String[] args) {
        // 1. 通过子类引用父类静态字段
        System.out.println(SubClass.parentStaticVar);
        // 只有Parent初始化，SubClass不初始化
        
        // 2. 通过数组定义引用类
        MyClass[] array = new MyClass[10];
        // MyClass不初始化
        
        // 3. 引用常量
        System.out.println(MyClass.CONSTANT);
        // 常量在编译期放入常量池，不初始化MyClass
    }
}
```



### 3.3 类加载器（ClassLoader）

#### 类加载器层次结构

```
┌─────────────────────────────────────┐
│   Bootstrap ClassLoader             │
│   (启动类加载器)                     │
│   加载：<JAVA_HOME>/lib              │
│   - rt.jar                          │
│   - java.lang.*                     │
│   - java.util.*                     │
│   C++实现，null表示                  │
└──────────────┬──────────────────────┘
               │ 父加载器
┌──────────────▼──────────────────────┐
│   Extension ClassLoader             │
│   (扩展类加载器)                     │
│   加载：<JAVA_HOME>/lib/ext          │
│   - javax.*                         │
│   Java实现                           │
└──────────────┬──────────────────────┘
               │ 父加载器
┌──────────────▼──────────────────────┐
│   Application ClassLoader           │
│   (应用程序类加载器/系统类加载器)     │
│   加载：classpath                    │
│   - 用户类                           │
│   Java实现                           │
└──────────────┬──────────────────────┘
               │ 父加载器
┌──────────────▼──────────────────────┐
│   Custom ClassLoader                │
│   (自定义类加载器)                   │
│   - Web容器加载器                    │
│   - 插件加载器                       │
│   - 热部署加载器                     │
└─────────────────────────────────────┘
```

#### 查看类加载器

```java
public class ClassLoaderDemo {
    public static void main(String[] args) {
        // 1. 查看当前类的类加载器
        ClassLoader classLoader = ClassLoaderDemo.class.getClassLoader();
        System.out.println("当前类加载器：" + classLoader);
        // 输出：sun.misc.Launcher$AppClassLoader@xxxxx
        
        // 2. 查看父加载器
        ClassLoader parent = classLoader.getParent();
        System.out.println("父加载器：" + parent);
        // 输出：sun.misc.Launcher$ExtClassLoader@xxxxx
        
        // 3. 查看父加载器的父加载器（Bootstrap）
        ClassLoader grandParent = parent.getParent();
        System.out.println("父父加载器：" + grandParent);
        // 输出：null（Bootstrap ClassLoader用C++实现）
        
        // 4. 核心类的类加载器
        ClassLoader stringLoader = String.class.getClassLoader();
        System.out.println("String类加载器：" + stringLoader);
        // 输出：null（由Bootstrap ClassLoader加载）
    }
}
```

### 3.4 双亲委派模型（Parent Delegation Model）

#### 双亲委派机制

**工作流程**：
1. 收到类加载请求，不直接加载
2. 委派给父加载器加载
3. 父加载器无法加载时，子加载器才尝试加载

```
                 收到加载请求
                      ↓
┌─────────────────────────────────────┐
│  检查是否已加载？                    │
│  (findLoadedClass)                  │
└──┬──────────────────────────┬───────┘
   │ 否                        │ 是
   ▼                          ▼
┌─────────────────────┐   直接返回
│ 有父加载器？         │
└──┬──────────┬───────┘
   │ 是        │ 否
   ▼          ▼
委派父加载器   尝试自己加载
   │          (findClass)
   ▼
父加载器递归
   │
   ▼
Bootstrap加载器
   │
   ▼
加载成功？
   │
   ├─是→ 返回Class
   │
   └─否→ 回到子加载器
         尝试自己加载
```

#### 双亲委派源码分析

```java
// java.lang.ClassLoader
protected Class<?> loadClass(String name, boolean resolve)
    throws ClassNotFoundException {
    synchronized (getClassLoadingLock(name)) {
        // 1. 检查类是否已经加载
        Class<?> c = findLoadedClass(name);
        
        if (c == null) {
            long t0 = System.nanoTime();
            try {
                // 2. 有父加载器，委派给父加载器
                if (parent != null) {
                    c = parent.loadClass(name, false);
                } else {
                    // 3. 没有父加载器，委派给Bootstrap加载器
                    c = findBootstrapClassOrNull(name);
                }
            } catch (ClassNotFoundException e) {
                // 父加载器无法加载
            }
            
            if (c == null) {
                // 4. 父加载器无法加载，调用自己的findClass
                long t1 = System.nanoTime();
                c = findClass(name);
                
                // 统计
                sun.misc.PerfCounter.getParentDelegationTime()
                    .addTime(t1 - t0);
                sun.misc.PerfCounter.getFindClassTime()
                    .addElapsedTimeFrom(t1);
                sun.misc.PerfCounter.getFindClasses().increment();
            }
        }
        
        // 5. 链接（可选）
        if (resolve) {
            resolveClass(c);
        }
        return c;
    }
}
```

#### 双亲委派的优势

```java
// 假设没有双亲委派，可能出现的问题：
package java.lang;

public class String {
    // 自定义的String类
    static {
        System.out.println("我是假的String类！");
    }
}

// 有了双亲委派：
// 1. 请求加载java.lang.String
// 2. 委派给父加载器
// 3. 最终由Bootstrap加载器加载JDK的String
// 4. 自定义的String永远不会被加载
// 
// 优势：
// ✅ 防止核心类被篡改（安全性）
// ✅ 避免类的重复加载（唯一性）
// ✅ 保证Java核心API不被破坏
```

### 3.5 打破双亲委派模型

#### 场景1：自定义类加载器

```java
public class CustomClassLoader extends ClassLoader {
    private String classPath;
    
    public CustomClassLoader(String classPath) {
        this.classPath = classPath;
    }
    
    @Override
    protected Class<?> findClass(String name) throws ClassNotFoundException {
        try {
            // 1. 读取class文件的字节数组
            byte[] classData = loadClassData(name);
            
            if (classData == null) {
                throw new ClassNotFoundException();
            }
            
            // 2. 将字节数组转换为Class对象
            return defineClass(name, classData, 0, classData.length);
            
        } catch (Exception e) {
            throw new ClassNotFoundException();
        }
    }
    
    private byte[] loadClassData(String className) throws Exception {
        String fileName = classPath + "/" 
            + className.replace('.', '/') + ".class";
        
        FileInputStream fis = new FileInputStream(fileName);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        
        int len;
        byte[] buffer = new byte[1024];
        while ((len = fis.read(buffer)) != -1) {
            baos.write(buffer, 0, len);
        }
        
        fis.close();
        return baos.toByteArray();
    }
    
    public static void main(String[] args) throws Exception {
        CustomClassLoader loader = new CustomClassLoader("/tmp/classes");
        
        // 加载自定义路径的类
        Class<?> clazz = loader.loadClass("com.example.MyClass");
        Object obj = clazz.newInstance();
        
        System.out.println("类加载器：" + obj.getClass().getClassLoader());
    }
}
```

#### 场景2：线程上下文类加载器（TCCL）

**问题**：SPI（Service Provider Interface）机制中，Bootstrap加载器需要加载用户代码。

```java
// JDBC示例
public class JDBCDemo {
    public static void main(String[] args) throws Exception {
        // DriverManager由Bootstrap加载器加载（rt.jar）
        // 但MySQL驱动在classpath，由AppClassLoader加载
        
        // 解决方案：线程上下文类加载器
        // DriverManager使用Thread.currentThread().getContextClassLoader()
        // 获取AppClassLoader来加载驱动
        
        Class.forName("com.mysql.jdbc.Driver");
        Connection conn = DriverManager.getConnection(
            "jdbc:mysql://localhost:3306/test", "root", "password");
    }
}

// DriverManager源码片段
public class DriverManager {
    static {
        loadInitialDrivers();
    }
    
    private static void loadInitialDrivers() {
        // 使用线程上下文类加载器
        ServiceLoader<Driver> loadedDrivers = ServiceLoader.load(
            Driver.class,
            Thread.currentThread().getContextClassLoader()  // ← 关键
        );
        
        Iterator<Driver> driversIterator = loadedDrivers.iterator();
        while(driversIterator.hasNext()) {
            driversIterator.next();
        }
    }
}
```

#### 场景3：热部署与热替换

```java
public class HotSwapClassLoader extends ClassLoader {
    public HotSwapClassLoader() {
        super(HotSwapClassLoader.class.getClassLoader());
    }
    
    @Override
    public Class<?> loadClass(String name) throws ClassNotFoundException {
        // 打破双亲委派：不委派给父加载器
        return findClass(name);
    }
    
    public Class<?> loadByte(byte[] classByte) {
        return defineClass(null, classByte, 0, classByte.length);
    }
}

// 热部署示例
public class HotDeployDemo {
    public static void main(String[] args) throws Exception {
        // 版本1
        HotSwapClassLoader loader1 = new HotSwapClassLoader();
        byte[] v1Bytes = loadClassBytes("MyClass_v1.class");
        Class<?> clazz1 = loader1.loadByte(v1Bytes);
        Object obj1 = clazz1.newInstance();
        
        // 版本2（重新加载）
        HotSwapClassLoader loader2 = new HotSwapClassLoader();
        byte[] v2Bytes = loadClassBytes("MyClass_v2.class");
        Class<?> clazz2 = loader2.loadByte(v2Bytes);
        Object obj2 = clazz2.newInstance();
        
        // 两个不同的Class对象
        System.out.println(clazz1 == clazz2);  // false
    }
}
```

#### 场景4：OSGi模块化

```
OSGi的类加载架构（网状结构）
         
    Bundle A ←→ Bundle B
       ↕           ↕
    Bundle C ←→ Bundle D
    
每个Bundle有自己的类加载器
可以相互委托，不遵循双亲委派
```

### 3.6 类的唯一性与类加载器

```java
public class ClassIdentityDemo {
    public static void main(String[] args) throws Exception {
        // 1. 同一个类加载器
        ClassLoader loader1 = new CustomClassLoader("/path1");
        Class<?> class1 = loader1.loadClass("com.example.User");
        Class<?> class2 = loader1.loadClass("com.example.User");
        System.out.println(class1 == class2);  // true
        
        // 2. 不同的类加载器
        ClassLoader loader2 = new CustomClassLoader("/path2");
        Class<?> class3 = loader2.loadClass("com.example.User");
        System.out.println(class1 == class3);  // false
        
        // 类的唯一性 = 类本身 + 类加载器
        // 即使class文件相同，类加载器不同，也是不同的类
    }
}
```

---

## 4. 字节码执行引擎

### 4.1 字节码概述

#### 什么是字节码

```java
// Java源代码
public class BytecodeDemo {
    public int add(int a, int b) {
        return a + b;
    }
}
```

编译后的字节码：
```
public int add(int, int);
  descriptor: (II)I
  flags: ACC_PUBLIC
  Code:
    stack=2, locals=3, args_size=3
       0: iload_1        // 加载参数a
       1: iload_2        // 加载参数b
       2: iadd           // 执行加法
       3: ireturn        // 返回结果
```

#### 查看字节码

```bash
# 编译
javac BytecodeDemo.java

# 查看字节码
javap -c BytecodeDemo.class

# 详细信息
javap -v BytecodeDemo.class
```

### 4.2 JVM指令集

#### 指令分类

```
JVM指令集（约200条指令）
│
├── 加载和存储指令
│   ├── load系列：从局部变量表加载到操作数栈
│   │   iload, lload, fload, dload, aload
│   └── store系列：从操作数栈存储到局部变量表
│       istore, lstore, fstore, dstore, astore
│
├── 运算指令
│   ├── 算术运算：add, sub, mul, div, rem, neg
│   ├── 位运算：shl, shr, ushr, and, or, xor
│   └── 比较运算：cmp, cmpl, cmpg
│
├── 类型转换指令
│   └── i2l, i2f, l2i, f2d, d2i, i2b, i2c, i2s
│
├── 对象创建与访问指令
│   ├── new, newarray, anewarray, multianewarray
│   ├── getfield, putfield, getstatic, putstatic
│   └── baload, caload, saload, iaload, laload...
│
├── 操作数栈管理指令
│   └── pop, pop2, dup, dup2, swap
│
├── 控制转移指令
│   ├── 条件跳转：ifeq, ifne, iflt, ifle, ifgt, ifge
│   ├── 复合条件跳转：if_icmpeq, if_icmpne, if_acmpeq...
│   ├── 多条件跳转：tableswitch, lookupswitch
│   └── 无条件跳转：goto, jsr, ret
│
├── 方法调用和返回指令
│   ├── invokevirtual：实例方法调用（虚方法）
│   ├── invokespecial：实例初始化、私有方法、父类方法
│   ├── invokestatic：静态方法调用
│   ├── invokeinterface：接口方法调用
│   ├── invokedynamic：动态方法调用（JDK 7+）
│   └── return系列：ireturn, lreturn, freturn, dreturn, areturn, return
│
├── 异常处理指令
│   └── athrow
│
└── 同步指令
    └── monitorenter, monitorexit
```

### 4.3 方法调用详解

#### 方法调用指令对比

```java
public class MethodInvocationDemo {
    // 静态方法
    public static void staticMethod() {
        // invokestatic
    }
    
    // 私有方法
    private void privateMethod() {
        // invokespecial
    }
    
    // 构造方法
    public MethodInvocationDemo() {
        // invokespecial
    }
    
    // 实例方法
    public void instanceMethod() {
        // invokevirtual（虚方法调用）
    }
    
    // final方法
    public final void finalMethod() {
        // invokevirtual（但不需要动态分派）
    }
    
    public void test() {
        staticMethod();         // invokestatic
        privateMethod();        // invokespecial
        this.instanceMethod();  // invokevirtual
        
        Runnable r = () -> {}; // invokedynamic（Lambda）
    }
}

interface MyInterface {
    void interfaceMethod();
}

class Impl implements MyInterface {
    @Override
    public void interfaceMethod() {
        // 调用：invokeinterface
    }
}
```

#### 方法调用的分派

**静态分派（Static Dispatch）**：
```java
public class StaticDispatchDemo {
    static class Human {}
    static class Man extends Human {}
    static class Woman extends Human {}
    
    public void sayHello(Human guy) {
        System.out.println("Hello, Human!");
    }
    
    public void sayHello(Man guy) {
        System.out.println("Hello, Man!");
    }
    
    public void sayHello(Woman guy) {
        System.out.println("Hello, Woman!");
    }
    
    public static void main(String[] args) {
        Human man = new Man();
        Human woman = new Woman();
        
        StaticDispatchDemo demo = new StaticDispatchDemo();
        demo.sayHello(man);    // Hello, Human!
        demo.sayHello(woman);  // Hello, Human!
        
        // 重载解析在编译期完成（静态分派）
        // 依据：变量的静态类型（Human）
    }
}
```

**动态分派（Dynamic Dispatch）**：
```java
public class DynamicDispatchDemo {
    static class Human {
        public void sayHello() {
            System.out.println("Hello, Human!");
        }
    }
    
    static class Man extends Human {
        @Override
        public void sayHello() {
            System.out.println("Hello, Man!");
        }
    }
    
    static class Woman extends Human {
        @Override
        public void sayHello() {
            System.out.println("Hello, Woman!");
        }
    }
    
    public static void main(String[] args) {
        Human man = new Man();
        Human woman = new Woman();
        
        man.sayHello();    // Hello, Man!
        woman.sayHello();  // Hello, Woman!
        
        // 方法重写解析在运行时完成（动态分派）
        // 依据：对象的实际类型（Man/Woman）
        // 通过invokevirtual指令实现
    }
}
```



#### 虚方法表（Virtual Method Table）

```
虚方法表优化动态分派

类加载时，在方法区建立虚方法表
┌─────────────────────────────┐
│       Human虚方法表          │
├─────────────────────────────┤
│ 0: toString() → Object      │
│ 1: equals() → Object        │
│ 2: hashCode() → Object      │
│ 3: sayHello() → Human       │
└─────────────────────────────┘

┌─────────────────────────────┐
│        Man虚方法表           │
├─────────────────────────────┤
│ 0: toString() → Object      │
│ 1: equals() → Object        │
│ 2: hashCode() → Object      │
│ 3: sayHello() → Man  ← 重写  │
│ 4: fight() → Man     ← 新增  │
└─────────────────────────────┘

方法调用：
1. 找到对象实际类型的虚方法表
2. 查找方法索引
3. 调用方法

优势：避免每次都搜索继承链
```

### 4.4 invokedynamic与Lambda

#### invokedynamic指令（JDK 7引入）

```java
public class InvokeDynamicDemo {
    public static void main(String[] args) {
        // Lambda表达式
        Runnable r = () -> System.out.println("Hello");
        
        // 字节码：
        // invokedynamic #2,  0
        //   BootstrapMethods:
        //     0: LambdaMetafactory.metafactory
        
        r.run();
    }
}
```

#### Lambda原理

```java
// 源代码
List<String> list = Arrays.asList("a", "b", "c");
list.forEach(s -> System.out.println(s));

// 等价于（编译器生成）
list.forEach(new Consumer<String>() {
    @Override
    public void accept(String s) {
        System.out.println(s);
    }
});

// 实际实现（invokedynamic）
// 不是创建匿名内部类，而是动态生成一个类
// 性能更好，生成的class文件更小
```

### 4.5 执行引擎

#### 解释器与编译器

```
┌──────────────────────────────────────┐
│          字节码                       │
└──────────────┬───────────────────────┘
               │
       ┌───────┴────────┐
       │                │
       ▼                ▼
┌─────────────┐  ┌──────────────────┐
│   解释器     │  │  JIT编译器        │
│ (Interpreter)│  │ (Just-In-Time)   │
│              │  │                  │
│ - 逐行执行   │  │ - 编译为机器码    │
│ - 启动快     │  │ - 执行快         │
│ - 执行慢     │  │ - 编译耗时       │
└─────────────┘  └──────────────────┘
       │                │
       └───────┬────────┘
               ▼
       ┌──────────────┐
       │   机器码执行  │
       └──────────────┘
```

#### HotSpot的执行模式

```bash
# 纯解释模式（已弃用）
-Xint

# 纯编译模式（已弃用）
-Xcomp

# 混合模式（默认）
-Xmixed

# 查看当前模式
java -version
# 输出：mixed mode
```

#### JIT编译器类型

**HotSpot的两个JIT编译器**：

| 编译器 | 别名 | 特点 | 适用场景 |
|--------|------|------|----------|
| **C1** | Client Compiler | 编译速度快，优化少 | 客户端程序、短生命周期 |
| **C2** | Server Compiler | 编译慢，优化激进 | 服务器程序、长时间运行 |

```bash
# 仅使用C1编译器
java -client MyApp

# 仅使用C2编译器
java -server MyApp

# 分层编译（Tiered Compilation，JDK 8默认）
java -XX:+TieredCompilation MyApp
```

#### 分层编译（Tiered Compilation）

```
编译层级：
┌────────────────────────────────────┐
│ Level 0: 解释执行                   │
│ - 收集性能监控信息                  │
└────────────┬───────────────────────┘
             │ 热点代码
             ▼
┌────────────────────────────────────┐
│ Level 1: C1编译（无性能监控）        │
│ - 简单优化                          │
└────────────┬───────────────────────┘
             │
             ▼
┌────────────────────────────────────┐
│ Level 2: C1编译（有限性能监控）      │
│ - 收集简单的性能信息                │
└────────────┬───────────────────────┘
             │
             ▼
┌────────────────────────────────────┐
│ Level 3: C1编译（完整性能监控）      │
│ - 收集详细的性能信息                │
└────────────┬───────────────────────┘
             │ 非常热的代码
             ▼
┌────────────────────────────────────┐
│ Level 4: C2编译                     │
│ - 激进优化                          │
│ - 逃逸分析                          │
│ - 标量替换                          │
│ - 栈上分配                          │
│ - 锁消除                            │
└────────────────────────────────────┘
```

### 4.6 热点代码探测（Hot Spot Detection）

#### 热点探测方法

**1. 基于采样（Sampling）**：
- 周期性检查线程栈顶
- 统计方法出现的频率
- 优点：简单，开销小
- 缺点：不精确

**2. 基于计数器（Counter-Based）**：
- HotSpot使用的方法
- 为每个方法维护计数器
- 优点：精确
- 缺点：有一定开销

#### 计数器类型

```java
public class HotSpotCounterDemo {
    /**
     * 两个计数器：
     * 
     * 1. 方法调用计数器（Invocation Counter）
     *    - 统计方法调用次数
     *    - 默认阈值：C1=1500, C2=10000
     *    - 热点方法触发编译
     * 
     * 2. 回边计数器（Back Edge Counter）
     *    - 统计循环回跳次数
     *    - 热点循环触发OSR编译
     *    - OSR：On-Stack Replacement（栈上替换）
     */
    
    // 热点方法示例
    public int hotMethod(int n) {
        int sum = 0;
        for (int i = 0; i < n; i++) {
            sum += i;
        }
        return sum;
    }
    
    public static void main(String[] args) {
        HotSpotCounterDemo demo = new HotSpotCounterDemo();
        
        // 调用足够多次，触发JIT编译
        for (int i = 0; i < 20000; i++) {
            demo.hotMethod(100);
        }
        
        // 此时hotMethod已被编译为本地代码
    }
}
```

#### 热点代码编译配置

```bash
# 方法调用计数器阈值
-XX:CompileThreshold=10000

# 回边计数器阈值
-XX:OnStackReplacePercentage=140

# 打印编译信息
-XX:+PrintCompilation

# 打印内联信息
-XX:+UnlockDiagnosticVMOptions -XX:+PrintInlining

# 禁用JIT编译（调试用）
-XX:-UseJITCompilation
```

#### 查看编译日志

```bash
java -XX:+PrintCompilation HotSpotDemo

# 输出示例：
#     24    1       3       java.lang.String::charAt (25 bytes)
#     25    2       3       java.lang.String::length (6 bytes)
#     ...
#   1234   10      4       HotSpotDemo::hotMethod (50 bytes)
#                  ↑
#            编译层级（Level 4 = C2编译）
```

### 4.7 JIT优化技术

#### 1. 方法内联（Inlining）

```java
public class InliningDemo {
    private int add(int a, int b) {
        return a + b;
    }
    
    public int calculate(int x) {
        return add(x, 10);  // 方法调用
    }
    
    // JIT编译后内联优化：
    public int calculate_optimized(int x) {
        return x + 10;  // 直接计算，无方法调用开销
    }
}

// 内联条件：
// - 方法字节码 < 35字节（-XX:MaxInlineSize）
// - 热点方法
// - 非虚方法或单一实现的虚方法
```

#### 2. 逃逸分析（Escape Analysis）

```java
public class EscapeAnalysisDemo {
    // 对象不逃逸（栈上分配）
    public void noEscape() {
        Point point = new Point(1, 2);
        int x = point.x + point.y;
        // point对象未逃逸出方法
        // JIT可以将其分配在栈上，方法结束自动回收
    }
    
    // 对象逃逸（堆上分配）
    private Point point;
    public void escape() {
        this.point = new Point(1, 2);
        // point赋值给实例变量，逃逸了
    }
    
    // 对象逃逸（堆上分配）
    public Point escape2() {
        return new Point(1, 2);
        // point作为返回值，逃逸了
    }
    
    static class Point {
        int x, y;
        Point(int x, int y) {
            this.x = x;
            this.y = y;
        }
    }
}

// 逃逸分析配置
// -XX:+DoEscapeAnalysis（JDK 8默认开启）
// -XX:-DoEscapeAnalysis（关闭）
```

#### 3. 标量替换（Scalar Replacement）

```java
public class ScalarReplacementDemo {
    static class Point {
        int x;
        int y;
    }
    
    public void test() {
        Point point = new Point();
        point.x = 1;
        point.y = 2;
        int sum = point.x + point.y;
    }
    
    // 标量替换后：
    public void test_optimized() {
        int x = 1;
        int y = 2;
        int sum = x + y;
        // Point对象被拆解为标量，不需要分配对象
    }
}

// 标量替换配置
// -XX:+EliminateAllocations（默认开启）
```

#### 4. 锁消除（Lock Elimination）

```java
public class LockEliminationDemo {
    public String concat(String s1, String s2) {
        StringBuffer sb = new StringBuffer();
        sb.append(s1);  // 内部有synchronized
        sb.append(s2);  // 内部有synchronized
        return sb.toString();
        
        // JIT检测到sb未逃逸，消除锁
        // 等价于：
        // StringBuilder sb = new StringBuilder();
    }
}

// 锁消除配置
// -XX:+EliminateLocks（默认开启）
// -XX:+DoEscapeAnalysis（需要逃逸分析支持）
```

#### 5. 锁膨胀（Lock Coarsening）

```java
public class LockCoarseningDemo {
    private Object lock = new Object();
    
    // 优化前
    public void before() {
        synchronized (lock) {
            // 操作1
        }
        synchronized (lock) {
            // 操作2
        }
        synchronized (lock) {
            // 操作3
        }
    }
    
    // 锁膨胀优化后
    public void after() {
        synchronized (lock) {
            // 操作1
            // 操作2
            // 操作3
        }
    }
}
```

#### 6. 公共子表达式消除

```java
public class CommonSubexpressionDemo {
    public int calculate(int a, int b, int c) {
        int x = a * b + c;
        int y = a * b - c;
        return x + y;
    }
    
    // 优化后（a * b只计算一次）
    public int calculate_optimized(int a, int b, int c) {
        int temp = a * b;
        int x = temp + c;
        int y = temp - c;
        return x + y;
    }
}
```

#### 7. 循环展开（Loop Unrolling）

```java
public class LoopUnrollingDemo {
    // 优化前
    public int sum(int[] arr) {
        int sum = 0;
        for (int i = 0; i < arr.length; i++) {
            sum += arr[i];
        }
        return sum;
    }
    
    // 循环展开优化后
    public int sum_optimized(int[] arr) {
        int sum = 0;
        int i = 0;
        // 一次循环处理4个元素
        for (; i < arr.length - 3; i += 4) {
            sum += arr[i];
            sum += arr[i+1];
            sum += arr[i+2];
            sum += arr[i+3];
        }
        // 处理剩余元素
        for (; i < arr.length; i++) {
            sum += arr[i];
        }
        return sum;
    }
}
```

---

## 5. 垃圾回收机制

### 5.1 什么是垃圾回收

#### 自动内存管理

```
手动内存管理（C/C++）      自动内存管理（Java）
┌────────────────┐          ┌────────────────┐
│  程序员负责：   │          │   JVM负责：     │
│  - malloc()    │          │  - new对象      │
│  - free()      │          │  - GC自动回收   │
│                │          │                │
│  问题：         │          │  优势：         │
│  - 内存泄漏    │          │  - 无内存泄漏   │
│  - 野指针      │          │  - 无野指针     │
│  - 重复释放    │          │  - 专注业务逻辑 │
└────────────────┘          └────────────────┘
```

#### GC需要解决的问题

1. **哪些内存需要回收？** → 垃圾识别算法
2. **什么时候回收？** → GC触发时机
3. **如何回收？** → 垃圾回收算法

### 5.2 如何判断对象已死

#### 方法1：引用计数法（Reference Counting）

```java
public class ReferenceCountingDemo {
    /**
     * 引用计数法：
     * - 每个对象维护一个引用计数器
     * - 有引用指向它时，计数+1
     * - 引用失效时，计数-1
     * - 计数为0时，对象可回收
     * 
     * 优点：实现简单，判定高效
     * 缺点：无法解决循环引用问题
     */
    
    static class Node {
        Node ref;
    }
    
    public static void main(String[] args) {
        Node a = new Node();  // a的计数=1
        Node b = new Node();  // b的计数=1
        
        a.ref = b;  // b的计数=2
        b.ref = a;  // a的计数=2
        
        a = null;   // a的计数=1（仍然>0）
        b = null;   // b的计数=1（仍然>0）
        
        // 循环引用！两个对象实际已不可达，但计数不为0
        // 引用计数法无法回收
    }
}
```

#### 方法2：可达性分析算法（Reachability Analysis）

**JVM采用的方法**：从GC Roots出发，向下搜索，能到达的对象是存活的。

```
GC Roots
    │
    ├─→ Object A ─→ Object C
    │      │
    │      └─→ Object D
    │
    └─→ Object B ─→ Object E
    
    Object F ←→ Object G（循环引用，但不可达）
    
可达：A, B, C, D, E（存活）
不可达：F, G（垃圾）
```

#### GC Roots包括哪些对象

```java
public class GCRootsDemo {
    /**
     * GC Roots对象：
     * 
     * 1. 虚拟机栈中引用的对象
     *    - 局部变量
     *    - 方法参数
     *    - 临时变量
     */
    public void method() {
        Object obj = new Object();  // obj是GC Root
    }
    
    /**
     * 2. 方法区中类静态属性引用的对象
     */
    private static Object staticObj = new Object();  // GC Root
    
    /**
     * 3. 方法区中常量引用的对象
     */
    private static final Object CONSTANT = new Object();  // GC Root
    
    /**
     * 4. 本地方法栈中JNI引用的对象
     */
    private native void nativeMethod();
    
    /**
     * 5. JVM内部引用
     *    - 基本类型的Class对象
     *    - 异常对象
     *    - 系统类加载器
     */
    
    /**
     * 6. 同步锁持有的对象
     */
    public void syncMethod() {
        synchronized (this) {  // this是GC Root
            // ...
        }
    }
    
    /**
     * 7. 反映JVM内部情况的JMXBean、JVMTI中注册的回调、本地代码缓存等
     */
}
```

### 5.3 Java引用类型

#### 四种引用类型

```java
import java.lang.ref.*;

public class ReferenceTypeDemo {
    /**
     * 1. 强引用（Strong Reference）
     *    - 最常见的引用
     *    - 只要强引用存在，GC永不回收
     */
    public void strongReference() {
        Object obj = new Object();  // 强引用
        // 只要obj可达，Object对象永不回收
    }
    
    /**
     * 2. 软引用（Soft Reference）
     *    - 内存充足时不回收
     *    - 内存不足时回收
     *    - 适用场景：内存敏感的缓存
     */
    public void softReference() {
        Object obj = new Object();
        SoftReference<Object> softRef = new SoftReference<>(obj);
        obj = null;
        
        // 内存充足：softRef.get() != null
        // 内存不足：GC后，softRef.get() == null
        
        Object retrieved = softRef.get();
        if (retrieved != null) {
            // 使用对象
        } else {
            // 对象已被回收，重新创建
        }
    }
    
    /**
     * 3. 弱引用（Weak Reference）
     *    - 下次GC时必定回收
     *    - 适用场景：WeakHashMap、ThreadLocal
     */
    public void weakReference() {
        Object obj = new Object();
        WeakReference<Object> weakRef = new WeakReference<>(obj);
        obj = null;
        
        System.out.println(weakRef.get());  // 可能是Object，也可能是null
        System.gc();
        System.out.println(weakRef.get());  // null（已被回收）
    }
    
    /**
     * 4. 虚引用（Phantom Reference）
     *    - 最弱的引用
     *    - get()永远返回null
     *    - 必须配合ReferenceQueue使用
     *    - 适用场景：跟踪对象被GC的状态，用于资源清理
     */
    public void phantomReference() {
        Object obj = new Object();
        ReferenceQueue<Object> queue = new ReferenceQueue<>();
        PhantomReference<Object> phantomRef = 
            new PhantomReference<>(obj, queue);
        
        System.out.println(phantomRef.get());  // null
        
        obj = null;
        System.gc();
        
        // 对象被GC后，phantomRef会进入queue
        Reference<? extends Object> ref = queue.poll();
        if (ref != null) {
            // 执行清理操作
        }
    }
}
```

#### 引用队列（Reference Queue）

```java
import java.lang.ref.*;

public class ReferenceQueueDemo {
    public static void main(String[] args) throws InterruptedException {
        ReferenceQueue<byte[]> queue = new ReferenceQueue<>();
        
        // 创建弱引用，关联队列
        WeakReference<byte[]> weakRef = new WeakReference<>(
            new byte[1024 * 1024],  // 1MB
            queue
        );
        
        System.out.println("GC前：" + weakRef.get());
        
        System.gc();
        Thread.sleep(100);
        
        System.out.println("GC后：" + weakRef.get());  // null
        
        // 从队列获取被回收的引用
        Reference<? extends byte[]> ref = queue.poll();
        System.out.println("队列中的引用：" + ref);
        System.out.println("引用相等：" + (ref == weakRef));  // true
    }
}
```

### 5.4 对象的生死

#### finalize()方法

```java
public class FinalizeDemo {
    private static FinalizeDemo instance = null;
    
    /**
     * finalize()方法：
     * - 对象被GC前最后的机会
     * - 只会被调用一次
     * - 不建议使用（已弃用）
     */
    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        System.out.println("finalize被调用");
        
        // 自我拯救：重新建立强引用
        instance = this;
    }
    
    public static void main(String[] args) throws InterruptedException {
        instance = new FinalizeDemo();
        
        // 第一次自救
        instance = null;
        System.gc();
        Thread.sleep(500);
        
        if (instance != null) {
            System.out.println("第一次自救成功");
        } else {
            System.out.println("第一次自救失败");
        }
        
        // 第二次尝试（失败，finalize只调用一次）
        instance = null;
        System.gc();
        Thread.sleep(500);
        
        if (instance != null) {
            System.out.println("第二次自救成功");
        } else {
            System.out.println("第二次自救失败");  // 输出这个
        }
    }
}
```

#### 对象死亡过程

```
对象不可达
    ↓
需要执行finalize()? ──否→ 直接回收
    ↓ 是
    │
放入F-Queue队列
    ↓
Finalizer线程执行finalize()
    ↓
再次可达性分析
    ↓
    ├─ 可达 → 拯救成功
    │
    └─ 不可达 → 回收
```



### 5.5 垃圾收集算法

#### 1. 标记-清除算法（Mark-Sweep）

```
标记阶段：
┌──┬──┬──┬──┬──┬──┬──┬──┐
│√ │  │√ │  │√ │√ │  │√ │
└──┴──┴──┴──┴──┴──┴──┴──┘
 存活 垃圾 存活 垃圾 存活 存活 垃圾 存活

清除阶段：
┌──┬──┬──┬──┬──┬──┬──┬──┐
│√ │  │√ │  │√ │√ │  │√ │
└──┴─────┴─────┴──┴─────┴──┘
   空闲   空闲      空闲

缺点：
1. 效率问题：标记和清除效率都不高
2. 空间问题：产生大量不连续的内存碎片
```

#### 2. 标记-复制算法（Mark-Copy）

```
From区（使用中）:
┌──┬──┬──┬──┬──┬──┬──┬──┐
│√ │  │√ │  │√ │√ │  │√ │
└──┴──┴──┴──┴──┴──┴──┴──┘
 存活 垃圾 存活 垃圾 存活 存活 垃圾 存活

To区（空闲）:
┌──────────────────────────┐
│                          │
└──────────────────────────┘

复制存活对象到To区：
To区:
┌──┬──┬──┬──┬──────────────┐
│√ │√ │√ │√ │              │
└──┴──┴──┴──┴──────────────┘
 连续存放      空闲空间

交换From和To，清空新的To区

优点：
1. 实现简单
2. 运行高效
3. 无内存碎片

缺点：
1. 浪费一半内存空间
2. 对象存活率高时，复制效率低
```

**改进：Appel式回收**
```
新生代布局：
┌────────┬──────┬──────┐
│  Eden  │  S0  │  S1  │
│  8/10  │ 1/10 │ 1/10 │
└────────┴──────┴──────┘

- Eden + 一个Survivor用于分配
- 一个Survivor用于复制
- 存活率低，浪费空间只有10%
```

#### 3. 标记-整理算法（Mark-Compact）

```
标记阶段：
┌──┬──┬──┬──┬──┬──┬──┬──┐
│√ │  │√ │  │√ │√ │  │√ │
└──┴──┴──┴──┴──┴──┴──┴──┘
 存活 垃圾 存活 垃圾 存活 存活 垃圾 存活

整理阶段（移动对象）：
┌──┬──┬──┬──┬──────────────┐
│√ │√ │√ │√ │              │
└──┴──┴──┴──┴──────────────┘
 存活对象     空闲空间

优点：
1. 无内存碎片
2. 无空间浪费

缺点：
1. 移动对象效率低
2. 需要更新所有引用（STW时间长）
```

#### 算法对比

| 算法 | 效率 | 内存利用率 | 内存碎片 | 适用场景 |
|------|------|-----------|---------|----------|
| **标记-清除** | 中 | 高 | 有 | 老年代（CMS） |
| **标记-复制** | 高 | 低（50%） | 无 | 新生代 |
| **标记-整理** | 低 | 高 | 无 | 老年代 |

### 5.6 分代收集理论

#### 分代假说

```
弱分代假说（Weak Generational Hypothesis）：
→ 绝大多数对象都是朝生夕死的

强分代假说（Strong Generational Hypothesis）：
→ 熬过越多次GC的对象越难消亡

跨代引用假说（Intergenerational Reference Hypothesis）：
→ 跨代引用相对于同代引用来说占极少数
```

#### 分代设计

```
┌─────────────────────────────────────────────┐
│              Java堆（Heap）                  │
├────────────────────────┬────────────────────┤
│      新生代 (Young)     │   老年代 (Old)      │
│  特点：对象朝生夕死     │  特点：对象长期存活  │
│  算法：标记-复制        │  算法：标记-整理/清除│
│  频率：高频GC           │  频率：低频GC        │
│  耗时：短               │  耗时：长            │
├────────────────────────┴────────────────────┤
│              元空间 (Metaspace，JDK 8+)      │
│              - 本地内存                      │
│              - 类元数据                      │
└─────────────────────────────────────────────┘
```

#### 记忆集与卡表（解决跨代引用）

```
老年代                   新生代
┌──────────┐            ┌──────────┐
│  Object A├───────────→│ Object B │
└──────────┘            └──────────┘
     ↓                        ↑
     └────────跨代引用─────────┘

问题：Minor GC时，需要扫描整个老年代？

解决方案：记忆集（Remembered Set）
┌──────────────────────────┐
│    老年代（卡表）          │
│  ┌──┬──┬──┬──┬──┬──┬──┐  │
│  │  │√ │  │  │√ │  │  │  │
│  └──┴──┴──┴──┴──┴──┴──┘  │
│     ↑           ↑         │
│  有跨代引用   有跨代引用   │
└──────────────────────────┘

Minor GC时，只需扫描标记的卡页
```

**卡表实现**：
```java
// 卡表：字节数组，每个元素对应一块内存区域（卡页）
byte[] cardTable = new byte[内存大小 / 512];  // 512字节为一卡页

// 当老年代对象引用新生代对象时
cardTable[老年代地址 >> 9] = 1;  // 标记脏卡

// Minor GC时，只扫描脏卡对应的内存区域
```

### 5.7 垃圾收集器

#### 垃圾收集器发展历程

```
时间线：
1999    2002    2004    2012    2017    2018
  │       │       │       │       │       │
  ↓       ↓       ↓       ↓       ↓       ↓
Serial  Parallel  CMS    G1      ZGC    Shenandoah
                                  │
                              低延迟时代
```

#### HotSpot垃圾收集器全景图

```
┌───────────── 新生代收集器 ─────────────┐
│                                        │
│   Serial      ParNew     Parallel     │
│    GC           GC       Scavenge     │
│                                        │
└───────────┬────────────────────────────┘
            │ 可搭配使用
┌───────────▼─── 老年代收集器 ────────────┐
│                                         │
│   Serial     CMS      Parallel Old     │
│    Old                                  │
│                                         │
└─────────────────────────────────────────┘

┌───────────── 整堆收集器 ────────────────┐
│                                         │
│        G1    ZGC    Shenandoah          │
│                                         │
└─────────────────────────────────────────┘
```

#### 收集器组合关系

```
推荐组合：
1. Serial + Serial Old（单线程，客户端）
2. ParNew + CMS（多线程，低延迟）
3. Parallel Scavenge + Parallel Old（多线程，高吞吐）
4. G1（JDK 9+默认，兼顾延迟和吞吐）
5. ZGC / Shenandoah（超低延迟）

已废弃组合：
- Serial + CMS
- ParNew + Serial Old
```

### 5.8 垃圾收集器详解

#### 1. Serial收集器

```
单线程收集器

GC线程      █████████████
用户线程    ──────┘          └────→
           暂停(STW)        继续运行

特点：
✓ 单线程
✓ 简单高效
✓ 适合单CPU环境
✓ Client模式默认新生代收集器

配置：
-XX:+UseSerialGC
```

```java
/**
 * Serial GC示例
 * VM参数：-XX:+UseSerialGC -Xms20m -Xmx20m -Xmn10m -XX:+PrintGCDetails
 */
public class SerialGCDemo {
    public static void main(String[] args) {
        byte[] allocation1 = new byte[2 * 1024 * 1024];  // 2MB
        byte[] allocation2 = new byte[2 * 1024 * 1024];
        byte[] allocation3 = new byte[2 * 1024 * 1024];
        byte[] allocation4 = new byte[4 * 1024 * 1024];  // 触发GC
    }
}
```

#### 2. ParNew收集器

```
多线程版本的Serial

GC线程      ████████████
            ████████████
            ████████████
用户线程    ──────┘         └────→
           暂停(STW)       继续运行

特点：
✓ Serial的多线程版本
✓ 可与CMS配合
✓ Server模式首选新生代收集器

配置：
-XX:+UseParNewGC
-XX:ParallelGCThreads=4  # GC线程数
```

#### 3. Parallel Scavenge收集器

```
吞吐量优先收集器

特点：
✓ 多线程
✓ 关注吞吐量
✓ 自适应调节策略（GC Ergonomics）

吞吐量 = 运行用户代码时间 / (运行用户代码时间 + GC时间)

配置：
-XX:+UseParallelGC                    # 使用Parallel Scavenge
-XX:MaxGCPauseMillis=200              # 最大GC停顿时间（毫秒）
-XX:GCTimeRatio=99                    # 吞吐量（1/(1+99)=1%时间用于GC）
-XX:+UseAdaptiveSizePolicy            # 自适应调节（默认开启）
```

```java
/**
 * Parallel GC示例
 * VM参数：-XX:+UseParallelGC -XX:MaxGCPauseMillis=100 -Xms20m -Xmx20m
 */
public class ParallelGCDemo {
    public static void main(String[] args) {
        List<byte[]> list = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
            list.add(new byte[1024 * 1024]);  // 1MB
        }
    }
}
```

#### 4. Serial Old收集器

```
Serial的老年代版本

特点：
✓ 单线程
✓ 标记-整理算法
✓ Client模式老年代收集器
✓ CMS的后备方案

配置：
-XX:+UseSerialOldGC（已废弃）
```

#### 5. Parallel Old收集器

```
Parallel Scavenge的老年代版本

特点：
✓ 多线程
✓ 标记-整理算法
✓ 吞吐量优先

配置：
-XX:+UseParallelOldGC
```

#### 6. CMS收集器（Concurrent Mark Sweep）

```
并发标记清除收集器

GC过程（4个阶段）：

1. 初始标记（STW）    █
   用户线程           ───┘  └───→

2. 并发标记          ████████████
   用户线程          ────────────→

3. 重新标记（STW）      ███
   用户线程           ────┘  └────→

4. 并发清除          ████████████
   用户线程          ────────────→

特点：
✓ 低延迟
✓ 并发收集
✓ 标记-清除算法
✓ 会产生内存碎片

配置：
-XX:+UseConcMarkSweepGC                    # 使用CMS
-XX:CMSInitiatingOccupancyFraction=70      # 老年代使用70%触发CMS
-XX:+UseCMSCompactAtFullCollection         # Full GC时整理内存
-XX:CMSFullGCsBeforeCompaction=0           # 多少次Full GC后整理
-XX:+CMSParallelRemarkEnabled              # 降低重新标记停顿
-XX:+CMSScavengeBeforeRemark               # 重新标记前Minor GC
```

**CMS详细过程**：
```java
/**
 * CMS GC示例
 * VM参数：
 * -XX:+UseConcMarkSweepGC
 * -XX:+PrintGCDetails
 * -XX:+PrintGCDateStamps
 * -Xms20m -Xmx20m -Xmn10m
 */
public class CMSGCDemo {
    public static void main(String[] args) {
        List<byte[]> list = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
            byte[] bytes = new byte[1024 * 1024];  // 1MB
            list.add(bytes);
            
            if (i % 10 == 0) {
                list.clear();  // 制造垃圾
            }
        }
    }
}
```

**CMS的问题**：
1. **CPU资源敏感**：并发阶段占用CPU
2. **浮动垃圾**：并发清除时产生的新垃圾无法回收
3. **内存碎片**：标记-清除算法产生碎片

**CMS失败处理**：
```
Concurrent Mode Failure：
- 原因：并发清除时，老年代空间不足
- 后果：启动Serial Old收集器，STW时间很长
- 解决：降低CMSInitiatingOccupancyFraction

Promotion Failed：
- 原因：新生代晋升时，老年代碎片化，无法容纳
- 后果：触发Full GC
- 解决：提前整理内存碎片
```

#### 7. G1收集器（Garbage First）

**G1的设计思想**：
```
传统分代：
┌────────────┬─────────────┐
│   新生代   │   老年代     │
└────────────┴─────────────┘

G1的Region：
┌──┬──┬──┬──┬──┬──┬──┬──┐
│E │E │S │O │O │H │E │O │
└──┴──┴──┴──┴──┴──┴──┴──┘
E=Eden, S=Survivor, O=Old, H=Humongous

- 将堆划分为多个大小相等的Region（1-32MB）
- 新生代、老年代是逻辑概念
- 可预测的停顿时间模型
```

**G1收集过程**：
```
1. 初始标记（STW）         █
   用户线程                ───┘  └───→

2. 并发标记               ████████████
   用户线程               ────────────→

3. 最终标记（STW）           ███
   用户线程                ────┘  └────→

4. 筛选回收（STW）         █████████
   用户线程                ────┘    └───→
```

**G1配置**：
```bash
# 启用G1（JDK 9+默认）
-XX:+UseG1GC

# 最大停顿时间目标（默认200ms）
-XX:MaxGCPauseMillis=200

# 堆大小
-Xms4g -Xmx4g

# Region大小（1-32MB，必须是2的幂）
-XX:G1HeapRegionSize=16m

# 新生代大小（G1会自动调整，不建议手动设置）
-XX:G1NewSizePercent=5      # 新生代最小占比
-XX:G1MaxNewSizePercent=60  # 新生代最大占比

# 并发标记线程数
-XX:ConcGCThreads=4

# 老年代Region占比达到多少触发Mixed GC
-XX:InitiatingHeapOccupancyPercent=45

# GC日志
-XX:+PrintGCDetails
-XX:+PrintGCDateStamps
-Xloggc:gc.log
```

**G1优势**：
1. 可预测的停顿时间
2. 无内存碎片（整理算法）
3. 空间整合（Region之间复制）
4. 兼顾吞吐量和延迟



```java
/**
 * G1 GC示例
 * VM参数：
 * -XX:+UseG1GC
 * -XX:MaxGCPauseMillis=200
 * -Xms2g -Xmx2g
 * -XX:+PrintGCDetails
 */
public class G1GCDemo {
    public static void main(String[] args) throws InterruptedException {
        List<byte[]> list = new ArrayList<>();
        
        while (true) {
            byte[] bytes = new byte[1024 * 1024];  // 1MB
            list.add(bytes);
            
            // 模拟对象回收
            if (list.size() > 500) {
                list.subList(0, 200).clear();
            }
            
            Thread.sleep(10);
        }
    }
}
```

**G1的Mixed GC**：
```
Mixed GC = Minor GC + 部分Old Region回收

回收集合（Collection Set）：
- 所有新生代Region
- 部分老年代Region（根据价值排序）
- 价值 = 回收收益 / 回收成本

目标：在停顿时间内回收价值最大的Region
```

#### 8. ZGC收集器（Z Garbage Collector）

**ZGC特性**：
- **超低延迟**：停顿时间不超过10ms
- **支持TB级堆**：最大支持16TB堆内存
- **并发回收**：几乎所有阶段都并发执行
- **Region动态大小**：小（2MB）、中（32MB）、大（N×2MB）

**ZGC核心技术**：
```
1. 着色指针（Colored Pointers）
   64位指针布局：
   ┌────────┬──┬──┬──┬──┬────────────────────────────┐
   │ 未使用 │M0│M1│R │F │      对象地址（42位）       │
   └────────┴──┴──┴──┴──┴────────────────────────────┘
   M0/M1: 标记位
   R: Remapped
   F: Finalizable

2. 读屏障（Load Barrier）
   每次对象引用时插入的代码片段
   用于实现并发标记和并发移动
```

**ZGC配置**：
```bash
# 启用ZGC（JDK 11+，JDK 15+生产就绪）
-XX:+UseZGC

# 堆大小
-Xms16g -Xmx16g

# 并发GC线程数
-XX:ConcGCThreads=4

# 分配速率限制（防止分配过快）
-XX:ZAllocationSpikeTolerance=2

# GC日志
-Xlog:gc:gc.log
```

```java
/**
 * ZGC示例
 * VM参数：
 * -XX:+UseZGC
 * -Xms4g -Xmx4g
 * -Xlog:gc*
 */
public class ZGCDemo {
    public static void main(String[] args) throws InterruptedException {
        System.out.println("ZGC Demo Start");
        
        List<byte[]> list = new ArrayList<>();
        
        while (true) {
            // 快速分配
            for (int i = 0; i < 100; i++) {
                list.add(new byte[1024 * 1024]);  // 1MB
            }
            
            // 模拟部分回收
            if (list.size() > 1000) {
                list.subList(0, 500).clear();
            }
            
            Thread.sleep(100);
        }
    }
}
```

#### 9. Shenandoah收集器

**Shenandoah特性**：
- **低延迟**：停顿时间稳定，与堆大小无关
- **并发整理**：支持并发整理算法
- **Brooks转发指针**：实现并发移动对象

**Shenandoah vs G1 vs ZGC**：
| 特性 | G1 | Shenandoah | ZGC |
|------|-----|-----------|-----|
| 目标停顿 | 可配置 | <10ms | <10ms |
| 最大堆 | ~32GB | TB级 | 16TB |
| 并发整理 | 否 | 是 | 是 |
| JDK版本 | JDK 7+ | JDK 12+ | JDK 11+ |
| 生产就绪 | JDK 9+ | JDK 15+ | JDK 15+ |

**Shenandoah配置**：
```bash
# 启用Shenandoah
-XX:+UseShenandoahGC

# 堆大小
-Xms8g -Xmx8g

# GC模式
-XX:ShenandoahGCMode=normal  # 默认模式

# GC日志
-Xlog:gc*:gc.log
```

### 5.9 GC日志分析

#### 开启GC日志

**JDK 8及之前**：
```bash
-XX:+PrintGCDetails
-XX:+PrintGCDateStamps
-XX:+PrintGCTimeStamps
-Xloggc:gc.log
-XX:+UseGCLogFileRotation
-XX:NumberOfGCLogFiles=10
-XX:GCLogFileSize=10M
```

**JDK 9+（统一日志）**：
```bash
-Xlog:gc*:file=gc.log:time,level,tags:filecount=10,filesize=10M
```

#### GC日志解读

**Minor GC示例**：
```
2024-01-15T10:30:45.123+0800: [GC (Allocation Failure)
    [PSYoungGen: 65536K->8192K(76288K)]
    131072K->73728K(251392K), 0.0123456 secs]
    [Times: user=0.04 sys=0.01, real=0.01 secs]

解读：
- GC (Allocation Failure)：因分配失败触发GC
- PSYoungGen：Parallel Scavenge新生代收集器
- 65536K->8192K(76288K)：新生代从65MB降到8MB，总容量76MB
- 131072K->73728K(251392K)：整个堆从128MB降到72MB，总容量245MB
- 0.0123456 secs：GC耗时
- user=0.04：用户态CPU时间
- sys=0.01：内核态CPU时间
- real=0.01：实际经过时间
```

**Full GC示例**：
```
2024-01-15T10:30:46.234+0800: [Full GC (Ergonomics)
    [PSYoungGen: 8192K->0K(76288K)]
    [ParOldGen: 65536K->73600K(175104K)]
    73728K->73600K(251392K),
    [Metaspace: 3456K->3456K(1056768K)], 0.0987654 secs]
    [Times: user=0.38 sys=0.02, real=0.10 secs]

解读：
- Full GC (Ergonomics)：自动触发的Full GC
- PSYoungGen：新生代清空
- ParOldGen：老年代回收
- Metaspace：元空间（无变化）
- Full GC耗时更长
```

**CMS GC示例**：
```
[GC (CMS Initial Mark) [1 CMS-initial-mark: 100000K(200000K)] 
    120000K(300000K), 0.0012345 secs]
[CMS-concurrent-mark-start]
[CMS-concurrent-mark: 0.123/0.123 secs]
[CMS-concurrent-preclean-start]
[CMS-concurrent-preclean: 0.012/0.012 secs]
[GC (CMS Final Remark) [YG occupancy: 20000K (100000K)]
    [Rescan (parallel), 0.0023456 secs]
    [weak refs processing, 0.0001234 secs]
    [class unloading, 0.0001234 secs]
    [scrub symbol table, 0.0001234 secs]
    [scrub string table, 0.0000123 secs]
    [1 CMS-remark: 100000K(200000K)] 120000K(300000K), 0.0030123 secs]
[CMS-concurrent-sweep-start]
[CMS-concurrent-sweep: 0.098/0.098 secs]
[CMS-concurrent-reset-start]
[CMS-concurrent-reset: 0.001/0.001 secs]
```

#### GC日志分析工具

1. **GCViewer**（开源）
   - 可视化GC日志
   - 分析吞吐量、延迟
   - 下载：https://github.com/chewiebug/GCViewer

2. **GCEasy**（在线）
   - 上传GC日志文件
   - 自动分析和建议
   - 网址：https://gceasy.io

3. **JDK自带工具**
   ```bash
   # jstat：查看GC统计信息
   jstat -gc <pid> 1000  # 每秒输出GC信息
   
   # jmap：查看堆信息
   jmap -heap <pid>
   ```

---

## 6. JVM性能监控与诊断

### 6.1 JDK命令行工具

#### 1. jps - 查看Java进程

```bash
# 查看所有Java进程
jps

# 输出完整类名
jps -l

# 输出JVM参数
jps -v

# 输出main方法参数
jps -m

# 示例输出
12345 org.example.MyApplication -Xms2g -Xmx4g
```

#### 2. jstat - 查看JVM统计信息

```bash
# 查看GC统计（每1000ms输出一次，共10次）
jstat -gc <pid> 1000 10

# 输出示例：
S0C    S1C    S0U    S1U      EC       EU        OC         OU       MC     MU    YGC   YGCT    FGC    FGCT     GCT
10240  10240   0     8192   102400   51200    204800    153600   20480  18432   45   0.456    3    0.234   0.690

# 列说明：
# S0C/S1C: Survivor0/1容量
# S0U/S1U: Survivor0/1使用量
# EC: Eden容量
# EU: Eden使用量
# OC: 老年代容量
# OU: 老年代使用量
# MC: 元空间容量
# MU: 元空间使用量
# YGC: Young GC次数
# YGCT: Young GC总耗时
# FGC: Full GC次数
# FGCT: Full GC总耗时
# GCT: GC总耗时

# 查看类加载统计
jstat -class <pid>

# 查看编译统计
jstat -compiler <pid>

# 查看垃圾回收统计
jstat -gcutil <pid> 1000 10
```

#### 3. jinfo - 查看/修改JVM配置

```bash
# 查看所有JVM参数
jinfo -flags <pid>

# 查看系统属性
jinfo -sysprops <pid>

# 查看特定参数
jinfo -flag UseG1GC <pid>

# 动态修改参数（仅支持manageable参数）
jinfo -flag +PrintGC <pid>
jinfo -flag -PrintGC <pid>

# 查看可动态修改的参数
java -XX:+PrintFlagsFinal -version | grep manageable
```

#### 4. jmap - 内存映射工具

```bash
# 查看堆信息
jmap -heap <pid>

# 输出示例：
Heap Configuration:
   MinHeapFreeRatio         = 40
   MaxHeapFreeRatio         = 70
   MaxHeapSize              = 4294967296 (4096.0MB)
   NewSize                  = 1431633920 (1365.3125MB)
   MaxNewSize               = 1431633920 (1365.3125MB)
   OldSize                  = 2863333376 (2730.6875MB)
   NewRatio                 = 2
   SurvivorRatio            = 8
   MetaspaceSize            = 21807104 (20.796875MB)
   CompressedClassSpaceSize = 1073741824 (1024.0MB)
   MaxMetaspaceSize         = 17592186044415 MB

# 生成堆转储文件（Heap Dump）
jmap -dump:format=b,file=heap.hprof <pid>

# 仅dump存活对象
jmap -dump:live,format=b,file=heap_live.hprof <pid>

# 查看类实例统计
jmap -histo <pid>

# 仅统计存活对象
jmap -histo:live <pid>

# 输出示例：
num     #instances         #bytes  class name
----------------------------------------------
   1:         50000       10000000  [B
   2:         30000        1920000  java.lang.String
   3:         10000        1600000  java.util.HashMap$Node
   ...
```

#### 5. jstack - 线程堆栈跟踪

```bash
# 打印线程堆栈
jstack <pid>

# 输出到文件
jstack <pid> > thread_dump.txt

# 检测死锁
jstack -l <pid>

# 输出示例：
"Thread-0" #12 prio=5 os_prio=31 tid=0x00007f8a1c800000 nid=0x5603 waiting on condition
   java.lang.Thread.State: WAITING (parking)
        at sun.misc.Unsafe.park(Native Method)
        at java.util.concurrent.locks.LockSupport.park(LockSupport.java:175)
        at java.util.concurrent.locks.AbstractQueuedSynchronizer$ConditionObject.await
        ...

# 线程状态说明：
# RUNNABLE: 运行中
# WAITING: 等待（park、wait等）
# TIMED_WAITING: 限时等待（sleep、wait(timeout)等）
# BLOCKED: 阻塞（等待锁）
# NEW: 新建
# TERMINATED: 终止
```

**死锁检测示例**：
```java
public class DeadlockDemo {
    private static Object lock1 = new Object();
    private static Object lock2 = new Object();
    
    public static void main(String[] args) {
        new Thread(() -> {
            synchronized (lock1) {
                System.out.println("Thread1 acquired lock1");
                try { Thread.sleep(100); } catch (InterruptedException e) {}
                synchronized (lock2) {
                    System.out.println("Thread1 acquired lock2");
                }
            }
        }).start();
        
        new Thread(() -> {
            synchronized (lock2) {
                System.out.println("Thread2 acquired lock2");
                try { Thread.sleep(100); } catch (InterruptedException e) {}
                synchronized (lock1) {
                    System.out.println("Thread2 acquired lock1");
                }
            }
        }).start();
    }
}

// jstack输出会显示：
// Found one Java-level deadlock:
// "Thread-1":
//   waiting to lock monitor 0x00007f8a1c001234 (object 0x00007f8a1d000000, a java.lang.Object),
//   which is held by "Thread-0"
// "Thread-0":
//   waiting to lock monitor 0x00007f8a1c001235 (object 0x00007f8a1d000008, a java.lang.Object),
//   which is held by "Thread-1"
```

#### 6. jcmd - 多功能诊断工具

```bash
# 列出可用命令
jcmd <pid> help

# 打印JVM版本
jcmd <pid> VM.version

# 打印JVM参数
jcmd <pid> VM.flags

# 打印系统属性
jcmd <pid> VM.system_properties

# 打印JVM性能计数器
jcmd <pid> PerfCounter.print

# 生成堆转储
jcmd <pid> GC.heap_dump /tmp/heap.hprof

# 执行GC
jcmd <pid> GC.run

# 打印类直方图
jcmd <pid> GC.class_histogram

# 打印线程堆栈
jcmd <pid> Thread.print

# Native Memory Tracking
jcmd <pid> VM.native_memory summary
```

#### 7. jhsdb - Java HotSpot Debugger

```bash
# 启动图形化调试器
jhsdb hsdb --pid <pid>

# 启动命令行调试器
jhsdb clhsdb --pid <pid>

# 分析core dump
jhsdb hsdb --core <core_file> --exe <java_path>
```

### 6.2 可视化监控工具

#### 1. JConsole

```bash
# 启动JConsole
jconsole

# 连接到指定进程
jconsole <pid>

# 连接到远程JVM
jconsole <host>:<port>

# 远程JVM需要配置：
java -Dcom.sun.management.jmxremote \
     -Dcom.sun.management.jmxremote.port=9999 \
     -Dcom.sun.management.jmxremote.authenticate=false \
     -Dcom.sun.management.jmxremote.ssl=false \
     -jar myapp.jar
```

**JConsole功能**：
- 概览：CPU、内存、线程、类
- 内存：堆、非堆使用情况
- 线程：线程状态、检测死锁
- 类：已加载类数量
- VM摘要：JVM信息
- MBean：管理和监控MBean

#### 2. VisualVM

```bash
# 启动VisualVM
jvisualvm

# 已集成到JDK中（JDK 8及之前）
# JDK 9+需单独下载：https://visualvm.github.io
```

**VisualVM功能**：
- 监视：CPU、内存、类、线程实时监控
- 线程：线程状态、时间线
- 抽样器：CPU和内存抽样分析
- Profiler：性能分析
- 堆转储分析：分析heap dump文件
- 线程转储分析：分析thread dump文件

**插件安装**：
```
工具 → 插件 → 可用插件
推荐插件：
- Visual GC
- BTrace Workbench
- Threads Inspector
```

#### 3. Java Mission Control (JMC)

```bash
# 启动JMC
jmc

# JFR记录（Java Flight Recorder）
java -XX:+UnlockCommercialFeatures \
     -XX:+FlightRecorder \
     -XX:FlightRecorderOptions=defaultrecording=true,dumponexit=true \
     -jar myapp.jar
```

**JMC功能**：
- JVM浏览器：连接本地/远程JVM
- Java Flight Recorder：低开销性能分析
- Mission Control：实时监控

#### 4. Arthas（阿里开源）

```bash
# 下载Arthas
curl -O https://arthas.aliyun.com/arthas-boot.jar

# 启动
java -jar arthas-boot.jar

# 选择要诊断的Java进程
```

**Arthas常用命令**：
```bash
# dashboard：实时数据面板
dashboard

# thread：查看线程
thread                    # 所有线程
thread -n 3               # CPU使用率前3的线程
thread <tid>              # 指定线程堆栈
thread -b                 # 检测死锁

# jad：反编译
jad com.example.MyClass

# watch：方法监控
watch com.example.MyClass method "{params,returnObj}" -x 2

# trace：方法调用路径
trace com.example.MyClass method

# monitor：方法调用统计
monitor -c 5 com.example.MyClass method

# tt：时间隧道（记录方法调用）
tt -t com.example.MyClass method

# heapdump：生成堆转储
heapdump /tmp/heap.hprof

# jvm：查看JVM信息
jvm

# memory：查看内存信息
memory

# sysprop：查看系统属性
sysprop

# sysenv：查看环境变量
sysenv

# logger：查看和修改日志级别
logger
logger --name ROOT --level debug
```



### 6.3 内存泄漏分析

#### 什么是内存泄漏

```java
public class MemoryLeakDemo {
    private static List<Object> list = new ArrayList<>();
    
    public static void main(String[] args) {
        while (true) {
            // 对象被静态集合持有，无法被GC回收
            list.add(new byte[1024 * 1024]);  // 1MB
            
            // 最终导致OutOfMemoryError
        }
    }
}
```

#### 常见内存泄漏场景

**1. 静态集合持有对象引用**
```java
public class StaticCollectionLeak {
    private static List<Object> cache = new ArrayList<>();
    
    public void addToCache(Object obj) {
        cache.add(obj);  // 永远不清理
    }
}
```

**2. 监听器未注销**
```java
public class ListenerLeak {
    public void registerListener() {
        EventBus.register(new MyListener());
        // 未注销监听器
    }
}
```

**3. ThreadLocal使用不当**
```java
public class ThreadLocalLeak {
    private static ThreadLocal<byte[]> threadLocal = new ThreadLocal<>();
    
    public void setData() {
        threadLocal.set(new byte[1024 * 1024]);
        // 线程池场景下，线程复用，数据不清理
    }
    
    // 正确做法：
    public void setDataCorrect() {
        try {
            threadLocal.set(new byte[1024 * 1024]);
            // 使用数据
        } finally {
            threadLocal.remove();  // 清理
        }
    }
}
```

**4. 连接未关闭**
```java
public class ConnectionLeak {
    public void query() {
        Connection conn = getConnection();
        // 没有关闭连接
        // 正确做法：使用try-with-resources
    }
    
    // 正确做法
    public void queryCorrect() {
        try (Connection conn = getConnection()) {
            // 使用连接
        } catch (SQLException e) {
            // 处理异常
        }
    }
}
```

#### 内存泄漏诊断步骤

**1. 监控内存使用**
```bash
# 使用jstat监控
jstat -gcutil <pid> 1000

# 观察老年代使用率是否持续上升
```

**2. 生成堆转储**
```bash
# 手动生成
jmap -dump:live,format=b,file=heap.hprof <pid>

# OOM时自动生成
java -XX:+HeapDumpOnOutOfMemoryError \
     -XX:HeapDumpPath=/tmp/heap.hprof \
     -jar myapp.jar
```

**3. 分析堆转储**

使用MAT（Memory Analyzer Tool）或VisualVM分析：

```
1. 查找泄漏点（Leak Suspects）
2. 查看支配树（Dominator Tree）
3. 分析对象引用路径（Path to GC Roots）
4. 查看类实例数量和大小
```

---

## 7. JVM调优实战

### 7.1 调优的目标

```
调优三角：
           低延迟
            /  \
           /    \
          /      \
    高吞吐 ────── 低内存
    
不可能同时满足所有目标，需要权衡
```

**调优目标示例**：
- **Web应用**：低延迟 > 高吞吐 > 低内存
- **批处理**：高吞吐 > 低内存 > 低延迟
- **移动端**：低内存 > 低延迟 > 高吞吐

### 7.2 JVM参数分类

#### 标准参数（-）

```bash
# 所有JVM都支持
-version              # 版本信息
-help                 # 帮助信息
-server               # Server模式
-client               # Client模式
-cp / -classpath      # 类路径
-D<name>=<value>      # 设置系统属性
```

#### 非标准参数（-X）

```bash
# 大部分JVM支持
-Xms<size>            # 初始堆大小
-Xmx<size>            # 最大堆大小
-Xmn<size>            # 新生代大小
-Xss<size>            # 栈大小
-Xint                 # 解释模式
-Xcomp                # 编译模式
-Xmixed               # 混合模式（默认）
```

#### 高级参数（-XX）

```bash
# JVM特定参数
# Boolean类型：-XX:+<option>（启用），-XX:-<option>（禁用）
-XX:+UseG1GC                    # 启用G1
-XX:-UseParallelGC              # 禁用Parallel

# 数值类型：-XX:<option>=<value>
-XX:MaxGCPauseMillis=200        # 最大停顿时间
-XX:MetaspaceSize=128m          # 元空间大小

# 查看所有参数
java -XX:+PrintFlagsFinal -version

# 查看默认参数
java -XX:+PrintCommandLineFlags -version
```

### 7.3 内存调优

#### 堆大小设置

```bash
# 基本原则：
# 1. Xms = Xmx（避免动态扩展开销）
# 2. 堆大小 = 系统内存 * 60%-80%
# 3. 新生代 = 堆大小 * 1/3 ~ 1/2

# 示例：8GB内存服务器
java -Xms4g -Xmx4g -Xmn1536m \
     -XX:SurvivorRatio=8 \
     -jar myapp.jar

# 大堆（>32GB）考虑：
# - 使用压缩指针限制（32GB）
# - 或使用ZGC/Shenandoah
```

#### 新生代调优

```bash
# 新生代配置
-Xmn2g                         # 直接指定大小
-XX:NewRatio=2                 # 老年代:新生代=2:1
-XX:SurvivorRatio=8            # Eden:Survivor=8:1
-XX:MaxTenuringThreshold=15    # 晋升年龄阈值

# 选择策略：
# - 对象生命周期短 → 增大新生代
# - 对象生命周期长 → 减小新生代
```

#### 元空间调优

```bash
# 元空间配置
-XX:MetaspaceSize=256m         # 初始大小
-XX:MaxMetaspaceSize=512m      # 最大大小

# 注意：
# - 不设置MaxMetaspaceSize，默认无限制
# - 类加载多（如Spring Boot）需增大
# - 动态代理多需增大
```

### 7.4 垃圾收集器选择与调优

#### 选择合适的垃圾收集器

```
场景决策树：

堆大小 < 100MB ?
    ├─是→ Serial GC
    └─否→ 应用类型？
            ├─ 单核CPU → Serial GC
            ├─ 多核CPU
            │   ├─ 延迟敏感（如Web应用）
            │   │   ├─ JDK 8 → CMS或G1
            │   │   └─ JDK 11+ → G1或ZGC
            │   └─ 吞吐量优先（如批处理）
            │       └─ Parallel GC
            └─ 超大堆（>16GB）且延迟敏感
                └─ ZGC或Shenandoah
```

#### CMS调优

```bash
# CMS配置
-XX:+UseConcMarkSweepGC
-XX:+UseCMSInitiatingOccupancyOnly
-XX:CMSInitiatingOccupancyFraction=70    # 老年代70%触发CMS
-XX:+CMSScavengeBeforeRemark             # 减少重新标记时间
-XX:+CMSParallelRemarkEnabled            # 并行重新标记
-XX:+UseCMSCompactAtFullCollection       # Full GC时整理
-XX:CMSFullGCsBeforeCompaction=0         # 每次Full GC都整理
-XX:+CMSClassUnloadingEnabled            # 类卸载
-XX:+ExplicitGCInvokesConcurrent         # System.gc()触发并发GC

# 问题诊断：
# 1. Concurrent Mode Failure → 降低CMSInitiatingOccupancyFraction
# 2. Promotion Failed → 增大新生代或提前整理碎片
# 3. 频繁Full GC → 增大堆或优化代码
```

#### G1调优

```bash
# G1配置
-XX:+UseG1GC
-XX:MaxGCPauseMillis=200                 # 目标停顿时间
-XX:G1HeapRegionSize=16m                 # Region大小
-XX:InitiatingHeapOccupancyPercent=45    # IHOP阈值
-XX:G1ReservePercent=10                  # 预留空间
-XX:G1MixedGCCountTarget=8               # Mixed GC目标次数
-XX:G1HeapWastePercent=5                 # 允许的堆浪费百分比

# 调优建议：
# 1. 不要设置新生代大小（让G1自动调整）
# 2. 不要设置-XX:+DisableExplicitGC（会影响元空间GC）
# 3. 观察GC日志，调整MaxGCPauseMillis
# 4. 大对象多 → 增大G1HeapRegionSize
```

#### ZGC调优

```bash
# ZGC配置
-XX:+UseZGC
-Xms16g -Xmx16g                          # 堆大小
-XX:ConcGCThreads=4                      # 并发线程数
-XX:ParallelGCThreads=8                  # 并行线程数
-XX:ZCollectionInterval=120              # GC间隔（秒）
-XX:ZAllocationSpikeTolerance=2          # 分配峰值容忍度

# 监控指标：
# 1. Max Pause Time < 10ms
# 2. GC频率
# 3. 堆使用率
```

### 7.5 JIT编译器调优

```bash
# JIT配置
-XX:+TieredCompilation                   # 分层编译（默认）
-XX:CompileThreshold=10000               # C2编译阈值
-XX:Tier3CompileThreshold=2000           # C1编译阈值

# 内联配置
-XX:MaxInlineSize=35                     # 最大内联方法大小
-XX:FreqInlineSize=325                   # 热点方法内联大小
-XX:InlineSmallCode=2000                 # 已编译方法内联大小

# 逃逸分析
-XX:+DoEscapeAnalysis                    # 启用（默认）
-XX:+EliminateAllocations                # 标量替换
-XX:+EliminateLocks                      # 锁消除

# 编译日志
-XX:+PrintCompilation                    # 打印编译信息
-XX:+UnlockDiagnosticVMOptions \
-XX:+PrintInlining                       # 打印内联信息
-XX:+LogCompilation \
-XX:LogFile=compilation.log              # 详细编译日志
```

### 7.6 实战案例

#### 案例1：电商应用高并发调优

**场景**：
- 8核16GB服务器
- 高并发Web应用
- 对延迟敏感

**初始配置**：
```bash
java -Xms8g -Xmx8g \
     -XX:+UseG1GC \
     -XX:MaxGCPauseMillis=200 \
     -XX:+PrintGCDetails \
     -XX:+PrintGCDateStamps \
     -Xloggc:gc.log \
     -jar ecommerce.jar
```

**问题**：GC停顿超过200ms

**分析**：
```bash
# 1. 查看GC日志
grep "pause" gc.log | awk '{print $NF}' | sort -n | tail -10

# 2. 发现Mixed GC停顿时间长
# 3. 老年代回收Region太多
```

**优化**：
```bash
java -Xms8g -Xmx8g \
     -XX:+UseG1GC \
     -XX:MaxGCPauseMillis=100 \          # 降低目标停顿
     -XX:G1MixedGCCountTarget=16 \       # 增加Mixed GC次数
     -XX:InitiatingHeapOccupancyPercent=30 \  # 提前触发标记
     -XX:G1HeapRegionSize=16m \
     -XX:+ParallelRefProcEnabled \       # 并行处理引用
     -XX:+PrintGCDetails \
     -Xloggc:gc.log \
     -jar ecommerce.jar
```

**结果**：停顿时间降至50ms以下

#### 案例2：批处理应用吞吐量优化

**场景**：
- 大数据处理
- 对吞吐量敏感
- 可容忍较长停顿

**优化配置**：
```bash
java -Xms16g -Xmx16g \
     -Xmn8g \                            # 大新生代
     -XX:+UseParallelGC \                # 吞吐量优先
     -XX:ParallelGCThreads=8 \
     -XX:MaxGCPauseMillis=500 \          # 容忍较长停顿
     -XX:GCTimeRatio=19 \                # 5%时间用于GC
     -XX:+UseAdaptiveSizePolicy \        # 自适应调整
     -jar batch-processor.jar
```

#### 案例3：内存泄漏诊断

**问题**：应用运行一段时间后OOM

**步骤1：监控内存**
```bash
# 持续监控
jstat -gcutil <pid> 1000 > jstat.log

# 观察老年代使用率持续上升
```

**步骤2：生成堆转储**
```bash
jmap -dump:live,format=b,file=heap1.hprof <pid>
# 等待一段时间
jmap -dump:live,format=b,file=heap2.hprof <pid>
```

**步骤3：使用MAT分析**
```
1. 打开heap2.hprof
2. 查看Leak Suspects报告
3. 发现：HashMap持有大量数据
4. 分析GC Roots路径
5. 定位代码位置
```

**步骤4：修复代码**
```java
// 问题代码
private static Map<String, User> cache = new HashMap<>();

public void addUser(User user) {
    cache.put(user.getId(), user);  // 永不清理
}

// 修复方案1：使用WeakHashMap
private static Map<String, User> cache = new WeakHashMap<>();

// 修复方案2：使用Guava Cache（带过期）
private static LoadingCache<String, User> cache = CacheBuilder.newBuilder()
    .maximumSize(10000)
    .expireAfterWrite(10, TimeUnit.MINUTES)
    .build(new CacheLoader<String, User>() {
        @Override
        public User load(String key) {
            return loadUser(key);
        }
    });
```

---

## 8. 高级特性与底层原理

### 8.1 对象内存布局

#### 对象结构

```
┌────────────────────────────────────┐
│          对象在内存中的布局          │
├────────────────────────────────────┤
│  1. 对象头 (Object Header)          │
│     ├─ Mark Word (标记字)           │
│     └─ Class Pointer (类型指针)     │
├────────────────────────────────────┤
│  2. 实例数据 (Instance Data)        │
│     - 对象的字段数据                │
├────────────────────────────────────┤
│  3. 对齐填充 (Padding)              │
│     - 填充到8字节的倍数             │
└────────────────────────────────────┘
```

#### Mark Word详解（64位JVM）

```
正常状态：
┌─────┬─────────┬─────┬─────┬─────┬─────┬─────┬─────┐
│未使用│hashcode │unused│age │0    │01   │64位 │
│25位 │31位     │1位  │4位 │biased|lock│     │
└─────┴─────────┴─────┴─────┴─────┴─────┴─────┴─────┘

轻量级锁：
┌───────────────────────────────────┬─────┐
│指向栈中锁记录的指针                │00   │
│62位                                │2位  │
└───────────────────────────────────┴─────┘

重量级锁：
┌───────────────────────────────────┬─────┐
│指向monitor的指针                   │10   │
│62位                                │2位  │
└───────────────────────────────────┴─────┘

GC标记：
┌───────────────────────────────────┬─────┐
│GC标记信息                          │11   │
└───────────────────────────────────┴─────┘
```

#### 查看对象布局

```java
// 使用JOL（Java Object Layout）
import org.openjdk.jol.info.ClassLayout;

public class ObjectLayoutDemo {
    private int a;
    private boolean b;
    private String c;
    
    public static void main(String[] args) {
        ObjectLayoutDemo obj = new ObjectLayoutDemo();
        System.out.println(ClassLayout.parseInstance(obj).toPrintable());
    }
}

// 输出示例：
// OFFSET  SIZE               TYPE DESCRIPTION
//      0    12                    (object header)
//     12     4                int ObjectLayoutDemo.a
//     16     1            boolean ObjectLayoutDemo.b
//     17     3                    (alignment/padding gap)
//     20     4   java.lang.String ObjectLayoutDemo.c
//     24     0                    (loss due to the next object alignment)
```

### 8.2 synchronized底层实现

#### 锁的升级过程

```
无锁 → 偏向锁 → 轻量级锁 → 重量级锁
(不可逆)

1. 偏向锁（Biased Locking）
   - 只有一个线程访问
   - CAS记录线程ID
   - 无需加锁/解锁操作

2. 轻量级锁（Lightweight Locking）
   - 多个线程交替访问（无竞争）
   - CAS操作
   - 自旋等待

3. 重量级锁（Heavyweight Locking）
   - 多个线程竞争激烈
   - 操作系统互斥量
   - 线程阻塞
```

#### synchronized字节码

```java
public class SynchronizedDemo {
    private Object lock = new Object();
    
    // 同步代码块
    public void syncBlock() {
        synchronized (lock) {
            // 代码
        }
    }
    
    // 同步方法
    public synchronized void syncMethod() {
        // 代码
    }
}

// 字节码（syncBlock）：
public void syncBlock();
  Code:
     0: aload_0
     1: getfield      #2  // Field lock
     4: dup
     5: astore_1
     6: monitorenter        // ← 获取锁
     7: aload_1
     8: monitorexit         // ← 释放锁
     9: goto          17
    12: astore_2
    13: aload_1
    14: monitorexit         // ← 异常时释放锁
    15: aload_2
    16: athrow
    17: return
```

### 8.3 volatile底层实现

#### volatile的作用

```java
public class VolatileDemo {
    /**
     * volatile两个作用：
     * 1. 保证可见性：一个线程修改，其他线程立即可见
     * 2. 禁止指令重排序
     */
    private volatile boolean flag = false;
    
    // 线程1
    public void writer() {
        flag = true;  // 写入后立即刷新到主内存
    }
    
    // 线程2
    public void reader() {
        if (flag) {   // 读取前从主内存重新加载
            // ...
        }
    }
}
```

#### 内存屏障（Memory Barrier）

```
volatile写操作：
  StoreStore屏障
  volatile写
  StoreLoad屏障

volatile读操作：
  volatile读
  LoadLoad屏障
  LoadStore屏障

作用：
- 防止重排序
- 强制刷新到主内存
- 强制从主内存读取
```

#### 单例模式的双重检查锁

```java
public class Singleton {
    // 必须使用volatile
    private static volatile Singleton instance;
    
    private Singleton() {}
    
    public static Singleton getInstance() {
        if (instance == null) {              // 第一次检查
            synchronized (Singleton.class) {
                if (instance == null) {      // 第二次检查
                    instance = new Singleton();
                }
            }
        }
        return instance;
    }
}

// 为什么需要volatile？
// new Singleton()分三步：
// 1. 分配内存
// 2. 初始化对象
// 3. 将instance指向内存
// 
// 可能重排序为1→3→2
// 其他线程可能看到未初始化的对象
// volatile禁止重排序
```



### 8.4 Java内存模型（JMM）

#### JMM抽象结构

```
┌─────────────┐     ┌─────────────┐
│   线程A      │     │   线程B      │
├─────────────┤     ├─────────────┤
│  本地内存A   │     │  本地内存B   │
│  (缓存)     │     │  (缓存)     │
└──────┬──────┘     └──────┬──────┘
       │                   │
       │     save/load     │
       └───────┬───────────┘
               ▼
       ┌──────────────┐
       │   主内存      │
       │  (共享变量)   │
       └──────────────┘
```

#### happens-before规则

```java
public class HappensBeforeDemo {
    private int a = 0;
    private volatile boolean flag = false;
    
    /**
     * happens-before规则：
     * 
     * 1. 程序顺序规则：单线程内，前面的操作 happens-before 后面的操作
     * 2. volatile规则：volatile写 happens-before volatile读
     * 3. 锁规则：解锁 happens-before 后续的加锁
     * 4. 传递性：A hb B，B hb C => A hb C
     * 5. start规则：线程start() hb 线程内的操作
     * 6. join规则：线程内操作 hb join()返回
     * 7. 中断规则：interrupt() hb 检测到中断
     * 8. 终结器规则：构造函数 hb finalize()
     */
    
    // 线程1
    public void writer() {
        a = 42;           // 1
        flag = true;      // 2 (volatile写)
    }
    
    // 线程2
    public void reader() {
        if (flag) {       // 3 (volatile读)
            int i = a;    // 4
            // 保证：1 hb 2 hb 3 hb 4
            // 所以：i一定是42
        }
    }
}
```

### 8.5 Native方法与JNI

#### 什么是Native方法

```java
public class NativeDemo {
    // Native方法声明
    public native String getSystemInfo();
    
    // 加载本地库
    static {
        System.loadLibrary("native-lib");
    }
    
    public static void main(String[] args) {
        NativeDemo demo = new NativeDemo();
        String info = demo.getSystemInfo();
        System.out.println(info);
    }
}
```

#### JNI实现（C/C++）

```c
// native-lib.c
#include <jni.h>
#include <string.h>

JNIEXPORT jstring JNICALL
Java_NativeDemo_getSystemInfo(JNIEnv *env, jobject obj) {
    // 返回系统信息
    const char *info = "System Info from Native Code";
    return (*env)->NewStringUTF(env, info);
}
```

#### 常见Native方法

```java
// Object类
public native int hashCode();
public final native Class<?> getClass();
public final native void notify();
public final native void notifyAll();
public final native void wait(long timeout);

// Thread类
public static native void sleep(long millis);
public static native void yield();
public static native Thread currentThread();

// System类
public static native long currentTimeMillis();
public static native long nanoTime();
public static native void arraycopy(Object src, int srcPos,
                                    Object dest, int destPos, int length);

// Unsafe类（sun.misc.Unsafe）
public native long allocateMemory(long bytes);
public native void freeMemory(long address);
public native void putLong(long address, long value);
public native long getLong(long address);
```

### 8.6 JVM沙箱安全机制

#### 安全模型

```
┌─────────────────────────────────────┐
│         JVM安全机制                  │
├─────────────────────────────────────┤
│  1. 类加载器（ClassLoader）          │
│     - 双亲委派模型                   │
│     - 防止核心类被篡改               │
├─────────────────────────────────────┤
│  2. 字节码验证器（Verifier）         │
│     - 文件格式验证                   │
│     - 元数据验证                     │
│     - 字节码验证                     │
│     - 符号引用验证                   │
├─────────────────────────────────────┤
│  3. 安全管理器（SecurityManager）    │
│     - 权限检查                       │
│     - 访问控制                       │
├─────────────────────────────────────┤
│  4. 访问控制器（AccessController）   │
│     - 基于栈的访问控制               │
└─────────────────────────────────────┘
```

#### SecurityManager示例

```java
public class SecurityManagerDemo {
    public static void main(String[] args) {
        // 启用安全管理器
        System.setSecurityManager(new SecurityManager());
        
        try {
            // 尝试读取文件
            System.getProperty("user.home");
            
            // 尝试退出JVM
            System.exit(0);  // 抛出SecurityException
            
        } catch (SecurityException e) {
            System.out.println("安全检查失败：" + e.getMessage());
        }
    }
}

// 启动时指定安全策略
// java -Djava.security.manager -Djava.security.policy=my.policy MyApp
```

#### 安全策略文件（my.policy）

```
grant {
    // 允许读取系统属性
    permission java.util.PropertyPermission "user.home", "read";
    
    // 允许读取指定目录
    permission java.io.FilePermission "/tmp/*", "read";
    
    // 允许网络连接
    permission java.net.SocketPermission "localhost:8080", "connect";
    
    // 禁止其他操作（默认）
};
```

### 8.7 字符串常量池

#### 字符串常量池位置变化

```
JDK 6及之前：
┌────────────┐
│   PermGen  │
│  ┌──────┐  │
│  │常量池│  │
│  └──────┘  │
└────────────┘

JDK 7：
┌────────────┐
│    堆      │
│  ┌──────┐  │
│  │常量池│  │
│  └──────┘  │
└────────────┘
┌────────────┐
│   PermGen  │
│  (其他)    │
└────────────┘

JDK 8+：
┌────────────┐
│    堆      │
│  ┌──────┐  │
│  │常量池│  │
│  └──────┘  │
└────────────┘
┌────────────┐
│  Metaspace │
│ (本地内存) │
└────────────┘
```

#### 字符串常量池详解

```java
public class StringPoolDemo {
    public static void main(String[] args) {
        // 字面量直接进入常量池
        String s1 = "hello";
        String s2 = "hello";
        System.out.println(s1 == s2);  // true
        
        // new创建新对象（堆中）
        String s3 = new String("hello");
        System.out.println(s1 == s3);  // false
        
        // intern()：尝试将字符串放入常量池
        String s4 = s3.intern();
        System.out.println(s1 == s4);  // true
        
        // JDK 7+的变化
        String s5 = new String("he") + new String("llo");
        String s6 = s5.intern();
        System.out.println(s5 == s6);  // JDK 7+: true, JDK 6: false
        // JDK 7+：常量池存储堆中对象的引用
        // JDK 6：常量池存储对象的副本
    }
}
```

#### 字符串拼接优化

```java
public class StringConcatDemo {
    public void concat1() {
        // 编译期优化为"abc"
        String s = "a" + "b" + "c";
    }
    
    public void concat2() {
        String a = "a";
        String b = "b";
        // 运行时拼接，StringBuilder优化
        String s = a + b + "c";
        
        // 等价于：
        // String s = new StringBuilder()
        //     .append(a)
        //     .append(b)
        //     .append("c")
        //     .toString();
    }
    
    public void concat3() {
        // 循环中拼接字符串（低效）
        String result = "";
        for (int i = 0; i < 10000; i++) {
            result += i;  // 每次创建新StringBuilder
        }
        
        // 优化：手动使用StringBuilder
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 10000; i++) {
            sb.append(i);
        }
        String result2 = sb.toString();
    }
}
```

### 8.8 类文件结构

#### Class文件格式

```
ClassFile {
    u4             magic;                    // 魔数：0xCAFEBABE
    u2             minor_version;            // 次版本号
    u2             major_version;            // 主版本号
    u2             constant_pool_count;      // 常量池大小
    cp_info        constant_pool[...];       // 常量池
    u2             access_flags;             // 访问标志
    u2             this_class;               // 类索引
    u2             super_class;              // 父类索引
    u2             interfaces_count;         // 接口数量
    u2             interfaces[...];          // 接口索引
    u2             fields_count;             // 字段数量
    field_info     fields[...];              // 字段表
    u2             methods_count;            // 方法数量
    method_info    methods[...];             // 方法表
    u2             attributes_count;         // 属性数量
    attribute_info attributes[...];          // 属性表
}
```

#### 查看Class文件

```bash
# 十六进制查看
hexdump -C MyClass.class | head

# 输出：
# 00000000  ca fe ba be 00 00 00 34  00 1d 0a 00 06 00 0f 09  |.......4........|
#           ↑魔数      ↑版本(52=JDK8)

# 使用javap反编译
javap -v MyClass.class

# 输出：
# Classfile /path/to/MyClass.class
#   Last modified 2024-01-15; size 512 bytes
#   MD5 checksum abc123...
#   Compiled from "MyClass.java"
# public class MyClass
#   minor version: 0
#   major version: 52  (JDK 8)
#   flags: ACC_PUBLIC, ACC_SUPER
# Constant pool:
#    #1 = Methodref          #6.#15
#    #2 = Fieldref           #16.#17
#    ...
```

### 8.9 压缩指针（Compressed Oops）

#### 什么是压缩指针

```
64位JVM的问题：
- 指针占8字节（vs 32位的4字节）
- 内存开销增加50%

压缩指针技术：
- 使用4字节表示堆引用
- 通过对齐（8字节）实现
- 支持最大32GB堆（4GB × 8）
```

#### 压缩指针原理

```
对象地址对齐到8字节：
0x00000000, 0x00000008, 0x00000010, ...

低3位始终为0，可以省略：
0x00000000 → 0x00000000 (右移3位)
0x00000008 → 0x00000001
0x00000010 → 0x00000002
...

使用时左移3位还原：
0x00000001 → 0x00000008 (左移3位)

4字节可表示：2^32 × 8 = 32GB
```

#### 压缩指针配置

```bash
# 查看是否启用压缩指针
java -XX:+PrintCommandLineFlags -version

# 启用（JDK 8默认，堆<32GB时）
-XX:+UseCompressedOops

# 禁用
-XX:-UseCompressedOops

# 压缩类指针（元空间中的类指针）
-XX:+UseCompressedClassPointers

# 对象对齐大小
-XX:ObjectAlignmentInBytes=8  # 默认8字节
```

**内存对比**：
```java
public class User {
    private long id;        // 8字节
    private String name;    // 4字节（压缩）或8字节（不压缩）
}

// 对象大小：
// 不压缩：对象头(16) + id(8) + name(8) = 32字节
// 压缩：  对象头(12) + id(8) + name(4) = 24字节
// 节省25%内存
```

---

## 9. 附录

### 9.1 常用JVM参数速查

#### 内存配置

```bash
-Xms<size>              # 初始堆大小
-Xmx<size>              # 最大堆大小
-Xmn<size>              # 新生代大小
-Xss<size>              # 栈大小
-XX:MetaspaceSize=<size>           # 元空间初始大小
-XX:MaxMetaspaceSize=<size>        # 元空间最大大小
-XX:MaxDirectMemorySize=<size>     # 直接内存最大大小
-XX:NewRatio=<n>                   # 老年代/新生代比例
-XX:SurvivorRatio=<n>              # Eden/Survivor比例
```

#### 垃圾回收器

```bash
# Serial
-XX:+UseSerialGC

# Parallel
-XX:+UseParallelGC
-XX:+UseParallelOldGC
-XX:ParallelGCThreads=<n>

# CMS
-XX:+UseConcMarkSweepGC
-XX:+UseParNewGC
-XX:CMSInitiatingOccupancyFraction=<n>
-XX:+UseCMSInitiatingOccupancyOnly

# G1
-XX:+UseG1GC
-XX:MaxGCPauseMillis=<n>
-XX:G1HeapRegionSize=<n>

# ZGC
-XX:+UseZGC

# Shenandoah
-XX:+UseShenandoahGC
```

#### GC日志

```bash
# JDK 8
-XX:+PrintGCDetails
-XX:+PrintGCDateStamps
-XX:+PrintGCTimeStamps
-Xloggc:<file>

# JDK 9+
-Xlog:gc*:file=<file>:time,level,tags
```

#### 调试诊断

```bash
-XX:+HeapDumpOnOutOfMemoryError    # OOM时生成堆转储
-XX:HeapDumpPath=<path>            # 堆转储路径
-XX:+PrintCommandLineFlags         # 打印JVM参数
-XX:+PrintFlagsFinal               # 打印所有参数
-XX:+UnlockDiagnosticVMOptions     # 解锁诊断选项
-XX:+PrintCompilation              # 打印JIT编译信息
```

### 9.2 JVM故障排查清单

#### OOM排查

```
1. 确定OOM类型
   - Java heap space → 堆内存不足
   - Metaspace → 类元数据过多
   - Direct buffer memory → 直接内存不足
   - unable to create new native thread → 线程过多

2. 收集信息
   - GC日志
   - Heap Dump
   - Thread Dump
   - 应用日志

3. 分析工具
   - MAT分析Heap Dump
   - GCViewer分析GC日志
   - jstack分析线程

4. 定位问题
   - 内存泄漏
   - 内存溢出
   - 配置不当

5. 解决方案
   - 修复代码
   - 调整JVM参数
   - 优化架构
```

#### CPU高排查

```bash
# 1. 找到Java进程
jps -l

# 2. 查看进程内线程CPU使用
top -Hp <pid>

# 3. 转换线程ID为16进制
printf "%x\n" <thread_id>

# 4. 查看线程堆栈
jstack <pid> | grep <hex_thread_id> -A 30

# 5. 分析热点代码
# 使用性能分析工具（JProfiler、Async-profiler）
```

#### 应用响应慢排查

```bash
# 1. 检查GC
jstat -gcutil <pid> 1000

# 2. 检查线程状态
jstack <pid> > threads.txt
# 查找BLOCKED、WAITING线程

# 3. 检查死锁
jstack -l <pid> | grep -A 10 "Found.*deadlock"

# 4. 检查数据库连接
# 查看连接池状态、慢SQL

# 5. 检查网络IO
# netstat、tcpdump

# 6. 性能分析
# 使用APM工具（SkyWalking、Pinpoint）
```

### 9.3 推荐学习资源

#### 书籍

1. **《深入理解Java虚拟机》**（第3版）- 周志明
   - 国内最权威的JVM书籍
   - 覆盖JVM各个方面

2. **《Java性能权威指南》** - Scott Oaks
   - 性能调优实战
   - 工具使用详解

3. **《HotSpot实战》** - 陈涛
   - HotSpot源码分析
   - 底层实现原理

#### 官方文档

- **Java虚拟机规范**：https://docs.oracle.com/javase/specs/jvms/se11/html/
- **HotSpot GC调优指南**：https://docs.oracle.com/en/java/javase/11/gctuning/
- **JDK工具文档**：https://docs.oracle.com/en/java/javase/11/tools/

#### 在线资源

- **OpenJDK源码**：https://github.com/openjdk/jdk
- **GCEasy**（GC日志分析）：https://gceasy.io
- **PerfMa**（性能魔方）：https://www.perfma.com

---

## 10. 总结

### 10.1 JVM知识体系图

```
                        JVM虚拟机
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
    内存管理            执行引擎           类加载
        │                  │                  │
    ┌───┴───┐          ┌───┴───┐          ┌───┴───┐
    │       │          │       │          │       │
   内存   垃圾       解释器   JIT        加载   双亲
   结构   回收                编译        机制   委派
    │       │          │       │          │       │
  ┌─┴─┐   ┌─┴─┐      ┌─┴─┐   ┌─┴─┐      ┌─┴─┐   ┌─┴─┐
  堆  栈   算法 收集器  热点  优化  验证 初始化 安全 沙箱
```

### 10.2 学习路径建议

**入门阶段**：
1. 理解JVM内存结构
2. 掌握垃圾回收基本概念
3. 学会使用基本监控工具

**进阶阶段**：
1. 深入理解GC算法和收集器
2. 掌握类加载机制
3. 学习JVM调优方法

**精通阶段**：
1. 理解JVM底层实现原理
2. 掌握性能分析和故障诊断
3. 阅读HotSpot源码

### 10.3 最佳实践

1. **合理设置堆大小**：-Xms = -Xmx，避免动态扩展
2. **选择合适的GC**：根据应用特点选择
3. **开启GC日志**：生产环境必须开启
4. **监控关键指标**：GC频率、停顿时间、内存使用
5. **避免过度优化**：先测量，再优化
6. **关注业务代码**：JVM调优不如代码优化

---

**文档完成时间**：2024年
**版本**：v1.0
**作者**：Kiro AI Assistant

> 本文档涵盖了JVM从基础到高级的所有核心知识点，适合Java开发者系统学习JVM原理与调优。建议结合实践项目，逐步深入理解。

