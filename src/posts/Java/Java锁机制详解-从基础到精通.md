# Java锁机制详解：从基础到精通

> 全面深入理解Java并发编程中的各种锁机制、实现原理与最佳实践

---

## 目录

1. [锁的基础概念](#1-锁的基础概念)
2. [悲观锁与乐观锁](#2-悲观锁与乐观锁)
3. [synchronized详解](#3-synchronized详解)
4. [JUC显式锁](#4-juc显式锁)
5. [读写锁](#5-读写锁)
6. [JVM底层锁优化](#6-jvm底层锁优化)
7. [死锁问题](#7-死锁问题)
8. [C++锁机制对比](#8-c锁机制对比)
9. [最佳实践与性能优化](#9-最佳实践与性能优化)

---

## 1. 锁的基础概念

### 1.1 什么是锁

```
锁（Lock）：
用于控制多个线程对共享资源的访问，保证数据的一致性和完整性。

核心作用：
1. 互斥性（Mutual Exclusion）：同一时刻只有一个线程能访问
2. 可见性（Visibility）：一个线程的修改对其他线程可见
3. 有序性（Ordering）：禁止指令重排序
```

### 1.2 锁的分类

```
按功能分类：
├── 互斥锁（Mutex）：同一时刻只有一个线程能持有
├── 读写锁（Read-Write Lock）：读读共享，读写互斥，写写互斥
├── 自旋锁（Spin Lock）：线程忙等待，不放弃CPU
├── 递归锁（Reentrant Lock）：同一线程可多次获取
└── 公平锁/非公平锁：按申请顺序获取 vs 竞争获取

按实现分类：
├── 悲观锁：假设会发生冲突，先加锁
│   └── synchronized、ReentrantLock
└── 乐观锁：假设不会冲突，使用时检测
    └── CAS（Compare-And-Swap）

按粒度分类：
├── 对象锁：锁住对象实例
├── 类锁：锁住Class对象
└── 分段锁：锁住数据的一部分（ConcurrentHashMap）

按状态分类（JVM层面）：
├── 无锁
├── 偏向锁（Biased Lock）
├── 轻量级锁（Lightweight Lock）
└── 重量级锁（Heavyweight Lock）
```

---

## 2. 悲观锁与乐观锁

### 2.1 悲观锁（Pessimistic Lock）

#### 概念与特点

```java
/**
 * 悲观锁：假设一定会发生冲突，每次访问数据都加锁
 * 
 * 特点：
 * 1. 独占锁：其他线程阻塞等待
 * 2. 适合写多读少的场景
 * 3. 性能开销较大
 */
public class PessimisticLockDemo {
    private int count = 0;
    
    // synchronized：悲观锁的典型实现
    public synchronized void increment() {
        count++;  // 独占访问，其他线程阻塞
    }
    
    // ReentrantLock：另一种悲观锁实现
    private final ReentrantLock lock = new ReentrantLock();
    
    public void decrement() {
        lock.lock();
        try {
            count--;
        } finally {
            lock.unlock();
        }
    }
}
```

#### 数据库中的悲观锁

```sql
-- 悲观锁：使用 SELECT FOR UPDATE
BEGIN;
SELECT * FROM account WHERE id = 1 FOR UPDATE;  -- 锁定行
UPDATE account SET balance = balance - 100 WHERE id = 1;
COMMIT;

-- 其他事务必须等待锁释放
```

### 2.2 乐观锁（Optimistic Lock）

#### 概念与特点

```java
/**
 * 乐观锁：假设不会发生冲突，使用时检测冲突
 * 
 * 特点：
 * 1. 无锁机制：不阻塞其他线程
 * 2. 适合读多写少的场景
 * 3. 性能较好，但可能需要重试
 */
public class OptimisticLockDemo {
    private AtomicInteger count = new AtomicInteger(0);
    
    // CAS：乐观锁的核心实现
    public void increment() {
        int oldValue, newValue;
        do {
            oldValue = count.get();           // 读取当前值
            newValue = oldValue + 1;          // 计算新值
        } while (!count.compareAndSet(oldValue, newValue));  // CAS更新
        
        // 如果更新失败（被其他线程修改），重试
    }
    
    // AtomicInteger内部使用CAS实现
    public void incrementSimple() {
        count.incrementAndGet();  // 等价于上面的循环
    }
}
```

#### CAS原理详解

```java
/**
 * CAS（Compare-And-Swap）：比较并交换
 * 
 * 原子操作：
 * boolean compareAndSet(expectedValue, newValue) {
 *     if (currentValue == expectedValue) {
 *         currentValue = newValue;
 *         return true;
 *     }
 *     return false;
 * }
 */
public class CASDemo {
    public static void main(String[] args) {
        AtomicInteger atomic = new AtomicInteger(0);
        
        // 线程1
        new Thread(() -> {
            int expect = 0;
            int update = 1;
            boolean success = atomic.compareAndSet(expect, update);
            System.out.println("Thread1: " + success);  // true
        }).start();
        
        // 线程2
        new Thread(() -> {
            int expect = 0;
            int update = 2;
            boolean success = atomic.compareAndSet(expect, update);
            System.out.println("Thread2: " + success);  // false（已被线程1修改）
        }).start();
    }
}
```

#### CAS底层实现

```java
// AtomicInteger源码
public class AtomicInteger {
    private static final Unsafe unsafe = Unsafe.getUnsafe();
    private static final long valueOffset;
    
    static {
        try {
            // 获取value字段的内存偏移量
            valueOffset = unsafe.objectFieldOffset
                (AtomicInteger.class.getDeclaredField("value"));
        } catch (Exception ex) { throw new Error(ex); }
    }
    
    private volatile int value;
    
    public final boolean compareAndSet(int expect, int update) {
        // 调用Unsafe的CAS方法（native方法）
        return unsafe.compareAndSwapInt(this, valueOffset, expect, update);
    }
}

// Unsafe的CAS方法（C++实现）
// 使用CPU的CMPXCHG指令（x86架构）
```

#### CAS的ABA问题

```java
/**
 * ABA问题：
 * 线程1读取A，准备改为B
 * 线程2将A改为B，又改回A
 * 线程1的CAS成功，但中间状态已改变
 */
public class ABAProblemDemo {
    private static AtomicInteger atomic = new AtomicInteger(100);
    
    public static void main(String[] args) throws InterruptedException {
        // 线程1
        Thread t1 = new Thread(() -> {
            int value = atomic.get();  // 读取100
            System.out.println("T1读取：" + value);
            
            try {
                Thread.sleep(1000);  // 模拟处理时间
            } catch (InterruptedException e) {}
            
            boolean success = atomic.compareAndSet(value, 200);
            System.out.println("T1修改：" + success + ", 当前值：" + atomic.get());
        });
        
        // 线程2：制造ABA
        Thread t2 = new Thread(() -> {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {}
            
            atomic.compareAndSet(100, 200);
            System.out.println("T2修改为200");
            atomic.compareAndSet(200, 100);
            System.out.println("T2修改回100");
        });
        
        t1.start();
        t2.start();
        t1.join();
        t2.join();
        
        // 输出：
        // T1读取：100
        // T2修改为200
        // T2修改回100
        // T1修改：true, 当前值：200
        // T1的CAS成功了，但期间值被改过
    }
}
```

#### 解决ABA问题：AtomicStampedReference

```java
/**
 * AtomicStampedReference：带版本号的CAS
 * 每次修改都更新版本号，避免ABA问题
 */
public class AtomicStampedReferenceDemo {
    // 初始值100，版本号0
    private static AtomicStampedReference<Integer> asr = 
        new AtomicStampedReference<>(100, 0);
    
    public static void main(String[] args) throws InterruptedException {
        // 线程1
        Thread t1 = new Thread(() -> {
            int[] stampHolder = new int[1];
            int value = asr.get(stampHolder);  // 读取值和版本号
            int stamp = stampHolder[0];
            System.out.println("T1读取：value=" + value + ", stamp=" + stamp);
            
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {}
            
            // CAS：检查值和版本号
            boolean success = asr.compareAndSet(value, 200, stamp, stamp + 1);
            System.out.println("T1修改：" + success);
        });
        
        // 线程2：制造ABA
        Thread t2 = new Thread(() -> {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {}
            
            int[] stampHolder = new int[1];
            int value = asr.get(stampHolder);
            int stamp = stampHolder[0];
            
            asr.compareAndSet(value, 200, stamp, stamp + 1);
            System.out.println("T2修改为200");
            
            value = asr.get(stampHolder);
            stamp = stampHolder[0];
            asr.compareAndSet(value, 100, stamp, stamp + 1);
            System.out.println("T2修改回100");
        });
        
        t1.start();
        t2.start();
        t1.join();
        t2.join();
        
        // 输出：
        // T1读取：value=100, stamp=0
        // T2修改为200
        // T2修改回100
        // T1修改：false  ← 版本号不匹配，修改失败
    }
}
```

#### 数据库中的乐观锁

```sql
-- 方案1：使用版本号
CREATE TABLE account (
    id INT PRIMARY KEY,
    balance DECIMAL(10,2),
    version INT DEFAULT 0
);

-- 更新时检查版本号
UPDATE account 
SET balance = balance - 100, version = version + 1
WHERE id = 1 AND version = 10;

-- 如果version不匹配，更新失败（被其他事务修改过）

-- 方案2：使用时间戳
CREATE TABLE account (
    id INT PRIMARY KEY,
    balance DECIMAL(10,2),
    update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

UPDATE account 
SET balance = balance - 100, update_time = NOW()
WHERE id = 1 AND update_time = '2024-01-15 10:00:00';
```

### 2.3 悲观锁 vs 乐观锁

| 特性 | 悲观锁 | 乐观锁 |
|------|--------|--------|
| **假设** | 一定会冲突 | 很少冲突 |
| **实现** | 加锁（synchronized、Lock） | CAS（AtomicXxx） |
| **阻塞** | 阻塞其他线程 | 不阻塞，失败重试 |
| **适用场景** | 写多读少，冲突频繁 | 读多写少，冲突少 |
| **性能** | 上下文切换开销大 | 无切换，但可能多次重试 |
| **死锁** | 可能死锁 | 不会死锁 |
| **公平性** | 可以保证公平 | 不保证公平 |

---

## 3. synchronized详解

### 3.1 synchronized的使用方式

```java
public class SynchronizedDemo {
    private int count = 0;
    private static int staticCount = 0;
    private final Object lock = new Object();
    
    /**
     * 1. 修饰实例方法：锁住当前对象实例（this）
     */
    public synchronized void method1() {
        count++;
    }
    
    // 等价于：
    public void method1Equivalent() {
        synchronized (this) {
            count++;
        }
    }
    
    /**
     * 2. 修饰静态方法：锁住类的Class对象
     */
    public static synchronized void method2() {
        staticCount++;
    }
    
    // 等价于：
    public static void method2Equivalent() {
        synchronized (SynchronizedDemo.class) {
            staticCount++;
        }
    }
    
    /**
     * 3. 同步代码块（对象锁）：锁住指定对象
     */
    public void method3() {
        synchronized (lock) {
            count++;
        }
    }
    
    /**
     * 4. 同步代码块（类锁）：锁住Class对象
     */
    public void method4() {
        synchronized (SynchronizedDemo.class) {
            staticCount++;
        }
    }
}
```

### 3.2 synchronized的特性

```java
/**
 * 特性1：可重入性（Reentrant）
 * 同一线程可以多次获取同一把锁
 */
public class ReentrantDemo {
    public synchronized void method1() {
        System.out.println("method1");
        method2();  // 可以调用，因为已持有锁
    }
    
    public synchronized void method2() {
        System.out.println("method2");
    }
    
    public static void main(String[] args) {
        new ReentrantDemo().method1();
        // 输出：
        // method1
        // method2
    }
}

/**
 * 特性2：互斥性（Mutual Exclusion）
 * 同一时刻只有一个线程能执行同步代码
 */
public class MutualExclusionDemo {
    private int count = 0;
    
    public synchronized void increment() {
        count++;
    }
    
    public static void main(String[] args) throws InterruptedException {
        MutualExclusionDemo demo = new MutualExclusionDemo();
        
        // 1000个线程，各执行1000次
        Thread[] threads = new Thread[1000];
        for (int i = 0; i < threads.length; i++) {
            threads[i] = new Thread(() -> {
                for (int j = 0; j < 1000; j++) {
                    demo.increment();
                }
            });
            threads[i].start();
        }
        
        for (Thread t : threads) {
            t.join();
        }
        
        System.out.println("Count: " + demo.count);  // 1000000（准确）
    }
}

/**
 * 特性3：可见性（Visibility）
 * synchronized保证内存可见性
 */
public class VisibilityDemo {
    private boolean flag = false;
    
    // 线程1
    public synchronized void writer() {
        flag = true;  // 释放锁时，刷新到主内存
    }
    
    // 线程2
    public synchronized void reader() {
        if (flag) {  // 获取锁时，从主内存读取
            System.out.println("flag is true");
        }
    }
}
```

### 3.3 synchronized的字节码

```java
public class SynchronizedBytecode {
    private Object lock = new Object();
    
    public void syncBlock() {
        synchronized (lock) {
            System.out.println("synchronized");
        }
    }
    
    public synchronized void syncMethod() {
        System.out.println("synchronized");
    }
}
```

字节码分析：
```
// syncBlock方法字节码
public void syncBlock();
  Code:
     0: aload_0
     1: getfield      #3     // Field lock:Ljava/lang/Object;
     4: dup
     5: astore_1
     6: monitorenter           // ← 获取锁
     7: getstatic     #4
    10: ldc           #5
    12: invokevirtual #6
    15: aload_1
    16: monitorexit            // ← 释放锁
    17: goto          25
    20: astore_2
    21: aload_1
    22: monitorexit            // ← 异常时释放锁
    23: aload_2
    24: athrow
    25: return

// syncMethod方法字节码
public synchronized void syncMethod();
  descriptor: ()V
  flags: ACC_PUBLIC, ACC_SYNCHRONIZED  // ← 方法标志
  Code:
     0: getstatic     #4
     3: ldc           #5
     5: invokevirtual #6
     8: return
```

### 3.4 synchronized锁的升级

```
锁升级过程（不可逆）：

无锁 → 偏向锁 → 轻量级锁 → 重量级锁

┌──────────┐
│  无锁状态 │ 初始状态
└─────┬────┘
      │ 第一个线程访问
      ▼
┌──────────┐
│  偏向锁   │ 只有一个线程访问，CAS记录线程ID
└─────┬────┘ 无需加锁/解锁操作
      │ 另一个线程尝试获取
      ▼
┌──────────┐
│ 轻量级锁  │ 多个线程交替访问（无竞争）
└─────┬────┘ CAS操作，自旋等待
      │ 线程竞争激烈
      ▼
┌──────────┐
│ 重量级锁  │ 多个线程竞争
└──────────┘ 操作系统互斥量，线程阻塞
```

#### 偏向锁详解

```java
/**
 * 偏向锁（Biased Locking）
 * 
 * 核心思想：大多数情况下，锁不仅不存在多线程竞争，
 *          而且总是由同一线程多次获得
 * 
 * 实现：在对象头Mark Word中记录线程ID
 *      后续该线程进入/退出同步块时，无需CAS操作
 */
public class BiasedLockDemo {
    public static void main(String[] args) throws InterruptedException {
        // JVM启动4秒后才启用偏向锁
        Thread.sleep(5000);
        
        Object lock = new Object();
        
        synchronized (lock) {
            // 第一次获取锁：变为偏向锁，记录线程ID
            System.out.println("First lock");
        }
        
        synchronized (lock) {
            // 第二次获取锁：检查线程ID，无需CAS
            System.out.println("Second lock");
        }
    }
}

// 偏向锁的Mark Word（64位）：
// |-------54位-------|--2位--|--1位--|--4位--|--1位--|--2位--|
// |   线程ID        |epoch |unused | age  |biased| 01   |
```

#### 轻量级锁详解

```java
/**
 * 轻量级锁（Lightweight Locking）
 * 
 * 核心思想：多个线程交替执行同步块，不存在实际竞争
 * 
 * 实现：线程在栈中创建Lock Record，
 *      使用CAS将对象头的Mark Word替换为指向Lock Record的指针
 */
public class LightweightLockDemo {
    private static Object lock = new Object();
    
    public static void main(String[] args) throws InterruptedException {
        // 禁用偏向锁
        // VM参数：-XX:-UseBiasedLocking
        
        Thread t1 = new Thread(() -> {
            synchronized (lock) {
                System.out.println("Thread1 acquired lock");
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {}
            }
        });
        
        Thread t2 = new Thread(() -> {
            try {
                Thread.sleep(50);  // 错开执行
            } catch (InterruptedException e) {}
            
            synchronized (lock) {
                System.out.println("Thread2 acquired lock");
            }
        });
        
        t1.start();
        t2.start();
        t1.join();
        t2.join();
        
        // 线程交替执行，使用轻量级锁
    }
}

// 轻量级锁的Mark Word：
// |------------------62位-------------------|--2位--|
// |   指向栈中Lock Record的指针              |  00   |
```

#### 重量级锁详解

```java
/**
 * 重量级锁（Heavyweight Locking）
 * 
 * 核心思想：线程竞争激烈，使用操作系统互斥量（Mutex）
 * 
 * 实现：未获取锁的线程进入阻塞状态，
 *      需要操作系统进行线程调度（用户态→内核态）
 */
public class HeavyweightLockDemo {
    private static Object lock = new Object();
    
    public static void main(String[] args) {
        // 10个线程同时竞争锁
        for (int i = 0; i < 10; i++) {
            new Thread(() -> {
                synchronized (lock) {
                    System.out.println(Thread.currentThread().getName() + 
                                     " acquired lock");
                    try {
                        Thread.sleep(100);  // 持有锁一段时间
                    } catch (InterruptedException e) {}
                }
            }, "Thread-" + i).start();
        }
        
        // 线程竞争激烈，升级为重量级锁
    }
}

// 重量级锁的Mark Word：
// |------------------62位-------------------|--2位--|
// |   指向Monitor对象的指针                  |  10   |
```

### 3.5 Monitor对象

```java
/**
 * Monitor（监视器/管程）
 * 
 * synchronized的底层实现依赖于Monitor对象
 * 每个Java对象都可以关联一个Monitor
 */

// Monitor结构（简化版）：
class Monitor {
    Thread owner;              // 持有锁的线程
    int recursions;            // 重入次数
    Queue<Thread> entrySet;    // 等待获取锁的线程队列
    Queue<Thread> waitSet;     // 调用wait()的线程队列
}

public class MonitorDemo {
    private final Object lock = new Object();
    
    public void method1() {
        synchronized (lock) {
            // 1. 进入：尝试获取Monitor
            //    - 成功：owner = 当前线程
            //    - 失败：进入entrySet等待
            
            System.out.println("Critical section");
            
            // 2. 退出：释放Monitor
            //    - owner = null
            //    - 唤醒entrySet中的一个线程
        }
    }
    
    public void method2() {
        synchronized (lock) {
            try {
                // wait()：释放锁，进入waitSet
                lock.wait();
                
                // 被唤醒后，重新竞争锁
            } catch (InterruptedException e) {}
        }
    }
    
    public void method3() {
        synchronized (lock) {
            // notify()：唤醒waitSet中的一个线程
            lock.notify();
            
            // notifyAll()：唤醒waitSet中的所有线程
            lock.notifyAll();
        }
    }
}
```

### 3.6 wait/notify机制

```java
/**
 * wait/notify：线程间通信机制
 * 
 * 必须在synchronized块中调用
 */
public class WaitNotifyDemo {
    private final Object lock = new Object();
    private boolean ready = false;
    
    // 生产者
    public void produce() {
        synchronized (lock) {
            System.out.println("生产数据...");
            ready = true;
            lock.notify();  // 唤醒消费者
        }
    }
    
    // 消费者
    public void consume() {
        synchronized (lock) {
            while (!ready) {  // 使用while而不是if
                try {
                    lock.wait();  // 等待生产者通知
                } catch (InterruptedException e) {}
            }
            System.out.println("消费数据");
        }
    }
    
    public static void main(String[] args) {
        WaitNotifyDemo demo = new WaitNotifyDemo();
        
        // 消费者线程
        new Thread(() -> demo.consume()).start();
        
        // 生产者线程
        new Thread(() -> {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {}
            demo.produce();
        }).start();
    }
}

/**
 * 为什么使用while而不是if？
 * 
 * 虚假唤醒（Spurious Wakeup）：
 * 线程可能在没有notify的情况下被唤醒
 * 使用while可以再次检查条件
 */
```

#### 生产者-消费者模式

```java
import java.util.LinkedList;
import java.util.Queue;

public class ProducerConsumerDemo {
    private final Queue<Integer> queue = new LinkedList<>();
    private final int MAX_SIZE = 10;
    private final Object lock = new Object();
    
    // 生产者
    class Producer implements Runnable {
        @Override
        public void run() {
            int value = 0;
            while (true) {
                synchronized (lock) {
                    // 队列满了，等待
                    while (queue.size() == MAX_SIZE) {
                        try {
                            lock.wait();
                        } catch (InterruptedException e) {}
                    }
                    
                    // 生产
                    queue.offer(value);
                    System.out.println("生产：" + value);
                    value++;
                    
                    // 通知消费者
                    lock.notifyAll();
                    
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {}
                }
            }
        }
    }
    
    // 消费者
    class Consumer implements Runnable {
        @Override
        public void run() {
            while (true) {
                synchronized (lock) {
                    // 队列空了，等待
                    while (queue.isEmpty()) {
                        try {
                            lock.wait();
                        } catch (InterruptedException e) {}
                    }
                    
                    // 消费
                    int value = queue.poll();
                    System.out.println("消费：" + value);
                    
                    // 通知生产者
                    lock.notifyAll();
                    
                    try {
                        Thread.sleep(200);
                    } catch (InterruptedException e) {}
                }
            }
        }
    }
    
    public static void main(String[] args) {
        ProducerConsumerDemo demo = new ProducerConsumerDemo();
        
        // 启动2个生产者
        new Thread(demo.new Producer()).start();
        new Thread(demo.new Producer()).start();
        
        // 启动3个消费者
        new Thread(demo.new Consumer()).start();
        new Thread(demo.new Consumer()).start();
        new Thread(demo.new Consumer()).start();
    }
}
```



---

## 4. JUC显式锁

### 4.1 Lock接口

```java
import java.util.concurrent.locks.Lock;

/**
 * Lock接口：JDK 5引入的显式锁
 * 
 * 相比synchronized的优势：
 * 1. 可中断：lockInterruptibly()
 * 2. 可超时：tryLock(timeout)
 * 3. 可尝试：tryLock()
 * 4. 公平锁：ReentrantLock(true)
 * 5. 条件变量：Condition
 */
public interface Lock {
    void lock();                                    // 获取锁
    void lockInterruptibly() throws InterruptedException;  // 可中断
    boolean tryLock();                              // 尝试获取
    boolean tryLock(long time, TimeUnit unit);      // 超时获取
    void unlock();                                  // 释放锁
    Condition newCondition();                       // 条件变量
}
```

### 4.2 ReentrantLock（可重入锁）

```java
import java.util.concurrent.locks.ReentrantLock;

/**
 * ReentrantLock：可重入的互斥锁
 * 
 * 特点：
 * 1. 可重入：同一线程可多次获取
 * 2. 可公平/不公平
 * 3. 可中断
 * 4. 可超时
 */
public class ReentrantLockDemo {
    private final ReentrantLock lock = new ReentrantLock();
    private int count = 0;
    
    // 基本使用
    public void increment() {
        lock.lock();
        try {
            count++;
        } finally {
            lock.unlock();  // 必须在finally中释放
        }
    }
    
    // 可重入性
    public void reentrant() {
        lock.lock();
        try {
            System.out.println("第一次获取锁");
            reentrantInner();  // 可以再次获取
        } finally {
            lock.unlock();
        }
    }
    
    private void reentrantInner() {
        lock.lock();
        try {
            System.out.println("第二次获取锁");
        } finally {
            lock.unlock();
        }
    }
    
    // 尝试获取锁
    public boolean tryIncrement() {
        if (lock.tryLock()) {
            try {
                count++;
                return true;
            } finally {
                lock.unlock();
            }
        }
        return false;  // 获取失败，立即返回
    }
    
    // 超时获取锁
    public boolean incrementWithTimeout() {
        try {
            if (lock.tryLock(1, TimeUnit.SECONDS)) {
                try {
                    count++;
                    return true;
                } finally {
                    lock.unlock();
                }
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        return false;
    }
    
    // 可中断获取锁
    public void incrementInterruptibly() throws InterruptedException {
        lock.lockInterruptibly();  // 可以被中断
        try {
            count++;
        } finally {
            lock.unlock();
        }
    }
}
```

#### 公平锁 vs 非公平锁

```java
import java.util.concurrent.locks.ReentrantLock;

/**
 * 公平锁（Fair Lock）：
 * - 按照请求锁的顺序获取锁（FIFO）
 * - 性能较低，需要维护队列
 * 
 * 非公平锁（Nonfair Lock）：
 * - 允许插队，先尝试直接获取锁
 * - 性能较高，吞吐量大
 * - 可能导致线程饥饿
 */
public class FairVsNonfairDemo {
    // 非公平锁（默认）
    private final ReentrantLock nonfairLock = new ReentrantLock();
    
    // 公平锁
    private final ReentrantLock fairLock = new ReentrantLock(true);
    
    public void testNonfair() {
        nonfairLock.lock();
        try {
            System.out.println(Thread.currentThread().getName() + " acquired lock");
            Thread.sleep(10);
        } catch (InterruptedException e) {
        } finally {
            nonfairLock.unlock();
        }
    }
    
    public void testFair() {
        fairLock.lock();
        try {
            System.out.println(Thread.currentThread().getName() + " acquired lock");
            Thread.sleep(10);
        } catch (InterruptedException e) {
        } finally {
            fairLock.unlock();
        }
    }
    
    public static void main(String[] args) {
        FairVsNonfairDemo demo = new FairVsNonfairDemo();
        
        // 测试非公平锁
        System.out.println("=== 非公平锁 ===");
        for (int i = 0; i < 5; i++) {
            new Thread(() -> demo.testNonfair(), "Thread-" + i).start();
        }
        
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {}
        
        // 测试公平锁
        System.out.println("\n=== 公平锁 ===");
        for (int i = 0; i < 5; i++) {
            new Thread(() -> demo.testFair(), "Thread-" + i).start();
        }
    }
}
```

### 4.3 Condition条件变量

```java
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Condition：比wait/notify更强大的线程通信机制
 * 
 * 优势：
 * 1. 一个Lock可以创建多个Condition
 * 2. 更精确的通知（不同条件不同队列）
 * 3. 支持公平性
 */
public class ConditionDemo {
    private final Lock lock = new ReentrantLock();
    private final Condition condition = lock.newCondition();
    private boolean ready = false;
    
    // 等待
    public void await() {
        lock.lock();
        try {
            while (!ready) {
                condition.await();  // 等待信号
            }
            System.out.println("收到信号");
        } catch (InterruptedException e) {
        } finally {
            lock.unlock();
        }
    }
    
    // 通知
    public void signal() {
        lock.lock();
        try {
            ready = true;
            condition.signal();  // 发送信号
        } finally {
            lock.unlock();
        }
    }
    
    public static void main(String[] args) {
        ConditionDemo demo = new ConditionDemo();
        
        new Thread(() -> demo.await()).start();
        
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {}
        
        new Thread(() -> demo.signal()).start();
    }
}
```

#### 多Condition示例：有界队列

```java
import java.util.LinkedList;
import java.util.Queue;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * 使用多个Condition实现生产者-消费者
 */
public class BoundedQueue<T> {
    private final Queue<T> queue = new LinkedList<>();
    private final int capacity;
    private final Lock lock = new ReentrantLock();
    private final Condition notFull = lock.newCondition();   // 队列未满
    private final Condition notEmpty = lock.newCondition();  // 队列非空
    
    public BoundedQueue(int capacity) {
        this.capacity = capacity;
    }
    
    // 生产
    public void put(T item) throws InterruptedException {
        lock.lock();
        try {
            while (queue.size() == capacity) {
                notFull.await();  // 等待队列未满
            }
            
            queue.offer(item);
            System.out.println("生产：" + item + ", 队列大小：" + queue.size());
            
            notEmpty.signal();  // 通知消费者
        } finally {
            lock.unlock();
        }
    }
    
    // 消费
    public T take() throws InterruptedException {
        lock.lock();
        try {
            while (queue.isEmpty()) {
                notEmpty.await();  // 等待队列非空
            }
            
            T item = queue.poll();
            System.out.println("消费：" + item + ", 队列大小：" + queue.size());
            
            notFull.signal();  // 通知生产者
            
            return item;
        } finally {
            lock.unlock();
        }
    }
    
    public static void main(String[] args) {
        BoundedQueue<Integer> queue = new BoundedQueue<>(5);
        
        // 生产者
        new Thread(() -> {
            for (int i = 0; i < 10; i++) {
                try {
                    queue.put(i);
                    Thread.sleep(100);
                } catch (InterruptedException e) {}
            }
        }, "Producer").start();
        
        // 消费者
        new Thread(() -> {
            for (int i = 0; i < 10; i++) {
                try {
                    queue.take();
                    Thread.sleep(200);
                } catch (InterruptedException e) {}
            }
        }, "Consumer").start();
    }
}
```

### 4.4 ReentrantReadWriteLock（读写锁）

```java
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * ReadWriteLock：读写锁
 * 
 * 特点：
 * 1. 读读共享：多个读线程可同时访问
 * 2. 读写互斥：读和写互斥
 * 3. 写写互斥：写和写互斥
 * 
 * 适用场景：读多写少
 */
public class ReadWriteLockDemo {
    private final ReadWriteLock rwLock = new ReentrantReadWriteLock();
    private final Lock readLock = rwLock.readLock();
    private final Lock writeLock = rwLock.writeLock();
    private int value = 0;
    
    // 读操作
    public int read() {
        readLock.lock();
        try {
            System.out.println(Thread.currentThread().getName() + " 读取：" + value);
            Thread.sleep(100);
            return value;
        } catch (InterruptedException e) {
            return -1;
        } finally {
            readLock.unlock();
        }
    }
    
    // 写操作
    public void write(int newValue) {
        writeLock.lock();
        try {
            System.out.println(Thread.currentThread().getName() + " 写入：" + newValue);
            this.value = newValue;
            Thread.sleep(100);
        } catch (InterruptedException e) {
        } finally {
            writeLock.unlock();
        }
    }
    
    public static void main(String[] args) {
        ReadWriteLockDemo demo = new ReadWriteLockDemo();
        
        // 10个读线程
        for (int i = 0; i < 10; i++) {
            new Thread(() -> demo.read(), "Reader-" + i).start();
        }
        
        // 2个写线程
        for (int i = 0; i < 2; i++) {
            int value = i;
            new Thread(() -> demo.write(value), "Writer-" + i).start();
        }
        
        // 读线程可以并发执行
        // 写线程独占执行
    }
}
```

#### 读写锁的锁降级

```java
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * 锁降级：写锁降级为读锁
 * 
 * 允许：持有写锁的同时获取读锁，然后释放写锁
 * 不允许：锁升级（持有读锁时获取写锁）→ 死锁
 */
public class LockDowngradingDemo {
    private final ReadWriteLock rwLock = new ReentrantReadWriteLock();
    private volatile boolean cacheValid = false;
    private Object data;
    
    public void processData() {
        rwLock.readLock().lock();
        if (!cacheValid) {
            // 缓存失效，需要更新
            rwLock.readLock().unlock();  // 释放读锁
            rwLock.writeLock().lock();    // 获取写锁
            
            try {
                // 双重检查
                if (!cacheValid) {
                    data = loadData();     // 加载数据
                    cacheValid = true;
                }
                
                // 锁降级：持有写锁的同时获取读锁
                rwLock.readLock().lock();
            } finally {
                rwLock.writeLock().unlock();  // 释放写锁
            }
        }
        
        try {
            useData(data);  // 使用数据
        } finally {
            rwLock.readLock().unlock();
        }
    }
    
    private Object loadData() {
        System.out.println("加载数据...");
        return new Object();
    }
    
    private void useData(Object data) {
        System.out.println("使用数据：" + data);
    }
}
```

### 4.5 StampedLock（JDK 8）

```java
import java.util.concurrent.locks.StampedLock;

/**
 * StampedLock：更快的读写锁
 * 
 * 特点：
 * 1. 乐观读：不加锁，读完验证
 * 2. 悲观读：加读锁
 * 3. 写锁：独占
 * 4. 不可重入
 * 5. 不支持Condition
 * 
 * 性能：比ReentrantReadWriteLock更快
 */
public class StampedLockDemo {
    private final StampedLock sl = new StampedLock();
    private double x, y;
    
    // 写操作
    public void move(double deltaX, double deltaY) {
        long stamp = sl.writeLock();  // 获取写锁
        try {
            x += deltaX;
            y += deltaY;
        } finally {
            sl.unlockWrite(stamp);  // 释放写锁
        }
    }
    
    // 乐观读
    public double distanceFromOrigin() {
        long stamp = sl.tryOptimisticRead();  // 乐观读
        double currentX = x;
        double currentY = y;
        
        if (!sl.validate(stamp)) {  // 验证：期间有写操作？
            stamp = sl.readLock();  // 升级为悲观读锁
            try {
                currentX = x;
                currentY = y;
            } finally {
                sl.unlockRead(stamp);
            }
        }
        
        return Math.sqrt(currentX * currentX + currentY * currentY);
    }
    
    // 悲观读
    public double distanceFromOriginPessimistic() {
        long stamp = sl.readLock();  // 获取读锁
        try {
            return Math.sqrt(x * x + y * y);
        } finally {
            sl.unlockRead(stamp);  // 释放读锁
        }
    }
    
    // 锁转换
    public void moveWithConversion(double deltaX, double deltaY) {
        long stamp = sl.readLock();  // 先获取读锁
        try {
            while (x == 0.0 && y == 0.0) {
                // 尝试转换为写锁
                long ws = sl.tryConvertToWriteLock(stamp);
                if (ws != 0L) {
                    stamp = ws;  // 转换成功
                    x = deltaX;
                    y = deltaY;
                    break;
                } else {
                    // 转换失败，释放读锁，获取写锁
                    sl.unlockRead(stamp);
                    stamp = sl.writeLock();
                }
            }
        } finally {
            sl.unlock(stamp);
        }
    }
}
```

### 4.6 Semaphore（信号量）

```java
import java.util.concurrent.Semaphore;

/**
 * Semaphore：信号量，控制同时访问资源的线程数
 * 
 * 应用场景：
 * 1. 限流
 * 2. 资源池（如数据库连接池）
 * 3. 停车场（限制车位）
 */
public class SemaphoreDemo {
    // 3个许可证
    private final Semaphore semaphore = new Semaphore(3);
    
    public void access() {
        try {
            semaphore.acquire();  // 获取许可证
            System.out.println(Thread.currentThread().getName() + " 进入，可用许可：" + 
                             semaphore.availablePermits());
            
            Thread.sleep(2000);  // 模拟访问
            
        } catch (InterruptedException e) {
        } finally {
            semaphore.release();  // 释放许可证
            System.out.println(Thread.currentThread().getName() + " 离开，可用许可：" + 
                             semaphore.availablePermits());
        }
    }
    
    public static void main(String[] args) {
        SemaphoreDemo demo = new SemaphoreDemo();
        
        // 10个线程尝试访问
        for (int i = 0; i < 10; i++) {
            new Thread(() -> demo.access(), "Thread-" + i).start();
        }
        
        // 同时只有3个线程能访问
    }
}

/**
 * 停车场示例
 */
class ParkingLot {
    private final Semaphore spaces;
    
    public ParkingLot(int spaces) {
        this.spaces = new Semaphore(spaces);
    }
    
    public void park() throws InterruptedException {
        spaces.acquire();
        System.out.println(Thread.currentThread().getName() + " 停车，剩余车位：" + 
                         spaces.availablePermits());
        
        Thread.sleep((long) (Math.random() * 5000));  // 停车时间
        
        spaces.release();
        System.out.println(Thread.currentThread().getName() + " 离开，剩余车位：" + 
                         spaces.availablePermits());
    }
    
    public static void main(String[] args) {
        ParkingLot lot = new ParkingLot(3);  // 3个车位
        
        // 10辆车
        for (int i = 0; i < 10; i++) {
            new Thread(() -> {
                try {
                    lot.park();
                } catch (InterruptedException e) {}
            }, "Car-" + i).start();
        }
    }
}
```

### 4.7 CountDownLatch（倒计时门闩）

```java
import java.util.concurrent.CountDownLatch;

/**
 * CountDownLatch：倒计时器
 * 
 * 作用：等待多个线程完成后再继续执行
 * 
 * 应用场景：
 * 1. 主线程等待多个子线程完成
 * 2. 多个线程等待某个事件发生
 */
public class CountDownLatchDemo {
    public static void main(String[] args) throws InterruptedException {
        int threadCount = 5;
        CountDownLatch latch = new CountDownLatch(threadCount);
        
        // 启动5个工作线程
        for (int i = 0; i < threadCount; i++) {
            new Thread(() -> {
                try {
                    System.out.println(Thread.currentThread().getName() + " 开始工作");
                    Thread.sleep((long) (Math.random() * 3000));
                    System.out.println(Thread.currentThread().getName() + " 完成工作");
                } catch (InterruptedException e) {
                } finally {
                    latch.countDown();  // 计数减1
                }
            }, "Worker-" + i).start();
        }
        
        System.out.println("主线程等待所有工作完成...");
        latch.await();  // 等待计数归零
        System.out.println("所有工作完成，主线程继续");
    }
}

/**
 * 应用：模拟赛跑
 */
class RaceDemo {
    public static void main(String[] args) throws InterruptedException {
        int runnerCount = 5;
        CountDownLatch startSignal = new CountDownLatch(1);  // 起跑信号
        CountDownLatch doneSignal = new CountDownLatch(runnerCount);  // 完成信号
        
        // 5个运动员
        for (int i = 0; i < runnerCount; i++) {
            new Thread(() -> {
                try {
                    System.out.println(Thread.currentThread().getName() + " 准备就绪");
                    startSignal.await();  // 等待起跑信号
                    
                    System.out.println(Thread.currentThread().getName() + " 开始跑");
                    Thread.sleep((long) (Math.random() * 3000));
                    System.out.println(Thread.currentThread().getName() + " 到达终点");
                    
                } catch (InterruptedException e) {
                } finally {
                    doneSignal.countDown();
                }
            }, "Runner-" + i).start();
        }
        
        Thread.sleep(1000);
        System.out.println("发令枪：预备...跑！");
        startSignal.countDown();  // 发出起跑信号
        
        doneSignal.await();  // 等待所有人完成
        System.out.println("比赛结束");
    }
}
```

### 4.8 CyclicBarrier（循环栅栏）

```java
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

/**
 * CyclicBarrier：循环栅栏
 * 
 * 作用：让一组线程相互等待，到达屏障点后一起继续执行
 * 
 * 与CountDownLatch的区别：
 * 1. CyclicBarrier可以重用
 * 2. CountDownLatch只能使用一次
 */
public class CyclicBarrierDemo {
    public static void main(String[] args) {
        int parties = 3;
        
        // 所有线程到达后执行的任务
        Runnable barrierAction = () -> {
            System.out.println("所有线程到达屏障，开始下一阶段");
        };
        
        CyclicBarrier barrier = new CyclicBarrier(parties, barrierAction);
        
        // 启动3个线程
        for (int i = 0; i < parties; i++) {
            new Thread(() -> {
                try {
                    System.out.println(Thread.currentThread().getName() + " 第一阶段");
                    Thread.sleep((long) (Math.random() * 2000));
                    System.out.println(Thread.currentThread().getName() + " 到达屏障1");
                    barrier.await();  // 等待其他线程
                    
                    System.out.println(Thread.currentThread().getName() + " 第二阶段");
                    Thread.sleep((long) (Math.random() * 2000));
                    System.out.println(Thread.currentThread().getName() + " 到达屏障2");
                    barrier.await();  // 再次等待（可重用）
                    
                    System.out.println(Thread.currentThread().getName() + " 第三阶段");
                    
                } catch (InterruptedException | BrokenBarrierException e) {
                    e.printStackTrace();
                }
            }, "Thread-" + i).start();
        }
    }
}

/**
 * 应用：多人游戏
 */
class MultiplayerGameDemo {
    public static void main(String[] args) {
        int playerCount = 4;
        CyclicBarrier barrier = new CyclicBarrier(playerCount, () -> {
            System.out.println("所有玩家准备就绪，游戏开始！\n");
        });
        
        for (int i = 0; i < playerCount; i++) {
            new Thread(() -> {
                try {
                    System.out.println(Thread.currentThread().getName() + " 加入游戏");
                    Thread.sleep((long) (Math.random() * 3000));
                    System.out.println(Thread.currentThread().getName() + " 准备就绪");
                    
                    barrier.await();  // 等待其他玩家
                    
                    System.out.println(Thread.currentThread().getName() + " 开始游戏");
                    
                } catch (InterruptedException | BrokenBarrierException e) {}
            }, "Player-" + i).start();
        }
    }
}
```



### 4.9 Exchanger（交换器）

```java
import java.util.concurrent.Exchanger;

/**
 * Exchanger：用于两个线程之间交换数据
 * 
 * 应用场景：
 * 1. 数据校对
 * 2. 双向数据传输
 */
public class ExchangerDemo {
    public static void main(String[] args) {
        Exchanger<String> exchanger = new Exchanger<>();
        
        // 线程1
        new Thread(() -> {
            try {
                String data1 = "来自线程1的数据";
                System.out.println("线程1准备交换：" + data1);
                
                String data2 = exchanger.exchange(data1);  // 交换数据
                
                System.out.println("线程1收到：" + data2);
            } catch (InterruptedException e) {}
        }).start();
        
        // 线程2
        new Thread(() -> {
            try {
                Thread.sleep(1000);  // 模拟准备时间
                
                String data1 = "来自线程2的数据";
                System.out.println("线程2准备交换：" + data1);
                
                String data2 = exchanger.exchange(data1);  // 交换数据
                
                System.out.println("线程2收到：" + data2);
            } catch (InterruptedException e) {}
        }).start();
    }
}

/**
 * 应用：数据校对
 */
class DataCheckDemo {
    public static void main(String[] args) {
        Exchanger<String> exchanger = new Exchanger<>();
        
        // 银行流水录入线程A
        new Thread(() -> {
            try {
                String bankData = "银行流水：100笔交易";
                System.out.println("线程A录入完成：" + bankData);
                
                String compareResult = exchanger.exchange(bankData);
                System.out.println("校对结果：" + compareResult);
            } catch (InterruptedException e) {}
        }, "ThreadA").start();
        
        // 银行流水录入线程B
        new Thread(() -> {
            try {
                String bankData = "银行流水：100笔交易";
                System.out.println("线程B录入完成：" + bankData);
                
                String dataFromA = exchanger.exchange(bankData);
                
                // 校对数据
                String result = bankData.equals(dataFromA) ? "数据一致" : "数据不一致";
                System.out.println("线程B校对：" + result);
            } catch (InterruptedException e) {}
        }, "ThreadB").start();
    }
}
```

---

## 5. 读写锁深入

### 5.1 读写锁的特点

```java
/**
 * 读写锁（ReadWriteLock）规则：
 * 
 * 1. 读-读：共享，可以同时进行
 * 2. 读-写：互斥，不能同时进行
 * 3. 写-写：互斥，不能同时进行
 * 4. 写-读：互斥，不能同时进行
 * 
 * ┌─────────┬──────┬──────┬──────┐
 * │         │ 读   │ 写   │      │
 * ├─────────┼──────┼──────┤      │
 * │ 读      │ ✓    │ ✗    │      │
 * │ 写      │ ✗    │ ✗    │      │
 * └─────────┴──────┴──────┴──────┘
 */

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * 应用：缓存实现
 */
public class CacheWithRWLock<K, V> {
    private final Map<K, V> cache = new HashMap<>();
    private final ReadWriteLock lock = new ReentrantReadWriteLock();
    
    // 读取缓存
    public V get(K key) {
        lock.readLock().lock();
        try {
            System.out.println(Thread.currentThread().getName() + " 读取：" + key);
            return cache.get(key);
        } finally {
            lock.readLock().unlock();
        }
    }
    
    // 写入缓存
    public void put(K key, V value) {
        lock.writeLock().lock();
        try {
            System.out.println(Thread.currentThread().getName() + " 写入：" + key);
            cache.put(key, value);
        } finally {
            lock.writeLock().unlock();
        }
    }
    
    // 清空缓存
    public void clear() {
        lock.writeLock().lock();
        try {
            System.out.println(Thread.currentThread().getName() + " 清空缓存");
            cache.clear();
        } finally {
            lock.writeLock().unlock();
        }
    }
    
    public static void main(String[] args) {
        CacheWithRWLock<String, Integer> cache = new CacheWithRWLock<>();
        
        // 先写入一些数据
        cache.put("key1", 1);
        cache.put("key2", 2);
        
        // 10个读线程
        for (int i = 0; i < 10; i++) {
            new Thread(() -> {
                for (int j = 0; j < 3; j++) {
                    cache.get("key1");
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {}
                }
            }, "Reader-" + i).start();
        }
        
        // 2个写线程
        for (int i = 0; i < 2; i++) {
            int value = i;
            new Thread(() -> {
                cache.put("key" + value, value * 100);
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {}
            }, "Writer-" + i).start();
        }
    }
}
```

### 5.2 读写锁的性能分析

```java
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * 读写锁性能对比
 */
public class RWLockPerformance {
    private int value = 0;
    
    // 普通锁
    private final Lock lock = new ReentrantLock();
    
    // 读写锁
    private final ReadWriteLock rwLock = new ReentrantReadWriteLock();
    
    // 使用普通锁读
    public int readWithLock() {
        lock.lock();
        try {
            Thread.sleep(1);  // 模拟读操作
            return value;
        } catch (InterruptedException e) {
            return -1;
        } finally {
            lock.unlock();
        }
    }
    
    // 使用读写锁读
    public int readWithRWLock() {
        rwLock.readLock().lock();
        try {
            Thread.sleep(1);
            return value;
        } catch (InterruptedException e) {
            return -1;
        } finally {
            rwLock.readLock().unlock();
        }
    }
    
    // 性能测试
    public static void main(String[] args) throws InterruptedException {
        RWLockPerformance demo = new RWLockPerformance();
        int threadCount = 10;
        int iterations = 100;
        
        // 测试普通锁
        long start1 = System.currentTimeMillis();
        Thread[] threads1 = new Thread[threadCount];
        for (int i = 0; i < threadCount; i++) {
            threads1[i] = new Thread(() -> {
                for (int j = 0; j < iterations; j++) {
                    demo.readWithLock();
                }
            });
            threads1[i].start();
        }
        for (Thread t : threads1) {
            t.join();
        }
        long time1 = System.currentTimeMillis() - start1;
        System.out.println("普通锁耗时：" + time1 + "ms");
        
        // 测试读写锁
        long start2 = System.currentTimeMillis();
        Thread[] threads2 = new Thread[threadCount];
        for (int i = 0; i < threadCount; i++) {
            threads2[i] = new Thread(() -> {
                for (int j = 0; j < iterations; j++) {
                    demo.readWithRWLock();
                }
            });
            threads2[i].start();
        }
        for (Thread t : threads2) {
            t.join();
        }
        long time2 = System.currentTimeMillis() - start2;
        System.out.println("读写锁耗时：" + time2 + "ms");
        
        System.out.println("性能提升：" + (time1 - time2) * 100.0 / time1 + "%");
    }
}
```

---

## 6. JVM底层锁优化

### 6.1 锁粗化（Lock Coarsening）

```java
/**
 * 锁粗化：将多个连续的加锁、解锁操作合并为一次
 * 
 * 减少频繁的加锁/解锁开销
 */
public class LockCoarseningDemo {
    private final Object lock = new Object();
    
    // 优化前：多次加锁
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
    
    // JIT优化后：合并为一次加锁
    public void after() {
        synchronized (lock) {
            // 操作1
            // 操作2
            // 操作3
        }
    }
    
    // 典型场景：StringBuffer
    public String appendString() {
        StringBuffer sb = new StringBuffer();
        
        // 每个append都有synchronized
        sb.append("a");
        sb.append("b");
        sb.append("c");
        
        // JIT会优化为：
        // synchronized (sb) {
        //     sb.append("a");
        //     sb.append("b");
        //     sb.append("c");
        // }
        
        return sb.toString();
    }
}
```

### 6.2 锁消除（Lock Elimination）

```java
/**
 * 锁消除：JIT编译器检测到不可能存在共享数据竞争的锁，
 *        消除这些锁
 * 
 * 前提：逃逸分析（Escape Analysis）
 */
public class LockEliminationDemo {
    // 示例1：局部变量的锁
    public String concat(String s1, String s2) {
        StringBuffer sb = new StringBuffer();  // 局部变量，不会逃逸
        sb.append(s1);  // synchronized被消除
        sb.append(s2);  // synchronized被消除
        return sb.toString();
    }
    
    // 示例2：锁对象不逃逸
    public void localLock() {
        Object lock = new Object();  // 局部对象
        synchronized (lock) {  // 锁被消除
            // 其他线程无法访问这个锁
        }
    }
    
    // 启用锁消除：
    // -XX:+DoEscapeAnalysis（默认开启）
    // -XX:+EliminateLocks（默认开启）
}
```

### 6.3 自旋锁与自适应自旋

```java
/**
 * 自旋锁（Spin Lock）：
 * 线程不放弃CPU，而是执行忙循环（自旋）等待锁
 * 
 * 优势：避免线程切换开销
 * 劣势：占用CPU资源
 * 
 * 适用场景：锁持有时间很短
 */
public class SpinLockDemo {
    private AtomicReference<Thread> owner = new AtomicReference<>();
    
    // 简单自旋锁实现
    public void lock() {
        Thread current = Thread.currentThread();
        
        // 自旋：不断尝试CAS
        while (!owner.compareAndSet(null, current)) {
            // 空循环，忙等待
        }
    }
    
    public void unlock() {
        Thread current = Thread.currentThread();
        owner.compareAndSet(current, null);
    }
    
    public static void main(String[] args) {
        SpinLockDemo lock = new SpinLockDemo();
        
        for (int i = 0; i < 5; i++) {
            new Thread(() -> {
                lock.lock();
                try {
                    System.out.println(Thread.currentThread().getName() + " acquired lock");
                    Thread.sleep(10);  // 短时间持有锁
                } catch (InterruptedException e) {
                } finally {
                    lock.unlock();
                }
            }).start();
        }
    }
}

/**
 * 自适应自旋（Adaptive Spinning）：
 * JVM根据历史信息动态调整自旋次数
 * 
 * - 如果自旋成功，下次增加自旋次数
 * - 如果自旋失败，下次减少自旋次数
 * 
 * JDK 6引入，默认开启
 * -XX:+UseSpinning（JDK 7+已移除，始终开启）
 */
```

### 6.4 锁优化总结

```
JVM锁优化技术：

1. 偏向锁（Biased Locking）
   └─ 只有一个线程访问，记录线程ID，无需CAS

2. 轻量级锁（Lightweight Locking）
   └─ 多线程交替访问，使用CAS，自旋等待

3. 重量级锁（Heavyweight Locking）
   └─ 竞争激烈，使用操作系统互斥量

4. 自旋锁（Spin Lock）
   └─ 不放弃CPU，忙等待

5. 自适应自旋（Adaptive Spinning）
   └─ 动态调整自旋次数

6. 锁粗化（Lock Coarsening）
   └─ 合并多次加锁/解锁

7. 锁消除（Lock Elimination）
   └─ 消除不必要的锁

配置参数：
-XX:+UseBiasedLocking           # 启用偏向锁（默认）
-XX:BiasedLockingStartupDelay=0 # 偏向锁启动延迟（默认4秒）
-XX:-UseBiasedLocking           # 禁用偏向锁
-XX:+DoEscapeAnalysis           # 启用逃逸分析（默认）
-XX:+EliminateLocks             # 启用锁消除（默认）
```

---

## 7. 死锁问题

### 7.1 死锁的四个必要条件

```
死锁（Deadlock）的四个必要条件：

1. 互斥条件（Mutual Exclusion）
   └─ 资源不能被多个线程共享

2. 请求与保持条件（Hold and Wait）
   └─ 持有资源的同时等待其他资源

3. 不剥夺条件（No Preemption）
   └─ 资源不能被强行剥夺

4. 循环等待条件（Circular Wait）
   └─ 存在资源的循环等待链

破坏任意一个条件，即可避免死锁
```

### 7.2 死锁示例

```java
/**
 * 经典死锁示例
 */
public class DeadlockDemo {
    private static final Object lock1 = new Object();
    private static final Object lock2 = new Object();
    
    public static void main(String[] args) {
        // 线程1：先锁lock1，再锁lock2
        Thread t1 = new Thread(() -> {
            synchronized (lock1) {
                System.out.println("Thread1获取lock1");
                
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {}
                
                System.out.println("Thread1等待lock2...");
                synchronized (lock2) {
                    System.out.println("Thread1获取lock2");
                }
            }
        }, "Thread1");
        
        // 线程2：先锁lock2，再锁lock1
        Thread t2 = new Thread(() -> {
            synchronized (lock2) {
                System.out.println("Thread2获取lock2");
                
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {}
                
                System.out.println("Thread2等待lock1...");
                synchronized (lock1) {
                    System.out.println("Thread2获取lock1");
                }
            }
        }, "Thread2");
        
        t1.start();
        t2.start();
        
        // 输出：
        // Thread1获取lock1
        // Thread2获取lock2
        // Thread1等待lock2...
        // Thread2等待lock1...
        // （死锁，程序hang住）
    }
}
```

### 7.3 死锁检测

```bash
# 使用jstack检测死锁
jstack <pid>

# 输出示例：
Found one Java-level deadlock:
=============================
"Thread2":
  waiting to lock monitor 0x00007f8a1c001234 (a java.lang.Object),
  which is held by "Thread1"
"Thread1":
  waiting to lock monitor 0x00007f8a1c001235 (a java.lang.Object),
  which is held by "Thread2"

Java stack information for the threads listed above:
===================================================
"Thread2":
        at DeadlockDemo.lambda$main$1(DeadlockDemo.java:28)
        - waiting to lock <0x00000007d5f04678> (a java.lang.Object)
        - locked <0x00000007d5f04688> (a java.lang.Object)
        ...
```

### 7.4 避免死锁的方法

```java
/**
 * 方法1：按顺序加锁
 * 破坏循环等待条件
 */
public class AvoidDeadlock1 {
    private static final Object lock1 = new Object();
    private static final Object lock2 = new Object();
    
    // 两个线程都按相同顺序加锁
    private static void method1() {
        synchronized (lock1) {  // 先lock1
            synchronized (lock2) {  // 再lock2
                System.out.println("method1");
            }
        }
    }
    
    private static void method2() {
        synchronized (lock1) {  // 先lock1
            synchronized (lock2) {  // 再lock2
                System.out.println("method2");
            }
        }
    }
}

/**
 * 方法2：使用tryLock()超时
 * 破坏请求与保持条件
 */
public class AvoidDeadlock2 {
    private final ReentrantLock lock1 = new ReentrantLock();
    private final ReentrantLock lock2 = new ReentrantLock();
    
    public void transfer() {
        while (true) {
            try {
                if (lock1.tryLock(100, TimeUnit.MILLISECONDS)) {
                    try {
                        if (lock2.tryLock(100, TimeUnit.MILLISECONDS)) {
                            try {
                                // 转账操作
                                System.out.println("转账成功");
                                break;
                            } finally {
                                lock2.unlock();
                            }
                        }
                    } finally {
                        lock1.unlock();
                    }
                }
                
                // 获取锁失败，释放已持有的锁，重试
                Thread.sleep(10);
                
            } catch (InterruptedException e) {
                break;
            }
        }
    }
}

/**
 * 方法3：使用Lock的tryLock()
 * 尝试获取所有锁，失败则全部释放
 */
public class AvoidDeadlock3 {
    private final ReentrantLock lock1 = new ReentrantLock();
    private final ReentrantLock lock2 = new ReentrantLock();
    
    public boolean transfer() {
        // 尝试获取lock1
        if (!lock1.tryLock()) {
            return false;
        }
        
        try {
            // 尝试获取lock2
            if (!lock2.tryLock()) {
                return false;  // 失败，lock1会在finally中释放
            }
            
            try {
                // 转账操作
                System.out.println("转账成功");
                return true;
            } finally {
                lock2.unlock();
            }
        } finally {
            lock1.unlock();
        }
    }
}

/**
 * 方法4：使用哲学家就餐问题的解决方案
 * 限制同时持有资源的数量
 */
public class DiningPhilosophers {
    private final Semaphore semaphore = new Semaphore(4);  // 最多4人同时吃
    private final ReentrantLock[] forks = new ReentrantLock[5];
    
    public DiningPhilosophers() {
        for (int i = 0; i < 5; i++) {
            forks[i] = new ReentrantLock();
        }
    }
    
    public void eat(int philosopher) {
        try {
            semaphore.acquire();  // 限制同时进餐人数
            
            int left = philosopher;
            int right = (philosopher + 1) % 5;
            
            forks[left].lock();
            forks[right].lock();
            
            try {
                System.out.println("哲学家" + philosopher + "在进餐");
                Thread.sleep(1000);
            } finally {
                forks[right].unlock();
                forks[left].unlock();
            }
            
        } catch (InterruptedException e) {
        } finally {
            semaphore.release();
        }
    }
}
```



---

## 8. C++锁机制对比

### 8.1 C++11线程库

```cpp
#include <mutex>
#include <thread>
#include <iostream>

/**
 * C++ std::mutex：互斥锁
 * 对应Java的synchronized/ReentrantLock
 */
class MutexDemo {
private:
    std::mutex mtx;
    int count = 0;

public:
    // 基本使用
    void increment() {
        mtx.lock();
        count++;
        mtx.unlock();
    }
    
    // RAII方式（推荐）
    void incrementSafe() {
        std::lock_guard<std::mutex> lock(mtx);  // 自动加锁/解锁
        count++;
    }  // 离开作用域自动unlock
    
    // unique_lock（更灵活）
    void incrementFlexible() {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        lock.unlock();  // 可以手动unlock
        // 其他操作...
        lock.lock();    // 可以重新lock
        count++;
    }
};
```

### 8.2 C++锁类型对比

```cpp
#include <mutex>
#include <shared_mutex>  // C++17

/**
 * 1. std::mutex - 互斥锁
 * 对应Java：synchronized, ReentrantLock
 */
std::mutex mutex;

void useStdMutex() {
    std::lock_guard<std::mutex> lock(mutex);
    // 临界区
}

/**
 * 2. std::recursive_mutex - 递归互斥锁
 * 对应Java：ReentrantLock（默认可重入）
 */
std::recursive_mutex recursive_mutex;

void recursiveFunction(int depth) {
    std::lock_guard<std::recursive_mutex> lock(recursive_mutex);
    if (depth > 0) {
        recursiveFunction(depth - 1);  // 可以递归获取锁
    }
}

/**
 * 3. std::timed_mutex - 定时互斥锁
 * 对应Java：ReentrantLock.tryLock(timeout)
 */
std::timed_mutex timed_mutex;

void useTimedMutex() {
    if (timed_mutex.try_lock_for(std::chrono::seconds(1))) {
        // 获取锁成功
        timed_mutex.unlock();
    } else {
        // 超时
    }
}

/**
 * 4. std::shared_mutex - 共享互斥锁（读写锁）
 * 对应Java：ReentrantReadWriteLock
 * C++17引入
 */
std::shared_mutex shared_mutex;

void readOperation() {
    std::shared_lock<std::shared_mutex> lock(shared_mutex);  // 共享锁（读）
    // 读操作
}

void writeOperation() {
    std::unique_lock<std::shared_mutex> lock(shared_mutex);  // 独占锁（写）
    // 写操作
}

/**
 * 5. std::shared_timed_mutex - 定时共享互斥锁
 * C++14引入
 */
std::shared_timed_mutex shared_timed_mutex;
```

### 8.3 C++条件变量

```cpp
#include <mutex>
#include <condition_variable>
#include <queue>

/**
 * std::condition_variable - 条件变量
 * 对应Java：Condition, wait/notify
 */
class ProducerConsumer {
private:
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<int> queue;
    const size_t MAX_SIZE = 10;

public:
    // 生产者
    void produce(int value) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // 等待队列有空间
        cv.wait(lock, [this]() { return queue.size() < MAX_SIZE; });
        
        queue.push(value);
        std::cout << "生产：" << value << std::endl;
        
        cv.notify_all();  // 通知消费者
    }
    
    // 消费者
    int consume() {
        std::unique_lock<std::mutex> lock(mtx);
        
        // 等待队列非空
        cv.wait(lock, [this]() { return !queue.empty(); });
        
        int value = queue.front();
        queue.pop();
        std::cout << "消费：" << value << std::endl;
        
        cv.notify_all();  // 通知生产者
        
        return value;
    }
};
```

### 8.4 C++原子操作

```cpp
#include <atomic>

/**
 * std::atomic - 原子类型
 * 对应Java：AtomicInteger, AtomicLong等
 */
class AtomicDemo {
private:
    std::atomic<int> counter{0};

public:
    // 原子递增
    void increment() {
        counter.fetch_add(1);  // counter++
    }
    
    // CAS操作
    bool compareAndSwap(int expected, int desired) {
        return counter.compare_exchange_strong(expected, desired);
    }
    
    // 内存序（Memory Order）
    void memoryOrderDemo() {
        // Relaxed：不保证顺序
        counter.store(1, std::memory_order_relaxed);
        
        // Acquire/Release：保证可见性
        counter.store(1, std::memory_order_release);
        int value = counter.load(std::memory_order_acquire);
        
        // Seq_cst：顺序一致性（最强，默认）
        counter.store(1, std::memory_order_seq_cst);
    }
};
```

### 8.5 C++自旋锁

```cpp
#include <atomic>

/**
 * C++自旋锁实现
 * 对应Java的自旋锁
 */
class SpinLock {
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;

public:
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // 自旋等待
        }
    }
    
    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

// 使用示例
SpinLock spin_lock;

void criticalSection() {
    spin_lock.lock();
    // 临界区代码
    spin_lock.unlock();
}
```

### 8.6 Java vs C++锁对比

| 特性 | Java | C++ |
|------|------|-----|
| **基本互斥锁** | synchronized<br>ReentrantLock | std::mutex<br>std::lock_guard |
| **递归锁** | ReentrantLock（默认） | std::recursive_mutex |
| **读写锁** | ReentrantReadWriteLock | std::shared_mutex (C++17) |
| **条件变量** | Condition<br>wait/notify | std::condition_variable |
| **原子操作** | AtomicXxx | std::atomic\<T\> |
| **自旋锁** | JVM内部实现 | std::atomic_flag |
| **超时锁** | tryLock(timeout) | std::timed_mutex |
| **RAII** | try-finally | lock_guard, unique_lock |
| **公平性** | ReentrantLock(true) | 无内置支持 |
| **锁升级** | 偏向锁→轻量→重量 | 无自动升级 |

### 8.7 C++锁的最佳实践

```cpp
#include <mutex>
#include <shared_mutex>
#include <memory>

/**
 * 最佳实践1：使用RAII管理锁
 */
class BestPractice1 {
private:
    std::mutex mtx;
    int data = 0;

public:
    // 推荐：使用lock_guard
    void goodMethod() {
        std::lock_guard<std::mutex> lock(mtx);
        data++;
    }  // 自动释放锁
    
    // 不推荐：手动加锁/解锁
    void badMethod() {
        mtx.lock();
        data++;
        mtx.unlock();  // 可能忘记unlock或异常导致不unlock
    }
};

/**
 * 最佳实践2：避免死锁 - 使用std::lock
 */
class BestPractice2 {
private:
    std::mutex mtx1, mtx2;

public:
    void transfer() {
        // 同时锁定两个mutex，避免死锁
        std::lock(mtx1, mtx2);
        
        std::lock_guard<std::mutex> lock1(mtx1, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
        
        // 转账操作
    }
    
    // C++17: std::scoped_lock
    void transferCpp17() {
        std::scoped_lock lock(mtx1, mtx2);  // 更简洁
        // 转账操作
    }
};

/**
 * 最佳实践3：使用unique_lock的灵活性
 */
class BestPractice3 {
private:
    std::mutex mtx;
    std::condition_variable cv;

public:
    void flexibleLocking() {
        std::unique_lock<std::mutex> lock(mtx);
        
        // 可以手动unlock
        lock.unlock();
        // 执行不需要锁的操作
        
        // 可以重新lock
        lock.lock();
        
        // 可以用于条件变量
        cv.wait(lock);
    }
};
```

### 8.8 性能对比

```
锁性能对比（从快到慢）：

1. 无锁（Lock-Free）
   └─ std::atomic, CAS
   └─ 最快，但适用场景有限

2. 自旋锁（Spin Lock）
   └─ 适合锁持有时间极短的场景
   └─ 避免线程切换开销

3. 用户态锁（User-Space Lock）
   └─ Java偏向锁、轻量级锁
   └─ C++ std::mutex（通常使用futex）

4. 内核态锁（Kernel-Space Lock）
   └─ Java重量级锁
   └─ pthread_mutex（部分实现）
   └─ 涉及系统调用，最慢

选择建议：
- 读多写少 → 读写锁
- 极短临界区 → 自旋锁
- 一般场景 → 互斥锁
- 无共享状态 → 无锁编程
```

---

## 9. 最佳实践与性能优化

### 9.1 锁的使用原则

```java
/**
 * 原则1：缩小锁的范围
 * 只锁必要的代码，减少持锁时间
 */
public class Principle1 {
    private final Object lock = new Object();
    
    // 不好：锁的范围太大
    public synchronized void bad() {
        // 不需要同步的操作
        prepareData();
        
        // 需要同步的操作
        updateSharedData();
        
        // 不需要同步的操作
        sendNotification();
    }
    
    // 好：只锁必要的部分
    public void good() {
        prepareData();  // 不加锁
        
        synchronized (lock) {
            updateSharedData();  // 加锁
        }
        
        sendNotification();  // 不加锁
    }
    
    private void prepareData() {}
    private void updateSharedData() {}
    private void sendNotification() {}
}

/**
 * 原则2：减少锁的粒度
 * 使用多个锁分别保护不同的数据
 */
public class Principle2 {
    // 不好：一个锁保护所有数据
    private final Object lock = new Object();
    private int data1, data2;
    
    public void badUpdate1() {
        synchronized (lock) {
            data1++;
        }
    }
    
    public void badUpdate2() {
        synchronized (lock) {
            data2++;
        }
    }
    
    // 好：不同的锁保护不同的数据
    private final Object lock1 = new Object();
    private final Object lock2 = new Object();
    
    public void goodUpdate1() {
        synchronized (lock1) {
            data1++;
        }
    }
    
    public void goodUpdate2() {
        synchronized (lock2) {
            data2++;  // 与update1并发执行
        }
    }
}

/**
 * 原则3：使用并发集合
 * 优先使用JUC提供的并发集合
 */
public class Principle3 {
    // 不好：手动同步
    private final Map<String, String> map = new HashMap<>();
    
    public synchronized String badGet(String key) {
        return map.get(key);
    }
    
    public synchronized void badPut(String key, String value) {
        map.put(key, value);
    }
    
    // 好：使用ConcurrentHashMap
    private final Map<String, String> goodMap = new ConcurrentHashMap<>();
    
    public String goodGet(String key) {
        return goodMap.get(key);  // 无需手动同步
    }
    
    public void goodPut(String key, String value) {
        goodMap.put(key, value);  // 内部已优化
    }
}

/**
 * 原则4：避免锁嵌套
 * 减少死锁风险
 */
public class Principle4 {
    private final Object lock1 = new Object();
    private final Object lock2 = new Object();
    
    // 不好：锁嵌套
    public void bad() {
        synchronized (lock1) {
            synchronized (lock2) {
                // 操作
            }
        }
    }
    
    // 好：拆分方法
    public void good() {
        synchronized (lock1) {
            operation1();
        }
        
        synchronized (lock2) {
            operation2();
        }
    }
    
    private void operation1() {}
    private void operation2() {}
}
```

### 9.2 锁的选择策略

```java
/**
 * 锁选择决策树
 */
public class LockSelection {
    /**
     * 场景1：简单的原子操作
     * → 使用Atomic类
     */
    private AtomicInteger counter = new AtomicInteger(0);
    
    public void scenario1() {
        counter.incrementAndGet();
    }
    
    /**
     * 场景2：简单的互斥访问
     * → 使用synchronized
     */
    public synchronized void scenario2() {
        // 简单操作
    }
    
    /**
     * 场景3：需要高级特性（超时、中断、公平性）
     * → 使用ReentrantLock
     */
    private final ReentrantLock lock3 = new ReentrantLock();
    
    public void scenario3() {
        if (lock3.tryLock()) {
            try {
                // 操作
            } finally {
                lock3.unlock();
            }
        }
    }
    
    /**
     * 场景4：读多写少
     * → 使用ReadWriteLock
     */
    private final ReadWriteLock rwLock = new ReentrantReadWriteLock();
    
    public void read() {
        rwLock.readLock().lock();
        try {
            // 读操作
        } finally {
            rwLock.readLock().unlock();
        }
    }
    
    /**
     * 场景5：极高性能要求，读多写少
     * → 使用StampedLock
     */
    private final StampedLock stampedLock = new StampedLock();
    
    public void optimisticRead() {
        long stamp = stampedLock.tryOptimisticRead();
        // 读取数据
        if (!stampedLock.validate(stamp)) {
            // 升级为读锁
        }
    }
    
    /**
     * 场景6：限流、资源池
     * → 使用Semaphore
     */
    private final Semaphore semaphore = new Semaphore(10);
    
    public void scenario6() {
        try {
            semaphore.acquire();
            // 访问资源
        } catch (InterruptedException e) {
        } finally {
            semaphore.release();
        }
    }
}
```

### 9.3 性能优化技巧

```java
/**
 * 技巧1：使用ThreadLocal避免同步
 */
public class Optimization1 {
    // 不好：每次都需要同步
    private static final SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd");
    
    public String badFormat(Date date) {
        synchronized (sdf) {
            return sdf.format(date);
        }
    }
    
    // 好：每个线程独立的实例
    private static final ThreadLocal<SimpleDateFormat> threadLocal = 
        ThreadLocal.withInitial(() -> new SimpleDateFormat("yyyy-MM-dd"));
    
    public String goodFormat(Date date) {
        return threadLocal.get().format(date);  // 无需同步
    }
}

/**
 * 技巧2：使用不可变对象
 */
public class Optimization2 {
    // 不好：可变对象需要同步
    private List<String> list = new ArrayList<>();
    
    public synchronized void addBad(String item) {
        list.add(item);
    }
    
    public synchronized List<String> getListBad() {
        return new ArrayList<>(list);  // 防御性拷贝
    }
    
    // 好：不可变对象
    private volatile List<String> immutableList = Collections.emptyList();
    
    public void addGood(String item) {
        List<String> newList = new ArrayList<>(immutableList);
        newList.add(item);
        immutableList = Collections.unmodifiableList(newList);  // 无需同步
    }
    
    public List<String> getListGood() {
        return immutableList;  // 无需拷贝
    }
}

/**
 * 技巧3：延迟初始化
 */
public class Optimization3 {
    // 双重检查锁定（DCL）
    private volatile Helper helper;
    
    public Helper getHelper() {
        if (helper == null) {  // 第一次检查（无锁）
            synchronized (this) {
                if (helper == null) {  // 第二次检查（有锁）
                    helper = new Helper();
                }
            }
        }
        return helper;
    }
    
    // 更好：使用Holder模式（推荐）
    private static class HelperHolder {
        static final Helper INSTANCE = new Helper();
    }
    
    public static Helper getInstance() {
        return HelperHolder.INSTANCE;  // 线程安全，无需锁
    }
    
    private static class Helper {}
}

/**
 * 技巧4：分段锁（Striping）
 */
public class Optimization4<T> {
    private final int STRIPE_COUNT = 16;
    private final Object[] locks = new Object[STRIPE_COUNT];
    private final List<T>[] buckets = new List[STRIPE_COUNT];
    
    public Optimization4() {
        for (int i = 0; i < STRIPE_COUNT; i++) {
            locks[i] = new Object();
            buckets[i] = new ArrayList<>();
        }
    }
    
    public void add(T item) {
        int index = Math.abs(item.hashCode() % STRIPE_COUNT);
        
        synchronized (locks[index]) {  // 只锁一个分段
            buckets[index].add(item);
        }
    }
    
    public boolean contains(T item) {
        int index = Math.abs(item.hashCode() % STRIPE_COUNT);
        
        synchronized (locks[index]) {
            return buckets[index].contains(item);
        }
    }
}
```

### 9.4 常见错误

```java
/**
 * 错误1：锁对象选择错误
 */
public class Mistake1 {
    // 错误：使用String字面量作为锁
    public void bad1() {
        synchronized ("lock") {  // String常量池，可能被其他代码使用
            // 操作
        }
    }
    
    // 错误：锁对象可能改变
    private String lock = "lock";
    
    public void bad2() {
        synchronized (lock) {  // lock可能被修改
            // 操作
        }
    }
    
    // 正确：使用私有final对象
    private final Object lock = new Object();
    
    public void good() {
        synchronized (lock) {
            // 操作
        }
    }
}

/**
 * 错误2：忘记释放锁
 */
public class Mistake2 {
    private final ReentrantLock lock = new ReentrantLock();
    
    // 错误：异常时不释放锁
    public void bad() {
        lock.lock();
        // 可能抛出异常
        riskyOperation();
        lock.unlock();  // 可能不执行
    }
    
    // 正确：在finally中释放
    public void good() {
        lock.lock();
        try {
            riskyOperation();
        } finally {
            lock.unlock();  // 一定执行
        }
    }
    
    private void riskyOperation() {}
}

/**
 * 错误3：持有锁时调用外部方法
 */
public class Mistake3 {
    private final Object lock = new Object();
    
    // 错误：持有锁时调用外部方法
    public void bad(Runnable callback) {
        synchronized (lock) {
            // 操作
            callback.run();  // 可能导致死锁或性能问题
        }
    }
    
    // 正确：释放锁后调用
    public void good(Runnable callback) {
        synchronized (lock) {
            // 操作
        }
        callback.run();  // 释放锁后调用
    }
}

/**
 * 错误4：在循环中获取/释放锁
 */
public class Mistake4 {
    private final Object lock = new Object();
    
    // 错误：频繁获取/释放锁
    public void bad() {
        for (int i = 0; i < 1000; i++) {
            synchronized (lock) {
                // 操作
            }
        }
    }
    
    // 正确：在循环外获取锁
    public void good() {
        synchronized (lock) {
            for (int i = 0; i < 1000; i++) {
                // 操作
            }
        }
    }
}
```

---

## 10. 总结

### 10.1 Java锁知识图谱

```
                    Java锁机制
                        │
        ┌───────────────┼───────────────┐
        │               │               │
    按实现分类      按功能分类      按状态分类
        │               │               │
    ┌───┴───┐       ┌───┴───┐       ┌───┴───┐
    │       │       │       │       │       │
  悲观锁  乐观锁   互斥锁  读写锁    无锁  偏向锁
    │       │       │       │       │       │
synchro- Atomic   Reetrant RW Lock  轻量锁  重量锁
nized    CAS      Lock
```

### 10.2 锁选择速查表

| 场景 | 推荐方案 | 原因 |
|------|----------|------|
| 简单计数器 | AtomicInteger | 无锁，性能最好 |
| 简单同步块 | synchronized | 简单，JVM优化好 |
| 需要超时/中断 | ReentrantLock | 灵活性 |
| 读多写少 | ReadWriteLock | 读并发 |
| 极高性能要求 | StampedLock | 乐观读 |
| 资源限流 | Semaphore | 控制并发数 |
| 等待多任务完成 | CountDownLatch | 一次性同步 |
| 多阶段同步 | CyclicBarrier | 可重用 |
| 线程间数据交换 | Exchanger | 双向通信 |

### 10.3 性能排序

```
性能从高到低：

1. 无锁（Lock-Free）
   └─ AtomicXxx, CAS

2. 偏向锁
   └─ 只有一个线程访问

3. 轻量级锁
   └─ 线程交替访问，自旋

4. 读写锁（读操作）
   └─ 读并发

5. 普通互斥锁
   └─ synchronized, ReentrantLock

6. 重量级锁
   └─ 线程竞争激烈，阻塞
```

### 10.4 关键要点

1. **优先使用高级并发工具**：JUC提供的工具通常比手写的锁更好
2. **缩小锁范围**：只锁必要的代码
3. **减少锁粒度**：使用多个锁保护不同数据
4. **避免锁嵌套**：减少死锁风险
5. **使用不可变对象**：避免同步
6. **合理使用ThreadLocal**：线程独立数据
7. **注意锁的开销**：频繁加锁/解锁影响性能
8. **测试并发代码**：使用压力测试验证

---

**文档完成时间**：2024年
**版本**：v1.0
**作者**：Kiro AI Assistant

> 本文档全面涵盖Java锁机制的各个方面，包括原理、实现、优化和最佳实践。建议结合实际项目场景，选择合适的锁机制，并进行性能测试验证。

