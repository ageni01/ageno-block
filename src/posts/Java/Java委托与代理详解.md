# Java 委托与代理详解

---

## 一、委托 (Delegation)

### 1.1 什么是委托

委托是一种**对象级别的复用模式**：一个对象将请求转发给另一个对象来处理。核心思想是"**我有这个能力，但我让你来做**"。

```
委托三要素：
  1. 委托者（Delegator）—— 发请求的对象
  2. 被委托者（Delegate）—— 实际处理请求的对象
  3. 共同接口（Interface）—— 约定能力
```

### 1.2 委托 vs 继承

```
继承（is-a）：               委托（has-a）：
  class Dog extends Animal     class Dog {
    void bark() {                Animal animal = new Animal();
      super.bark();              void bark() {
    }                               animal.bark();
  }                             }
```

| 对比维度 | 继承 | 委托 |
|----------|------|------|
| 关系 | is-a（白盒复用）| has-a（黑盒复用）|
| 耦合度 | 高（子类依赖父类实现）| 低（只依赖接口）|
| 灵活性 | 编译期确定，不能运行时改变 | 运行时切换被委托者 |
| 多继承 | Java 不支持 | 可委托多个对象 |
| 封装破坏 | 子类可能依赖父类内部细节 | 完全封装 |

**优先使用委托而非继承（"组合优于继承"）** — Effective Java 第 18 条。

### 1.3 委托代码示例

```java
// 1. 共同接口
interface Printer {
    void print(String message);
}

// 2. 被委托者（实际干活）
class CanonPrinter implements Printer {
    @Override
    public void print(String message) {
        System.out.println("Canon 打印机打印: " + message);
    }
}

class HpPrinter implements Printer {
    @Override
    public void print(String message) {
        System.out.println("HP 打印机打印: " + message);
    }
}

// 3. 委托者（自己不干活，转发请求）
class PrintManager implements Printer {
    private Printer printer;  // 持有被委托者

    public PrintManager(Printer printer) {
        this.printer = printer;
    }

    // 运行时可以切换
    public void setPrinter(Printer printer) {
        this.printer = printer;
    }

    @Override
    public void print(String message) {
        // 不做实际打印，委托给 printer
        printer.print(message);
    }
}

// 使用
PrintManager manager = new PrintManager(new CanonPrinter());
manager.print("Hello");          // Canon 打印机打印: Hello

manager.setPrinter(new HpPrinter());
manager.print("World");          // HP 打印机打印: World
```

### 1.4 委托的应用场景

| 场景 | 说明 |
|------|------|
| **策略模式** | 运行时切换算法（如支付方式选择）|
| **装饰器模式** | 增强功能，将请求委托给被装饰对象 |
| **桥接模式** | 抽象与实现分离，通过委托组合 |
| **适配器模式** | 将不兼容接口委托给适配器处理 |
| **事件/监听器** | 事件源委托给 Listener 处理事件 |
| **Spring DelegatingFilterProxy** | 将 Filter 委托给 Spring 管理的 Bean |

### 1.5 委托的 UML

```
  ┌──────────────┐          ┌──────────────────┐
  │ <<interface>>│          │   PrintManager   │
  │   Printer    │◄─────────│   (Delegator)    │
  └──────────────┘          │ - printer:Printer│
         ▲                  │ + print()        │
         │                  │ + setPrinter()   │
  ┌──────┴───────┐          └────────┬─────────┘
  │ CanonPrinter │                   │ 委托
  │ HpPrinter    │                   │
  └──────────────┘          ┌────────▼─────────┐
                            │  CanonPrinter    │
                            │  (Delegate)      │
                            └──────────────────┘
```

---

## 二、代理 (Proxy)

### 2.1 什么是代理

代理模式：为另一个对象提供一个**替身或占位符**，以控制对这个对象的访问。代理和被代理对象**实现同一接口**，代理持有被代理对象的引用，并在调用前后执行额外操作。

```
代理的核心：
  调用方 → 代理对象 →（前置处理）→ 被代理对象 →（后置处理）→ 返回结果
```

### 2.2 代理 vs 委托

| | 委托 (Delegation) | 代理 (Proxy) |
|--|--|--|
| 目的 | 复用能力，转发请求 | 控制访问，增强功能 |
| 关注点 | "谁来做"（被委托者）| "额外做什么"（增强逻辑）|
| 关系 | 委托者持有被委托者 | 代理持有被代理对象 |
| 典型场景 | 策略模式、组合复用 | AOP、延迟加载、远程调用 |
| 被代理/委托者是否知情 | 不知情（被委托者只管干活）| 不知情（被代理者不知有代理）|

> **一句话区分：** 委托是"你帮我做"；代理是"我替你做，但我还能加点东西"。

### 2.3 静态代理

```java
// 共同接口
interface Image {
    void display();
}

// 被代理对象（真实对象）
class RealImage implements Image {
    private String filename;

    public RealImage(String filename) {
        this.filename = filename;
        loadFromDisk();  // 加载图片（开销大）
    }

    private void loadFromDisk() {
        System.out.println("从磁盘加载: " + filename);
    }

    @Override
    public void display() {
        System.out.println("显示图片: " + filename);
    }
}

// 静态代理
class ImageProxy implements Image {
    private RealImage realImage;  // 持有被代理对象
    private String filename;

    public ImageProxy(String filename) {
        this.filename = filename;
    }

    @Override
    public void display() {
        // 前置增强：延迟加载
        if (realImage == null) {
            realImage = new RealImage(filename);
        }

        // 前置增强：日志
        System.out.println("[LOG] 开始显示图片: " + filename);

        // 调用被代理对象
        realImage.display();

        // 后置增强：日志
        System.out.println("[LOG] 图片显示完成");
    }
}

// 使用
Image image = new ImageProxy("photo.jpg");
image.display();  // 第一次才真正加载
image.display();  // 第二次不再加载
```

**静态代理的缺点：**
- 每个类都需要手动编写代理类
- 如果接口增加方法，代理类和被代理类都需要修改
- 类爆炸（随着代理类增多）

### 2.4 JDK 动态代理

利用 Java 反射机制，**运行时动态生成代理类**，无需手动编写代理类。

```
原理：
  Proxy.newProxyInstance() → 运行时生成 $Proxy0 类
  → $Proxy0 实现了目标接口
  → 所有方法调用转发给 InvocationHandler.invoke()
```

```java
import java.lang.reflect.Proxy;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

// 1. 被代理对象实现接口
class UserServiceImpl implements UserService {
    @Override
    public void addUser(String name) {
        System.out.println("添加用户: " + name);
    }

    @Override
    public void deleteUser(int id) {
        System.out.println("删除用户: " + id);
    }
}

// 2. InvocationHandler（增强逻辑）
class LogHandler implements InvocationHandler {
    private final Object target;  // 被代理对象

    public LogHandler(Object target) {
        this.target = target;
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
        // 前置增强
        System.out.println("[前置日志] 调用方法: " + method.getName());

        // 调用被代理对象的方法
        Object result = method.invoke(target, args);

        // 后置增强
        System.out.println("[后置日志] 方法执行完成: " + method.getName());

        return result;
    }
}

// 3. 创建代理
UserService target = new UserServiceImpl();
UserService proxy = (UserService) Proxy.newProxyInstance(
    target.getClass().getClassLoader(),
    target.getClass().getInterfaces(),
    new LogHandler(target)
);

proxy.addUser("张三");
// 输出：
// [前置日志] 调用方法: addUser
// 添加用户: 张三
// [后置日志] 方法执行完成: addUser
```

**JDK 动态代理的限制：**
- **只能代理接口**（被代理类必须实现接口）
- 代理类继承 `Proxy`，Java 单继承，不能再继承其他类

### 2.5 CGLIB 动态代理

当类没有实现接口时，使用 CGLIB（Code Generation Library）通过**继承**生成子类代理。

```
原理：
  Enhancer.create() → 生成被代理类的子类
  → 覆盖（重写）非 final 方法
  → MethodInterceptor 拦截方法调用
```

```java
import net.sf.cglib.proxy.Enhancer;
import net.sf.cglib.proxy.MethodInterceptor;
import net.sf.cglib.proxy.MethodProxy;
import java.lang.reflect.Method;

// 1. 被代理类（没有接口）
class UserService {
    public void addUser(String name) {
        System.out.println("添加用户: " + name);
    }

    public final void finalMethod() {
        System.out.println("final 方法不能被代理");
    }
}

// 2. MethodInterceptor
class CglibProxy implements MethodInterceptor {
    @Override
    public Object intercept(Object obj, Method method, Object[] args, MethodProxy proxy)
            throws Throwable {
        System.out.println("[前置] " + method.getName());

        // 调用父类方法（即被代理类的方法）
        Object result = proxy.invokeSuper(obj, args);

        System.out.println("[后置] " + method.getName());
        return result;
    }
}

// 3. 创建代理
Enhancer enhancer = new Enhancer();
enhancer.setSuperclass(UserService.class);
enhancer.setCallback(new CglibProxy());

UserService proxy = (UserService) enhancer.create();
proxy.addUser("张三");
// proxy.finalMethod();  // final 方法不会被代理
```

**CGLIB 的限制：**
- 无法代理 `final` 类
- 无法代理 `final` 方法
- 需要引入 CGLIB 依赖

### 2.6 JDK 动态代理 vs CGLIB 对比

| 对比维度 | JDK 动态代理 | CGLIB 动态代理 |
|----------|-------------|----------------|
| 代理方式 | 接口代理（Proxy + InvocationHandler）| 继承代理（Enhancer + MethodInterceptor）|
| 要求 | 必须实现接口 | 无需接口，但不能是 final 类/方法 |
| 底层 | 反射（Method.invoke）| ASM 字节码框架，直接生成字节码 |
| 性能 | 较低（反射调用）| 较高（直接调用）|
| 创建代理速度 | 快 | 慢（生成字节码开销）|
| 调用性能（JDK 8+）| **已大幅提升，接近 CGLIB** | 略优于 JDK 代理 |

> **Spring 的选择策略：**
> - 如果目标类**实现了接口** → 使用 JDK 动态代理
> - 如果目标类**没有实现接口** → 使用 CGLIB 动态代理
> - Spring Boot 2.0+ 默认强制使用 CGLIB（spring.aop.proxy-target-class=true）

### 2.7 Java 动态代理源码分析（简略）

```
Proxy.newProxyInstance(classLoader, interfaces, handler)
  │
  ├── 1. 从缓存查找或生成代理类字节码
  │     └── ProxyGenerator.generateProxyClass() 生成 $Proxy0.class
  │
  ├── 2. 加载代理类 $Proxy0
  │     └── native defineClass0() 将字节码加载到 JVM
  │
  └── 3. 构造代理实例
        └── $Proxy0(InvocationHandler handler) 构造器
              └── super(handler)

$Proxy0 代理类：
  class $Proxy0 extends Proxy implements UserService {
      private static Method m1 = ...;  // addUser 方法
      private static Method m2 = ...;  // toString 方法
      ...

      public void addUser(String name) {
          super.h.invoke(this, m1, new Object[]{name});
      }
  }
```

---

## 三、代理模式的应用场景

### 3.1 常见代理类型

| 代理类型 | 目的 | 示例 |
|----------|------|------|
| **远程代理** | 屏蔽远程调用细节 | RPC、gRPC、Dubbo |
| **虚拟代理** | 延迟加载大对象 | 图片懒加载 |
| **保护代理** | 权限控制 | 安全框架（Spring Security）|
| **缓存代理** | 缓存结果 | 查询缓存（Redis 代理）|
| **日志代理** | 记录操作日志 | AOP 日志 |
| **事务代理** | 管理数据库事务 | Spring @Transactional |
| **同步代理** | 线程安全控制 | Collections.synchronizedList() |

### 3.2 AOP 的本质

AOP（面向切面编程）就是**动态代理 + 过滤器链**：

```
Spring AOP 执行流程：

  ├── 调用 proxy.method()
  │     └── JDK/CGLIB 代理拦截
  │           └── InvocationHandler / MethodInterceptor
  │                 └── 构建拦截器链（Interceptor Chain）
  │                       ├── @Before 前置通知
  │                       ├── 实际方法调用
  │                       ├── @Around 环绕通知
  │                       ├── @AfterReturning 返回通知
  │                       └── @AfterThrowing 异常通知
  └── 返回结果
```

### 3.3 与装饰器模式的区别

| | 代理 (Proxy) | 装饰器 (Decorator) |
|--|--|--|
| 目的 | 控制访问 | 增强功能 |
| 关系 | 代理与被代理通常编译时固定 | 装饰器可多层嵌套 |
| 创建 | 代理通常由框架创建（调用方不知代理存在）| 调用方显式包装 |
| 访问 | 代理可以控制是否访问被代理对象 | 装饰器必须调用被装饰对象 |
| 典型 | AOP、远程调用 | IO 流（BufferedReader -> FileReader）|

---

## 四、实战案例

### 4.1 用 JDK 动态代理实现 AOP 日志

```java
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
@interface Loggable {
    String value() default "";
}

class LogAspect implements InvocationHandler {
    private final Object target;

    public LogAspect(Object target) {
        this.target = target;
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
        // 只对有 @Loggable 注解的方法做日志
        if (method.isAnnotationPresent(Loggable.class)) {
            Loggable log = method.getAnnotation(Loggable.class);
            long start = System.currentTimeMillis();
            System.out.println("[开始] " + log.value() + " - " + System.currentTimeMillis());

            try {
                Object result = method.invoke(target, args);
                long cost = System.currentTimeMillis() - start;
                System.out.println("[结束] " + log.value() + " - 耗时: " + cost + "ms");
                return result;
            } catch (Exception e) {
                System.out.println("[异常] " + log.value() + " - " + e.getMessage());
                throw e;
            }
        }
        return method.invoke(target, args);
    }

    @SuppressWarnings("unchecked")
    public static <T> T createProxy(T target) {
        return (T) Proxy.newProxyInstance(
                target.getClass().getClassLoader(),
                target.getClass().getInterfaces(),
                new LogAspect(target));
    }
}

// 使用
interface BizService {
    @Loggable("业务处理")
    void process(String data);
}

class BizServiceImpl implements BizService {
    @Override
    public void process(String data) {
        System.out.println("执行业务: " + data);
    }
}

BizService service = LogAspect.createProxy(new BizServiceImpl());
service.process("测试数据");
```

### 4.2 用委托实现策略模式

```java
// 支付策略
interface PaymentStrategy {
    void pay(double amount);
}

class AliPay implements PaymentStrategy {
    @Override
    public void pay(double amount) {
        System.out.println("支付宝支付: " + amount);
    }
}

class WeChatPay implements PaymentStrategy {
    @Override
    public void pay(double amount) {
        System.out.println("微信支付: " + amount);
    }
}

class CreditCardPay implements PaymentStrategy {
    @Override
    public void pay(double amount) {
        System.out.println("信用卡支付: " + amount);
    }
}

// 委托者（上下文）
class PaymentContext {
    private PaymentStrategy strategy;

    public PaymentContext(PaymentStrategy strategy) {
        this.strategy = strategy;
    }

    // 运行时切换策略
    public void setStrategy(PaymentStrategy strategy) {
        this.strategy = strategy;
    }

    // 委托给策略对象
    public void executePayment(double amount) {
        strategy.pay(amount);
    }
}

// 使用
PaymentContext context = new PaymentContext(new AliPay());
context.executePayment(100);          // 支付宝支付: 100

context.setStrategy(new WeChatPay());
context.executePayment(200);          // 微信支付: 200
```

---

## 五、总结对比

```
┌──────────────────────────────────────────────────────────────┐
│                        委托 vs 代理                          │
├────────────┬────────────────────────┬────────────────────────┤
│            │        委托            │         代理           │
├────────────┼────────────────────────┼────────────────────────┤
│ 核心思想   │ 让别人代替我做         │ 替别人做，并加点东西   │
│ 关注点     │ 能力复用/策略切换      │ 访问控制/功能增强      │
│ 代码侧重   │ 持有被委托者，转发请求 │ 持有被代理者，增强调用 │
│ 创建方式   │ 手动组合               │ 手动或动态生成         │
│ 经典模式   │ 策略模式、桥接模式     │ AOP、延迟加载、远程调用│
│ 典型框架   │ 业务逻辑组合           │ Spring AOP、MyBatis    │
├────────────┼────────────────────────┼────────────────────────┤
│ 动态代理   │ JDK Proxy  /  CGLIB    │                        │
├────────────┼────────────────────────┼────────────────────────┤
│ JDK Proxy  │ 要求接口，反射调用     │                        │
│ CGLIB      │ 无需接口，生成子类     │                        │
│ Spring AOP │ 接口 → JDK，无接口 → CGLIB                       │
└────────────┴────────────────────────────────────────────────┘
```

---

## 附录：Spring AOP 中的代理机制

```java
// Spring 通过 @EnableAspectJAutoProxy 开启 AOP
@Configuration
@EnableAspectJAutoProxy  // proxyTargetClass=false → JDK 代理（默认）
public class AppConfig {}

// 或者强制 CGLIB
@EnableAspectJAutoProxy(proxyTargetClass = true)

// Spring Boot 2.x+ 默认 proxyTargetClass = true
// 即默认使用 CGLIB 代理（即使有接口也使用 CGLIB）
```

**自调用问题：**
```java
@Service
public class UserService {
    @Transactional
    public void addUser(String name) {
        // ...
        updateLog(name);  // 自调用：@Transactional 注解失效！
    }

    @Transactional
    public void updateLog(String name) {
        // ...
    }
}
// 因为自调用不会走代理对象，this.updateLog() 直接调用了本类方法
// 解决：注入自身代理
@Service
public class UserService {
    @Autowired
    private UserService self;  // 注入代理对象

    @Transactional
    public void addUser(String name) {
        self.updateLog(name);  // 调用代理对象的方法
    }
}
```
