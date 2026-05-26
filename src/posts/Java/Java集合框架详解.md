# Java 集合框架详解 — Map、Set、List 与高并发

---

## 一、Java 集合框架总览

```
Collection (接口)
  ├── List (接口)     — 有序，可重复
  │     ├── ArrayList       — 数组实现
  │     ├── LinkedList      — 双向链表实现
  │     ├── Vector          — 线程安全（已过时）
  │     └── CopyOnWriteArrayList  — 线程安全（并发包）
  │
  ├── Set (接口)      — 无序，不可重复
  │     ├── HashSet         — 基于 HashMap
  │     ├── LinkedHashSet   — 维护插入顺序
  │     ├── TreeSet         — 排序（红黑树）
  │     └── CopyOnWriteArraySet — 线程安全（并发包）
  │
  └── Queue (接口)    — 队列
        ├── PriorityQueue   — 优先级队列
        ├── ArrayDeque      — 双端队列
        └── ConcurrentLinkedQueue — 并发安全

Map (接口)
  ├── HashMap          — 最常用（数组+链表+红黑树）
  ├── LinkedHashMap    — 维护插入/访问顺序
  ├── TreeMap          — 排序（红黑树）
  ├── Hashtable        — 线程安全（已过时）
  ├── ConcurrentHashMap — 高并发（分段锁/CAS）
  └── IdentityHashMap  — 引用比较（== 而非 equals）
```

---

## 二、List 详解

### 2.1 ArrayList

```
底层：Object[] 数组
特点：查询快 O(1)，增删慢 O(n)（涉及数组复制）
容量：默认 10，扩容 1.5 倍
线程安全：否
```

```java
List<String> list = new ArrayList<>();
list.add("A");
list.add("B");
list.get(0);         // O(1)
list.remove(0);      // O(n) — 后续元素前移
```

**适用场景：** 频繁随机读取、尾部追加、不频繁的中间插入/删除。

### 2.2 LinkedList

```
底层：双向链表（Node<E> first, last）
特点：增删快 O(1)（两端），查询慢 O(n)（遍历）
线程安全：否
同时实现了 List 和 Deque（双端队列）
```

```java
LinkedList<String> list = new LinkedList<>();
list.addFirst("A");
list.addLast("B");
list.removeFirst();
list.removeLast();
```

**适用场景：** 频繁头尾操作（作为栈/队列/双端队列使用）。

### 2.3 Vector（已过时，不推荐）

```
底层：Object[] 数组
线程安全：synchronized 修饰所有方法（性能差）
扩容：默认 10，可指定扩容增量
```

> **替代方案：** 需要线程安全时使用 `CopyOnWriteArrayList` 或 `Collections.synchronizedList(new ArrayList<>())`。

### 2.4 CopyOnWriteArrayList（高并发推荐）

```
底层：volatile + ReentrantLock
读写分离：读操作无锁，写操作复制新数组
线程安全：完全线程安全
```

```java
CopyOnWriteArrayList<String> list = new CopyOnWriteArrayList<>();
list.add("A");       // 加锁 + 复制数组 + 替换引用
list.get(0);         // 无锁，直接读
```

| 操作 | 性能特点 |
|------|----------|
| `get()` | 极高（无锁）|
| `add()` | 低（复制整个数组）|
| `remove()` | 低（复制数组）|

> **适用场景：** 读多写少（如缓存白名单、配置信息、监听器列表）。
> **不适用场景：** 写频繁（每次写都复制数组，内存和性能开销大）。

### 2.5 List 对比总结

| 实现类 | 底层结构 | 查询 | 增删 | 线程安全 | 适用场景 |
|--------|----------|------|------|----------|----------|
| ArrayList | 数组 | O(1) | O(n) | 否 | 随机读多，写少 |
| LinkedList | 链表 | O(n) | O(1)两端 | 否 | 频繁头尾操作 |
| Vector | 数组 | O(1) | O(n) | 是（重锁）| ⚠️ 已过时 |
| CopyOnWriteArrayList | 数组+CAS | O(1) | 高开销 | 是（读写分离）| 读多写极少 |

---

## 三、Set 详解

### 3.1 HashSet

```
底层：HashMap（内部维护一个 HashMap<E, Object>）
特点：无序，不重复
允许 null：允许一个 null
线程安全：否
```

```java
Set<String> set = new HashSet<>();
set.add("A");
set.contains("A");  // O(1)
set.remove("A");    // O(1)
```

**适用场景：** 快速去重、快速存在性判断（不需要顺序）。

### 3.2 LinkedHashSet

```
底层：LinkedHashMap
特点：维护插入顺序（双向链表 + HashMap）
线程安全：否
```

```java
Set<String> set = new LinkedHashSet<>();
set.add("C");
set.add("A");
set.add("B");
// 遍历顺序：C → A → B（插入顺序）
```

**适用场景：** 需要去重且保持插入顺序（如 LRU 缓存的部分场景）。

### 3.3 TreeSet

```
底层：TreeMap（红黑树）
特点：有序（自然排序或 Comparator）
线程安全：否
```

```java
Set<String> set = new TreeSet<>();
set.add("C");
set.add("A");
set.add("B");
// 遍历顺序：A → B → C（自然排序）
```

**适用场景：** 需要排序的去重集合（如排行榜、字典序排列）。

### 3.4 CopyOnWriteArraySet（高并发推荐）

```
底层：CopyOnWriteArrayList（内部包装）
线程安全：是
```

```java
CopyOnWriteArraySet<String> set = new CopyOnWriteArraySet<>();
set.add("A");
```

> **注意：** 底层是数组，`contains()` 和 `remove()` 为 O(n)。适合读多写少、集合较小的场景。

### 3.5 ConcurrentHashMap.newKeySet()（高并发推荐）

```
底层：ConcurrentHashMap 的 KeySet 视图
线程安全：高性能并发
```

```java
Set<String> set = ConcurrentHashMap.newKeySet();
set.add("A");   // 高性能并发写入
```

**适用场景：** 高并发场景下的高吞吐去重集合。

### 3.6 Set 对比总结

| 实现类 | 底层 | 顺序 | 查找性能 | 线程安全 | 适用场景 |
|--------|------|------|----------|----------|----------|
| HashSet | HashMap | 无序 | O(1) | 否 | 通用去重 |
| LinkedHashSet | LinkedHashMap | 插入顺序 | O(1) | 否 | 保持顺序去重 |
| TreeSet | TreeMap | 排序 | O(log n) | 否 | 需要排序 |
| CopyOnWriteArraySet | CopyOnWriteArrayList | 插入顺序 | O(n) | 是 | 读多写少去重 |
| `ConcurrentHashMap.newKeySet()` | ConcurrentHashMap | 无序 | O(1) | 是（最高并发）| 高并发去重 |

---

## 四、Map 详解

### 4.1 HashMap（核心）

```
底层：数组 + 链表 + 红黑树（Java 8+）
初始容量：16
负载因子：0.75
扩容：2 倍
线程安全：否
允许 null：key 和 value 均可为 null
```

**存储结构：**
```
table[]
  ├── [0]   null
  ├── [1]   Node<K,V> —> Node<K,V> —> Node<K,V>   (链表，长度 >8 转为红黑树)
  ├── [2]   TreeNode<K,V>  (红黑树)
  └── [15]  Node<K,V>
```

**put 流程：**
```
put(key, value)
  │
  ├── 计算 hash = key.hashCode() ^ (h >>> 16)
  ├── 计算下标 index = (n - 1) & hash
  ├── 若该位置为空 → 直接放入
  ├── 若不为空 → 遍历链表/红黑树
  │     ├── hash 相同且 key 相同 → 替换 value
  │     └── 不同 → 尾插（Java 8+）
  ├── 链表长度 ≥8 且数组长度 ≥64 → 转为红黑树
  └── 元素总数 > threshold（容量×0.75）→ 扩容

扩容：创建 2 倍大小新数组，rehash 所有元素
```

**性能影响因素：**
- **初始容量：** 预估大小后指定初始容量，避免频繁扩容
- **负载因子：** 0.75 是时间与空间的最佳平衡
- **hashCode() 质量：** 分布越均匀，碰撞越少

```java
// 预估容量避免扩容
int expectedSize = 1000;
HashMap<String, String> map = new HashMap<>((int) (expectedSize / 0.75f) + 1);
```

**适用场景：** 绝大多数键值对存储，不需要线程安全。

### 4.2 LinkedHashMap

```
继承自 HashMap，额外维护双向链表保持顺序
```

```java
// 插入顺序（默认）
LinkedHashMap<String, String> map = new LinkedHashMap<>();
map.put("C", "c");
map.put("A", "a");
// 遍历顺序：C → A

// 访问顺序（可用于 LRU 缓存）
LinkedHashMap<String, String> lru = new LinkedHashMap<>(16, 0.75f, true) {
    @Override
    protected boolean removeEldestEntry(Map.Entry eldest) {
        return size() > 100;  // 超过 100 条移除最久未访问
    }
};
```

**适用场景：** LRU 缓存、需要保持顺序的 Map。

### 4.3 TreeMap

```
底层：红黑树（Red-Black Tree）
特点：key 按自然顺序或 Comparator 排序
线程安全：否
```

```java
TreeMap<String, Integer> map = new TreeMap<>();
map.put("B", 2);
map.put("A", 1);
map.put("C", 3);
// 遍历顺序：A(1) → B(2) → C(3)

// 相关方法
map.firstKey();        // 最小 key
map.lastKey();         // 最大 key
map.subMap("A", "C");  // 子范围
map.headMap("B");      // 小于 B 的所有
```

**适用场景：** 需要有序的 Map（范围查询、排名、前缀匹配）。

### 4.4 Hashtable（已过时，不推荐）

```
线程安全：synchronized 修饰所有方法（性能差）
不允许 null key/value
```

> **替代方案：** 使用 `ConcurrentHashMap`。

### 4.5 ConcurrentHashMap（高并发首选）

#### 底层原理（Java 8+）

```
数据结构：数组 + 链表 + 红黑树
并发控制：CAS + synchronized（锁定链表/红黑树头节点）
并发粒度：每个桶独立锁定，而非整个表
```

**核心机制：**

| 机制 | 说明 |
|------|------|
| **CAS 自旋** | 插入空桶时用 CAS 无锁操作 |
| **synchronized 锁桶** | 桶已有元素时只锁链表/红黑树头节点 |
| **sizeCtl** | volatile 控制扩容状态 |
| **多线程协助扩容** | 其他线程可帮忙迁移数据 |
| **TreeBin** | 红黑树根节点封装，锁粒度细化 |

**put 流程（简化）：**
```
put(key, value)
  │
  ├── 计算 hash
  ├── 死循环（自旋）
  │     ├── 桶为空 → CAS 插入 → 成功则退出
  │     ├── 正在扩容 → 协助扩容
  │     ├── 桶不为空 → synchronized(头节点) 锁定
  │     │     ├── 遍历链表/红黑树插入或替换
  │     │     └── 链表转红黑树
  │     └── 检查是否需要扩容
```

```java
ConcurrentHashMap<String, String> map = new ConcurrentHashMap<>();
map.put("key", "value");     // 线程安全的高性能写入
map.get("key");               // 无锁读取
map.computeIfAbsent("key", k -> loadFromDB(k));  // 原子计算

// 批量初始化
ConcurrentHashMap<String, String> map = new ConcurrentHashMap<>(initialCapacity);
```

> **注意：** ConcurrentHashMap 不允许 null key 和 null value（防止二义性）。

#### ConcurrentHashMap vs Hashtable

| 特性 | ConcurrentHashMap | Hashtable |
|------|-------------------|-----------|
| 锁粒度 | 桶级别（细粒度）| 整表锁（粗粒度）|
| 并发读 | 无锁（读操作不用锁）| 需要锁 |
| 性能 | 高（支持大量并发）| 低（串行化）|
| null key/value | 不允许 | 不允许 |
| 迭代器 | 弱一致性（不抛 CME）| fail-fast |

### 4.6 IdentityHashMap

```
比较 key 时使用 == 而非 equals()
适合对象引用比较
```

```java
IdentityHashMap<String, String> map = new IdentityHashMap<>();
String a = new String("key");
String b = new String("key");
map.put(a, "v1");
map.put(b, "v2");
// map.size() = 2（a != b，虽然 equals 相等）
```

**适用场景：** 序列化、代理、对象引用追踪等对引用敏感的场景。

### 4.7 Map 对比总结

| 实现类 | 底层 | 顺序 | 查找性能 | 线程安全 | key 可为 null | 适用场景 |
|--------|------|------|----------|----------|:---:|----------|
| HashMap | 数组+链表+红黑树 | 无序 | O(1) | 否 | ✔ | 通用（无并发）|
| LinkedHashMap | HashMap+双向链表 | 插/访顺序 | O(1) | 否 | ✔ | LRU/有顺序需求 |
| TreeMap | 红黑树 | 排序 | O(log n) | 否 | ✔ | 范围查询/排序 |
| Hashtable | 数组+链表 | 无序 | O(1) | 是（整表锁）| ✘ | ⚠️ 已过时 |
| **ConcurrentHashMap** | 数组+链表+红黑树 | 无序 | O(1) | **是（CAS+桶锁）** | ✘ | **高并发首选** |
| IdentityHashMap | 数组 | 无序 | O(1) | 否 | ✔ | 引用比较 |

---

## 五、高并发场景集合选择指南

### 5.1 一句话总结

```
高并发读写 → ConcurrentHashMap（Map）/ ConcurrentHashMap.newKeySet()（Set）
读多写极少 → CopyOnWriteArrayList（List）/ CopyOnWriteArraySet（Set）
普通单线程 → HashMap / ArrayList / HashSet
需要排序   → TreeMap / TreeSet
需要顺序   → LinkedHashMap / LinkedHashSet
```

### 5.2 决策树

```
Q：是否需要线程安全？
  ├── 否 → 使用非并发集合（性能最优）
  │     └── 需要顺序？→ LinkedHashMap/LinkedHashSet
  │     └── 需要排序？→ TreeMap/TreeSet
  │     └── 不需要  → HashMap/HashSet/ArrayList
  │
  └── 是 → 是否需要高并发？
        ├── 否，偶尔并发 → Collections.synchronizedXxx()
        │     ★ 但实际工作中几乎不会使用，已过时
        │
        └── 是，高并发
              ├── Map  → ConcurrentHashMap
              ├── Set  → ConcurrentHashMap.newKeySet()
              ├── List → 看读写比例
              │     ├── 读多写极少 → CopyOnWriteArrayList
              │     ├── 写多读也多 → 加锁的 ArrayList / 队列
              │     └── 写多读少 → 考虑 Queue 而非 List
              └── Queue → ConcurrentLinkedQueue / LinkedBlockingQueue
```

### 5.3 常见场景推荐

| 场景 | 推荐集合 | 原因 |
|------|----------|------|
| **本地缓存** | ConcurrentHashMap | 高性能并发读写 |
| **用户 Session** | ConcurrentHashMap | 高并发访问 |
| **去重（高并发）** | `ConcurrentHashMap.newKeySet()` | 高性能并发写 |
| **操作日志（尾部追加）** | CopyOnWriteArrayList | 读多写少 |
| **最近最久未使用缓存** | LinkedHashMap（accessOrder=true）| LRU 算法内置 |
| **排行榜** | TreeSet / TreeMap | 自动排序 |
| **消息队列** | ConcurrentLinkedQueue / LinkedBlockingQueue | 线程安全队列 |
| **去重（单线程）** | HashSet | 简单高效 |
| **KV 存储（单线程）** | HashMap | 性能最优 |
| **黑白名单（少量变更）** | CopyOnWriteArraySet | 读多写极少 |

### 5.4 并发工具补充

```java
// 读写锁（适合读多写少且需要 List 全部特性的场景）
ReadWriteLock rwLock = new ReentrantReadWriteLock();
List<String> list = new ArrayList<>();
rwLock.readLock().lock();   // 读操作
rwLock.writeLock().lock();  // 写操作

// 阻塞队列（线程池常用）
BlockingQueue<Runnable> queue = new LinkedBlockingQueue<>();
BlockingQueue<Runnable> queue = new ArrayBlockingQueue<>(100);

// 并发跳表（SortedMap 的并发版本）
ConcurrentSkipListMap<String, String> skipMap = new ConcurrentSkipListMap<>();
ConcurrentSkipListSet<String> skipSet = new ConcurrentSkipListSet<>();
```

### 5.5 性能对比（粗略参考）

```
单线程写入性能（数据越大差距越大）：
  HashMap（最快） > LinkedHashMap > TreeMap > ConcurrentHashMap > Hashtable（最慢）

高并发读取性能：
  ConcurrentHashMap ≈ HashMap（volatile 读几乎无开销）
  CopyOnWriteArrayList ≈ ArrayList
  Hashtable（最慢，全表锁）

高并发写入性能：
  ConcurrentHashMap >> Hashtable >> Vector
```

---

## 六、面试常见问题

### 6.1 HashMap 容量为什么是 2 的幂？

> 因为 `(n - 1) & hash` 等价于 `hash % n`，但位运算比取模快得多。当 n 是 2 的幂时，`n-1` 的二进制是全 1，能均匀散列。

### 6.2 HashMap 红黑树为什么阈值是 8？

> 根据泊松分布，在负载因子 0.75 下，链表长度达到 8 的概率极低（约 0.0000006）。长度 ≥ 8 时说明 hash 碰撞严重，用红黑树优化 O(n) → O(log n)。

### 6.3 ConcurrentHashMap 的迭代器为什么是"弱一致性"？

> 迭代器遍历时，如果其他线程修改了 Map，迭代器不会抛 `ConcurrentModificationException`，但可能看到旧的元素（或看不到新元素）。这是为了性能而牺牲强一致性。

### 6.4 HashMap 和 ConcurrentHashMap 为什么不允许直接操作红黑树？

> 为了封装复杂性。红黑树的旋转、变色等操作是内部实现细节，对外只暴露 Node 接口，TreeNode 继承 Node。

---

## 附录：速查表

```java
// ===== 单线程（性能优先）=====
Map       → HashMap
Set       → HashSet
List      → ArrayList
有序 Map   → LinkedHashMap
排序 Map   → TreeMap
排序 Set   → TreeSet
队列       → ArrayDeque / LinkedList
栈         → ArrayDeque

// ===== 多线程（线程安全 / 高并发）=====
Map       → ConcurrentHashMap
Set       → ConcurrentHashMap.newKeySet()
有序 Map   → ConcurrentSkipListMap
排序 Map   → ConcurrentSkipListMap
排序 Set   → ConcurrentSkipListSet
List      → CopyOnWriteArrayList（读多写少）
队列       → LinkedBlockingQueue / ConcurrentLinkedQueue
双端队列   → LinkedBlockingDeque
