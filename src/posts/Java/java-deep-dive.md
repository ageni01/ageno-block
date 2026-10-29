---
title: "Java 深入浅出：多线程、AOP、锁、Spring、Netty 源码级分析"

category:
  - Java
tag:
  - Java
---
# Java 深入浅出：多线程、AOP、锁、Spring、Netty 源码级分析

> 本文从 JDK 源码、Spring 源码、Netty 源码层面深入剖析 Java 核心技术，涵盖多线程、AOP、锁机制、Spring IoC/AOP/事务、Spring Boot 自动配置、Netty 架构与自定义协议、Socket 编程等核心主题。

---

## 目录

1. [Java 多线程详解](#1-java-多线程详解)
2. [Java AOP 详解](#2-java-aop-详解)
3. [锁与条件变量](#3-锁与条件变量)
4. [Spring 源码详解](#4-spring-源码详解)
5. [Spring Boot 源码详解](#5-spring-boot-源码详解)
6. [Netty 详解](#6-netty-详解)
7. [Netty 自定义协议](#7-netty-自定义协议)
8. [高性能自定义协议（Netty）](#8-高性能自定义协议netty)
9. [Java Socket 详解](#9-java-socket-详解)

---

## 1. Java 多线程详解

### 1.1 Thread 与 Runnable 底层

Java 多线程的根基在于 `java.lang.Thread` 类和 `java.lang.Runnable` 接口。从源码层面来看，`Thread` 类本身实现了 `Runnable` 接口，这意味着 Thread 本质上也是一个 Runnable。当我们创建一个 Thread 对象并调用其 `start()` 方法时，JVM 会通过 `start0()` 这个 native 方法向操作系统申请创建一个新的操作系统线程。这个新线程随后会调用 `Thread` 对象的 `run()` 方法。如果构造 Thread 时传入了 Runnable 实例，则 `run()` 方法会委托给该实例的 `run()` 执行；否则，子类需要重写 `run()` 方法。

理解 `start()` 和 `run()` 的本质区别非常重要：调用 `start()` 会在操作系统层面创建一条新的执行路径，而直接调用 `run()` 仅仅是普通的方法调用，在当前线程中同步执行。`Thread` 与 `Runnable` 之间应用的是策略设计模式——Thread 负责线程的创建和生命周期管理，而 Runnable 封装了具体要执行的业务逻辑，二者各司其职。

```java
public class Thread implements Runnable {
    private Runnable target;
    
    public Thread(Runnable target) {
        this.target = target;
    }
    
    @Override
    public void run() {
        if (target != null) {
            target.run();
        }
    }
    
    private native void start0();
    
    public synchronized void start() {
        if (threadStatus != 0)
            throw new IllegalThreadStateException();
        group.add(this);
        start0();
    }
}
```

**源码层面的关键点分析**：
- `start()` 方法是 `synchronized` 的，确保同一线程不会重复启动
- `start0()` 是 native 方法，在 HotSpot 中对应 `JVM_StartThread` 函数
- `threadStatus` 用于状态检查，初始为 0，`start()` 后由 JVM 修改
- `group.add(this)` 将线程加入线程组，线程组用于批量管理线程

### 1.2 线程生命周期（源码级别）

JDK 中 `Thread.State` 枚举定义了线程的 6 种状态，这些状态直接映射到 JVM 内部的线程状态管理机制中。从操作系统层面来看，Java 的线程状态与操作系统线程状态之间存在映射关系，但并非一一对应。

```java
public enum State {
    NEW,           // 新建：线程对象已创建，尚未调用 start()
    RUNNABLE,      // 就绪/运行：JVM 层面，对应 OS 的 Ready + Running
    BLOCKED,       // 阻塞：等待监视器锁（synchronized 的隐式锁）
    WAITING,       // 等待：需显式唤醒（Object.wait, Thread.join, LockSupport.park）
    TIMED_WAITING, // 超时等待：带超时参数（Thread.sleep, Object.wait(timeout)）
    TERMINATED;    // 终止：线程执行完毕
}
```

**状态变迁路径**：`NEW` 状态的线程对象在 Java 堆中已分配完毕，但还没有关联操作系统线程。调用 `start()` 后，JVM 通过 `start0()` native 方法创建操作系统线程，此时线程进入 `RUNNABLE` 状态。当线程尝试进入 `synchronized` 保护的代码块但锁被其他线程持有时，线程进入 `BLOCKED` 状态。当线程调用 `Object.wait()`、`Thread.join()` 或 `LockSupport.park()` 时，它会主动让出 CPU 并进入 `WAITING` 或 `TIMED_WAITING` 状态。

```
NEW ──start()──▶ RUNNABLE ──run()完成──▶ TERMINATED
RUNNABLE ──synchronized未竞争到锁──▶ BLOCKED
RUNNABLE ──Object.wait()/LockSupport.park()──▶ WAITING
RUNNABLE ──Thread.sleep(time)/Object.wait(timeout)──▶ TIMED_WAITING
```

`Thread.getState()` 是 native 方法，JVM 内部通过 `java_lang_Thread` 类获取操作系统线程的当前状态并映射到 Java 的枚举值。在 HotSpot 源码中，Java 线程状态由 `JavaThread` 类管理，通过 `thread_state` 枚举实现与 OS 线程状态的转换。

### 1.3 线程池：ThreadPoolExecutor 源码解析

#### ctl 核心控制字段

`ThreadPoolExecutor` 是 Java 线程池的核心实现，其设计极其精妙。它使用一个 `AtomicInteger` 类型的 `ctl` 字段同时维护线程池的**运行状态**（高 3 位）和**工作线程数**（低 29 位）。这种位运算设计的最大好处是：一次 CAS 操作可以原子性地同时修改状态和线程数，避免了使用两个独立变量时可能出现的竞态条件。

```java
private final AtomicInteger ctl = new AtomicInteger(ctlOf(RUNNING, 0));

// 状态常量
private static final int COUNT_BITS = Integer.SIZE - 3; // 29
private static final int COUNT_MASK = (1 << COUNT_BITS) - 1; // 低 29 位全 1

// 线程池状态（高 3 位）
private static final int RUNNING    = -1 << COUNT_BITS; // 111 + 29个0
private static final int SHUTDOWN   =  0 << COUNT_BITS; // 000
private static final int STOP       =  1 << COUNT_BITS; // 001
private static final int TIDYING    =  2 << COUNT_BITS; // 010
private static final int TERMINATED =  3 << COUNT_BITS; // 011

// 提取工作线程数
private static int workerCountOf(int c) { return c & COUNT_MASK; }
// 提取运行状态
private static int runStateOf(int c) { return c & ~COUNT_MASK; }
// 合并状态和数量
private static int ctlOf(int rs, int wc) { return rs | wc; }
```

**线程池状态变迁分析**：`RUNNING` 是 `-1 << 29`（高 3 位为 `111`），`SHUTDOWN` 为 `0 << 29`，`STOP` 为 `1 << 29`。这种设计使得状态的大小顺序为 `RUNNING < SHUTDOWN < STOP < TIDYING < TERMINATED`，因此可以用 `runStateAtLeast(c, state)` 简洁地判断是否处于或超过某个状态。

```
RUNNING ──shutdown()──▶ SHUTDOWN（拒绝新任务，但处理队列中已有任务）
RUNNING ──shutdownNow()──▶ STOP（停止所有任务，返回未执行任务队列）
SHUTDOWN ──队列为空且worker数为0──▶ TIDYING ──terminated()──▶ TERMINATED
STOP ──worker数为0──▶ TIDYING ──terminated()──▶ TERMINATED
```

#### execute() 方法的三路策略

`execute()` 方法体现了线程池的核心调度逻辑，它将任务分配分为三条路径，形成三级降级策略：

```java
public void execute(Runnable command) {
    int c = ctl.get();
    if (workerCountOf(c) < corePoolSize) {
        if (addWorker(command, true))
            return;
        c = ctl.get();
    }
    if (isRunning(c) && workQueue.offer(command)) {
        int recheck = ctl.get();
        if (!isRunning(recheck) && remove(command))
            reject(command);
        else if (workerCountOf(recheck) == 0)
            addWorker(null, false);
    } else if (!addWorker(command, false)) {
        reject(command);
    }
}
```

**第一条路径（核心线程）**：如果当前工作线程数小于核心线程数（`corePoolSize`），则创建新的核心线程来执行这个任务。这是最优先的方式，因为新创建的核心线程后续还能从队列中获取任务执行。

**第二条路径（工作队列）**：如果核心线程已满，尝试将任务加入阻塞队列等待。这里有一个重要的**双重检查**机制——在成功入队后，再次检查线程池的状态：如果线程池已关闭，则从队列中移除任务并执行拒绝策略。如果此时工作线程数为零（例如核心线程数配置为 0），则创建一个空的 Worker 来执行队列中的任务。

**第三条路径（非核心线程）**：如果队列已满，尝试创建非核心线程（即超出 `corePoolSize` 但不超过 `maximumPoolSize` 的线程）。如果这也失败，则执行拒绝策略。

这种三级降级设计确保了线程池在不同负载下的平滑行为：低负载时使用核心线程，中等负载时使用队列缓冲，高负载时创建临时线程，极端负载时触发拒绝策略。

#### addWorker 源码分析

`addWorker` 方法分为两个阶段：第一阶段通过 CAS 无锁算法安全地递增 `workerCount`，第二阶段创建 `Worker` 对象并启动线程。

```java
private boolean addWorker(Runnable firstTask, boolean core) {
    retry:
    for (int c = ctl.get();;) {
        if (runStateAtLeast(c, SHUTDOWN)
            && (runStateAtLeast(c, STOP) || firstTask != null || workQueue.isEmpty()))
            return false;

        for (;;) {
            int wc = workerCountOf(c);
            if (wc >= CAPACITY || wc >= (core ? corePoolSize : maximumPoolSize))
                return false;
            if (compareAndIncrementWorkerCount(c))
                break retry;
            c = ctl.get();
            if (runStateAtLeast(c, SHUTDOWN))
                continue retry;
        }
    }

    boolean workerStarted = false;
    boolean workerAdded = false;
    Worker w = null;
    try {
        w = new Worker(firstTask);
        final Thread t = w.thread;
        if (t != null) {
            final ReentrantLock mainLock = this.mainLock;
            mainLock.lock();
            try {
                int c = ctl.get();
                if (isRunning(c) || (runStateLessThan(c, STOP) && firstTask == null)) {
                    if (t.isAlive())
                        throw new IllegalThreadStateException();
                    workers.add(w);
                    workerAdded = true;
                }
            } finally {
                mainLock.unlock();
            }
            if (workerAdded) {
                t.start();
                workerStarted = true;
            }
        }
    } finally {
        if (!workerStarted)
            addWorkerFailed(w);
    }
    return workerStarted;
}
```

**深入分析第一阶段**：外层循环检查线程池状态是否允许添加 Worker（例如 SHUTDOWN 状态下如果 firstTask 不为空则不允许，因为 SHUTDOWN 状态不再接收新任务）。内层循环通过 CAS 原子性地递增 `workerCount`。这里使用了**双重循环 CAS** 的经典无锁并发设计模式——当 CAS 竞争失败时，重新读取 `ctl` 值重试。这种设计避免了使用重量级锁，在高并发场景下性能更好。

**第二阶段**：创建 `Worker` 实例后，在 `mainLock`（`ReentrantLock`）保护下将 Worker 添加到 `workers`（`HashSet<Worker>`）集合中。注意添加前的状态检查——此时线程池必须是 RUNNING 状态，或者处于 SHUTDOWN 状态但 firstTask 为 null（即创建空 Worker 处理队列中已有的任务）。添加成功后调用 `t.start()` 启动 Worker 线程，进入 `runWorker` 核心循环。如果启动失败，`addWorkerFailed` 方法会回滚——从 `workers` 集合中移除 Worker、减少 `workerCount` 并尝试终止线程池。
```

#### Worker 内部类的精妙设计

`Worker` 继承自 `AbstractQueuedSynchronizer`（AQS），实现了 `Runnable`。它本身就是一个**可执行的同步器**——既封装了工作线程的生命周期，又通过 AQS 实现了线程的中断控制。

```java
private final class Worker extends AbstractQueuedSynchronizer implements Runnable {
    final Thread thread;
    Runnable firstTask;

    Worker(Runnable firstTask) {
        setState(-1);
        this.firstTask = firstTask;
        this.thread = getThreadFactory().newThread(this);
    }

    @Override
    public void run() {
        runWorker(this);
    }
}
```

**Worker 继承 AQS 的设计动机**：利用 AQS 的排他锁语义来控制 Worker 的中断。在 `runWorker` 执行任务时，Worker 会通过 `w.lock()` 获取自身的排他锁。当外部调用 `shutdownNow()` 需要中断所有 Worker 时，`interruptWorkers()` 方法会遍历所有 Worker，如果某个 Worker 正在执行任务（即持有锁），则不会中断它。这保证了任务执行的完整性——任务正在运行时不会被中断打断。`setState(-1)` 在构造时将 state 初始化为 -1，可以防止 Worker 在 `runWorker` 调用 `w.unlock()` 之前被中断。

#### runWorker 核心循环

`runWorker` 是线程池中每个工作线程的"心脏"——它在一个无限循环中不断从 `workQueue` 获取任务执行。这个循环的设计极其精巧，兼顾了任务执行、中断处理和线程退出等多个方面：

```java
final void runWorker(Worker w) {
    Thread wt = Thread.currentThread();
    Runnable task = w.firstTask;
    w.firstTask = null;
    w.unlock();
    boolean completedAbruptly = true;
    try {
        while (task != null || (task = getTask()) != null) {
            w.lock();
            if ((runStateAtLeast(ctl.get(), STOP) ||
                 (Thread.interrupted() && runStateAtLeast(ctl.get(), STOP))) &&
                !wt.isInterrupted())
                wt.interrupt();
            try {
                beforeExecute(wt, task);
                try {
                    task.run();
                    afterExecute(task, null);
                } catch (Throwable ex) {
                    afterExecute(task, ex);
                    throw ex;
                }
            } finally {
                task = null;
                w.completedTasks++;
                w.unlock();
            }
        }
        completedAbruptly = false;
    } finally {
        processWorkerExit(w, completedAbruptly);
    }
}
```

**执行流程分析**：首先执行 firstTask（构造时传入的初始任务），然后循环从 `getTask()` 获取后续任务。`getTask()` 方法根据配置决定使用 `workQueue.take()`（阻塞等待）还是 `workQueue.poll(keepAliveTime, ...)`（超时等待）。如果启用了 `allowCoreThreadTimeOut` 或当前线程数超过核心线程数，则使用超时等待；否则使用阻塞等待。

**任务执行前的中断检查**：如果线程池状态已经达到 STOP，或当前线程被中断且状态达到 STOP，则给工作线程设置中断标志。这是为了保证线程池在停止过程中能够及时响应。

**任务异常处理**：如果 `task.run()` 抛出异常，会被 `afterExecute` 捕获记录，然后重新抛出中断循环。`completedAbruptly` 标记用于区分正常退出和异常退出——异常退出时 `processWorkerExit` 会创建新的 Worker 来补充。

**`processWorkerExit`** 执行退出清理：从 `workers` 集合中移除该 Worker、递减 `workerCount`、如果线程池没有停止则尝试补充新的 Worker（以维持核心线程数）。

`getTask()` 从 `workQueue` 拉取任务的逻辑如下：

```java
private Runnable getTask() {
    boolean timedOut = false;
    for (;;) {
        int c = ctl.get();
        // 如果线程池已停止，或 SHUTDOWN 且队列为空 → 减少 worker 并返回 null
        if (runStateAtLeast(c, SHUTDOWN) && (runStateAtLeast(c, STOP) || workQueue.isEmpty())) {
            decrementWorkerCount();
            return null;
        }
        int wc = workerCountOf(c);
        boolean timed = allowCoreThreadTimeOut || wc > corePoolSize;
        if ((wc > maximumPoolSize || (timed && timedOut))
            && (wc > 1 || workQueue.isEmpty())) {
            if (compareAndDecrementWorkerCount(c))
                return null;
            continue;
        }
        try {
            Runnable r = timed ?
                workQueue.poll(keepAliveTime, TimeUnit.NANOSECONDS) :
                workQueue.take(); // 阻塞
            if (r != null)
                return r;
            timedOut = true;
        } catch (InterruptedException retry) {
            timedOut = false;
        }
    }
}
```

### 1.4 Future/Callable 与 CompletableFuture

#### Callable 与 FutureTask

`Callable` 可返回结果和抛出异常，`FutureTask` 是 `RunnableFuture` 的实现：

```java
public interface Callable<V> {
    V call() throws Exception;
}

// FutureTask 内部状态机
private volatile int state;
private static final int NEW          = 0; // 新建
private static final int COMPLETING   = 1; // 结果正在设置
private static final int NORMAL       = 2; // 正常完成
private static final int EXCEPTIONAL  = 3; // 异常
private static final int CANCELLED    = 4; // 取消
private static final int INTERRUPTING = 5; // 正在中断
private static final int INTERRUPTED  = 6; // 已中断

public void run() {
    if (state != NEW || !UNSAFE.compareAndSwapObject(this, runnerOffset, null, Thread.currentThread()))
        return;
    try {
        Callable<V> c = callable;
        if (c != null && state == NEW) {
            V result;
            boolean ran;
            try {
                result = c.call();
                ran = true;
            } catch (Throwable ex) {
                result = null;
                ran = false;
                setException(ex); // state → EXCEPTIONAL
            }
            if (ran)
                set(result); // state → COMPLETING → NORMAL
        }
    } finally {
        runner = null;
        int state = this.state;
        if (state >= INTERRUPTING)
            handlePossibleCancellationInterrupt(s);
    }
}

// get() 阻塞等待结果
public V get() throws InterruptedException, ExecutionException {
    int s = state;
    if (s <= COMPLETING)
        s = awaitDone(false, 0L); // 挂起当前线程
    return report(s);
}

private int awaitDone(boolean timed, long nanos) throws InterruptedException {
    // 通过 CAS 构建 Treiber 栈（等待线程链表）
    WaitNode q = new WaitNode();
    q.thread = Thread.currentThread();
    // ...
    for (;;) {
        if (signalled) {
            q.thread = null;
            return s;
        }
        if (s > COMPLETING) // 已完成
            return s;
        // LockSupport.park 挂起线程
        LockSupport.park(this);
    }
}
```

#### CompletableFuture 源码分析

`CompletableFuture` 是 Java 8 引入的异步编程利器，它实现了 `Future` 和 `CompletionStage` 接口。它的核心设计思想是**基于事件驱动的 DAG（有向无环图）任务依赖模型**，不需要显式地阻塞等待结果。相比传统的 `Future.get()` 阻塞等待，CompletableFuture 通过回调机制实现了非阻塞的异步编排。

**核心数据结构**只有两个字段：

```java
volatile Object result;       // 存放最终结果或 AltResult（异常包装）
volatile Completion stack;    // Treiber 栈，存放所有依赖操作
```

`result` 的设计非常巧妙：正常结果直接存放，异常结果包装为 `AltResult` 对象。`stack` 是一个无锁的 Treiber 栈（单向链表），所有通过 `thenApply`、`thenCompose`、`whenComplete` 等方法注册的后续操作都会被压入这个栈中。

**Completion 节点**是依赖操作的基本单元：

```java
abstract static class Completion extends ForkJoinTask<Void>
    implements Runnable, AsynchronousCompletionTask {
    volatile Completion next;
    abstract CompletableFuture<?> tryFire(int mode);
}

// 具体实现子类
static final class UniApply<T,V> extends Completion {
    Executor executor;             // 异步执行的线程池
    CompletableFuture<V> dep;      // 后续 CompletableFuture
    Function<? super T,? extends V> fn; // 转换函数
    
    final CompletableFuture<V> tryFire(int mode) {
        // 检查前置 stage 是否已完成
        // 如果完成则执行 fn.apply() 并设置结果到 dep
        // 根据 mode（SYNC/ASYNC/NESTED）决定执行方式
    }
}
```

**零异步线程设计**：这是 CompletableFuture 最精妙的设计点。当你调用 `complete()` 方法设置结果时，当前线程会直接执行所有后续的 `Completion.tryFire()` 方法，而不是启动一个新线程。这种方式避免了不必要的线程切换开销。

```java
public boolean complete(T value) {
    boolean triggered = completeValue(value);
    postComplete();
    return triggered;
}

final void postComplete() {
    CompletableFuture<?> f = this;
    Completion h;
    while ((h = f.stack) != null) {
        if (f.casStack(h, h.next)) {
            if ((d = h.tryFire(NESTED)) == null) {
                postFire();
            }
        }
    }
}
```

`postComplete()` 使用 CAS 遍历 Treiber 栈，依次触发所有依赖的 Completion。`tryFire(NESTED)` 返回 `null` 表示执行完成，如果返回非 null 的 CompletableFuture，则表示链式调用需要继续执行。这种设计保证了在单线程内完成整个依赖链的执行，无需额外线程参与。

**常用方法源码流程**：

```java
public static <U> CompletableFuture<U> supplyAsync(Supplier<U> supplier) {
    return asyncSupplyStage(asyncPool, supplier);
}

public <U> CompletableFuture<U> thenApply(Function<? super T,? extends U> fn) {
    return uniApplyStage(null, fn);  // executor 为 null 时同步执行
}
```

`thenApply` 内部创建了一个新的 `CompletableFuture`（`dep`），并创建一个 `UniApply` 节点将其与前置 stage 关联。当前置 stage 完成时，会遍历其 Completion 栈并执行 `UniApply.tryFire()`——检查前置 stage 的 `result` 是否可用，如果可用则执行 `fn.apply()`，设置结果到 `dep`，并继续触发 `dep` 的依赖。整个链条就像多米诺骨牌一样依次倒下，而推动第一张骨牌的就是 `complete()` 调用或异步任务的完成。

---

## 2. Java AOP 详解

### 2.1 代理模式

代理模式是 AOP 的底层基础设施。在 Java 中，代理分为静态代理和动态代理两大类，而动态代理又分为 JDK 动态代理和 CGLIB 动态代理两种实现方式。

#### 静态代理

静态代理在编译期就确定了代理关系。代理类和目标类实现了相同的接口，代理类持有目标类的引用，在调用目标方法前后插入横切逻辑。这种方式的缺点是每代理一个接口就需要编写一个独立的代理类，当目标类数量增多时代码膨胀严重，维护成本高。

```java
interface Service {
    void execute();
}

class RealService implements Service {
    public void execute() { System.out.println("执行业务逻辑"); }
}

class StaticProxy implements Service {
    private Service target;
    public StaticProxy(Service target) { this.target = target; }
    public void execute() {
        System.out.println("前置增强：日志记录");
        target.execute();
        System.out.println("后置增强：性能统计");
    }
}
```

#### JDK 动态代理

JDK 动态代理在运行时动态生成实现了指定接口的代理类字节码。它通过 `InvocationHandler` 接口将目标类的所有方法调用集中到一个处理器中，实现了横切逻辑的集中管理和复用。但 JDK 动态代理**只能代理接口**，无法直接代理类。

#### CGLIB 动态代理

CGLIB（Code Generation Library）通过 ASM 字节码框架在运行时生成目标类的子类作为代理，因此可以代理普通类而无需接口。它通过覆盖目标类的非 final 方法来实现增强。CGLIB 内部采用 FastClass 机制来提升调用性能，避免了 JDK 反射调用的开销。

### 2.2 JDK Proxy 源码分析

`Proxy.newProxyInstance()` 是 JDK 动态代理的入口方法，整个流程分为三步：获取代理类的 Class 对象、通过反射获取构造函数、创建代理实例。

```java
@CallerSensitive
public static Object newProxyInstance(ClassLoader loader, Class<?>[] interfaces,
                                      InvocationHandler h) {
    Objects.requireNonNull(h);
    final Class<?> caller = System.getSecurityManager() == null ? null : Reflection.getCallerClass();
    Class<?> proxyClass = getProxyClass(caller, loader, interfaces);
    final Constructor<?> cons = proxyClass.getConstructor(constructorParams);
    return cons.newInstance(new Object[]{h});
}
```

**核心机制：`getProxyClass()` → `ProxyClassFactory`**

`ProxyClassFactory` 是 `Proxy` 类内部的静态工厂，它负责动态生成代理类的字节码并加载到 JVM 中：

```java
private static final class ProxyClassFactory implements BiFunction<ClassLoader, Class<?>[], Class<?>> {
    private static final String proxyClassNamePrefix = "$Proxy";
    private static final AtomicLong nextUniqueNumber = new AtomicLong();

    @Override
    public Class<?> apply(ClassLoader loader, Class<?>[] interfaces) {
        long num = nextUniqueNumber.getAndIncrement();
        String proxyName = proxyPkg + "." + proxyClassNamePrefix + num;
        byte[] proxyClassFile = ProxyGenerator.generateProxyClass(proxyName, interfaces, accessFlags);
        return defineClass0(loader, proxyName, proxyClassFile, 0, proxyClassFile.length);
    }
}
```

`ProxyGenerator.generateProxyClass()` 是生成字节码的关键方法，它使用字节码技术（在 JDK 内部实现，OpenJDK 中可以看到其源码）生成一个继承自 `Proxy` 类并实现了所有指定接口的 Java 类。生成的类结构如下所示：

```java
// 反编译生成的 $Proxy0 类
public final class $Proxy0 extends Proxy implements Service {
    private static Method m1; // equals
    private static Method m2; // toString
    private static Method m3; // execute
    private static Method m0; // hashCode

    static {
        try {
            m3 = Class.forName("Service").getMethod("execute", new Class[0]);
        } catch (NoSuchMethodException e) {
            throw new NoSuchMethodError(e.getMessage());
        }
    }

    public $Proxy0(InvocationHandler h) { super(h); }

    @Override
    public void execute() {
        try {
            super.h.invoke(this, m3, (Object[]) null);
        } catch (RuntimeException | Error e) { throw e;
        } catch (Throwable t) { throw new UndeclaredThrowableException(t); }
    }
}
```

**JDK 动态代理的限制**：代理类已经继承了 `Proxy` 类，由于 Java 的单继承限制，它不能再继承其他类。因此 JDK 动态代理只能代理接口，不能直接代理类。所有的方法调用都会被转发到 `InvocationHandler.invoke()` 方法，由开发者决定如何处理。

### 2.3 CGLIB 原理

CGLIB（Code Generation Library）使用 ASM 字节码框架在运行时动态生成目标类的子类，并通过覆盖非 final 方法来实现代理增强。与 JDK 动态代理不同，CGLIB 不需要目标类实现接口，因此可以代理任何普通类。

**Enhancer 核心类**是 CGLIB 的入口：

```java
public class Enhancer {
    private Class superclass;
    private Callback[] callbacks;
    private CallbackFilter callbackFilter;

    public Object create() {
        return createHelper();
    }

    private Object createHelper() {
        Object key = KEY_FACTORY.newInstance(superclass, interfaces, callbacks.length, ...);
        Class genClass = CLASS_CACHE.get(key);
        if (genClass == null) {
            genClass = superclass.isInterface() ? createInterface(key) : createClass(key);
        }
        return firstInstance(genClass);
    }
}
```

`Enhancer` 使用了类缓存机制：相同的参数组合会生成相同的类 key，如果缓存中已有对应的代理类则直接复用，避免重复生成字节码。

**FastClass 机制**——这是 CGLIB 性能优于 JDK 反射调用的关键所在。CGLIB 为每个被代理类和代理类各生成一个 FastClass。FastClass 通过方法签名计算出一个整型索引，并使用 switch 语句直接跳转到目标方法的调用点，完全避免了反射调用：

```java
class TargetFastClass {
    public Object invoke(int index, Object obj, Object[] args) {
        Target t = (Target) obj;
        switch(index) {
            case 0: t.execute(); return null;
            case 1: return t.process((String)args[0]);
        }
        return null;
    }

    public int getIndex(String signature) {
        switch(signature) {
            case "execute()V": return 0;
            case "process(Ljava/lang/String;)Ljava/lang/String;": return 1;
        }
        return -1;
    }
}
```

**MethodProxy** 封装了 FastClass 的索引信息，允许在方法拦截器中直接调用父类方法（即目标类的原始方法），而无需经过拦截器链：

```java
class MethodProxy {
    private FastClass fastClass;
    private FastClass superFastClass;
    private int f1;
    private int f2;

    public Object invokeSuper(Object obj, Object[] args) {
        return fastClass.invoke(f2, obj, args);
    }
}
```

**CGlib 的调用链**：当代理对象的方法被调用时，CGLIB 生成的子类会先调用 `MethodInterceptor.intercept()`，在拦截器中可以通过 `MethodProxy.invokeSuper()` 直接调用父类的原始方法。这种 `子类 → 拦截器 → 父类` 的三层结构使得 CGLIB 能够在方法执行前后灵活地插入横切逻辑。

### 2.4 Spring AOP 源码流程

#### @EnableAspectJAutoProxy

```java
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Import(AspectJAutoProxyRegistrar.class) // 关键：导入注册器
public @interface EnableAspectJAutoProxy {
    boolean proxyTargetClass() default false;
    boolean exposeProxy() default false;
}

class AspectJAutoProxyRegistrar implements ImportBeanDefinitionRegistrar {
    @Override
    public void registerBeanDefinitions(AnnotationMetadata metadata, BeanDefinitionRegistry registry) {
        // 注册 AnnotationAwareAspectJAutoProxyCreator
        AopConfigUtils.registerAspectJAnnotationAutoProxyCreatorIfNecessary(registry);
        // 设置 proxyTargetClass 和 exposeProxy 属性
    }
}
```

#### AnnotationAwareAspectJAutoProxyCreator 继承链

```
AnnotationAwareAspectJAutoProxyCreator
  → AspectJAwareAdvisorAutoProxyCreator
    → AbstractAdvisorAutoProxyCreator
      → AbstractAutoProxyCreator
        implements SmartInstantiationAwareBeanPostProcessor, BeanFactoryAware
```

核心是 `BeanPostProcessor`，在 bean 初始化后执行 `postProcessAfterInitialization`。

#### wrapIfNecessary

```java
// AbstractAutoProxyCreator
@Override
public Object postProcessAfterInitialization(Object bean, String beanName) {
    if (bean instanceof AopInfrastructureBean) return bean;
    if (!beanTypeInCache.containsKey(beanName)) return bean;
    
    // 核心：如果需要切面则包装
    return wrapIfNecessary(bean, beanName, cacheKey);
}

protected Object wrapIfNecessary(Object bean, String beanName, Object cacheKey) {
    // 1. 获取该 bean 匹配的 Advice/Advisor
    Object[] specificInterceptors = getAdvicesAndAdvisorsForBean(bean.getClass(), beanName, null);
    if (specificInterceptors != DO_NOT_PROXY) {
        this.advisedBeans.put(cacheKey, Boolean.TRUE);
        // 2. 创建代理
        Object proxy = createProxy(bean.getClass(), beanName, specificInterceptors, new SingletonTargetSource(bean));
        this.proxyCache.put(cacheKey, proxy);
        return proxy;
    }
    this.advisedBeans.put(cacheKey, Boolean.FALSE);
    return bean;
}
```

#### createProxy → 创建代理

```java
// AbstractAutoProxyCreator
protected Object createProxy(Class<?> beanClass, String beanName,
                             Object[] specificInterceptors, TargetSource targetSource) {
    ProxyFactory proxyFactory = new ProxyFactory();
    proxyFactory.copyFrom(this);
    
    // 添加 Advisor
    Advisor[] advisors = buildAdvisors(beanName, specificInterceptors);
    proxyFactory.addAdvisors(advisors);
    proxyFactory.setTargetSource(targetSource);
    // 配置代理接口
    proxyFactory.setInterfaces(ClassUtils.getAllInterfacesForClass(beanClass));
    
    // 决定使用 JDK Proxy 还是 CGLIB
    return proxyFactory.getProxy(getProxyClassLoader());
}

// ProxyFactory.getProxy()
public Object getProxy(ClassLoader classLoader) {
    // AopProxy 有两种实现：JdkDynamicAopProxy / CglibAopProxy
    return createAopProxy().getProxy(classLoader);
}

// DefaultAopProxyFactory
public AopProxy createAopProxy(AdvisedSupport config) {
    if (config.isOptimize() || config.isProxyTargetClass() || 
        hasNoUserSuppliedProxyInterfaces(config)) {
        Class<?> targetClass = config.getTargetClass();
        if (targetClass.isInterface() || Proxy.isProxyClass(targetClass))
            return new JdkDynamicAopProxy(config);
        return new ObjenesisCglibAopProxy(config);
    } else {
        return new JdkDynamicAopProxy(config);
    }
}
```

**JDK 代理**：`JdkDynamicAopProxy` 实现 `InvocationHandler`：

```java
final class JdkDynamicAopProxy implements AopProxy, InvocationHandler, Serializable {
    private final AdvisedSupport advised;

    @Override
    public Object getProxy(ClassLoader classLoader) {
        // 调用 Proxy.newProxyInstance
        return Proxy.newProxyInstance(classLoader, proxiedInterfaces, this);
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
        // 获取拦截器链
        List<Object> chain = this.advised.getInterceptorsAndDynamicInterceptionAdvice(method, targetClass);
        if (chain.isEmpty()) {
            // 无切面直接反射调用
            return method.invoke(target, args);
        } else {
            // 创建 MethodInvocation 并执行
            invocation = new ReflectiveMethodInvocation(proxy, target, method, args, targetClass, chain);
            return invocation.proceed();
        }
    }
}
```

### 2.5 切面执行链

#### ReflectiveMethodInvocation.proceed()

```java
public class ReflectiveMethodInvocation implements ProxyMethodInvocation {
    protected final Object proxy;
    protected final Object target;
    protected final Method method;
    protected final Object[] arguments;
    private final Class<?> targetClass;
    private List<Object> interceptorsAndDynamicMethodMatchers;
    private int currentInterceptorIndex = -1; // 当前执行位置

    @Override
    public Object proceed() throws Throwable {
        // 遍历完拦截器 → 执行目标方法
        if (this.currentInterceptorIndex == this.interceptorsAndDynamicMethodMatchers.size() - 1) {
            return invokeJoinpoint(); // 反射调用目标方法
        }
        
        Object interceptorOrInterceptionAdvice =
            this.interceptorsAndDynamicMethodMatchers.get(++this.currentInterceptorIndex);
        
        // 动态方法匹配器（较少见）
        if (interceptorOrInterceptionAdvice instanceof InterceptorAndDynamicMethodMatcher) {
            // 匹配成功才执行
            return ((InterceptorAndDynamicMethodMatcher) interceptorOrInterceptionAdvice)
                .interceptor.invoke(this);
        }
        
        // 普通拦截器：直接调用
        return ((MethodInterceptor) interceptorOrInterceptionAdvice).invoke(this);
    }
}
```

**执行链示意图：**

```
proceed()
  → [0] MethodBeforeAdviceInterceptor.invoke()
        → 前置通知
        → methodInvocation.proceed()
  → [1] AfterReturningAdviceInterceptor.invoke()
        → methodInvocation.proceed()       ← 实际执行在内部
        → 返回后通知
  → [2] MethodInterceptor.invoke()        ← Around 通知环绕
        → methodInvocation.proceed()
  → [3] invokeJoinpoint()                 ← 最终目标方法
```

**环绕通知（Around Advice）** 的 `proceed()` 调用流程体现了**责任链模式**，每个拦截器决定是否执行下一个。

---

## 3. 锁与条件变量

### 3.1 synchronized 原理

synchronized 是 Java 最基础的同步机制，在 JDK 6 之后经历了重大的性能优化。其底层实现涉及 JVM 的对象头、Mark Word 和锁升级机制。

#### 对象头与 Mark Word

Java 中每个对象都可以作为锁，这个能力来自 JVM 对象头中的 **Mark Word**。Java 对象在内存中由三部分组成：对象头（Header）、实例数据（Instance Data）和对齐填充（Padding）。

对于 64 位 JVM（默认启用指针压缩）：

```
+--------------------------------------------------------------+
| 对象头 (12-16 bytes)                                          |
|   - Mark Word (8 bytes)                                       |
|   - Klass Pointer (4 bytes 压缩后 / 8 bytes 未压缩)           |
|   - [数组长度] (4 bytes, 仅数组对象)                          |
+--------------------------------------------------------------+
| 实例数据                                                      |
+--------------------------------------------------------------+
| 对齐填充 (8字节对齐)                                          |
+--------------------------------------------------------------+
```

**Mark Word 的位级布局**（64 位 JVM）——同一个 64 位空间中，根据锁状态不同存放着完全不同的数据：

| 状态 | 标志位 | 内容结构 |
|------|--------|---------|
| 无锁 | 01 | unused(25) + identity_hashcode(31) + unused(1) + age(4) + biased_lock=0 + tag=01 |
| 偏向锁 | 01 | thread(54) + epoch(2) + age(4) + biased_lock=1 + tag=01 |
| 轻量级锁 | 00 | ptr_to_lock_record(62) |
| 重量级锁 | 10 | ptr_to_heavy_monitor(62) |
| GC 标记 | 11 | 空（GC 时使用） |

**深入理解 Mark Word**：Mark Word 的设计体现了 JVM 对空间效率的极致追求。同一个 64 位的空间在无锁状态下存放 identity hashcode 和 GC 分代年龄（用于判断对象是否需要晋升到老年代）；偏向锁状态下存放线程 ID 和偏向纪元（epoch）；轻量级锁状态下存放栈中 Lock Record 的指针；重量级锁状态下存放 ObjectMonitor 的指针。

#### 锁升级过程

JVM 对 synchronized 进行了多级优化，锁可以"升级"但不可"降级"（GC 除外）：

```
无锁 ──▶ 偏向锁 ──▶ 轻量级锁 ──▶ 重量级锁（不可逆）
```

**偏向锁（Biased Locking）**：偏向锁的核心思想是——大多数锁不存在竞争，同一个线程往往会反复获取同一个锁。当线程第一次获取锁时，JVM 通过 CAS 将当前线程 ID 写入 Mark Word 的偏向锁字段。此后，同一线程再次获取这个锁时，只需检查 Mark Word 中的线程 ID 是否与自己匹配，不需要任何 CAS 操作或内核态切换，性能极高。当另一个线程尝试获取偏向锁时，偏向锁会进入**撤销（revocation）**流程，在全局安全点（safepoint）检查原持有线程的状态，如果原线程已结束则重新偏向，否则升级为轻量级锁。由于偏向锁的撤销成本较高，JDK 15 起默认禁用，JDK 21 中已完全移除。

**轻量级锁（Lightweight Locking）**：当第二个线程竞争锁时，偏向锁撤销并升级为轻量级锁。加锁过程如下：
1. 在当前线程的栈帧中分配 Lock Record 空间
2. 将对象头的 Mark Word 复制到 Lock Record 的 displaced header 中
3. 通过 CAS 尝试将对象头的 Mark Word 替换为指向 Lock Record 的指针
4. 如果 CAS 成功，当前线程获得锁，Mark Word 的标志位变为 00
5. 如果 CAS 失败，说明存在真正竞争，轻量级锁会自旋一定次数尝试获取锁，自旋仍失败则膨胀为重量级锁

轻量级锁解锁时，CAS 将 displaced header 替换回对象头。如果替换成功，解锁完成；如果替换失败，说明锁已膨胀为重量级锁，需进入重量级锁的解锁流程。

**重量级锁（Heavyweight Locking）**：重量级锁通过操作系统层面的互斥量实现，涉及到用户态到内核态的切换，开销最大。其核心数据结构是 `ObjectMonitor`：

```cpp
class ObjectMonitor {
    volatile int   _count;          // 竞争数量
    volatile int   _waiters;        // wait 线程数
    volatile int   _recursions;     // 重入次数
    ObjectWaiter*  _EntryList;      // 等待锁的线程队列
    ObjectWaiter*  _WaitSet;        // 调用 wait() 的线程队列
    ParkEvent*     _ParkEvent;      // 用于 park 当前线程
    void*          _object;         // 关联的对象
    void*          _owner;          // 当前持有锁的线程
};
```

重量级锁加锁流程：
1. 尝试 CAS 设置 `_owner` 为当前线程 → 如果成功则获得锁
2. 失败 → 自旋尝试（自适应自旋，根据系统负载和之前的自旋结果动态调整）
3. 自旋失败 → 将当前线程封装为 `ObjectWaiter` 对象，加入 `_EntryList` 队列
4. 调用 `park()` 阻塞线程 → 线程挂起，进入内核态等待
5. 解锁时从 `_EntryList` 中唤醒等待线程 → 被唤醒的线程再次竞争锁

### 3.2 AQS 源码完整分析

`AbstractQueuedSynchronizer` 是 `java.util.concurrent` 包的基石。`ReentrantLock`、`CountDownLatch`、`Semaphore`、`ReentrantReadWriteLock` 等几乎所有 JUC 同步器都基于 AQS 实现。理解 AQS 是深入理解 Java 并发的必经之路。

#### 顶层设计架构

AQS 的核心设计包含三个要素：
1. **volatile int state**：同步状态，由子类通过 `getState()`、`setState()` 和 `compareAndSetState()` 操作
2. **CLH 队列**：一个 FIFO 的双向链表，管理所有等待获取锁的线程
3. **CAS 操作**：使用 Unsafe 提供的 CAS 原语保证所有并发操作的原子性

```java
public abstract class AbstractQueuedSynchronizer extends AbstractOwnableSynchronizer {
    private volatile int state;
    private transient volatile Node head;
    private transient volatile Node tail;

    private static final Unsafe unsafe = Unsafe.getUnsafe();
    private static final long stateOffset;
    private static final long headOffset;
    private static final long tailOffset;
}
```

**Node 节点**是 CLH 队列的基本组成单元。它封装了等待线程、等待状态以及前驱后继指针：

```java
static final class Node {
    volatile int waitStatus;
    volatile Node prev;
    volatile Node next;
    volatile Thread thread;

    // waitStatus 常量及其含义
    static final int CANCELLED = 1;   // 线程因超时或中断被取消
    static final int SIGNAL = -1;     // 当前节点释放锁后需要唤醒后继
    static final int CONDITION = -2;  // 节点在条件队列中等待
    static final int PROPAGATE = -3;  // 共享模式下释放需要传播
    static final int INITIAL = 0;     // 初始状态

    static final Node SHARED = new Node();
    static final Node EXCLUSIVE = null;
}
```

CLH 队列的**头节点是一个虚节点或哨兵节点**，它不关联任何线程。队列中的第一个真正等待线程是 `head.next`。这种设计的优点是简化了队列操作——入队只需 CAS 更新 tail，出队只需移动 head。

#### acquire() 独占获取的完整路径

`acquire()` 方法是 AQS 独占模式获取同步状态的入口。它的代码极其简洁，但内涵丰富：

```java
public final void acquire(int arg) {
    if (!tryAcquire(arg) &&
        acquireQueued(addWaiter(Node.EXCLUSIVE), arg))
        selfInterrupt();
}
```

这个方法只有两行代码，但包含三个核心步骤：尝试获取、加入队列、自旋等待。

**第一步 `addWaiter()`**——将当前线程包装为 Node 并加入 CLH 队列尾部。这里使用了无锁入队算法（Michael-Scott 队列算法），通过 CAS 保证并发入队的线程安全：

```java
private Node addWaiter(Node mode) {
    Node node = new Node(mode);
    for (;;) {
        Node oldTail = tail;
        if (oldTail != null) {
            node.setPrevRelaxed(oldTail);
            if (compareAndSetTail(oldTail, node)) {
                oldTail.next = node;
                return node;
            }
        } else {
            initializeSyncQueue();
        }
    }
}
```

注意 `setPrevRelaxed(oldTail)` 使用的是 `setPrevRelaxed` 而不是普通的 `prev = oldTail`——这是利用了 VarHandle 的宽松 put 语义，因为这里不需要 volatile 语义，后续的 `compareAndSetTail` 会通过 CAS 的 memory barrier 保证可见性。

**第二步 `acquireQueued()`**——在队列中循环自旋，直到获取锁：

```java
final boolean acquireQueued(final Node node, int arg) {
    boolean interrupted = false;
    try {
        for (;;) {
            final Node p = node.predecessor();
            if (p == head && tryAcquire(arg)) {
                setHead(node);
                p.next = null;
                return interrupted;
            }
            if (shouldParkAfterFailedAcquire(p, node))
                interrupted |= parkAndCheckInterrupt();
        }
    } catch (Throwable t) {
        cancelAcquire(node);
        throw t;
    }
}
```

核心逻辑是：每次循环检查当前节点的前驱是否是 head，如果是则尝试获取锁。这种机制保证了 AQS 的 FIFO 特性——只有队首的等待者才能竞争锁，先来后到的顺序得到保证。如果前驱不是 head，或获取失败，则调用 `shouldParkAfterFailedAcquire` 检查是否需要阻塞。

**第三步 `shouldParkAfterFailedAcquire()`**——根据前驱节点状态决定当前线程是否可以安全地阻塞：

```java
private static boolean shouldParkAfterFailedAcquire(Node pred, Node node) {
    int ws = pred.waitStatus;
    if (ws == Node.SIGNAL)
        return true;
    if (ws > 0) {
        do {
            node.prev = pred = pred.prev;
        } while (pred.waitStatus > 0);
        pred.next = node;
    } else {
        pred.compareAndSetWaitStatus(ws, Node.SIGNAL);
    }
    return false;
}
```

这个方法维护 CLH 队列的合法性：如果前驱被取消（`CANCELLED = 1`），则跳过所有已取消的前驱，直到找到未取消的节点作为新的前驱；如果前驱状态为 0 或 PROPAGATE，则 CAS 将其设为 SIGNAL，表示前驱释放锁时应该唤醒当前节点；如果前驱已经是 SIGNAL，则当前线程可以安全地 park 阻塞。

**`cancelAcquire()`**：如果 `acquireQueued` 中抛出异常（例如中断导致的异常），会调用 `cancelAcquire` 将当前节点的 waitStatus 设为 CANCELLED，并从队列中移除。CANCELLED 的节点在后续的 `shouldParkAfterFailedAcquire` 和 `unparkSuccessor` 中会被跳过。

#### release 独占释放

```java
public final boolean release(int arg) {
    if (tryRelease(arg)) {         // 子类实现
        Node h = head;
        if (h != null && h.waitStatus != 0)
            unparkSuccessor(h);    // 唤醒后继
        return true;
    }
    return false;
}

private void unparkSuccessor(Node node) {
    int ws = node.waitStatus;
    if (ws < 0)
        node.compareAndSetWaitStatus(ws, 0);
    Node s = node.next;
    if (s == null || s.waitStatus > 0) { // 后继为空或取消 → 从 tail 向前找
        s = null;
        for (Node p = tail; p != node && p != null; p = p.prev)
            if (p.waitStatus <= 0)
                s = p;
    }
    if (s != null)
        LockSupport.unpark(s.thread);
}
```

### 3.3 ReentrantLock 源码

`ReentrantLock` 是 AQS 最典型的应用。它使用 AQS 的独占模式实现了可重入锁，并提供了公平/非公平两种获取策略。

#### 公平锁与非公平锁的设计哲学

`ReentrantLock` 内部有一个 `Sync` 抽象类继承 AQS，然后有 `NonfairSync` 和 `FairSync` 两个具体实现：

```java
public class ReentrantLock implements Lock, Serializable {
    private final Sync sync;

    public ReentrantLock() { sync = new NonfairSync(); }
    public ReentrantLock(boolean fair) { sync = fair ? new FairSync() : new NonfairSync(); }
}
```

**非公平锁（NonfairSync）的核心代码**：

```java
static final class NonfairSync extends Sync {
    final void lock() {
        if (compareAndSetState(0, 1))
            setExclusiveOwnerThread(Thread.currentThread());
        else
            acquire(1);
    }

    protected final boolean tryAcquire(int acquires) {
        return nonfairTryAcquire(acquires);
    }
}

final boolean nonfairTryAcquire(int acquires) {
    final Thread current = Thread.currentThread();
    int c = getState();
    if (c == 0) {
        if (compareAndSetState(0, acquires)) {
            setExclusiveOwnerThread(current);
            return true;
        }
    } else if (current == getExclusiveOwnerThread()) {
        int nextc = c + acquires;
        if (nextc < 0) throw new Error("Maximum lock count exceeded");
        setState(nextc);
        return true;
    }
    return false;
}
```

**非公平锁的核心特征**是"插队"行为：`lock()` 方法第一行就尝试 CAS 抢锁，不管 CLH 队列中是否有线程在等待。这种插队可能导致队列中长时间等待的线程一直得不到锁（线程饥饿），但换来的是更高的吞吐量——因为避免了线程的 park/unpark 开销。

**公平锁（FairSync）的核心代码**：

```java
static final class FairSync extends Sync {
    final void lock() {
        acquire(1);
    }

    protected final boolean tryAcquire(int acquires) {
        final Thread current = Thread.currentThread();
        int c = getState();
        if (c == 0) {
            if (!hasQueuedPredecessors() && compareAndSetState(0, acquires)) {
                setExclusiveOwnerThread(current);
                return true;
            }
        } else if (current == getExclusiveOwnerThread()) {
            int nextc = c + acquires;
            if (nextc < 0) throw new Error("Maximum lock count exceeded");
            setState(nextc);
            return true;
        }
        return false;
    }
}
```

公平锁没有直接 CAS 抢锁的步骤，而是直接调用 `acquire(1)` 进入 AQS 的标准获取流程。`tryAcquire` 中通过 `hasQueuedPredecessors()` 检查 CLH 队列中是否有线程比当前线程等待更久——如果有，即使锁是空闲状态，当前线程也会获取失败，加入队列尾部等待。这保证了严格的 FIFO 顺序，避免了线程饥饿。

**`hasQueuedPredecessors()` 的实现**：
```java
public final boolean hasQueuedPredecessors() {
    Node h = head;
    Node t = tail;
    Node s;
    return h != t && ((s = h.next) == null || s.thread != Thread.currentThread());
}
```

这个方法的逻辑值得仔细推敲：`h != t` 表示队列非空（头尾不是同一个节点）。`h.next == null` 表示头节点的后继尚未设置（`addWaiter` 中先设置 prev 再设置 next，这种中间状态需要处理——如果当前线程恰好是在这个间隙中调用了 `hasQueuedPredecessors`，那么返回 true 是安全的，让当前线程去排队）。`s.thread != Thread.currentThread()` 表示头节点的后继不是当前线程，即队列中还有更早的等待者。

**unlock 逻辑**在两个实现中完全相同：

```java
public void unlock() {
    sync.release(1);
}

protected final boolean tryRelease(int releases) {
    int c = getState() - releases;
    if (Thread.currentThread() != getExclusiveOwnerThread())
        throw new IllegalMonitorStateException();
    boolean free = false;
    if (c == 0) {
        free = true;
        setExclusiveOwnerThread(null);
    }
    setState(c);
    return free;
}
```

`tryRelease` 体现了可重入锁的释放逻辑：只有 `state` 完全归零时才真正释放锁（`free = true`），然后通过 AQS 的 `release()` 唤醒后继节点。如果 `state` 不为零（说明有重入），则只是减少重入计数，锁仍然被当前线程持有。

### 3.4 ReentrantReadWriteLock 原理

`ReentrantReadWriteLock` 维护一对锁：读锁（共享锁）和写锁（独占锁）。它的内部 `Sync` 类巧妙地用一个 `int` 类型的 `state` 同时管理读锁和写锁的计数——高 16 位存读锁数量，低 16 位存写锁重入次数：

```java
public class ReentrantReadWriteLock implements ReadWriteLock, Serializable {
    private final ReadLock readerLock;
    private final WriteLock writerLock;
    private final Sync sync;

    abstract static class Sync extends AbstractQueuedSynchronizer {
        static final int SHARED_SHIFT   = 16;
        static final int SHARED_UNIT    = (1 << SHARED_SHIFT);
        static final int MAX_COUNT      = (1 << SHARED_SHIFT) - 1;
        static final int EXCLUSIVE_MASK = (1 << SHARED_SHIFT) - 1;

        static int sharedCount(int c)    { return c >>> SHARED_SHIFT; }
        static int exclusiveCount(int c) { return c & EXCLUSIVE_MASK; }
    }
}
```

**写锁获取（独占）**：写锁只有当没有其他线程持有读锁或写锁时才能获取。写锁支持重入，重入次数记录在低 16 位中：

```java
protected final boolean tryAcquire(int acquires) {
    Thread current = Thread.currentThread();
    int c = getState();
    int w = exclusiveCount(c);
    if (c != 0) {
        if (w == 0 || current != getExclusiveOwnerThread())
            return false;
        if (w + exclusiveCount(acquires) > MAX_COUNT)
            throw new Error("Maximum lock count exceeded");
        setState(c + acquires);
        return true;
    }
    if (writerShouldBlock() || !compareAndSetState(c, c + acquires))
        return false;
    setExclusiveOwnerThread(current);
    return true;
}
```

**读锁获取（共享）**：在没有写锁（或写锁被当前线程持有）时，多个线程可以同时获取读锁。读锁支持通过 `ThreadLocal` 记录每个线程的重入次数：

```java
protected final int tryAcquireShared(int unused) {
    Thread current = Thread.currentThread();
    int c = getState();
    if (exclusiveCount(c) != 0 && getExclusiveOwnerThread() != current)
        return -1;
    int r = sharedCount(c);
    if (!readerShouldBlock() && r < MAX_COUNT && compareAndSetState(c, c + SHARED_UNIT)) {
        // ThreadLocal HoldCounter 记录每个线程的读锁重入
        return 1;
    }
    return fullTryAcquireShared(current);
}
```

**写锁降级（Lock Downgrade）**：一个持有写锁的线程可以再获取读锁，然后释放写锁，最终只持有读锁。这种模式在需要保证从写操作到读操作的数据可见性时非常有用。例如在缓存场景中：先更新缓存（写操作），然后获取读锁并释放写锁，这样其他线程后续获取读锁时就能看到最新的缓存数据。

### 3.5 Condition 源码分析

`Condition` 是 AQS 内部实现的条件变量。每个 `ConditionObject` 维护一个独立的**条件等待队列**（单向链表），与 AQS 的 CLH 同步队列相互独立但配合工作：

```java
public class ConditionObject implements Condition, java.io.Serializable {
    private transient Node firstWaiter;
    private transient Node lastWaiter;
}
```

**await() 源码逐步解析**——释放锁 → 加入条件队列 → 阻塞等待 → signal 后竞争锁：

```java
public final void await() throws InterruptedException {
    if (Thread.interrupted()) throw new InterruptedException();
    Node node = addConditionWaiter();            // 1. 加入条件队列
    int savedState = fullyRelease(node);         // 2. 完全释放持有锁
    int interruptMode = 0;
    while (!isOnSyncQueue(node)) {               // 3. 等待被 signal
        LockSupport.park(this);                  // 4. 阻塞
        if ((interruptMode = checkInterruptWhileWaiting(node)) != 0)
            break;
    }
    if (acquireQueued(node, savedState))         // 5. 竞争锁
        interruptMode = REINTERRUPT;
    if (node.nextWaiter != null)
        unlinkCancelledWaiters();
    if (interruptMode != 0)
        reportInterruptAfterWait(interruptMode);
}
```

**逐步骤深入分析**：
1. **`addConditionWaiter()`**：将当前线程封装为 Node（`waitStatus = CONDITION`），加入条件队列的尾部。如果最后一个等待者被取消，则从队列中清理所有已取消的节点。
2. **`fullyRelease(node)`**：完全释放当前线程持有的所有锁重入次数（因为 Condition.await 要求线程持有锁）。这里会调用 `getState()` 获取当前的重入计数，然后通过 `release(savedState)` 一次性释放所有。如果释放失败（说明当前线程不是锁的持有者），则抛出 `IllegalMonitorStateException`。
3. **`isOnSyncQueue(node)`**：检查节点是否被转移到 AQS 同步队列。节点在条件队列中时状态是 CONDITION，在同步队列中时状态是 0 或 SIGNAL。如果还在条件队列中，说明还没有被 signal，继续阻塞。
4. **`LockSupport.park(this)`**：阻塞当前线程。线程在这里挂起，直到被 signal 后的 `unpark` 唤醒。
5. **`acquireQueued(node, savedState)`**：被 signal 后，节点已转移到同步队列，这里通过 AQS 的标准 `acquireQueued` 流程重新竞争锁。

**signal() 源码分析**——将条件队列的头节点转移到同步队列：

```java
public final void signal() {
    if (!isHeldExclusively()) throw new IllegalMonitorStateException();
    Node first = firstWaiter;
    if (first != null)
        doSignal(first);
}

private void doSignal(Node first) {
    do {
        if ((firstWaiter = first.nextWaiter) == null)
            lastWaiter = null;
        first.nextWaiter = null;
    } while (!transferForSignal(first) && (first = firstWaiter) != null);
}

final boolean transferForSignal(Node node) {
    if (!node.compareAndSetWaitStatus(Node.CONDITION, 0))
        return false;
    Node p = enq(node);
    int ws = p.waitStatus;
    if (ws > 0 || !p.compareAndSetWaitStatus(ws, Node.SIGNAL))
        LockSupport.unpark(node.thread);
    return true;
}
```

**signal 的核心流程**：
1. 从条件队列头节点开始，通过 `doSignal` 循环找出第一个可以被转移到同步队列的节点（跳过已取消的节点）
2. `transferForSignal` 先 CAS 将节点的 `waitStatus` 从 CONDITION 改为 0，表示它不再在条件队列中
3. `enq(node)` 将节点加入 AQS 同步队列尾部（如果队列为空则先初始化一个哨兵头节点）
4. 检查前驱节点的状态：如果前驱已取消（ws > 0），或 CAS 设置前驱为 SIGNAL 失败，则直接 `unpark` 当前线程

**`signalAll()`** 的区别是循环 `doSignal(first)` 直到条件队列为空——将条件队列中的所有节点都转移到同步队列。

**整个 Condition 的协作流程总结**：

```
await():  当前线程持有锁 → 加入条件队列 → fullyRelease 释放锁 → park 阻塞
signal(): 条件队列头节点 → transferForSignal → 转移到同步队列 → 设置 SIGNAL → unpark
await() 恢复: acquireQueued 竞争锁 → 获取锁后继续执行
```

这种设计使得多个线程可以在同一个 Condition 上等待不同的条件满足，而锁资源可以在等待期间被其他线程利用，极大提高了并发效率。

### 3.6 LockSupport 底层实现

`LockSupport` 是基于 `Unsafe` 的线程阻塞原语，是 AQS、Condition 和 `FutureTask` 等所有并发工具的基础设施。理解了 LockSupport，就理解了 Java 并发的底层线程控制机制。

```java
public class LockSupport {
    private static final Unsafe U = Unsafe.getUnsafe();
    private static final long PARKBLOCKER_OFFSET;

    static {
        PARKBLOCKER_OFFSET = U.objectFieldOffset
            (Thread.class.getDeclaredField("parkBlocker"));
    }

    public static void park(Object blocker) {
        Thread t = Thread.currentThread();
        setBlocker(t, blocker);
        U.park(false, 0L);
        setBlocker(t, null);
    }

    public static void unpark(Thread thread) {
        if (thread != null)
            U.unpark(thread);
    }
}
```

**Unsafe.park/unpark 的底层实现**：在 HotSpot JVM 中，`Unsafe.park()` 通过 `Parker` 类实现。在 Linux 上使用 `pthread_cond_wait` + `pthread_mutex_lock` 或 `futex` 系统调用；在 macOS 上使用 `pthread_cond_wait`；在 Windows 上使用 `WaitForSingleObject` 或 `SleepConditionVariableSRW`。`Parker` 内部维护一个 `_counter` 计数器实现"许可证"语义。

**许可证（Permit）机制**：`unpark` 相当于给线程颁发了一个许可证（将 `_counter` 设为 1）。`park` 检查许可证：如果有许可证（`_counter > 0`），则消费许可证（`_counter = 0`）并立即返回；如果没有许可证，则阻塞线程直到有新的许可证。由于许可证最多只有一个，连续两次 `unpark` 不会累积两个许可证。

**`park()` 与 `Object.wait()` 的核心区别**：
- `park()` 不需要持有监视器锁（`synchronized`），使用更加灵活
- `park()` 不会抛出 `InterruptedException`，通过 `Thread.interrupted()` 检查中断状态
- `unpark()` 可以先于 `park()` 调用——这解决了复杂并发场景中"先发送通知后等待"的问题
- `park()` 支持"假唤醒"（spurious wakeup），因此需要在循环中检查等待条件（AQS 的 `acquireQueued` 就是典型的循环检查模式）

### 3.7 StampedLock 原理

`StampedLock` 是 JDK 8 引入的读写锁改进版本，通过乐观读机制进一步提升了读多写少场景的并发性能。状态变量 `state` 使用了一个 `long`（64 位）来同时管理写锁和读锁：

- 第 8 位（`WBIT`）：写锁标志，写锁被持有时该位为 1
- 低 7 位（`RBITS`）：读锁数量（最大 126），每次获取读锁 +1
- 高 24 位：读锁溢出计数（当读锁超过 126 时使用）

```java
public class StampedLock implements java.io.Serializable {
    private volatile long state;
    private static final long RUNIT = 1L;
    private static final long WBIT  = 1L << 7;
    private static final long RBITS = WBIT - 1;
    private static final long RFULL = RBITS - 1;
    private static final long ABITS = RBITS | WBIT;
    private static final long SBITS = ~RBITS;
    
    public long writeLock() {
        long s, next;
        return ((((s = state) & ABITS) == 0L &&
                 U.compareAndSwapLong(this, STATE, s, next = s + WBIT)) ?
                next : acquireWrite(false, false));
    }
    
    public long tryOptimisticRead() {
        long s;
        return (((s = state) & WBIT) == 0L) ? (s & SBITS) : 0L;
    }
    
    public boolean validate(long stamp) {
        U.loadFence();
        return (stamp & SBITS) == (state & SBITS);
    }
}
```

**乐观读（Optimistic Read）的核心思想**：读操作先不加锁地读取数据，然后通过 `validate()` 验证在读操作期间是否有写操作发生。如果没有写操作发生，则读取成功并返回；如果有写操作发生，则升级为常规读锁重新读取。这种机制在写操作极少的情况下，读操作完全不需要加锁，性能极高。

```java
// 标准使用模式
long stamp = lock.tryOptimisticRead();
int data = sharedData;
if (!lock.validate(stamp)) {
    stamp = lock.readLock();
    try {
        data = sharedData;
    } finally {
        lock.unlockRead(stamp);
    }
}
```

**StampedLock 的重要特性与局限**：
- **乐观读不加锁**：读操作完全不阻塞写操作，这是与 `ReentrantReadWriteLock` 最大的区别
- **不可重入**：同一线程不能重复获取已持有的锁，否则会造成死锁
- **不支持 Condition**：`StampedLock` 不提供 `Condition` 机制
- **写锁获取时的等待**：如果存在乐观读正在验证，写锁获取可能需要等待
- **内部策略**：`StampedLock` 大量使用自旋 + CAS 来避免线程挂起，只有在自旋次数超过阈值时才真正挂起线程。这种设计在锁竞争不激烈的场景下性能极高，但竞争激烈时可能导致 CPU 使用率升高

---

## 4. Spring 源码详解

### 4.1 IoC 容器：refresh() 方法 12 步源码分析

`AbstractApplicationContext.refresh()` 是 Spring IoC 容器的核心入口方法，它定义了整个 Spring 容器初始化的完整流程，共 12 步。从 BeanFactory 的创建到 Bean 的实例化，每个步骤都有明确的职责。下面逐步骤进行源码分析：

```java
@Override
public void refresh() throws BeansException, IllegalStateException {
    synchronized (this.startupShutdownMonitor) {
        // Step 1: 刷新前准备
        prepareRefresh();
        
        // Step 2: 获取 BeanFactory（通常为 DefaultListableBeanFactory）
        ConfigurableListableBeanFactory beanFactory = obtainFreshBeanFactory();
        
        // Step 3: BeanFactory 预准备（设置 ClassLoader、SpEL、属性编辑器等）
        prepareBeanFactory(beanFactory);
        
        try {
            // Step 4: BeanFactory 后置处理（子类扩展）
            postProcessBeanFactory(beanFactory);
            
            // Step 5: 调用 BeanFactoryPostProcessors
            invokeBeanFactoryPostProcessors(beanFactory);
            
            // Step 6: 注册 BeanPostProcessors
            registerBeanPostProcessors(beanFactory);
            
            // Step 7: 初始化 MessageSource（国际化）
            initMessageSource();
            
            // Step 8: 初始化事件广播器
            initApplicationEventMulticaster();
            
            // Step 9: 刷新特定子类的容器（如 Web 容器）
            onRefresh();
            
            // Step 10: 注册事件监听器
            registerListeners();
            
            // Step 11: 实例化所有非懒加载单例 Bean
            finishBeanFactoryInitialization(beanFactory);
            
            // Step 12: 完成刷新（发布事件、启动 Lifecycle）
            finishRefresh();
            
        } catch (BeansException ex) {
            destroyBeans();
            cancelRefresh(ex);
            throw ex;
        }
    }
}
```

**refresh() 方法的12步详细解析**：

**Step 1 — `prepareRefresh()`**：容器刷新的准备工作。设置容器的启动时间（`startupDate`）和激活标记（`active = true`），初始化属性源（PropertySource）和环境变量占位符解析器，还会检查 Environment 中的必填属性是否已设置。

**Step 2 — `obtainFreshBeanFactory()`**：获取全新的 BeanFactory。对于 `AbstractRefreshableApplicationContext`，这一步会销毁旧的 BeanFactory（如果存在），然后创建一个新的 `DefaultListableBeanFactory`，并通过 `loadBeanDefinitions()` 加载配置中所有的 BeanDefinition。BeanDefinition 是 Spring IoC 的信息载体，包含了 Bean 的类名、作用域、构造参数、属性值、初始化/销毁方法等全部定义信息。加载完成后，BeanFactory 中就包含了所有需要管理 Bean 的蓝图。

**Step 3 — `prepareBeanFactory(beanFactory)`**：配置 BeanFactory 的标准特性，包括设置 ClassLoader、添加 `ApplicationContextAwareProcessor`（处理各种 Aware 回调）、注册忽略自动注入的接口、注册 `BeanPostProcessor` 以及注册特殊的单例 Bean（如 `environment`、`systemProperties`、`systemEnvironment` 等）。这些准备确保 BeanFactory 具备运行所需的基本设施。

**Step 4 — `postProcessBeanFactory(beanFactory)`**：模板方法模式的子类扩展点。具体容器子类可以在此对 BeanFactory 进行自定义后处理。例如 `AnnotationConfigServletWebServerApplicationContext` 会在此注册 Web 相关的 scope（`session`、`request`、`application`）。

**Step 5 — `invokeBeanFactoryPostProcessors(beanFactory)`**：执行所有 `BeanFactoryPostProcessor`，对 BeanDefinition 进行修改（如属性占位符替换）。执行顺序严格遵循 PriorityOrdered → Ordered → 其他。`ConfigurationClassPostProcessor` 是最重要的处理器——它解析 `@Configuration`、`@ComponentScan`、`@Import`、`@Bean` 等注解，将配置类中定义的 Bean 注册到容器中。可以说没有它就等于没有基于注解的 Spring。

**Step 6 — `registerBeanPostProcessors(beanFactory)`**：注册所有 `BeanPostProcessor`，同样按优先级排序。这些处理器将在后续每个 Bean 初始化前后被调用。例如 `AutowiredAnnotationBeanPostProcessor` 处理 `@Autowired` 和 `@Value`，`CommonAnnotationBeanPostProcessor` 处理 `@PostConstruct` 和 `@PreDestroy`。

**Step 7 — `initMessageSource()`**：初始化国际化消息源 `MessageSource`，用于支持 i18n 功能。

**Step 8 — `initApplicationEventMulticaster()`**：初始化事件广播器，默认使用 `SimpleApplicationEventMulticaster`（同步调用监听器）。

**Step 9 — `onRefresh()`**：Spring Boot 创建内嵌 Web 服务器的关键步骤。`ServletWebServerApplicationContext` 的重写会在此创建并启动内嵌的 Tomcat、Jetty 或 Undertow。

**Step 10 — `registerListeners()`**：注册容器中的 `ApplicationListener` 监听器 Bean，并将之前积累的早期事件发送给匹配的监听器。

**Step 11 — `finishBeanFactoryInitialization(beanFactory)`**：实例化所有非懒加载的单例 Bean。这是最耗时的一步，遍历所有 BeanDefinition 并调用 `getBean()` 创建实例、注入属性、执行初始化。循环依赖在此通过三级缓存解决。

**Step 12 — `finishRefresh()`**：发布 `ContextRefreshedEvent` 事件通知监听器容器已就绪、启动 Lifecycle Bean、清除缓存。至此 Spring 容器完全启动。

**Step 2：`obtainFreshBeanFactory()` 详细流程：**

```java
// AbstractRefreshableApplicationContext
protected final void refreshBeanFactory() throws BeansException {
    if (hasBeanFactory()) {
        destroyBeans();
        closeBeanFactory();
    }
    DefaultListableBeanFactory beanFactory = createBeanFactory();
    beanFactory.setSerializationId(getId());
    customizeBeanFactory(beanFactory);
    loadBeanDefinitions(beanFactory); // 加载 BeanDefinition
    this.beanFactory = beanFactory;
}
```

**Step 5：`invokeBeanFactoryPostProcessors()` 执行顺序（极其重要）：**

```java
protected void invokeBeanFactoryPostProcessors(ConfigurableListableBeanFactory beanFactory) {
    PostProcessorRegistrationDelegate.invokeBeanFactoryPostProcessors(beanFactory, getBeanFactoryPostProcessors());
    // 执行优先级：
    // 1. BeanDefinitionRegistryPostProcessor → 实现 PriorityOrdered
    // 2. BeanDefinitionRegistryPostProcessor → 实现 Ordered
    // 3. 其他 BeanDefinitionRegistryPostProcessor
    // 4. BeanFactoryPostProcessor → PriorityOrdered
    // 5. BeanFactoryPostProcessor → Ordered
    // 6. 其他 BeanFactoryPostProcessor
}
```

### 4.2 依赖注入：getBean → doCreateBean → 循环依赖三级缓存

#### getBean 流程

```java
// AbstractBeanFactory
@Override
public Object getBean(String name) throws BeansException {
    return doGetBean(name, null, null, false);
}

@SuppressWarnings("unchecked")
protected <T> T doGetBean(String name, Class<T> requiredType, Object[] args, boolean typeCheckOnly) {
    String beanName = transformedBeanName(name);
    Object sharedInstance = getSingleton(beanName); // 先从缓存获取
    if (sharedInstance != null && args == null) {
        beanInstance = getObjectForBeanInstance(sharedInstance, name, beanName, null);
    } else {
        // 检查是否在创建中（循环依赖检测）
        if (isPrototypeCurrentlyInCreation(beanName)) throw new BeanCurrentlyInCreationException(beanName);
        
        // 获取父容器
        BeanFactory parentBeanFactory = getParentBeanFactory();
        if (parentBeanFactory != null && !containsBeanDefinition(beanName)) {
            return parentBeanFactory.getBean(nameToLookup);
        }
        
        if (!typeCheckOnly) markBeanAsCreated(beanName);
        
        try {
            RootBeanDefinition mbd = getMergedLocalBeanDefinition(beanName);
            // 创建依赖 bean（@DependsOn）
            String[] dependsOn = mbd.getDependsOn();
            if (dependsOn != null) {
                for (String dep : dependsOn) {
                    registerDependentBean(dep, beanName);
                    getBean(dep);
                }
            }
            
            // 创建单例
            if (mbd.isSingleton()) {
                sharedInstance = getSingleton(beanName, () -> createBean(beanName, mbd, args));
                beanInstance = getObjectForBeanInstance(sharedInstance, name, beanName, mbd);
            }
            // 创建原型
            else if (mbd.isPrototype()) {
                // ...
            }
        }
    }
    return beanInstance;
}
```

**`doGetBean` 执行流程解析**：`getBean` 方法是 Bean 生命周期的总入口。首先尝试从 `getSingleton(beanName)` 获取一级缓存中已创建好的 Bean——如果命中则直接返回（避免重复创建）。如果缓存未命中，`doGetBean` 开始完整的 Bean 创建流程：先检查是否是 prototype 类型的循环引用（抛异常），再检查父容器，然后合并 BeanDefinition（`getMergedLocalBeanDefinition`），接着创建 `@DependsOn` 指定的依赖 Bean 以确保依赖容器先被创建。最后，对于单例 Bean，调用 `getSingleton(beanName, () -> createBean(...))`——这个 `getSingleton` 方法会先尝试从一级缓存获取（双重检查），未命中则执行 Lambda 回调创建 Bean，创建完成后将 Bean 放入一级缓存。

#### createBean → doCreateBean

```java
// AbstractAutowireCapableBeanFactory
@Override
protected Object createBean(String beanName, RootBeanDefinition mbd, Object[] args) {
    // 解析 beanClass
    Class<?> resolvedClass = resolveBeanClass(mbd, beanName);
    // 准备方法覆盖（lookup-method / replace-method）
    mbd.prepareMethodOverrides();
    // 给 BeanPostProcessors 机会返回代理（如 @Configuration 的代理）
    Object bean = resolveBeforeInstantiation(beanName, mbd);
    if (bean != null) return bean; // 短路
    return doCreateBean(beanName, mbd, args);
}

protected Object doCreateBean(String beanName, RootBeanDefinition mbd, Object[] args) {
    BeanWrapper instanceWrapper = null;
    if (mbd.isSingleton()) {
        instanceWrapper = this.factoryBeanInstanceCache.remove(beanName);
    }
    if (instanceWrapper == null) {
        // Step 1: 实例化（构造函数/工厂方法）
        instanceWrapper = createBeanInstance(beanName, mbd, args);
    }
    Object bean = instanceWrapper.getWrappedInstance();
    
    // Step 2: 提前暴露 singleton 工厂（解决循环依赖的关键）
    boolean earlySingletonExposure = (mbd.isSingleton() && this.allowCircularReferences 
                                      && isSingletonCurrentlyInCreation(beanName));
    if (earlySingletonExposure) {
        addSingletonFactory(beanName, () -> getEarlyBeanReference(beanName, mbd, bean));
    }
    
    Object exposedObject = bean;
    try {
        // Step 3: 属性填充（依赖注入）
        populateBean(beanName, mbd, instanceWrapper);
        // Step 4: 初始化（aware → postProcessBeforeInitialization → init → postProcessAfterInitialization）
        exposedObject = initializeBean(beanName, exposedObject, mbd);
    }
    // ...
    return exposedObject;
}
```

#### 循环依赖三级缓存

```java
// DefaultSingletonBeanRegistry
/** 一级缓存：完全初始化好的单例 */
private final Map<String, Object> singletonObjects = new ConcurrentHashMap<>(256);
/** 二级缓存：早期单例（未完成属性填充，可提前暴露解决循环依赖） */
private final Map<String, Object> earlySingletonObjects = new HashMap<>(16);
/** 三级缓存：单例工厂（用于生成早期引用，含 AOP 代理逻辑） */
private final Map<String, ObjectFactory<?>> singletonFactories = new HashMap<>(16);
```

**循环依赖解决流程（A → B → A）：**

```
getBean(A) → 创建 A → 实例化 A → addSingletonFactory(A, factoryA) → populateBean(A)
  → 发现依赖 B → getBean(B) → 创建 B → 实例化 B → addSingletonFactory(B, factoryB) → populateBean(B)
    → 发现依赖 A → getSingleton(A)
      → 一级缓存 miss
      → 二级缓存 miss
      → 三级缓存 hit → 调用 factoryA.getEarlyBeanReference() → 获取 A 的早期引用（可能已 AOP）
      → 将 A 放入二级缓存，删除三级缓存
      → 返回 A 的早期引用给 B
  → B 完成属性注入 → B 初始化完成 → B 放入一级缓存
→ A 继续 populateBean → A 初始化完成 → A 放入一级缓存
```

**三级缓存为什么需要三级？**
- 一级：完全初始化对象
- 二级：未完全初始化但已暴露的对象（解决循环依赖）
- 三级：`ObjectFactory` —— 如果 bean 需要 AOP，只有三级缓存能在提前暴露时应用 `BeanPostProcessor`

如果没有 AOP，二级缓存就够了。三级缓存的存在是为了让 AOP 代理在循环依赖中正确生成。

### 4.3 Bean 生命周期完整流程

```
                   instantiate
    ┌──────────────────────────────────────┐
    │  1. 实例化 Bean（构造函数/工厂方法）   │
    └──────────────┬───────────────────────┘
                   ↓
    ┌──────────────────────────────────────┐
    │  2. 填充属性（populateBean）           │
    └──────────────┬───────────────────────┘
                   ↓
    ┌──────────────────────────────────────┐
    │  3. 设置 Aware 接口                    │
    │  - setBeanName(BeanNameAware)          │
    │  - setBeanClassLoader(BeanClassLoaderAware)│
    │  - setBeanFactory(BeanFactoryAware)    │
    │  - setApplicationContext(ApplicationContextAware)│
    └──────────────┬───────────────────────┘
                   ↓
    ┌──────────────────────────────────────┐
    │  4. BeanPostProcessor.postProcessBeforeInitialization│
    │  (如 @PostConstruct、@Autowired)      │
    └──────────────┬───────────────────────┘
                   ↓
    ┌──────────────────────────────────────┐
    │  5. InitializingBean.afterPropertiesSet│
    │  + @Bean(initMethod="")               │
    └──────────────┬───────────────────────┘
                   ↓
    ┌──────────────────────────────────────┐
    │  6. BeanPostProcessor.postProcessAfterInitialization│
    │  (AOP 在此处创建代理)                   │
    └──────────────┬───────────────────────┘
                   ↓
    ┌──────────────────────────────────────┐
    │  7. Bean 就绪（放入一级缓存）          │
    └──────────────┬───────────────────────┘
                   ↓
    ┌──────────────────────────────────────┐
    │  8. 销毁                               │
    │  - @PreDestroy                         │
    │  - DisposableBean.destroy()            │
    │  - @Bean(destroyMethod="")             │
    └──────────────────────────────────────┘
```

```java
// AbstractAutowireCapableBeanFactory.initializeBean
protected Object initializeBean(String beanName, Object bean, RootBeanDefinition mbd) {
    // 3. 调用 Aware 方法
    if (bean instanceof Aware) {
        if (bean instanceof BeanNameAware)
            ((BeanNameAware) bean).setBeanName(beanName);
        if (bean instanceof BeanClassLoaderAware)
            ((BeanClassLoaderAware) bean).setBeanClassLoader(getBeanClassLoader());
        if (bean instanceof BeanFactoryAware)
            ((BeanFactoryAware) bean).setBeanFactory(this);
    }
    
    // 4. BeanPostProcessor 前置处理
    Object wrappedBean = bean;
    wrappedBean = applyBeanPostProcessorsBeforeInitialization(wrappedBean, beanName);
    
    // 5. 初始化方法
    invokeInitMethods(beanName, wrappedBean, mbd);
    
    // 6. BeanPostProcessor 后置处理（AOP 代理在此创建）
    wrappedBean = applyBeanPostProcessorsAfterInitialization(wrappedBean, beanName);
    return wrappedBean;
}
```

### 4.4 Spring 事务源码

#### @Transactional 工作原理

```java
// @Transactional 启用
@Target({ElementType.METHOD, ElementType.TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Inherited
@Documented
public @interface Transactional {
    Class<? extends Throwable>[] rollbackFor() default {};
    Propagation propagation() default Propagation.REQUIRED;
    Isolation isolation() default Isolation.DEFAULT;
    // ...
}
```

核心组件链：

```
@EnableTransactionManagement
  → TransactionManagementConfigurationSelector
    → AutoProxyRegistrar (注册 InfrastructureAdvisorAutoProxyCreator)
    → ProxyTransactionManagementConfiguration
      → BeanFactoryTransactionAttributeSourceAdvisor (Advisor)
      → TransactionInterceptor (Advice)
      → AnnotationTransactionAttributeSource (切点解析)
```

#### TransactionInterceptor

```java
public class TransactionInterceptor extends TransactionAspectSupport
        implements MethodInterceptor, Serializable {

    @Override
    public Object invoke(MethodInvocation invocation) throws Throwable {
        // 委托给父类
        return invokeWithinTransaction(invocation.getMethod(), invocation.getThis().getClass(),
                                      invocation::proceed, invocation);
    }
}

// TransactionAspectSupport
protected Object invokeWithinTransaction(Method method, Class<?> targetClass,
                                         InvocationCallback invocation, Object target) {
    // 1. 获取事务属性
    TransactionAttributeSource tas = getTransactionAttributeSource();
    TransactionAttribute txAttr = tas.getTransactionAttribute(method, targetClass);
    
    // 2. 获取 PlatformTransactionManager
    PlatformTransactionManager ptm = determineTransactionManager(txAttr);
    
    // 3. 根据声明式事务执行
    if (txAttr != null && ptm instanceof CallbackPreferringPlatformTransactionManager) {
        // 回调优先（如 JTA）
        return ((CallbackPreferringPlatformTransactionManager) ptm).execute(txAttr, status -> {
            // ...
        });
    } else {
        // 模板方法模式
        TransactionInfo txInfo = createTransactionIfNecessary(ptm, txAttr, joinpointIdentification);
        Object retVal = null;
        try {
            retVal = invocation.proceedWithInvocation(); // 执行目标方法
        } catch (Throwable ex) {
            // 事务回滚（根据 rollbackFor 判断）
            completeTransactionAfterThrowing(txInfo, ex);
            throw ex;
        } finally {
            cleanupTransactionInfo(txInfo);
        }
        // 提交事务
        commitTransactionAfterReturning(txInfo);
        return retVal;
    }
}
```

#### 事务同步管理器

```java
public abstract class TransactionSynchronizationManager {
    // 每个线程持有独立的资源
    private static final ThreadLocal<Map<Object, Object>> resources =
        new NamedThreadLocal<>("Transactional resources");
    
    private static final ThreadLocal<Set<TransactionSynchronization>> synchronizations =
        new NamedThreadLocal<>("Transaction synchronizations");
    
    private static final ThreadLocal<String> currentTransactionName =
        new NamedThreadLocal<>("Current transaction name");
    
    private static final ThreadLocal<Boolean> actualTransactionActive =
        new NamedThreadLocal<>("Actual transaction active");
    
    // 绑定资源（如 DataSource 连接）
    public static void bindResource(Object key, Object value) {
        Map<Object, Object> map = resources.get();
        if (map == null) {
            map = new HashMap<>();
            resources.set(map);
        }
        map.put(key, value);
    }
    
    // 获取资源
    public static Object getResource(Object key) {
        Map<Object, Object> map = resources.get();
        if (map == null) return null;
        return map.get(key);
    }
}
```

**事务传播行为实现：**

`PROPAGATION_REQUIRED`（默认）：若当前无事务则创建，有则加入（挂起当前事务信息）。

```java
// AbstractPlatformTransactionManager.getTransaction
@Override
public final TransactionStatus getTransaction(TransactionDefinition definition) throws TransactionException {
    // 获取当前事务
    Object transaction = doGetTransaction(); // 子类实现
    
    if (isExistingTransaction(transaction)) {
        // 已存在事务 → 根据传播行为处理
        return handleExistingTransaction(definition, transaction, debugEnabled);
    }
    
    // PROPAGATION_REQUIRED → 创建新事务
    if (definition.getPropagationBehavior() == TransactionDefinition.PROPAGATION_REQUIRED
        || definition.getPropagationBehavior() == TransactionDefinition.PROPAGATION_REQUIRES_NEW
        || definition.getPropagationBehavior() == TransactionDefinition.PROPAGATION_NESTED) {
        SuspendedResourcesHolder suspendedResources = suspend(null);
        // 创建事务
        DefaultTransactionStatus status = newTransactionStatus(definition, transaction, true,
            newTransaction, debugEnabled, suspendedResources);
        doBegin(transaction, definition); // DataSourceTransactionManager 中开启连接
        return status;
    }
    // ...
}
```

---

## 5. Spring Boot 源码详解

### 5.1 @SpringBootApplication

```java
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@SpringBootConfiguration
@EnableAutoConfiguration          // ★ 自动配置核心
@ComponentScan(excludeFilters = {  // 组件扫描
    @Filter(type = FilterType.CUSTOM, classes = TypeExcludeFilter.class),
    @Filter(type = FilterType.CUSTOM, classes = AutoConfigurationExcludeFilter.class) })
public @interface SpringBootApplication {
    @AliasFor(annotation = EnableAutoConfiguration.class)
    Class<?>[] exclude() default {};
    @AliasFor(annotation = ComponentScan.class)
    String[] basePackages() default {};
}

@Configuration
public @interface SpringBootConfiguration {}
```

### 5.2 @EnableAutoConfiguration

```java
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@AutoConfigurationPackage        // 注册当前包名
@Import(AutoConfigurationImportSelector.class) // ★ 核心
public @interface EnableAutoConfiguration {
    Class<?>[] exclude() default {};
    String[] excludeName() default {};
}
```

### 5.3 AutoConfigurationImportSelector

```java
public class AutoConfigurationImportSelector
        implements DeferredImportSelector, BeanClassLoaderAware, ... {

    @Override
    public String[] selectImports(AnnotationMetadata annotationMetadata) {
        if (!isEnabled(annotationMetadata)) return NO_IMPORTS;
        // 加载自动配置元数据
        AutoConfigurationEntry autoConfigurationEntry = getAutoConfigurationEntry(annotationMetadata);
        return StringUtils.toStringArray(autoConfigurationEntry.getConfigurations());
    }

    protected AutoConfigurationEntry getAutoConfigurationEntry(AnnotationMetadata annotationMetadata) {
        AnnotationAttributes attributes = getAttributes(annotationMetadata);
        // ★ 从 spring.factories 加载所有自动配置类名
        List<String> configurations = getCandidateConfigurations(annotationMetadata, attributes);
        configurations = removeDuplicates(configurations);
        // 排除指定的配置
        Set<String> exclusions = getExclusions(annotationMetadata, attributes);
        configurations.removeAll(exclusions);
        // 过滤（按 @ConditionalOnClass 等条件过滤）
        configurations = filter(configurations, autoConfigurationMetadata);
        fireAutoConfigurationImportEvents(configurations, exclusions);
        return new AutoConfigurationEntry(configurations, exclusions);
    }

    // 从 spring.factories 中加载 EnableAutoConfiguration 对应的值
    protected List<String> getCandidateConfigurations(AnnotationMetadata metadata, AnnotationAttributes attributes) {
        List<String> configurations = SpringFactoriesLoader.loadFactoryNames(
            getSpringFactoriesLoaderFactoryClass(), getBeanClassLoader());
        Assert.notEmpty(configurations, "No auto configuration classes found in META-INF/spring.factories");
        return configurations;
    }
}
```

**`spring.factories` 加载原理：**

```java
public static List<String> loadFactoryNames(Class<?> factoryType, @Nullable ClassLoader classLoader) {
    ClassLoader classLoaderToUse = classLoader;
    if (classLoaderToUse == null) classLoaderToUse = SpringFactoriesLoader.class.getClassLoader();
    String factoryTypeName = factoryType.getName();
    // 加载所有 META-INF/spring.factories 文件
    return loadSpringFactories(classLoaderToUse).getOrDefault(factoryTypeName, Collections.emptyList());
}

private static Map<String, List<String>> loadSpringFactories(ClassLoader classLoader) {
    Enumeration<URL> urls = classLoader.getResources(FACTORIES_RESOURCE_LOCATION);
    // FACTORIES_RESOURCE_LOCATION = "META-INF/spring.factories"
    // 合并所有 spring.factories 中的内容
    // 格式：
    // org.springframework.boot.autoconfigure.EnableAutoConfiguration=\
    //   org.springframework.boot.autoconfigure.admin.SpringApplicationAdminJmxAutoConfiguration,\
    //   org.springframework.boot.autoconfigure.web.servlet.DispatcherServletAutoConfiguration,\
    //   ...
}
```

### 5.4 SpringApplication.run() 完整流程

```java
public ConfigurableApplicationContext run(String... args) {
    long startTime = System.nanoTime();
    DefaultBootstrapContext bootstrapContext = createBootstrapContext();
    ConfigurableApplicationContext context = null;
    // 配置 headless 模式
    configureHeadlessProperty();
    // 1. 获取 SpringApplicationRunListeners（从 spring.factories）
    SpringApplicationRunListeners listeners = getRunListeners(args);
    listeners.starting(bootstrapContext);
    try {
        // 2. 解析命令行参数
        ApplicationArguments applicationArguments = new DefaultApplicationArguments(args);
        // 3. 准备环境（加载 application.yml/application.properties 等）
        ConfigurableEnvironment environment = prepareEnvironment(listeners, bootstrapContext, applicationArguments);
        // 4. 打印 Banner
        Banner printedBanner = printBanner(environment);
        // 5. 创建 ApplicationContext
        context = createApplicationContext();
        context.setApplicationStartup(this.applicationStartup);
        // 6. 准备上下文（注册单例 bean、加载配置类等）
        prepareContext(bootstrapContext, context, environment, listeners, applicationArguments, printedBanner);
        // 7. 刷新上下文（调用 AbstractApplicationContext.refresh()）
        refreshContext(context);
        // 8. 刷新后处理
        afterRefresh(context, applicationArguments);
        listeners.started(context);
        // 9. 运行 ApplicationRunner 和 CommandLineRunner
        callRunners(context, applicationArguments);
    } catch (Throwable ex) {
        handleRunFailure(context, ex, listeners);
        throw new IllegalStateException(ex);
    }
    listeners.ready(context, startupTime);
    return context;
}
```

**准备环境（配置加载优先级）：**

```java
private ConfigurableEnvironment prepareEnvironment(SpringApplicationRunListeners listeners,
        DefaultBootstrapContext bootstrapContext, ApplicationArguments applicationArguments) {
    // 创建或使用已有的 Environment
    ConfigurableEnvironment environment = getOrCreateEnvironment();
    // 配置 PropertySources
    configureEnvironment(environment, applicationArguments.getSourceArgs());
    // 加载配置：按优先级从高到低
    // 1. 命令行参数 (--server.port=8080)
    // 2. JNDI 属性
    // 3. 系统属性 (System.getProperties())
    // 4. OS 环境变量
    // 5. application-{profile}.yml
    // 6. application.yml
    // 7. @PropertySource
    ConfigurationPropertySources.attach(environment);
    listeners.environmentPrepared(bootstrapContext, environment);
    return environment;
}
```

### 5.5 内嵌 Tomcat 启动流程

```java
// ServletWebServerApplicationContext.onRefresh()
@Override
protected void onRefresh() {
    super.onRefresh();
    try {
        createWebServer(); // 创建内嵌 Web 服务器
    } catch (Throwable ex) {
        throw new ApplicationContextException("Unable to start web server", ex);
    }
}

private void createWebServer() {
    WebServer webServer = this.webServer;
    ServletContext servletContext = getServletContext();
    if (webServer == null && servletContext == null) {
        // 从工厂获取 WebServer
        ServletWebServerFactory factory = getWebServerFactory();
        // 获取 TomcatServletWebServerFactory → 创建 Tomcat 实例
        this.webServer = factory.getWebServer(getSelfInitializer());
    } else if (servletContext != null) {
        try {
            getSelfInitializer().onStartup(servletContext);
        } catch (ServletException ex) { /* ... */ }
    }
    initPropertySources();
}

// TomcatServletWebServerFactory.getWebServer()
@Override
public WebServer getWebServer(ServletContextInitializer... initializers) {
    // 创建标准 Tomcat 实例
    Tomcat tomcat = new Tomcat();
    File baseDir = (this.baseDirectory != null) ? this.baseDirectory : createTempDir("tomcat");
    tomcat.setBaseDir(baseDir.getAbsolutePath());
    // 创建连接器
    Connector connector = new Connector(this.protocol); // "org.apache.coyote.http11.Http11NioProtocol"
    tomcat.getService().addConnector(connector);
    // 配置连接器（端口、超时等）
    customizeConnector(connector);
    tomcat.setConnector(connector);
    tomcat.getHost().setAutoDeploy(false);
    // 添加 Servlet 容器
    configureEngine(tomcat.getEngine());
    for (Connector additionalConnector : this.additionalTomcatConnectors) {
        tomcat.getService().addConnector(additionalConnector);
    }
    // 准备上下文
    prepareContext(tomcat.getHost(), initializers);
    // 返回 TomcatWebServer（启动包装）
    return getTomcatWebServer(tomcat);
}
```

### 5.6 配置加载优先级

Spring Boot 配置属性加载优先级（高到低）：

```
1. 命令行参数（--key=value）
2. JNDI 属性（java:comp/env）
3. 系统属性（System.getProperties()）
4. OS 环境变量（SPRING_APPLICATION_JSON / SERVER_PORT）
5. RandomValuePropertySource（random.*）
6. application-{profile}.jar 包外
7. application-{profile}.jar 包内
8. application.jar 包外
9. application.jar 包内
10. @PropertySource 注解
11. SpringApplication.setDefaultProperties 默认属性
```

**ConfigFileApplicationListener 加载 YAML 流程：**

```java
// 搜索路径：
// classpath:, classpath:/config, file:./, file:./config/
// file:./config/*/

// 配置文件名：
// application.yml, application.yaml, application.properties

// Profile 特定文件：
// application-{profile}.yml 优先级高于 application.yml
```

---

## 6. Netty 详解

### 6.1 Netty 架构

Netty 是一个异步事件驱动的网络应用框架，其核心架构围绕 EventLoop、Channel、ChannelPipeline 和 ChannelHandler 四大组件构建。理解这四者之间的协作关系是掌握 Netty 的关键。

**核心架构图**：

```
                        ┌─────────────────────────────────┐
                        │         ChannelPipeline          │
                        │  ┌───────────┬───────────────┐  │
                        │  │ Handler A  │  Handler B    │  │
                        │  │ (inbound)  │  (outbound)   │  │
                        │  └───────────┴───────────────┘  │
                        └────────────┬────────────────────┘
                                     │
┌──────────────┐       ┌─────────────▼──────────────────────┐
│  EventLoop   │──────▶│            Channel                  │
│  (单线程)     │       │  ┌────────────────────────────────┐│
│  Selector    │       │  │  ChannelConfig                 ││
│  TaskQueue   │       │  │  Unsafe (底层读写操作)           ││
└──────────────┘       │  └────────────────────────────────┘│
                       └────────────────────────────────────┘
                                  │
                       ┌──────────▼──────────┐
                       │     ByteBuf          │
                       │   (引用计数池化)      │
                       └─────────────────────┘
```

**四大组件的关系**：
- **EventLoop**：每个 EventLoop 绑定一个单线程和一个 Selector，负责处理注册到它上面的所有 Channel 的 IO 事件和异步任务
- **Channel**：代表一个网络连接，封装了 Java NIO 的 SocketChannel，提供配置、读写等操作
- **ChannelPipeline**：Channel 上的拦截器链，负责处理 IO 事件和请求（责任链模式）
- **ChannelHandler**：具体的业务处理器，处理读事件（inbound）或写事件（outbound）

```java
EventLoopGroup bossGroup = new NioEventLoopGroup(1);
EventLoopGroup workerGroup = new NioEventLoopGroup();

ServerBootstrap b = new ServerBootstrap();
b.group(bossGroup, workerGroup)
 .channel(NioServerSocketChannel.class)
 .childHandler(new ChannelInitializer<SocketChannel>() {
     @Override
     protected void initChannel(SocketChannel ch) {
         ch.pipeline().addLast(new LoggingHandler())
                      .addLast(new MyBusinessHandler());
     }
 });
```

在 Netty 的线程模型中，**EventLoop 贯穿始终**：一个 Channel 的整个生命周期（连接建立、数据读写、心跳检测等）都由同一个 EventLoop 线程处理，这种设计消除了多线程并发带来的锁竞争问题，也保证了事件处理的顺序性。

### 6.2 Reactor 线程模型源码分析

#### NioEventLoop 创建

```java
// NioEventLoopGroup 构造函数
public NioEventLoopGroup(int nThreads, Executor executor, EventExecutorChooserFactory chooserFactory,
                          SelectorProvider selectorProvider, SelectStrategyFactory selectStrategyFactory,
                          RejectedExecutionHandler rejectedExecutionHandler) {
    super(nThreads, executor, chooserFactory,
          selectorProvider, selectStrategyFactory,
          RejectedExecutionHandlers.reject());
}

// MultithreadEventLoopGroup.newChild()
@Override
protected EventLoop newChild(Executor executor, Object... args) throws Exception {
    // 创建 NioEventLoop，每个包含一个 Selector
    return new NioEventLoop(this, executor, (SelectorProvider) args[0],
        ((SelectStrategyFactory) args[1]).newSelectStrategy(),
        (RejectedExecutionHandler) args[2]);
}
```

#### NioEventLoop.run() 核心循环

`NioEventLoop.run()` 是 Netty Reactor 线程模型的核心——每个 EventLoop 线程都在这个无限循环中运行，不断执行 select → 处理 IO 事件 → 执行异步任务三部曲：

```java
@Override
protected void run() {
    int selectCnt = 0;
    for (;;) {
        try {
            int strategy;
            try {
                strategy = selectStrategy.calculateStrategy(selectNowSupplier, hasTasks());
                switch (strategy) {
                    case SelectStrategy.CONTINUE:
                        continue;
                    case SelectStrategy.BUSY_WAIT:
                    case SelectStrategy.SELECT:
                        long delay = delayedTaskNanos();
                        if (delay > 0) {
                            strategy = select(delay);
                        } else {
                            strategy = selectNow();
                        }
                }
            }

            selectCnt++;
            if (selectCnt >= SELECTOR_AUTO_REBUILD_THRESHOLD) {
                rebuildSelector();
                selectCnt = 0;
            }

            processSelectedKeys();
            runAllTasks();
            runAllTasksFromTail();
        } catch (Error e) { throw e;
        } catch (Throwable t) { logger.warn("...", t);
        } finally { }
    }
}
```

**三部曲详解**：
1. **select策略计算**：根据当前是否有异步任务来决定 select 策略。如果没有任务，阻塞等待 IO 事件（`selector.select(timeout)`）；如果有任务，执行非阻塞的 `selectNow()` 立即返回后先处理部分任务，再处理 IO 事件。这种设计避免了 IO 事件响应的延迟。
2. **处理 IO 事件**：`processSelectedKeys()` 遍历 `selectedKeys` 集合，处理就绪的 Channel。对于每个 SelectionKey，取出绑定的 attachment（Netty 内部包装的 AbstractNioChannel），并根据就绪事件类型（OP_ACCEPT、OP_READ、OP_WRITE）调用对应的处理方法。
3. **处理异步任务**：`runAllTasks()` 执行 EventLoop 中积压的异步任务。这些任务来自用户调用 `eventLoop.execute()`、`channel.write()` 等操作提交的任务。Netty 通过 MPSC（多生产者单消费者）无锁队列来管理这些任务，避免了多线程提交任务时的锁竞争。

**JDK epoll 空轮询 bug 的修复**：JDK 的 NIO 实现存在一个著名的 epoll bug——`Selector.select(timeout)` 在没有 IO 事件时也会提前返回 0，导致 CPU 空转。Netty 通过在 `run()` 循环中统计连续无事件 select 的次数，当超过阈值（`SELECTOR_AUTO_REBUILD_THRESHOLD`，默认 512）时，重建 Selector 对象并重新注册所有 Channel，从根本上避免了这个 bug 的影响：

### 6.3 ByteBuf 源码分析

Netty 的 ByteBuf 是对 Java NIO ByteBuffer 的全面改进和增强。它通过分区索引、引用计数、内存池化和直接内存管理，解决了 NIO ByteBuffer 的诸多限制。

#### 分类体系

ByteBuf 根据内存位置和池化策略分为多个子类：

```
ByteBuf
├── HeapByteBuf          (堆内存：数据在 JVM 堆中，GC 管理)
│   └── PooledHeapByteBuf (池化的堆内存 ByteBuf)
├── DirectByteBuf        (直接内存：数据在 OS 本地内存，避免 GC)
│   └── PooledDirectByteBuf (池化的直接内存 ByteBuf)
├── CompositeByteBuf     (复合缓冲区：零拷贝组合多个 ByteBuf)
└── UnreleasableByteBuf  (不可释放包装：用于特殊场景)
```

#### 核心结构——双指针设计

ByteBuf 相比 NIO ByteBuffer 最重要的改进是使用**读指针（readerIndex）**和**写指针（writerIndex）**代替了 NIO 的 position 和 limit：

```java
public abstract class ByteBuf implements ReferenceCounted, Comparable<ByteBuf> {
    int readerIndex;
    int writerIndex;
    int maxCapacity;

    public int readableBytes() { return writerIndex - readerIndex; }
    public int writableBytes() { return maxCapacity - writerIndex; }
}
```

这种双指针设计的优势在于：读写操作完全独立，不需要 flip() 来切换模式。每次写入时自动推进 writerIndex，每次读取时自动推进 readerIndex，可以随时知道还有多少可读/可写字节。

#### 引用计数机制

ByteBuf 使用引用计数（Reference Counted）机制来管理内存的释放。每个 ByteBuf 创建时 `refCnt = 1`，每次调用 `retain()` 时 `refCnt` 增加，调用 `release()` 时 `refCnt` 减少。当 `refCnt` 归零时，ByteBuf 自动释放底层内存（对于 Direct ByteBuf 意味着归还 OS 内存）：

```java
// AbstractReferenceCountedByteBuf
private volatile int refCnt;

@Override
public boolean release() {
    return release0(1);
}

private boolean release0(int decrement) {
    int oldRef = refCntUpdater.getAndAdd(this, -decrement);
    if (oldRef == decrement) {
        deallocate();
        return true;
    }
    return false;
}
```

**引用计数的使用模式**：在 ChannelPipeline 中，每个 Handler 处理完 ByteBuf 后应调用 `release()` 释放（或传递到下一个 Handler）。编码器（Encoder）将 ByteBuf 传递给下一个 Handler 时不需要 release，因为下一个 Handler 会负责。解码器（Decoder）decode 完成后调用 `ctx.fireChannelRead(list.remove(0))` 时，ByteBuf 的所有权转移到了下一个 Handler。

#### 池化 ByteBuf（PooledByteBufAllocator）

```java
// PooledByteBufAllocator 使用内存池管理
// 内存分配层次
PoolArena (直接内存/堆内存各有)
├── PoolSubpage (小于 8KB)
│   └── Tiny (512B 以下)
│   └── Small (512B ~ 8KB)
├── PoolChunkList (8KB ~ 16MB)
│   └── PoolChunk (16MB)
│       └── PoolSubpage (8KB 一页)
│       └── run (大块分配)
```

**`PoolThreadCache`**：每个线程缓存，减少锁竞争：

```java
final class PoolThreadCache {
    // 线程本地缓存的 PoolArena
    private final PoolArena<byte[]> heapArena;
    private final PoolArena<ByteBuffer> directArena;
    
    // 各种大小级别的缓存队列
    private final MemoryRegionCache<byte[]>[] smallSubPageHeapCaches;
    private final MemoryRegionCache<ByteBuffer>[] smallSubPageDirectCaches;
    
    // 分配 ByteBuf
    PooledByteBuf<T> allocate(PooledByteBuf<T> buf, int reqCapacity, int maxCapacity) {
        if (allocateNormal(buf, reqCapacity, maxCapacity)) return buf;
        // 从 arena 分配
        return allocateTinyOrSmall(buf, reqCapacity, maxCapacity);
    }
}
```

### 6.4 ChannelPipeline 责任链模式

```java
public class DefaultChannelPipeline implements ChannelPipeline {
    final AbstractChannelHandlerContext head;  // 头节点
    final AbstractChannelHandlerContext tail;  // 尾节点
    
    // 双向链表结构
    static final class HeadContext extends AbstractChannelHandlerContext
            implements ChannelOutboundHandler, ChannelInboundHandler {
        private final Unsafe unsafe;
        // head 处理 outbound 事件（最终调用 unsafe 读写）
    }
    
    static final class TailContext extends AbstractChannelHandlerContext
            implements ChannelInboundHandler {
        // tail 处理未消费的 inbound 事件（默认丢弃）
    }
}
```

**Inbound 事件传播：**

```java
// 从 head 开始向后传播
@Override
public final ChannelPipeline fireChannelRead(Object msg) {
    AbstractChannelHandlerContext.invokeChannelRead(head, msg);
    return this;
}

// AbstractChannelHandlerContext
static void invokeChannelRead(final AbstractChannelHandlerContext next, Object msg) {
    EventExecutor executor = next.executor();
    if (executor.inEventLoop()) {
        next.invokeChannelRead(msg);
    } else {
        executor.execute(() -> next.invokeChannelRead(msg)); // 切换到 EventLoop 线程
    }
}

// 递归调用下一个 handler
private void invokeChannelRead(Object msg) {
    if (invokeHandler()) {
        try {
            ((ChannelInboundHandler) handler()).channelRead(this, msg);
        } catch (Throwable t) {
            notifyHandlerException(t);
        }
    } else {
        fireChannelRead(msg); // 跳过
    }
}

// 每个 handler 调用 ctx.fireChannelRead() 传递到下一个
@Override
public void channelRead(ChannelHandlerContext ctx, Object msg) {
    ctx.fireChannelRead(msg);
}
```

**Outbound 事件传播：**

```java
// 从 tail 开始向前传播
@Override
public final ChannelFuture write(Object msg) {
    return tail.write(msg);
}

// AbstractChannelHandlerContext
private void write(Object msg, boolean flush, ChannelPromise promise) {
    // 找到下一个 outbound handler（向前找）
    AbstractChannelHandlerContext next = findContextOutbound(flush ? MASK_WRITE : MASK_FLUSH);
    // ...
    next.invokeWrite(msg, promise);
}
```

---

## 7. Netty 自定义协议

### 7.1 ByteToMessageDecoder / MessageToByteEncoder

#### ByteToMessageDecoder

```java
public abstract class ByteToMessageDecoder extends ChannelInboundHandlerAdapter {
    ByteBuf cumulation; // 累积缓冲（解决粘包）

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        if (msg instanceof ByteBuf) {
            ByteBuf data = (ByteBuf) msg;
            // 累积数据
            if (cumulation == null) {
                cumulation = data;
            } else {
                cumulation = compose(cumulation, data);
            }
            // 循环解析
            try {
                callDecode(ctx, cumulation, out);
            } finally {
                // 释放已读数据
                if (cumulation != null && !cumulation.isReadable()) {
                    cumulation.release();
                    cumulation = null;
                }
            }
        } else {
            ctx.fireChannelRead(msg);
        }
    }

    protected void callDecode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
        while (in.isReadable()) {
            int oldReaderIndex = in.readerIndex();
            int outSize = out.size();
            // 子类实现的 decode 方法
            decode(ctx, in, out);
            if (outSize == out.size()) {
                if (oldReaderIndex == in.readerIndex()) {
                    // 没有读取任何数据 → 数据不够，退出循环
                    break;
                }
            }
        }
    }

    protected abstract void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out);
}
```

#### MessageToByteEncoder

```java
public abstract class MessageToByteEncoder<I> extends ChannelOutboundHandlerAdapter {

    @Override
    public void write(ChannelHandlerContext ctx, Object msg, ChannelPromise promise) throws Exception {
        ByteBuf buf = null;
        try {
            if (acceptOutboundMessage(msg)) {
                @SuppressWarnings("unchecked")
                I cast = (I) msg;
                buf = allocateBuffer(ctx, cast, false);
                // 子类实现编码
                encode(ctx, cast, buf);
                ctx.write(buf, promise);
            } else {
                ctx.write(msg, promise);
            }
        } catch (Exception e) {
            buf.release();
            throw e;
        }
    }

    protected abstract void encode(ChannelHandlerContext ctx, I msg, ByteBuf out) throws Exception;
}
```

### 7.2 LengthFieldBasedFrameDecoder 详解

```java
public class LengthFieldBasedFrameDecoder extends ByteToMessageDecoder {
    private final int maxFrameLength;       // 最大帧长度
    private final int lengthFieldOffset;    // 长度字段偏移
    private final int lengthFieldLength;    // 长度字段字节数
    private final int lengthAdjustment;     // 长度调整
    private final int initialBytesToStrip;  // 剥离的初始字节数

    @Override
    protected final void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) throws Exception {
        Object decoded = decode(ctx, in);
        if (decoded != null) out.add(decoded);
    }

    protected Object decode(ChannelHandlerContext ctx, ByteBuf in) throws Exception {
        // 1. 检查是否有足够的字节读取长度字段
        if (in.readableBytes() < lengthFieldEndOffset) return null;
        
        // 2. 读取实际数据长度
        int actualLengthFieldOffset = in.readerIndex() + lengthFieldOffset;
        long frameLength = getUnadjustedFrameLength(in, actualLengthFieldOffset, lengthFieldLength, byteOrder);
        if (frameLength < 0) throw new CorruptedFrameException("negative length");
        frameLength += lengthAdjustment;
        
        // 3. 验证帧长度
        if (frameLength < lengthFieldEndOffset) throw new CorruptedFrameException("too small");
        if (frameLength > maxFrameLength) throw new TooLongFrameException("too large");
        
        // 4. 检查是否有足够的字节包含完整的帧
        if (in.readableBytes() < frameLength) return null;
        
        // 5. 剥离初始字节
        int skipLength = initialBytesToStrip;
        if (skipLength > frameLength) throw new CorruptedFrameException("skip > frame");
        in.skipBytes(skipLength);
        
        int actualFrameLength = (int) (frameLength - skipLength);
        // 提取帧（retain 避免复制）
        ByteBuf frame = in.retainedSlice(in.readerIndex(), actualFrameLength);
        in.readerIndex(in.readerIndex() + actualFrameLength);
        return frame;
    }
}
```

### 7.3 自定义协议设计实例

#### 协议格式

```
+---------------------------------------------------------------+
|  魔数(4B)  |  版本(1B)  |  序列化方式(1B)  |  指令类型(1B)       |
+---------------------------------------------------------------+
|  请求ID(8B) |  数据长度(4B) |  数据体(N)                        |
+---------------------------------------------------------------+
```

```java
public class Packet {
    private int magicNumber = 0xCAFEBABE;   // 魔数
    private byte version = 1;               // 版本号
    private byte serializerType;             // 序列化方式: 0-JDK, 1-JSON, 2-Protobuf
    private byte commandType;                // 指令类型: 0-心跳, 1-请求, 2-响应
    private long requestId;                  // 请求 ID
    private int dataLength;                  // 数据长度
    private byte[] data;                     // 数据体
}
```

#### 解码器

```java
public class PacketDecoder extends ByteToMessageDecoder {
    // 协议头部固定长度: 4 + 1 + 1 + 1 + 8 + 4 = 19
    private static final int HEADER_LENGTH = 19;

    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
        if (in.readableBytes() < HEADER_LENGTH) return;

        in.markReaderIndex(); // 标记位置
        int magicNumber = in.readInt();
        if (magicNumber != 0xCAFEBABE) {
            ctx.close(); // 非法连接
            return;
        }
        byte version = in.readByte();
        byte serializerType = in.readByte();
        byte commandType = in.readByte();
        long requestId = in.readLong();
        int dataLength = in.readInt();

        if (in.readableBytes() < dataLength) {
            in.resetReaderIndex(); // 数据不够
            return;
        }

        byte[] data = new byte[dataLength];
        in.readBytes(data);

        // 构造包
        Packet packet = new Packet();
        packet.setVersion(version);
        packet.setSerializerType(serializerType);
        packet.setCommandType(commandType);
        packet.setRequestId(requestId);
        packet.setDataLength(dataLength);
        // 反序列化（根据 serializerType 选择方式）
        packet.setData(data);
        out.add(packet);
    }
}
```

#### 编码器

```java
public class PacketEncoder extends MessageToByteEncoder<Packet> {
    @Override
    protected void encode(ChannelHandlerContext ctx, Packet packet, ByteBuf out) {
        out.writeInt(packet.getMagicNumber());
        out.writeByte(packet.getVersion());
        out.writeByte(packet.getSerializerType());
        out.writeByte(packet.getCommandType());
        out.writeLong(packet.getRequestId());
        // 序列化数据（根据 serializerType）
        byte[] data = packet.getData();
        out.writeInt(data.length);
        out.writeBytes(data);
    }
}
```

#### 编解码器 Codec 设计模式

```java
// 组合编解码器
public class PacketCodec extends ChannelDuplexHandler {
    // 同时处理 inbound 和 outbound
    // 实际上通常是组合 decoder + encoder
}

// ChannelInitializer 注册
public class ServerChannelInitializer extends ChannelInitializer<SocketChannel> {
    @Override
    protected void initChannel(SocketChannel ch) {
        ch.pipeline()
            .addLast(new LengthFieldBasedFrameDecoder(Integer.MAX_VALUE, 15, 4, 0, 0))
            .addLast(new PacketDecoder())
            .addLast(new PacketEncoder())
            .addLast(new PacketHandler());
    }
}
```

---

## 8. 高性能自定义协议（Netty）

### 8.1 零拷贝技术

#### FileRegion（文件传输零拷贝）

```java
// 使用 FileRegion 发送文件，内核态直接传输，无需拷贝到用户态
public class FileServerHandler extends ChannelInboundHandlerAdapter {
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        if (msg instanceof FileRegion) {
            ctx.write(msg); // 直接通过 Channel 发送
        }
    }
}

// 创建 FileRegion
FileChannel fileChannel = new FileInputStream("file.txt").getChannel();
DefaultFileRegion region = new DefaultFileRegion(fileChannel, 0, fileChannel.size());
ctx.writeAndFlush(region);
```

**底层**：`FileRegion` 最终调用 `FileChannel.transferTo()` → OS 级别 `sendfile()` 系统调用。

#### CompositeByteBuf

```java
// 组合多个 ByteBuf，避免内存复制
CompositeByteBuf composite = Unpooled.compositeBuffer();
ByteBuf header = Unpooled.buffer(10);
ByteBuf body = Unpooled.buffer(100);
// 写入数据
composite.addComponents(true, header, body); // 自动增加 writerIndex
// composite 不复制数据，只是维护一个 Component[] 数组引用
```

### 8.2 内存池（PooledByteBufAllocator）

**分配策略：**

```java
// 全局可用的分配器（推荐使用）
Channel channel = ...;
ByteBufAllocator alloc = channel.alloc(); // 返回 PooledByteBufAllocator

// 分配直接内存
ByteBuf buf = alloc.directBuffer(4096); 
// 内部流程：
// 1. 从当前线程的 PoolThreadCache 查找
// 2. 如果线程缓存有 → 直接返回
// 3. 从 PoolArena 分配
//    - < 512B: 从 tiny 缓存分配
//    - < 8KB: 从 small 缓存分配
//    - < 16MB: 从 PoolChunkList 分配
//    - > 16MB: 非池化分配
```

**Recycler 对象池：**

```java
// Netty 内部使用 Recycler 池化对象（如 ChannelOutboundBuffer.Entry）
public abstract class Recycler<T> {
    // 每个线程一个 Stack
    private final FastThreadLocal<Stack<T>> threadLocal = new FastThreadLocal<>();

    public final T get() {
        Stack<T> stack = threadLocal.get();
        DefaultHandle<T> handle = stack.pop();
        if (handle == null) {
            handle = stack.newHandle();
            handle.value = newObject(handle); // 创建新对象
        }
        return (T) handle.value;
    }

    public final boolean recycle(T obj, Handle<T> handle) {
        DefaultHandle<T> h = (DefaultHandle<T>) handle;
        h.stack.push(h); // 压回栈
        return true;
    }

    protected abstract T newObject(Handle<T> handle);
}
```

**弱引用回收**：当线程消亡时，`FastThreadLocal` 的弱引用机制会清理该线程的 `Stack`，对象最终被 GC。

### 8.3 无锁设计

#### MPSC 队列

Netty 使用 MPSC（Multi-Producer Single-Consumer）队列，用于 EventLoop 的任务队列：

```java
// 在 NioEventLoop 中使用
private final Queue<Runnable> taskQueue;

// 默认使用 MpscLinkedQueue（多生产者单消费者无锁队列）
// NioEventLoop 构造时：
taskQueue = newTaskQueue0(this.maxPendingTasks);

// MpscLinkedQueue 底层使用 LinkedQueueNode 和 CAS
public class MpscLinkedQueue<E> extends MpscLinkedQueueTailRef<E> 
    implements Queue<E> {
    
    // 生产者（多个线程可同时 offer）
    @Override
    public boolean offer(E e) {
        LinkedQueueNode<E> newNode = new LinkedQueueNode<>(e);
        // CAS 更新尾节点
        LinkedQueueNode<E> prevTail = getAndSetTailRef(newNode);
        prevTail.soNext(newNode); // StoreStore barrier
        return true;
    }
    
    // 消费者（单线程，EventLoop 自身）
    @Override
    public E poll() {
        LinkedQueueNode<E> currConsumerNode = consumerNode;
        LinkedQueueNode<E> next = currConsumerNode.lvNext();
        if (next != null) {
            // ...
            return next.getAndNullValue();
        }
        return null;
    }
}
```

#### Atomic 变量

```java
// Netty 大量使用 AtomicFieldUpdater 代替 AtomicInteger/AutomicLong 以减少对象开销
private static final AtomicIntegerFieldUpdater<AbstractReferenceCountedByteBuf> refCntUpdater =
    AtomicIntegerFieldUpdater.newUpdater(AbstractReferenceCountedByteBuf.class, "refCnt");
```

### 8.4 VarHandle / Unsafe

**VarHandle（JDK 9+）：**

```java
// 使用 VarHandle 替代 AtomicFieldUpdater
private static final VarHandle HANDLE;
static {
    try {
        HANDLE = MethodHandles.lookup().findStaticVarHandle(
            AbstractReferenceCountedByteBuf.class, "refCnt", int.class);
    } catch (Exception e) { throw new Error(e); }
}

// 使用
HANDLE.getAndAdd(this, -1); // CAS 操作
```

**Unsafe 直接内存操作：**

```java
// Netty 内部使用 PlatformDependent 封装 Unsafe 操作
// PlatformDependent 会根据 JVM 版本自适应

// 分配直接内存
long address = PlatformDependent.allocateMemory(size);

// 写入内存
PlatformDependent.putByte(address, value);
PlatformDependent.putInt(address + offset, value);

// 释放
PlatformDependent.freeMemory(address);
```

### 8.5 批量刷新与写水位

```java
// 写水位配置
class ServerChannelInitializer extends ChannelInitializer<SocketChannel> {
    @Override
    protected void initChannel(SocketChannel ch) {
        ch.config().setWriteBufferHighWaterMark(64 * 1024);  // 高水位 64KB
        ch.config().setWriteBufferLowWaterMark(32 * 1024);   // 低水位 32KB
    }
}

// 批量刷新
public class BatchFlushHandler extends ChannelDuplexHandler {
    private int pendingWrites = 0;
    private static final int BATCH_SIZE = 100;

    @Override
    public void write(ChannelHandlerContext ctx, Object msg, ChannelPromise promise) {
        ctx.write(msg, promise);
        pendingWrites++;
        if (pendingWrites >= BATCH_SIZE) {
            ctx.flush();
            pendingWrites = 0;
        }
    }

    @Override
    public void flush(ChannelHandlerContext ctx) {
        if (pendingWrites > 0) {
            ctx.flush();
            pendingWrites = 0;
        }
    }
}
```

**写水位自动控制：**

```java
// NioSocketChannel（或 AbstractChannel）
// OutboundBuffer 中的 pendingWriteBytes 超过 highWaterMark 时
// 设置 channel 为不可写
// 当低于 lowWaterMark 时恢复可写
// handler 可以通过 channel.isWritable() 判断

@Override
public void channelWritabilityChanged(ChannelHandlerContext ctx) {
    if (!ctx.channel().isWritable()) {
        // 暂停写入，避免 OOM
        ctx.channel().config().setAutoRead(false);
    } else {
        // 恢复读取
        ctx.channel().config().setAutoRead(true);
    }
}
```

### 8.6 正确的半包/粘包处理

```java
// 方案一：LengthFieldBasedFrameDecoder（推荐）
ch.pipeline().addLast(new LengthFieldBasedFrameDecoder(
    65536,  // maxFrameLength
    0,      // lengthFieldOffset
    4,      // lengthFieldLength（数据长度占用 4 字节）
    0,      // lengthAdjustment
    4       // initialBytesToStrip（剥离长度字段本身）
));

// 方案二：自定义变长解码器
public class VariableLengthDecoder extends ByteToMessageDecoder {
    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
        in.markReaderIndex();
        if (!checkMagic(in)) { in.resetReaderIndex(); ctx.close(); return; }
        if (in.readableBytes() < 4) { in.resetReaderIndex(); return; }
        int length = in.readInt();
        if (in.readableBytes() < length) { in.resetReaderIndex(); return; }
        ByteBuf frame = in.retainedSlice(in.readerIndex(), length);
        in.readerIndex(in.readerIndex() + length);
        out.add(frame);
    }
}
```

### 8.7 Epoll/Kqueue 原生传输

```java
// 使用 Epoll（Linux）或 Kqueue（macOS/BSD）替代 NIO

// Linux 平台使用 Epoll
EventLoopGroup bossGroup = new EpollEventLoopGroup(1);
EventLoopGroup workerGroup = new EpollEventLoopGroup();
ServerBootstrap b = new ServerBootstrap();
b.group(bossGroup, workerGroup)
 .channel(EpollServerSocketChannel.class)  // 使用 Epoll
 .option(ChannelOption.SO_BACKLOG, 1024)
 .childOption(ChannelOption.SO_KEEPALIVE, true)
 .childHandler(...);

// macOS 平台使用 Kqueue
// EventLoopGroup bossGroup = new KqueueEventLoopGroup(1);
// b.channel(KqueueServerSocketChannel.class);
```

**Epoll vs NIO：**

```
特性                NIO（Java 原生）           Epoll（Netty 封装）
────────           ──────────────            ─────────────────
多路复用器          Selector                  EpollEventLoop
底层               select/poll/epoll         epoll_create/epoll_ctl/epoll_wait
边缘触发            不支持                     支持 EPOLLET
水平触发            默认模式                    默认支持
额外特性            无                         EPOLLRDHUP（连接关闭）、支持 TCP_QUICKACK、零拷贝
大连接数            正常                       性能更优（尤其大量连接）
```

---

## 9. Java Socket 详解

### 9.1 BIO（Blocking IO）

BIO 是 Java 最早的网络编程模型，其核心特点是**每个连接由独立的线程处理**，IO 操作是阻塞的——线程在数据就绪之前会被操作系统挂起，不消耗 CPU。BIO 模型在连接数较少的场景下简单直观，但当连接数增多时，每连接一线程的模式会导致线程数量膨胀，线程上下文切换开销极大。

**BIO 服务端代码示例**：

```java
public class BioServer {
    public static void main(String[] args) throws IOException {
        ServerSocket serverSocket = new ServerSocket(8080);
        while (true) {
            Socket socket = serverSocket.accept();     // 阻塞等待连接
            new Thread(() -> handleConnection(socket)).start(); // 每连接一线程
        }
    }

    private static void handleConnection(Socket socket) {
        try {
            InputStream in = socket.getInputStream();
            OutputStream out = socket.getOutputStream();
            byte[] buf = new byte[1024];
            int len;
            while ((len = in.read(buf)) != -1) {        // 阻塞等待数据
                out.write(buf, 0, len);
                out.flush();
            }
        } catch (IOException e) { e.printStackTrace();
        } finally { close(socket); }
    }
}
```

**BIO 源码追踪与底层分析**：

```java
// java.net.ServerSocket.accept() → native 方法
// 调用链: accept → implAccept → socketAccept → JVM 底层
// 底层实现: JVM_SocketAccept → NET_Accept → accept() 系统调用
// 线程在 accept() 中阻塞，从操作系统的已完成连接队列（全连接队列）中获取连接

// java.net.SocketInputStream.read() → native 方法
// 调用链: socketRead0 → JVM_SocketRead → NET_Read → read() 系统调用
// 底层实现: recv(fd, buf, len, flags) 系统调用 —— 线程完全挂起直到数据到达

// SocketOutputStream.write() → socketWrite0 → JVM_SocketWrite
// 底层实现: send(fd, buf, len, flags) 系统调用
```

BIO 模式的根本问题是**线程阻塞导致资源浪费**：一个线程在等待 IO 时不能做任何其他事情。当并发连接数达到数千个时，每个连接需要一个独立的线程，而操作系统能同时支持的线程数是有限的，大量线程的上下文切换也会拖垮系统性能。

### 9.2 NIO（Non-blocking IO）

NIO 是 Java 1.4 引入的非阻塞 IO 模型，核心是 Reactor 模式——通过一个 Selector 多路复用器管理多个 Channel 的 IO 事件，**一个线程可以同时处理成千上万个连接**。NIO 的三大核心组件是 Channel、Buffer 和 Selector。

**三大核心组件**：
- **Channel**（通道）：类似于流，但可以双向读写（InputStream/OutputStream 是单向的）。常见实现包括 `SocketChannel`、`ServerSocketChannel`、`FileChannel`
- **Buffer**（缓冲区）：数据的容器，所有 IO 操作都是通过 Buffer 来读写。JDK 提供了 `ByteBuffer`、`CharBuffer`、`IntBuffer` 等类型的 Buffer
- **Selector**（多路复用器）：注册 Channel 并监听其 IO 事件（OP_ACCEPT、OP_READ、OP_WRITE、OP_CONNECT），一个 Selector 可以管理数千个 Channel

**NIO 服务端完整代码示例**：

```java
public class NioServer {
    public static void main(String[] args) throws IOException {
        Selector selector = Selector.open();
        ServerSocketChannel serverChannel = ServerSocketChannel.open();
        serverChannel.configureBlocking(false);
        serverChannel.bind(new InetSocketAddress(8080));
        serverChannel.register(selector, SelectionKey.OP_ACCEPT);

        while (true) {
            int readyChannels = selector.select();
            if (readyChannels == 0) continue;

            Set<SelectionKey> selectedKeys = selector.selectedKeys();
            Iterator<SelectionKey> keyIterator = selectedKeys.iterator();

            while (keyIterator.hasNext()) {
                SelectionKey key = keyIterator.next();
                if (key.isAcceptable()) {
                    ServerSocketChannel ssc = (ServerSocketChannel) key.channel();
                    SocketChannel sc = ssc.accept();
                    sc.configureBlocking(false);
                    sc.register(selector, SelectionKey.OP_READ);
                } else if (key.isReadable()) {
                    SocketChannel sc = (SocketChannel) key.channel();
                    ByteBuffer buf = ByteBuffer.allocate(1024);
                    int bytesRead = sc.read(buf);
                    if (bytesRead > 0) {
                        buf.flip();
                        sc.write(buf);
                    } else if (bytesRead < 0) {
                        sc.close();
                    }
                }
                keyIterator.remove();
            }
        }
    }
}
```

**Selector.select() 底层原理**：`Selector.select()` 是 NIO 的核心——它阻塞等待注册在 Selector 上的 Channel 中有 IO 事件就绪。不同操作系统有不同的底层实现：

| 操作系统 | Selector 实现类 | 底层机制 |
|---------|----------------|---------|
| Linux | `EPollSelectorImpl` | `epoll_create` / `epoll_ctl` / `epoll_wait` |
| macOS | `KQueueSelectorImpl` | `kqueue` / `kevent` |
| Windows | `WindowsSelectorImpl` | `select` 系统调用（或 IOCP 包装） |

Linux 下 NIO 的核心调用链：
```
Selector.open() → EPollSelectorProvider → EPollSelectorImpl
  → EPollArrayWrapper.epollCreate()  → epoll_create(1) 创建 epoll 实例
serverSocketChannel.register(selector, OP_ACCEPT)
  → EPollArrayWrapper.add(fd) → epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event)
selector.select()  → EPollArrayWrapper.poll(timeout) → epoll_wait(epfd, events, maxEvents, timeout)
```

**NIO vs BIO 核心优势**：NIO 通过 Selector 实现了**一个线程管理多个连接**，解决了 BIO 中\"每连接一线程\"导致的线程膨胀问题。在连接数多但活跃连接少的场景下（典型的长连接服务），NIO 的性能远远优于 BIO。但 NIO 的编程模型比 BIO 复杂得多——需要处理 Buffer 的读写切换、半包粘包、状态管理等复杂问题，这也是 Netty 等框架存在的意义。

### 9.3 AIO（Async IO）

AIO（NIO.2，Java 7 引入）是真正的异步非阻塞 IO 模型——应用提交 IO 操作后立即返回，操作系统在 IO 操作完成后通过回调通知应用。与 NIO 的\"主动轮询\"不同，AIO 是\"操作系统主动通知\"。

**AIO 代码示例**（使用 `CompletionHandler` 回调）：

```java
public class AioServer {
    public static void main(String[] args) throws IOException {
        AsynchronousServerSocketChannel server =
            AsynchronousServerSocketChannel.open();
        server.bind(new InetSocketAddress(8080));

        server.accept(null, new CompletionHandler<AsynchronousSocketChannel, Void>() {
            @Override
            public void completed(AsynchronousSocketChannel client, Void attachment) {
                server.accept(null, this); // 继续接受下一个连接
                ByteBuffer buf = ByteBuffer.allocate(1024);
                client.read(buf, buf, new CompletionHandler<Integer, ByteBuffer>() {
                    @Override
                    public void completed(Integer result, ByteBuffer buffer) {
                        buffer.flip();
                        client.write(buffer);
                    }
                    @Override
                    public void failed(Throwable exc, ByteBuffer buffer) { }
                });
            }
            @Override
            public void failed(Throwable exc, Void attachment) { }
        });

        Thread.sleep(Long.MAX_VALUE); // 主线程不阻塞
    }
}
```

**AIO 在不同平台的底层差异**：

```
Windows:  IOCP（I/O Completion Port）— 真正的操作系统级异步 IO
Linux:    通过 epoll + 线程池封装模拟异步 IO
macOS:    通过 kqueue 封装模拟异步 IO
```

**重要说明**：Linux 上的 AIO 实现并不是真正的异步 IO——JDK 在 Linux 上使用 epoll + 线程池模拟异步行为（`AsynchronousChannelProvider` 的实现本质上是一个 NIO 事件驱动的封装层）。Windows 上的 IOCP 才是真正的操作系统级异步 IO——应用提交读写请求后，操作系统内核完成整个 IO 操作后通过完成端口通知应用。AIO 在实际生产中使用较少，因为 NIO + Netty 的 Reactor 模型在大多数场景下已经足够高效，且 AIO 的编程模型并不比 NIO 更简单。

### 9.4 Socket 参数详解

```java
// 设置 Socket 参数
Socket socket = new Socket();
// 或 ServerSocketChannel 的 option

// 1. SO_RCVBUF — 接收缓冲区大小（建议在连接前设置）
socket.setReceiveBufferSize(65536); // 64KB
// 对应 TCP 窗口大小，在 accept 前设置才有效
// 源码：setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size))

// 2. SO_SNDBUF — 发送缓冲区大小
socket.setSendBufferSize(65536);
// 源码：setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size))

// 3. TCP_NODELAY — 禁用 Nagle 算法（实时性要求高时开启）
socket.setTcpNoDelay(true);
// Nagle 算法：将多个小包合并发送，减少网络小包数量
// 实时应用（如游戏、RPC）应启用 TCP_NODELAY
// 源码：setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag))

// 4. SO_KEEPALIVE — 保活探测
socket.setKeepAlive(true);
// 当 2 小时（默认）无数据交换时，发送保活探测包
// 源码：setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag))
// Linux 可通过 /proc/sys/net/ipv4/tcp_keepalive_time 等调整

// 5. SO_REUSEADDR — 允许重用 TIME_WAIT 状态的端口
socket.setReuseAddress(true);
// 服务端重启时绑定相同地址，避免 "Address already in use"
// 源码：setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))

// 6. SO_LINGER — 关闭时的等待时间
socket.setSoLinger(true, 0); // 立即关闭（发送 RST）
// true, 30: 关闭时最多等待 30 秒发送完数据
// false: 默认，close 立即返回，内核后台发送
// true, 0: 立即关闭，发送 RST 而不是 FIN
// 源码：setsockopt(fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger))

// 7. TCP_QUICKACK — 快速 ACK（仅 Linux）
// Netty 中通过 EpollChannelOption.TCP_QUICKACK 设置
// 禁用延迟确认（Delayed ACK），减少延迟
// 通常与 TCP_NODELAY 配合使用
```

**各参数适用场景：**

| 参数 | 适用场景 | 说明 |
|------|---------|------|
| `SO_RCVBUF` | 大流量服务端 | 接收窗口越大，吞吐量可能越高 |
| `SO_SNDBUF` | 大流量客户端 | 发送缓冲区大小影响背压 |
| `TCP_NODELAY` | 实时通信、RPC | 关闭 Nagle 算法，减少延迟 |
| `SO_KEEPALIVE` | 长连接服务 | 检测死连接，默认 2 小时 |
| `SO_REUSEADDR` | 服务端快速重启 | 避免 TIME_WAIT 导致绑定失败 |
| `SO_LINGER` + `(true, 0)` | 快速关闭连接 | 发送 RST 而不是 FIN |
| `TCP_QUICKACK` | 低延迟场景 | 关闭延迟确认（与 Nagle 配合） |

**性能调优参数完整示例：**

```java
// Netty 服务端调优
ServerBootstrap b = new ServerBootstrap();
b.group(bossGroup, workerGroup)
 .channel(EpollServerSocketChannel.class)
 .option(ChannelOption.SO_BACKLOG, 1024)          // 全连接队列大小
 .option(ChannelOption.SO_REUSEADDR, true)         // 地址重用
 .childOption(ChannelOption.TCP_NODELAY, true)     // 禁用 Nagle
 .childOption(ChannelOption.SO_KEEPALIVE, true)    // 保活
 .childOption(ChannelOption.SO_RCVBUF, 65536)      // 接收缓冲区 64KB
 .childOption(ChannelOption.SO_SNDBUF, 65536)      // 发送缓冲区 64KB
 .childOption(ChannelOption.WRITE_BUFFER_WATER_MARK, 
              new WriteBufferWaterMark(32768, 65536)) // 写水位
 .childOption(EpollChannelOption.TCP_QUICKACK, true)  // 快速 ACK
 .childOption(ChannelOption.ALLOCATOR, 
              PooledByteBufAllocator.DEFAULT)       // 池化内存分配器
 .childHandler(new ServerChannelInitializer());
```

---

## 总结

本文从源码层面深入剖析了 Java 核心技术体系：

| 主题 | 核心要点 |
|------|---------|
| **多线程** | Thread 生命周期、ThreadPoolExecutor 的 ctl/Worker/runWorker 设计、FutureTask 状态机、CompletableFuture 的 DAG 依赖 |
| **AOP** | JDK Proxy 字节码生成、CGLIB FastClass 索引、Spring AOP 责任链执行 |
| **锁机制** | synchronized Mark Word 锁升级、AQS CLH 队列、ReentrantLock 公平/非公平、Condition 条件队列、StampedLock 乐观读 |
| **Spring IoC** | refresh() 12 步、getBean/doCreateBean 流程、三级缓存解决循环依赖 |
| **Spring Boot** | AutoConfigurationImportSelector + spring.factories、SpringApplication.run() 流程、内嵌 Tomcat |
| **Netty** | NioEventLoop Reactor 模型、ByteBuf 池化/引用计数、Pipeline 责任链、Epoll 原生传输 |
| **自定义协议** | LengthFieldBasedFrameDecoder、魔数+版本+序列化+指令+ID+长度的协议设计 |
| **高性能** | FileRegion 零拷贝、PooledByteBufAllocator、Recycler、MPSC 无锁队列、批量刷新 |
| **Socket** | BIO/NIO/AIO 底层实现、Socket 参数（TCP_NODELAY、SO_KEEPALIVE、SO_REUSEADDR 等） |

---

> 参考来源：OpenJDK 源码、Spring Framework 源码、Netty 4.x 源码、Linux man pages
