# Java 反射与注解详解

---

## 一、Java 反射 (Reflection)

### 1.1 什么是反射

反射是 Java 提供的一种**运行时自省机制**，允许程序在运行时获取任何类的内部信息（构造器、方法、字段、注解等），并直接操作任意对象的内部属性及方法。

```
Java 程序运行阶段：
  源代码 (.java) ──javac──> 字节码 (.class) ──JVM──> 运行时对象

  反射操作的就是 .class 字节码文件加载到 JVM 后生成的 Class 对象
```

### 1.2 反射的核心类（位于 `java.lang.reflect` 包）

| 类名 | 用途 |
|------|------|
| `Class<?>` | 代表一个类或接口（位于 java.lang） |
| `Constructor<T>` | 代表类的构造方法 |
| `Method` | 代表类的方法 |
| `Field` | 代表类的字段/属性 |
| `Parameter` | 代表方法参数 |
| `Modifier` | 提供访问修饰符的解析 |
| `Array` | 动态创建和操作数组 |

### 1.3 获取 Class 对象的三种方式

```java
// 方式一：通过 Class.forName() — 最常用（类名已知字符串）
Class<?> clazz1 = Class.forName("com.example.User");

// 方式二：通过 .class 语法 — 编译时类型已知
Class<?> clazz2 = User.class;

// 方式三：通过对象的 getClass() — 已有实例
User user = new User();
Class<?> clazz3 = user.getClass();
```

### 1.4 反射常用操作示例

#### 1.4.1 获取类信息

```java
Class<?> clazz = User.class;

// 获取类名
String className = clazz.getName();              // com.example.User
String simpleName = clazz.getSimpleName();       // User

// 获取包名
Package pkg = clazz.getPackage();

// 获取修饰符
int mod = clazz.getModifiers();
boolean isPublic = Modifier.isPublic(mod);

// 获取父类
Class<?> superClass = clazz.getSuperclass();

// 获取接口
Class<?>[] interfaces = clazz.getInterfaces();
```

> **注意区别：**
> - `getFields()` — 获取所有 `public` 字段（含继承的）
> - `getDeclaredFields()` — 获取本类所有字段（不含继承的，但包括 private）
> - `getMethods()` — 获取所有 `public` 方法（含继承的）
> - `getDeclaredMethods()` — 获取本类所有方法（不含继承的）
> - 构造器同上规律

#### 1.4.2 操作字段

```java
Class<?> clazz = User.class;
Object obj = clazz.getDeclaredConstructor().newInstance();

Field field = clazz.getDeclaredField("name");

// 私有字段必须设置可访问
field.setAccessible(true);

// 读取字段值
Object value = field.get(obj);

// 设置字段值
field.set(obj, "张三");
```

#### 1.4.3 调用方法

```java
Class<?> clazz = User.class;
Object obj = clazz.getDeclaredConstructor().newInstance();

// 获取方法（方法名 + 参数类型）
Method method = clazz.getDeclaredMethod("setName", String.class);

// 私有方法必须设置可访问
method.setAccessible(true);

// 调用方法
method.invoke(obj, "张三");

// 获取返回值
Method getMethod = clazz.getDeclaredMethod("getName");
String name = (String) getMethod.invoke(obj);
```

#### 1.4.4 操作构造器

```java
Class<?> clazz = User.class;

// 调用无参构造
Object obj1 = clazz.getDeclaredConstructor().newInstance();

// 调用有参构造
Constructor<?> constructor = clazz.getDeclaredConstructor(String.class, int.class);
Object obj2 = constructor.newInstance("张三", 25);

// 调用私有构造器
Constructor<?> privateCon = clazz.getDeclaredConstructor(String.class);
privateCon.setAccessible(true);
Object obj3 = privateCon.newInstance("李四");
```

### 1.5 反射的优缺点

| 优点 | 缺点 |
|------|------|
| 运行时动态获取类信息，实现框架的通用性 | **性能较低** — 反射调用比直接调用慢（因动态解析） |
| 提高代码灵活性（Spring / MyBatis 核心依赖反射） | **安全限制** — 某些安全环境禁止反射 |
| 可以访问私有成员 | **破坏封装性** — 违反 OOP 原则 |
| 实现动态代理的基础 | **可维护性差** — 编译期无法发现错误 |

> **性能优化：** 如果频繁使用反射，可以缓存 `Method` / `Field` 对象（`setAccessible(true)` 后缓存），避免反复查找。

### 1.6 反射的应用场景

| 场景 | 说明 | 示例 |
|------|------|------|
| **框架 IOC** | 根据配置动态创建对象 | Spring IOC 容器 |
| **ORM 映射** | 将数据库字段映射到对象属性 | MyBatis / Hibernate |
| **动态代理** | 运行时生成代理类 | AOP 实现 |
| **序列化/反序列化** | 动态读写对象字段 | JSON / XML 转换 |
| **注解处理器** | 运行时读取和处理注解 | JUnit / Spring |
| **开发工具** | 类的结构分析 | IDE 的自动补全 |

---

## 二、Java 注解 (Annotation)

### 2.1 什么是注解

注解是 Java 5 引入的一种**元数据机制**，它本身不包含业务逻辑，而是为代码提供标记信息，供编译器或运行时框架解析使用。

```
格式：@AnnotationName [(参数)]
```

### 2.2 内置注解

| 注解 | 作用位置 | 说明 |
|------|----------|------|
| `@Override` | 方法 | 标识重写父类方法 |
| `@Deprecated` | 任意 | 标记已过时 |
| `@SuppressWarnings` | 任意 | 抑制编译器警告 |
| `@FunctionalInterface` | 接口 | 标识函数式接口（Java 8） |
| `@SafeVarargs` | 方法/构造器 | 抑制堆污染警告 |

### 2.3 元注解（注解的注解）

| 元注解 | 作用 |
|--------|------|
| `@Retention` | 注解保留策略 |
| `@Target` | 注解可放置的位置 |
| `@Documented` | 是否被 javadoc 记录 |
| `@Inherited` | 是否允许子类继承 |
| `@Repeatable` | 是否可重复标注（Java 8） |

#### @Retention 策略

```java
@Retention(RetentionPolicy.SOURCE)    // 源码级：编译后丢弃（如 @Override）
@Retention(RetentionPolicy.CLASS)     // 字节码级：保留到 .class，运行时不可获取（默认）
@Retention(RetentionPolicy.RUNTIME)   // 运行时级：JVM 加载后仍保留，可通过反射读取
```

#### @Target 位置

```java
@Target(ElementType.TYPE)              // 类、接口、枚举
@Target(ElementType.FIELD)             // 字段
@Target(ElementType.METHOD)            // 方法
@Target(ElementType.PARAMETER)         // 参数
@Target(ElementType.CONSTRUCTOR)       // 构造器
@Target(ElementType.LOCAL_VARIABLE)    // 局部变量
@Target(ElementType.ANNOTATION_TYPE)   // 注解类型
@Target(ElementType.PACKAGE)           // 包
@Target(ElementType.TYPE_PARAMETER)    // 类型参数（Java 8）
@Target(ElementType.TYPE_USE)          // 类型使用（Java 8）
```

### 2.4 自定义注解

```java
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD)
public @interface ExcelColumn {
    String name() default "";
    int order() default 0;
    boolean required() default false;
}
```

**注解元素支持的类型：**
- 基本类型（int, long, double 等）
- String
- Class
- 枚举
- 注解
- 以上类型的数组

### 2.5 反射读取注解

```java
// 读取类上的注解
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface TableName {
    String value();
}

// 使用
@TableName("t_user")
public class User { ... }

// 解析
Class<?> clazz = User.class;
if (clazz.isAnnotationPresent(TableName.class)) {
    TableName table = clazz.getAnnotation(TableName.class);
    String tableName = table.value();  // "t_user"
}
```

```java
// 读取字段上的注解
public class User {
    @ExcelColumn(name = "用户名", order = 1)
    private String username;

    @ExcelColumn(name = "年龄", order = 2)
    private int age;
}

// 解析
Field[] fields = User.class.getDeclaredFields();
for (Field field : fields) {
    if (field.isAnnotationPresent(ExcelColumn.class)) {
        ExcelColumn col = field.getAnnotation(ExcelColumn.class);
        System.out.println(field.getName() + " -> " + col.name());
    }
}
```

### 2.6 反射 + 注解的经典架构

```
Spring 框架中：
   @Component ──> 类注解，标记为 Bean
   @Autowired  ──> 字段/方法注解，自动注入
   @Transactional ──> 方法注解，事务管理

   Spring 启动时通过反射扫描包，读取注解信息，完成 Bean 的创建和注入。
```

### 2.7 注意事项

1. **只有 `@Retention(RUNTIME)` 的注解才能在运行时通过反射读取**
2. 注解本身不包含逻辑，需要配合反射或 APT 处理
3. 注解继承：父类注解如果标记了 `@Inherited`，子类可以继承；但接口上的注解不会被实现类继承
4. 注解的默认值不能为 null，但可以用空字符串或特殊值表示"未设置"

---

## 三、反射 + 注解 实战案例

### 3.1 简易 ORM 映射

```java
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
@interface Table {
    String value();
}

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD)
@interface Column {
    String value();
}

@Table("t_user")
class User {
    @Column("user_id")
    private Long id;
    @Column("user_name")
    private String name;
    @Column("user_age")
    private int age;
}

// 解析器
class EntityMapper {
    public static String toInsertSql(Object obj) throws Exception {
        Class<?> clazz = obj.getClass();
        Table table = clazz.getAnnotation(Table.class);
        StringBuilder sql = new StringBuilder("INSERT INTO " + table.value() + " (");

        Field[] fields = clazz.getDeclaredFields();
        StringBuilder values = new StringBuilder(") VALUES (");
        for (Field field : fields) {
            field.setAccessible(true);
            Column col = field.getAnnotation(Column.class);
            if (col != null) {
                sql.append(col.value()).append(", ");
                values.append("'").append(field.get(obj)).append("', ");
            }
        }
        sql.setLength(sql.length() - 2);
        values.setLength(values.length() - 2);
        sql.append(values).append(")");
        return sql.toString();
    }
}
```

---

## 附录：常用反射 API 速查表

```java
// === Class 对象 ===
Class.forName("全限定类名")
类.class
对象.getClass()

// === 构造器 ===
clazz.getConstructors()
clazz.getDeclaredConstructors()
clazz.getConstructor(参数类型...)
clazz.getDeclaredConstructor(参数类型...)
constructor.newInstance(参数...)

// === 方法 ===
clazz.getMethods()
clazz.getDeclaredMethods()
clazz.getMethod("方法名", 参数类型...)
clazz.getDeclaredMethod("方法名", 参数类型...)
method.invoke(对象, 参数...)

// === 字段 ===
clazz.getFields()
clazz.getDeclaredFields()
clazz.getField("字段名")
clazz.getDeclaredField("字段名")
field.set(对象, 值)
field.get(对象)
field.setAccessible(true)  // 绕过访问权限检查
