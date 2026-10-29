---
title: "Java 多线程与线程安全详解"

category:
  - Java
tag:
  - Java
---
# Java 多线程与线程安全详解

> 涵盖：线程基础、synchronized、Lock、Condition、Socket + 多线程、并发容器、线程池、锁优化

---

## 目录

1. [线程基础](#1-线程基础)
2. [synchronized 同步代码块](#2-synchronized-同步代码块)
3. [Lock 体系](#3-lock-体系)
4. [Condition 条件变量](#4-condition-条件变量)
5. [Socket + 多线程](#5-socket--多线程)
6. [volatile 与原子类](#6-volatile-与原子类)
7. [并发容器](#7-并发容器)
8. [线程池](#8-线程池)
9. [死锁与锁优化](#9-死锁与锁优化)
10. [最佳实践与常见问题](#10-最佳实践与常见问题)

---

## 1. 线程基础

### 1.1 创建线程的三种方式

```java
// 方式一：继承 Thread
class MyThread extends Thread {
    @Override
    public void run() {
        System.out.println("Thread running");
    }
}
new MyThread().start();

// 方式二：实现 Runnable（推荐，避免单继承限制）
class MyTask implements Runnable {
    @Override
    public void run() {
        System.out.println("Task running");
    }
}
new Thread(new MyTask()).start();

// 方式三：实现 Callable（带返回值 + 异常）
class MyCallable implements Callable<String> {
    @Override
    public String call() {
        return "Result";
    }
}
FutureTask<String> ft = new FutureTask<>(new MyCallable());
new Thread(ft).start();
String result = ft.get(); // 阻塞等待结果
```

### 1.2 线程生命周期

```
NEW ──start()──> RUNNABLE ──run()──> TERMINATED
                    │
              获得CPU时间片
                    │
               RUNNING（隐含在RUNNABLE中）
                    │
          ┌─────────┼──────────┐
    等待锁    wait()/join()   sleep()/I/O
          │         │          │
     BLOCKED   WAITING    TIMED_WAITING
```

- **NEW**: 已创建但未 start
- **RUNNABLE**: 可运行（就绪 + 运行中，JVM 视角不区分）
- **BLOCKED**: 等待监视器锁（synchronized）
- **WAITING**: 无限期等待（wait/join/park）
- **TIMED_WAITING**: 限期等待（sleep/wait(timeout)/join(timeout)）
- **TERMINATED**: 执行完毕

### 1.3 线程常用方法

| 方法 | 说明 |
|------|------|
| `start()` | 启动线程 |
| `join()` | 等待线程结束 |
| `sleep(ms)` | 当前线程休眠（不释放锁） |
| `yield()` | 让出 CPU（仍可能被调度回） |
| `interrupt()` | 中断线程（设置中断标志） |
| `isInterrupted()` | 检查中断标志 |
| `Thread.interrupted()` | 检查并清除中断标志 |
| `setDaemon(true)` | 设为守护线程 |

---

## 2. synchronized 同步代码块

### 2.1 三种用法

```java
// 1. 同步实例方法 —— 锁是当前对象 this
public synchronized void instanceMethod() {
    // 临界区
}

// 2. 同步静态方法 —— 锁是当前类的 Class 对象
public static synchronized void staticMethod() {
    // 临界区
}

// 3. 同步代码块 —— 锁是任意对象
public void blockMethod() {
    synchronized (this) {       // 锁 this
        // 临界区
    }
    synchronized (lockObj) {    // 锁自定义对象
        // 临界区
    }
    synchronized (MyClass.class) { // 锁 Class 对象
        // 临界区
    }
}
```

### 2.2 底层原理

- **字节码层面**: `monitorenter` / `monitorexit` 指令
- **JVM 对象头**: Mark Word 存储锁信息
- **锁升级过程（JDK 1.6+）**:

```
无锁 ──> 偏向锁 ──> 轻量级锁 ──> 重量级锁 (OS mutex)
```

| 锁状态 | 描述 | 适用场景 |
|--------|------|----------|
| 无锁 | 无竞争 | 单线程 |
| 偏向锁 | CAS 在 Mark Word 记录线程 ID | 只有一个线程访问 |
| 轻量级锁 | CAS 自旋获取锁 | 少量线程、短时间持有 |
| 重量级锁 | 操作系统互斥量，线程阻塞 | 竞争激烈、长时间持有 |

### 2.3 特性

- **可重入性**: 同一线程可多次获得同一把锁
- **互斥性**: 同一时刻只有一个线程持有锁
- **可见性**: 进入 synchronized 会刷新缓存，退出会写回主存（happens-before）
- **保证原子性**: 临界区代码不可被中断穿插

---

## 3. Lock 体系

### 3.1 Lock 接口 vs synchronized

| 对比项 | synchronized | Lock |
|--------|-------------|------|
| 语法 | 关键字，自动释放 | 接口，需手动 unlock |
| 锁获取 | 阻塞式 | tryLock 可非阻塞 |
| 中断响应 | 不响应中断 | lockInterruptibly |
| 超时 | 不支持 | tryLock(timeout) |
| 公平性 | 非公平 | 可设置公平/非公平 |
| 条件变量 | wait/notify | Condition |
| 读写分离 | 不支持 | ReadWriteLock |

### 3.2 ReentrantLock

```java
public class Counter {
    private final Lock lock = new ReentrantLock();   // 默认非公平
    // private final Lock lock = new ReentrantLock(true); // 公平锁
    private int count = 0;

    public void increment() {
        lock.lock();          // ⚠️ 必须放在 try 外
        try {
            count++;
        } finally {
            lock.unlock();    // ⚠️ 必须在 finally 中释放
        }
    }

    // 尝试获取锁（非阻塞）
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

    // 超时获取
    public boolean timedIncrement(long timeout, TimeUnit unit)
            throws InterruptedException {
        if (lock.tryLock(timeout, unit)) {
            try {
                count++;
                return true;
            } finally {
                lock.unlock();
            }
        }
        return false;
    }

    // 可中断获取
    public void interruptibleIncrement() throws InterruptedException {
        lock.lockInterruptibly();
        try {
            count++;
        } finally {
            lock.unlock();
        }
    }
}
```

### 3.3 ReentrantReadWriteLock

```java
public class ReadWriteCache {
    private final ReentrantReadWriteLock rwLock = new ReentrantReadWriteLock();
    private final Lock readLock = rwLock.readLock();
    private final Lock writeLock = rwLock.writeLock();
    private final Map<String, Object> cache = new HashMap<>();

    // 读-读不互斥，读-写互斥，写-写互斥
    public Object get(String key) {
        readLock.lock();
        try {
            return cache.get(key);
        } finally {
            readLock.unlock();
        }
    }

    public void put(String key, Object value) {
        writeLock.lock();
        try {
            cache.put(key, value);
        } finally {
            writeLock.unlock();
        }
    }
}
```

### 3.4 StampedLock（JDK 1.8+）

```java
public class StampedCache {
    private final StampedLock sl = new StampedLock();
    private int value;

    // 悲观读（与写锁互斥）
    public int read() {
        long stamp = sl.readLock();
        try {
            return value;
        } finally {
            sl.unlockRead(stamp);
        }
    }

    // 乐观读（无锁，不阻塞写线程）
    public int optimisticRead() {
        long stamp = sl.tryOptimisticRead();
        int current = value;           // 脏读可能
        if (!sl.validate(stamp)) {     // 验证期间被写 ?
            stamp = sl.readLock();
            try {
                current = value;       // 重读
            } finally {
                sl.unlockRead(stamp);
            }
        }
        return current;
    }

    public void write(int newValue) {
        long stamp = sl.writeLock();
        try {
            value = newValue;
        } finally {
            sl.unlockWrite(stamp);
        }
    }
}
```

---

## 4. Condition 条件变量

### 4.1 Condition vs wait/notify

| | synchronized + wait/notify | Lock + Condition |
|---|---|---|
| 创建方式 | 内置 | `lock.newCondition()` |
| 数量 | 一个等待集 | 多个 Condition |
| 唤醒范围 | notify 随机唤醒一个 | signal 精确唤醒 |
| 超时 | wait(timeout) | await(timeout) |
| 中断 | 不响应 | 响应中断 |

### 4.2 经典模型：BoundedQueue（生产者-消费者）

```java
public class BoundedQueue<T> {
    private final Lock lock = new ReentrantLock();
    private final Condition notFull  = lock.newCondition(); // 不满
    private final Condition notEmpty = lock.newCondition(); // 不空
    private final Object[] items;
    private int putIndex, takeIndex, count;

    public BoundedQueue(int capacity) {
        items = new Object[capacity];
    }

    public void put(T item) throws InterruptedException {
        lock.lock();
        try {
            while (count == items.length) {
                notFull.await();        // 队列满，等待"不满"
            }
            items[putIndex] = item;
            if (++putIndex == items.length) putIndex = 0;
            count++;
            notEmpty.signal();          // 通知"不空"
        } finally {
            lock.unlock();
        }
    }

    @SuppressWarnings("unchecked")
    public T take() throws InterruptedException {
        lock.lock();
        try {
            while (count == 0) {
                notEmpty.await();       // 队列空，等待"不空"
            }
            T item = (T) items[takeIndex];
            items[takeIndex] = null;
            if (++takeIndex == items.length) takeIndex = 0;
            count--;
            notFull.signal();           // 通知"不满"
            return item;
        } finally {
            lock.unlock();
        }
    }
}
```

### 4.3 wait/notify 实现（synchronized 版）

```java
public class SimpleBoundedQueue<T> {
    private final Object[] items;
    private int putIndex, takeIndex, count;

    public SimpleBoundedQueue(int capacity) {
        items = new Object[capacity];
    }

    public synchronized void put(T item) throws InterruptedException {
        while (count == items.length) {
            wait();                     // 必须持有锁
        }
        items[putIndex] = item;
        if (++putIndex == items.length) putIndex = 0;
        count++;
        notifyAll();                    // 尽量用 notifyAll 而非 notify
    }

    @SuppressWarnings("unchecked")
    public synchronized T take() throws InterruptedException {
        while (count == 0) {
            wait();
        }
        T item = (T) items[takeIndex];
        items[takeIndex] = null;
        if (++takeIndex == items.length) takeIndex = 0;
        count--;
        notifyAll();
        return item;
    }
}
```

### 4.4 Condition 注意事项

```java
// ✅ 正确：while 循环检查条件（防止虚假唤醒）
while (condition) {
    condition.await();
}

// ❌ 错误：if 判断（可能被虚假唤醒）
if (condition) {
    condition.await(); // 一旦被唤醒，不再检查条件
}

// ⚠️ signal vs signalAll
// signal   - 唤醒一个等待线程（可能导致"信号丢失"）
// signalAll- 唤醒所有等待线程（更安全，但可能有性能开销）
```

---

## 5. Socket + 多线程

### 5.1 经典：每连接一线程模型

```java
public class EchoServer {
    private final ServerSocket serverSocket;
    private final ExecutorService threadPool;

    public EchoServer(int port, int poolSize) throws IOException {
        serverSocket = new ServerSocket(port);
        threadPool = Executors.newFixedThreadPool(poolSize);
    }

    public void start() {
        System.out.println("Server listening on port " +
                serverSocket.getLocalPort());
        while (!serverSocket.isClosed()) {
            try {
                Socket socket = serverSocket.accept(); // 阻塞
                threadPool.submit(new ClientHandler(socket));
            } catch (IOException e) {
                if (!serverSocket.isClosed()) {
                    e.printStackTrace();
                }
            }
        }
    }

    public void shutdown() throws IOException {
        threadPool.shutdown();
        serverSocket.close();
    }

    private static class ClientHandler implements Runnable {
        private final Socket socket;

        ClientHandler(Socket socket) { this.socket = socket; }

        @Override
        public void run() {
            try (socket;
                 BufferedReader in = new BufferedReader(
                         new InputStreamReader(socket.getInputStream()));
                 PrintWriter out = new PrintWriter(
                         socket.getOutputStream(), true)) {
                String msg;
                while ((msg = in.readLine()) != null) {
                    System.out.println("Received: " + msg);
                    out.println("Echo: " + msg);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
```

### 5.2 线程安全的聊天室（共享资源 + 锁）

```java
public class ChatRoom {
    // 共享资源：在线用户集合 — 需线程安全
    private final Set<String> users =
            ConcurrentHashMap.newKeySet();
    // 每个用户对应的输出流
    private final Map<String, PrintWriter> writers =
            new ConcurrentHashMap<>();

    // 广播消息（遍历集合，无锁安全）
    public void broadcast(String sender, String msg) {
        for (PrintWriter writer : writers.values()) {
            writer.println("[" + sender + "] " + msg);
        }
    }

    // 用户加入（ConcurrentHashMap 本身安全，无需额外锁）
    public void join(String name, PrintWriter writer) {
        if (!users.add(name)) {
            throw new IllegalArgumentException("Name taken");
        }
        writers.put(name, writer);
        broadcast("System", name + " joined the room");
    }

    public void leave(String name) {
        users.remove(name);
        writers.remove(name);
        broadcast("System", name + " left the room");
    }
}

// 为每个客户端分配一个 ReadThread + WriteThread（分离读写锁）
public class ChatClientHandler {
    private final Socket socket;
    private final ChatRoom room;
    private final String name;

    public ChatClientHandler(Socket socket, ChatRoom room, String name) {
        this.socket = socket;
        this.room = room;
        this.name = name;
    }

    public void handle() throws IOException {
        BufferedReader in = new BufferedReader(
                new InputStreamReader(socket.getInputStream()));
        PrintWriter out = new PrintWriter(
                socket.getOutputStream(), true);

        room.join(name, out);

        // 读线程：持续读取客户端消息
        Thread readThread = new Thread(() -> {
            try {
                String msg;
                while ((msg = in.readLine()) != null) {
                    if ("/quit".equalsIgnoreCase(msg.trim())) break;
                    room.broadcast(name, msg);
                }
            } catch (IOException e) {
                // 客户端断开
            } finally {
                room.leave(name);
                try { socket.close(); } catch (IOException ignored) {}
            }
        });
        readThread.setDaemon(true);
        readThread.start();
    }
}
```

### 5.3 NIO 多路复用（Selector 模型）

```java
public class NioEchoServer {
    private Selector selector;
    private final ExecutorService pool = Executors.newFixedThreadPool(4);

    public void start(int port) throws IOException {
        selector = Selector.open();
        ServerSocketChannel ssc = ServerSocketChannel.open();
        ssc.configureBlocking(false);
        ssc.socket().bind(new InetSocketAddress(port));
        ssc.register(selector, SelectionKey.OP_ACCEPT);

        while (true) {
            selector.select(); // 阻塞直到有事件就绪
            Set<SelectionKey> keys = selector.selectedKeys();
            Iterator<SelectionKey> it = keys.iterator();
            while (it.hasNext()) {
                SelectionKey key = it.next();
                it.remove();
                if (key.isAcceptable()) {
                    accept(ssc);
                } else if (key.isReadable()) {
                    // 读事件交给线程池处理
                    pool.submit(() -> read(key));
                }
            }
        }
    }

    private void accept(ServerSocketChannel ssc) throws IOException {
        SocketChannel sc = ssc.accept();
        sc.configureBlocking(false);
        sc.register(selector, SelectionKey.OP_READ);
    }

    private void read(SelectionKey key) {
        // 处理读事件（临界区需要确保 buffer 线程安全）
        // 通常每个 SelectionKey 关联自己的 buffer，避免竞争
    }
}
```

### 5.4 Socket 多线程中的线程安全问题

| 问题 | 解决方案 |
|------|----------|
| 多个线程写同一个 OutputStream | 加锁 / 使用独立流 |
| 共享用户列表被并发修改 | ConcurrentHashMap / CopyOnWriteArrayList |
| 读-写相互阻塞 | 读写分离 + ReadWriteLock / 独立线程 |
| 连接超时 / 僵尸连接 | 心跳 + 超时检测线程 |
| 共享计数器（在线人数） | AtomicInteger / LongAdder |

---

## 6. volatile 与原子类

### 6.1 volatile 关键字

```java
public class Singleton {
    private static volatile Singleton instance; // volatile 防止指令重排序

    private Singleton() {}

    public static Singleton getInstance() {
        if (instance == null) {
            synchronized (Singleton.class) {
                if (instance == null) {
                    instance = new Singleton();
                    // 1. 分配内存
                    // 2. 初始化对象
                    // 3. 赋值给引用（volatile 禁止 2/3 重排）
                }
            }
        }
        return instance;
    }
}
```

**volatile 语义**:
- **可见性**: 写 volatile 变量会立即刷新到主存，读会从主存读取
- **有序性**: 禁止指令重排序（内存屏障）
- **不保证原子性**: `count++` 这种 read-modify-write 仍需加锁

### 6.2 原子类（Atomic*）

```java
public class AtomicCounter {
    private final AtomicInteger count = new AtomicInteger(0);

    public int increment() {
        return count.incrementAndGet();     // CAS
        // return count.getAndIncrement();  // ++i
    }

    public int decrement() {
        return count.decrementAndGet();
    }

    public int get() {
        return count.get();
    }

    // CAS 手动操作
    public boolean trySet(int expected, int newValue) {
        return count.compareAndSet(expected, newValue);
    }

    // JDK 8+ LongAdder（高并发下优于 AtomicLong）
    private final LongAdder counter = new LongAdder();
    public void add() { counter.increment(); }
    public long sum() { return counter.sum(); }
}
```

| 类 | 适用场景 |
|----|----------|
| AtomicBoolean / AtomicInteger / AtomicLong | 单个计数/状态 |
| LongAdder / LongAccumulator | 高并发写多读少 |
| AtomicReference / AtomicReferenceFieldUpdater | 对象/字段 CAS |
| AtomicIntegerArray / AtomicLongArray | 数组元素原子操作 |
| AtomicStampedReference / AtomicMarkableReference | 解决 ABA 问题 |

---

## 7. 并发容器

### 7.1 容器对比

| 非线程安全 | 线程安全（同步） | 线程安全（并发） |
|-----------|----------------|-----------------|
| HashMap | Hashtable / Collections.synchronizedMap | ConcurrentHashMap |
| ArrayList | Vector / synchronizedList | CopyOnWriteArrayList |
| TreeSet | synchronizedSortedSet | ConcurrentSkipListSet |
| LinkedList | synchronizedList | ConcurrentLinkedQueue |
| StringBuilder | StringBuffer | — |
| TreeMap | synchronizedSortedMap | ConcurrentSkipListMap |
| ArrayDeque | synchronizedDeque | ConcurrentLinkedDeque |

### 7.2 ConcurrentHashMap 详解

```java
// JDK 8+：数组 + 链表/红黑树 + CAS + synchronized
// 锁粒度：每个桶（Node）独立锁

ConcurrentHashMap<String, Integer> map = new ConcurrentHashMap<>();

// 原子复合操作（无需外部加锁）
map.putIfAbsent("key", 1);         // 不存在才 put
map.replace("key", 1, 2);          // CAS replace
map.computeIfAbsent("key", k -> expensiveLoad(k)); // 不存在则计算

// 批量操作（非原子性，需要外部同步时加锁）
map.forEach((k, v) -> System.out.println(k + "=" + v));
map.search(1, (k, v) -> v > 100 ? k : null);
map.reduce(1, (k, v) -> v, Long::sum);
```

### 7.3 CopyOnWriteArrayList

```java
// 适用：读多写少（写时复制整个数组）
// 读：无锁（直接读原数组）
// 写：加锁 + 复制新数组 + 替换引用
CopyOnWriteArrayList<String> list = new CopyOnWriteArrayList<>();

list.add("a");   // 内部 ReentrantLock + Arrays.copyOf
list.get(0);     // 不加锁
```

### 7.4 BlockingQueue（阻塞队列 — 生产者消费者首选）

| 实现类 | 特性 |
|--------|------|
| ArrayBlockingQueue | 有界，数组，FIFO |
| LinkedBlockingQueue | 可选有界，链表，FIFO |
| PriorityBlockingQueue | 无界，优先级 |
| DelayQueue | 延迟出队 |
| SynchronousQueue | 零容量，直接交付 |
| LinkedTransferQueue | 无界，TransferQueue 接口 |

```java
// 四组方法对比
//        Throws     Special  Blocks    Times out
// add    true/Exc   offer()  put()     offer(e, t, u)
// remove Exc/t      poll()   take()    poll(t, u)
// element Exc/t     peek()    —          —

BlockingQueue<Runnable> queue = new LinkedBlockingQueue<>(100);

// 生产者
queue.put(task);          // 满时阻塞

// 消费者
Runnable task = queue.take();  // 空时阻塞
```

---

## 8. 线程池

### 8.1 参数详解

```java
public ThreadPoolExecutor(
    int corePoolSize,          // 核心线程数
    int maximumPoolSize,       // 最大线程数
    long keepAliveTime,        // 空闲线程存活时间
    TimeUnit unit,             // 时间单位
    BlockingQueue<Runnable> workQueue, // 任务队列
    ThreadFactory threadFactory,       // 线程工厂
    RejectedExecutionHandler handler   // 拒绝策略
);
```

### 8.2 执行流程

```
提交任务
    │
    ├── 当前线程数 < corePoolSize? → 创建核心线程执行
    │
    ├── 尝试加入工作队列
    │   ├── 成功 → 等待调度
    │   └── 失败 → 继续
    │
    ├── 当前线程数 < maximumPoolSize? → 创建非核心线程执行
    │
    └── 拒绝策略 handler
```

### 8.3 拒绝策略

| 策略 | 行为 |
|------|------|
| AbortPolicy（默认） | 抛 RejectedExecutionException |
| CallerRunsPolicy | 调用者线程自己执行 |
| DiscardPolicy | 静默丢弃 |
| DiscardOldestPolicy | 丢弃队列中最旧的任务 |

### 8.4 Executors 工厂（不推荐生产使用）

```java
// ⚠️ 这些方法都有隐患，建议手动 new ThreadPoolExecutor

ExecutorService pool1 = Executors.newFixedThreadPool(10);
// 队列无界 → OOM

ExecutorService pool2 = Executors.newCachedThreadPool();
// 最大线程 Integer.MAX_VALUE → 线程泄漏

ScheduledExecutorService pool3 = Executors.newScheduledThreadPool(5);
// 相对安全，但建议手动指定参数
```

### 8.5 最佳实践

```java
public class ThreadPoolConfig {

    // CPU 密集型：core = CPU + 1
    // IO 密集型：core = CPU * (1 + IO等待时间 / CPU时间)

    private static final int CPU_COUNT = Runtime.getRuntime().availableProcessors();
    private static final int CORE_POOL_SIZE = CPU_COUNT + 1;
    private static final int MAX_POOL_SIZE = CPU_COUNT * 2;

    private static final ThreadPoolExecutor EXECUTOR = new ThreadPoolExecutor(
            CORE_POOL_SIZE,
            MAX_POOL_SIZE,
            60L, TimeUnit.SECONDS,
            new LinkedBlockingQueue<>(1000),
            new ThreadFactoryBuilder().setNameFormat("biz-pool-%d").build(),
            new ThreadPoolExecutor.CallerRunsPolicy()
    );

    public static void submit(Runnable task) {
        EXECUTOR.submit(task);
    }

    public static void shutdown() {
        EXECUTOR.shutdown();
        try {
            if (!EXECUTOR.awaitTermination(30, TimeUnit.SECONDS)) {
                EXECUTOR.shutdownNow();
            }
        } catch (InterruptedException e) {
            EXECUTOR.shutdownNow();
            Thread.currentThread().interrupt();
        }
    }
}
```

---

## 9. 死锁与锁优化

### 9.1 死锁产生的四个必要条件

1. **互斥**: 资源同时只能一个线程用
2. **持有并等待**: 持有一个资源等待另一个
3. **不可剥夺**: 已获得的资源不能被强行剥夺
4. **循环等待**: 多个线程形成环状等待

### 9.2 经典死锁案例

```java
public class DeadlockExample {
    private final Object lockA = new Object();
    private final Object lockB = new Object();

    public void method1() {
        synchronized (lockA) {
            // do something
            synchronized (lockB) { // 等待 lockB
            }
        }
    }

    public void method2() {
        synchronized (lockB) {
            // do something
            synchronized (lockA) { // 等待 lockA → 死锁!
            }
        }
    }
}
```

### 9.3 避免死锁的策略

```java
// 策略1：固定锁顺序（破坏循环等待）
public void transfer(Object from, Object to) {
    int fromHash = System.identityHashCode(from);
    int toHash   = System.identityHashCode(to);

    Object first  = fromHash < toHash ? from : to;
    Object second = fromHash < toHash ? to   : from;

    synchronized (first) {
        synchronized (second) {
            // 转账逻辑
        }
    }
}

// 策略2：tryLock 超时（破坏持有并等待）
public boolean transferWithTimeout(Account from, Account to, long timeout) {
    long stopTime = System.nanoTime() + TimeUnit.MILLISECONDS.toNanos(timeout);
    while (true) {
        if (from.lock.tryLock()) {
            try {
                if (to.lock.tryLock()) {
                    try {
                        // 转账
                        return true;
                    } finally { to.lock.unlock(); }
                }
            } finally { from.lock.unlock(); }
        }
        if (System.nanoTime() >= stopTime) return false;
        Thread.sleep(1); // 避免忙等
    }
}
```

### 9.4 锁优化技术（JDK 内置）

| 技术 | 说明 |
|------|------|
| **锁升级** | 偏向 → 轻量 → 重量，自动适应 |
| **锁消除** | JIT 检测到无共享时去除锁 |
| **锁粗化** | 将相邻同步块合并 |
| **自旋锁** | 短时间等待时循环尝试，避免挂起 |
| **自适应自旋** | JVM 根据历史调整自旋次数 |

---

## 10. 最佳实践与常见问题

### 10.1 线程安全准则

```markdown
1. 能不共享就不共享 —— ThreadLocal
2. 能不可变就不可变 —— final + 不可变对象
3. 能用并发容器就用并发容器 —— ConcurrentHashMap
4. 能用原子类就不用锁 —— AtomicInteger
5. 锁的粒度尽可能小 —— 分段锁 / 读写分离
6. 同步块中不要执行耗时操作 —— 数据库查询、I/O
7. 不在持有锁时调用外部方法 —— 防止意外死锁
```

### 10.2 ThreadLocal 使用

```java
public class UserContext {
    private static final ThreadLocal<User> userHolder = new ThreadLocal<>();

    public static void set(User user) { userHolder.set(user); }
    public static User get()          { return userHolder.get(); }
    public static void remove()       { userHolder.remove(); }  // 防止内存泄漏
}

// 请求入口设置
UserContext.set(currentUser);
try {
    // 业务逻辑（任何地方可获取当前用户）
    processRequest();
} finally {
    UserContext.remove(); // ⚠️ 必须清理
}
```

### 10.3 线程安全 vs 性能

```java
// ❌ 错误：使用 synchronized 包装 ArrayList
private List<String> list = Collections.synchronizedList(new ArrayList<>());

// ✅ 正确：确定并发读写需求后选择合适的容器
private List<String> list = new CopyOnWriteArrayList<>(); // 读多写少
private List<String> list = new ConcurrentLinkedQueue<>(); // FIFO 队列

// ❌ 错误：对整个方法加锁（粒度太大）
public synchronized int computeScore() { ... }

// ✅ 正确：只锁必要部分
public int computeScore() {
    // 无共享的预处理
    synchronized (this) {
        // 真正需要保护的临界区
    }
    // 无共享的后处理
}
```

### 10.4 常见问题排查

| 症状 | 可能原因 | 诊断工具 |
|------|----------|----------|
| CPU 100% | 死循环 / 自旋 | jstack / top -H |
| 无响应 | 死锁 | jstack + 检测死锁 |
| 内存 OOM | 线程泄漏 / 无界队列 | jmap / MAT |
| 性能下降 | 锁竞争激烈 | JMC / perf |
| 数据不一致 | 缺少同步 / 可见性 | jcstress / 代码审查 |

```bash
# 常用诊断命令
jps                          # 查看 Java 进程
jstack <pid>                 # 打印线程栈（定位死锁）
jstack <pid> | grep -A 10 "BLOCKED"  # 查找阻塞线程
jvisualvm                    # 可视化监控
jconsole                     # JMX 控制台
```

---

## 附录：常用代码片段

### 安全单例（双重检查 + volatile）

```java
public class Singleton {
    private static volatile Singleton instance;
    private Singleton() {}
    public static Singleton getInstance() {
        if (instance == null) {
            synchronized (Singleton.class) {
                if (instance == null) {
                    instance = new Singleton();
                }
            }
        }
        return instance;
    }
}
```

### 限流器（令牌桶）

```java
public class RateLimiterDemo {
    private final RateLimiter limiter = RateLimiter.create(100); // 100 QPS

    public void handleRequest() {
        if (limiter.tryAcquire()) {
            // 处理请求
        } else {
            // 限流拒绝
        }
    }
}
```

### 安全的懒加载

```java
public class LazyCache<K, V> {
    private final ConcurrentHashMap<K, Future<V>> cache = new ConcurrentHashMap<>();

    public V get(K key, Callable<V> loader) throws Exception {
        Future<V> f = cache.get(key);
        if (f == null) {
            FutureTask<V> ft = new FutureTask<>(loader);
            f = cache.putIfAbsent(key, ft);
            if (f == null) {
                f = ft;
                ft.run(); // 真正执行
            }
        }
        return f.get();
    }
}
```

### 优雅停机

```java
Runtime.getRuntime().addShutdownHook(new Thread(() -> {
    System.out.println("Shutting down...");
    executor.shutdown();
    try {
        executor.awaitTermination(30, TimeUnit.SECONDS);
    } catch (InterruptedException e) {
        executor.shutdownNow();
        Thread.currentThread().interrupt();
    }
}));
```

---

> **参考**: 《Java Concurrency in Practice》、《深入理解 Java 虚拟机》、《Java 并发编程的艺术》
