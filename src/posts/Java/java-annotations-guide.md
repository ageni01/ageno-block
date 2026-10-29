---
title: "Java 注解详解"

category:
  - Java
tag:
  - Java
---
# Java 注解详解

## 一、JDK 内置注解

### 1. 基础注解

```java
// 1. @Override - 标记方法重写父类方法
public class Parent {
    public void sayHello() {
        System.out.println("Hello from parent");
    }
}

public class Child extends Parent {
    @Override  // 编译器会检查是否真的重写了父类方法
    public void sayHello() {
        System.out.println("Hello from child");
    }
}

// 2. @Deprecated - 标记过时的 API
public class OldApi {
    @Deprecated  // 调用此方法编译器会给出警告
    public void oldMethod() { }

    // 可以加 since/forRemoval 说明版本
    @Deprecated(since = "2.0", forRemoval = true)
    public void legacyMethod() { }
}

// 3. @SuppressWarnings - 压制编译器警告
public class WarningDemo {
    @SuppressWarnings("unchecked")
    public void unsafeCast() {
        List rawList = new ArrayList();
        List<String> strList = rawList;
    }

    @SuppressWarnings({"unchecked", "rawtypes"})
    public void multipleWarnings() {
        List list = new ArrayList();
        list.add("test");
    }
}

// 4. @FunctionalInterface - 标记函数式接口
@FunctionalInterface
public interface MyFunction {
    void execute();
    default void log() { System.out.println("logging"); }
    static void staticMethod() {}
}

// 5. @SafeVarargs - 抑制堆污染警告
public class SafeVarargsDemo {
    @SafeVarargs
    public static <T> void printAll(T... args) {
        for (T arg : args) System.out.println(arg);
    }
}
```

### 2. 元注解（注解的注解）

```java
import java.lang.annotation.*;

// @Target - 指定注解修饰的目标
@Target({
    ElementType.TYPE,            // 类、接口、枚举
    ElementType.METHOD,          // 方法
    ElementType.FIELD,           // 字段
    ElementType.PARAMETER,       // 参数
    ElementType.CONSTRUCTOR,     // 构造方法
    ElementType.LOCAL_VARIABLE,  // 局部变量
    ElementType.ANNOTATION_TYPE, // 注解类型
    ElementType.PACKAGE,         // 包
    ElementType.TYPE_PARAMETER,  // 类型参数 (Java 8+)
    ElementType.TYPE_USE         // 类型使用 (Java 8+)
})
@Retention(RetentionPolicy.RUNTIME) // SOURCE|CLASS|RUNTIME
@Documented
@Inherited  // 子类可继承父类的该注解（仅对类生效）
public @interface MyAnnotation { }

// @Repeatable - 允许重复使用同一注解
@Repeatable(Schedules.class)
@interface Schedule {
    String dayOfWeek();
    String time();
}
@interface Schedules {
    Schedule[] value();
}

@Schedule(dayOfWeek = "Mon", time = "10:00")
@Schedule(dayOfWeek = "Wed", time = "14:00")
public class MeetingPlan { }
```

---

## 二、Spring Boot / Spring 框架注解

### 1. 核心注解

```java
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

// @SpringBootApplication = @Configuration + @EnableAutoConfiguration + @ComponentScan
@SpringBootApplication
public class Application {
    public static void main(String[] args) {
        SpringApplication.run(Application.class, args);
    }
}
```

```java
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Bean;

@Configuration  // 标记该类为配置类
public class AppConfig {

    @Bean  // 将返回值注册为 Spring Bean，默认名称 = 方法名
    public UserService userService() { return new UserServiceImpl(); }

    @Bean(name = "myDataSource")
    @Lazy  // 懒加载
    @Scope("prototype")  // 每次获取创建新实例（默认 singleton）
    public DataSource dataSource() { return new HikariDataSource(); }

    @Bean(initMethod = "init", destroyMethod = "cleanup")
    public MyService myService() { return new MyService(); }
}
```

```java
@Configuration
@Import({DatabaseConfig.class, CacheConfig.class})  // 导入其他配置类
@ImportResource("classpath:beans.xml")              // 导入 XML 配置
@PropertySource("classpath:application.properties") // 加载属性文件
public class RootConfig { }
```

### 2. 组件扫描与 Bean 注册

```java
import org.springframework.stereotype.Component;
import org.springframework.stereotype.Service;
import org.springframework.stereotype.Repository;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RestController;

@Component // 通用组件，默认 bean 名称：类名首字母小写
public class UserServiceImpl implements UserService { }

@Service   // 业务层
public class OrderService { }

@Repository // 数据访问层（自动转换持久层异常）
public class UserRepository { }

@Controller // MVC 控制器（返回视图）
public class HomeController { }

@RestController // = @Controller + @ResponseBody（返回 JSON/XML）
public class ApiController { }
```

### 3. 依赖注入

```java
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.context.annotation.Primary;
import javax.annotation.Resource;

@Service
public class OrderService {

    @Autowired  // 按类型注入（默认 required=true）
    private UserRepository userRepository;

    @Autowired(required = false) // 找不到时保留 null
    private CacheService cacheService;

    @Autowired
    @Qualifier("mysqlDataSource") // 按名称注入
    private DataSource dataSource;

    @Bean @Primary // 多个同类型时优先注入
    public DataSource mainDataSource() { return new HikariDataSource(); }

    @Resource(name = "userRepository") // JSR-250：先按名称再按类型
    private UserRepository repo;

    // 构造器注入（推荐方式）
    private final PaymentService paymentService;
    public OrderService(PaymentService paymentService) {
        this.paymentService = paymentService;
    }

    @Autowired
    public void setLogger(Logger logger) { this.logger = logger; } // Setter 注入
}
```

### 4. 属性注入与配置绑定

```java
import org.springframework.beans.factory.annotation.Value;

@Service
public class ConfigService {

    @Value("${app.name}")                     // 从配置文件读取
    private String appName;

    @Value("${app.version:1.0.0}")            // 带默认值
    private String version;

    @Value("${server.port:8080}")
    private int port;

    @Value("#{systemProperties['user.home']}") // SpEL 表达式
    private String userHome;

    @Value("classpath:data.txt")              // 注入 Resource
    private Resource dataFile;

    @Value("${app.servers}")                  // 注入数组/列表
    private String[] servers;
}
```

```java
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.validation.annotation.Validated;
import javax.validation.constraints.*;

@ConfigurationProperties(prefix = "app.datasource") // 批量绑定配置
@Component
public class DataSourceProperties {

    private String url;
    private String username;
    private String password;
    private int maxActive = 10;

    // 必须提供 getter/setter
}
```

```java
@ConfigurationProperties(prefix = "app.mail")
@Validated  // 开启校验
@Component
public class MailProperties {

    @NotBlank(message = "host 不能为空")
    private String host;

    @Min(1) @Max(65535)
    private int port = 25;

    @Email
    private String from;
}
```

### 5. Web 层注解

```java
import org.springframework.web.bind.annotation.*;
import org.springframework.http.ResponseEntity;
import org.springframework.http.HttpStatus;

@RestController
@RequestMapping("/api/users")
public class UserController {

    @GetMapping
    public List<User> getAll() { return userService.findAll(); }

    @GetMapping("/{id}")                                    // GET /api/users/123
    public User getById(@PathVariable Long id) {            // 路径参数
        return userService.findById(id);
    }

    @GetMapping("/search")                                  // GET /api/users/search?name=xx&page=1
    public List<User> search(
            @RequestParam String name,                      // 必须参数
            @RequestParam(defaultValue = "1") int page,     // 可选带默认值
            @RequestParam(required = false) String sort) {  // 可选参数
        return userService.search(name, page, sort);
    }

    @PostMapping                                            // POST /api/users
    @ResponseStatus(HttpStatus.CREATED)                     // 201
    public User create(@RequestBody @Valid User user) {     // 请求体 JSON -> 对象
        return userService.create(user);
    }

    @PutMapping("/{id}")                                    // PUT /api/users/123
    public User update(@PathVariable Long id, @RequestBody User user) {
        return userService.update(id, user);
    }

    @DeleteMapping("/{id}")                                 // DELETE /api/users/123
    @ResponseStatus(HttpStatus.NO_CONTENT)                  // 204
    public void delete(@PathVariable Long id) {
        userService.delete(id);
    }

    @PatchMapping("/{id}")                                  // PATCH /api/users/123
    public User partialUpdate(@PathVariable Long id,
                              @RequestBody Map<String, Object> updates) {
        return userService.partialUpdate(id, updates);
    }

    @RequestMapping(
        value = "/query",
        method = {RequestMethod.GET, RequestMethod.POST},
        params = "admin=true",
        headers = "X-API-Version=1",
        consumes = "application/json",
        produces = "application/json;charset=UTF-8")
    public User query() { return userService.query(); }

    @GetMapping("/header")
    public String getHeader(
            @RequestHeader("User-Agent") String userAgent,
            @RequestHeader Map<String, String> allHeaders) {
        return userAgent;
    }

    @GetMapping("/cookie")
    public String getCookie(@CookieValue("sessionId") String sessionId) {
        return sessionId;
    }

    @PostMapping("/form")
    public String handleForm(@ModelAttribute UserForm form) { // 请求参数绑定到对象
        return "success";
    }

    @GetMapping("/response")
    public ResponseEntity<User> customResponse() {
        return ResponseEntity.ok()
                .header("X-Custom", "value")
                .body(userService.findById(1L));
    }
}
```

### 6. 异常处理

```java
import org.springframework.web.bind.annotation.*;

@ControllerAdvice // 全局控制器增强
@ResponseBody
public class GlobalExceptionHandler {

    @ExceptionHandler(ResourceNotFoundException.class)
    @ResponseStatus(HttpStatus.NOT_FOUND)
    public ErrorResponse handleNotFound(ResourceNotFoundException ex) {
        return new ErrorResponse(404, ex.getMessage());
    }

    @ExceptionHandler(ValidationException.class)
    @ResponseStatus(HttpStatus.BAD_REQUEST)
    public ErrorResponse handleValidation(ValidationException ex) {
        return new ErrorResponse(400, ex.getMessage());
    }

    @ExceptionHandler(Exception.class) // 兜底处理
    @ResponseStatus(HttpStatus.INTERNAL_SERVER_ERROR)
    public ErrorResponse handleGeneric(Exception ex) {
        return new ErrorResponse(500, "服务器内部错误");
    }
}

// 限制作用范围
@ControllerAdvice(basePackages = "com.example.api")
@ControllerAdvice(assignableTypes = UserController.class)

// 在单个控制器中处理异常
@RestController
public class UserController {
    @ExceptionHandler(IllegalArgumentException.class)
    @ResponseStatus(HttpStatus.BAD_REQUEST)
    public ErrorResponse handleIllegalArg(IllegalArgumentException ex) {
        return new ErrorResponse(400, ex.getMessage());
    }
}
```

### 7. 事务管理

```java
import org.springframework.transaction.annotation.Transactional;
import org.springframework.transaction.annotation.Isolation;
import org.springframework.transaction.annotation.Propagation;

@Service
public class OrderService {

    @Transactional // 默认只对 RuntimeException 回滚
    public void createOrder(Order order) {
        orderRepository.save(order);
        inventoryService.deductStock(order.getProductId(), order.getQuantity());
        // 任一操作异常，事务自动回滚
    }

    @Transactional(
        propagation = Propagation.REQUIRED,            // 支持当前事务，没有则新建（默认）
        isolation = Isolation.READ_COMMITTED,           // 读已提交
        timeout = 30,
        readOnly = false,
        rollbackFor = Exception.class,                  // 指定回滚异常
        noRollbackFor = IllegalArgumentException.class) // 指定不回滚异常
    public void processPayment(Payment payment) throws Exception {
        paymentRepository.save(payment);
        notificationService.sendNotification(payment);
    }

    // 传播行为
    // REQUIRED:    支持当前事务，没有则新建（默认）
    // SUPPORTS:    支持当前事务，没有则以非事务执行
    // MANDATORY:   必须有当前事务，否则抛异常
    // REQUIRES_NEW:总是新建事务，挂起当前事务
    // NOT_SUPPORTED:以非事务执行，挂起当前事务
    // NEVER:       以非事务执行，有事务则抛异常
    // NESTED:      嵌套事务（JDBC Savepoint）

    // 隔离级别
    // DEFAULT:          数据库默认
    // READ_UNCOMMITTED: 读未提交（可能脏读）
    // READ_COMMITTED:   读已提交（避免脏读）
    // REPEATABLE_READ:  可重复读
    // SERIALIZABLE:     串行化（性能最差）

    @Transactional(readOnly = true) // 查询设为只读（性能优化）
    public List<Order> findAll() { return orderRepository.findAll(); }
}
```

### 8. AOP 面向切面编程

```java
import org.aspectj.lang.annotation.*;
import org.aspectj.lang.ProceedingJoinPoint;
import org.springframework.stereotype.Component;

@Aspect
@Component
public class LoggingAspect {

    // @Pointcut - 定义切点
    @Pointcut("execution(* com.example.service.*.*(..))")
    public void serviceLayer() {}

    @Pointcut("@annotation(GetMapping)")
    public void getMappings() {}

    @Pointcut("bean(userService)")
    public void userServiceBean() {}

    @Pointcut("serviceLayer() && !getMappings()") // && || ! 组合
    public void combined() {}

    @Before("serviceLayer()") // 方法执行前
    public void beforeService(JoinPoint jp) {
        log.info("调用方法: {}", jp.getSignature().getName());
    }

    @AfterReturning(pointcut = "serviceLayer()", returning = "result") // 正常返回后
    public void afterReturning(JoinPoint jp, Object result) {
        log.info("返回: {}", result);
    }

    @AfterThrowing(pointcut = "serviceLayer()", throwing = "ex") // 抛出异常后
    public void afterThrowing(JoinPoint jp, Exception ex) {
        log.error("异常: {}", ex.getMessage());
    }

    @After("serviceLayer()") // finally 块
    public void after(JoinPoint jp) {
        log.info("方法执行完毕");
    }

    @Around("@annotation(LogExecutionTime)") // 环绕通知
    public Object measureTime(ProceedingJoinPoint jp) throws Throwable {
        long start = System.currentTimeMillis();
        try { return jp.proceed(); }
        finally {
            log.info("耗时: {}ms", System.currentTimeMillis() - start);
        }
    }
}
```

### 9. 异步与定时任务

```java
import org.springframework.scheduling.annotation.*;
import org.springframework.stereotype.Component;
import java.util.concurrent.CompletableFuture;

@Service
public class AsyncService {

    @Async // 需在配置类加 @EnableAsync
    public void sendNotification(User user) {
        Thread.sleep(2000);
        System.out.println("通知发送完成");
    }

    @Async
    public CompletableFuture<User> findUserAsync(Long id) {
        User user = userRepository.findById(id).orElse(null);
        return CompletableFuture.completedFuture(user);
    }
}

@Component
@EnableScheduling
public class ScheduledTasks {

    @Scheduled(fixedRate = 5000)         // 每 5 秒（从上一次开始计时）
    public void reportTime() {
        System.out.println(LocalDateTime.now());
    }

    @Scheduled(fixedDelay = 5000)        // 上次执行完后延迟 5 秒
    public void processQueue() { }

    @Scheduled(fixedDelay = 5000, initialDelay = 10000) // 启动后等 10 秒
    public void delayedTask() { }

    @Scheduled(cron = "0 0 3 * * ?")     // 每天凌晨 3 点
    @Scheduled(cron = "0 */5 * * * ?")   // 每 5 分钟
    @Scheduled(cron = "0 0 9-18 * * MON-FRI") // 工作日 9-18 点每小时
    public void cronTask() { }
    // Cron: 秒 分 时 日 月 周
}
```

### 10. 缓存

```java
import org.springframework.cache.annotation.*;

@Service
@CacheConfig(cacheNames = "users") // 类级别默认缓存名称
public class UserServiceImpl implements UserService {

    @Cacheable // 缓存结果：先查缓存，没有则执行方法并缓存
    public User findById(Long id) {
        return userRepository.findById(id).orElse(null);
    }

    @Cacheable(
        cacheNames = "users",
        key = "#id",                     // SpEL 表达式
        keyGenerator = "customKeyGenerator",
        unless = "#result == null",      // 结果为 null 时不缓存
        condition = "#id > 0",
        sync = true)                     // 同步缓存（防击穿）
    public User findWithCache(Long id) { }

    @CachePut(cacheNames = "users", key = "#user.id") // 执行方法并更新缓存
    public User update(User user) { return userRepository.save(user); }

    @CacheEvict(cacheNames = "users", key = "#id") // 清除指定缓存
    public void delete(Long id) { userRepository.deleteById(id); }

    @CacheEvict(cacheNames = "users", allEntries = true) // 清除所有
    public void clearAllCache() { }

    @Caching( // 组合多个缓存操作
        cacheable = @Cacheable(cacheNames = "users", key = "#id"),
        put = @CachePut(cacheNames = "cache2", key = "#id"),
        evict = @CacheEvict(cacheNames = "cache3", key = "#id"))
    public User complexCache(Long id) { return null; }
}
```

### 11. 校验注解

```java
import javax.validation.constraints.*;
import javax.validation.Valid;
import org.springframework.validation.annotation.Validated;

public class UserCreateRequest {

    @NotBlank(message = "用户名不能为空")
    @Size(min = 2, max = 20)
    private String username;

    @NotBlank @Length(min = 6, max = 30)
    private String password;

    @Email @NotBlank
    private String email;

    @NotNull @Min(1) @Max(150)
    private Integer age;

    @Pattern(regexp = "^1[3-9]\\d{9}$", message = "手机号格式不正确")
    private String phone;

    @Positive   @PositiveOrZero
    @Negative   @NegativeOrZero
    private Long categoryId;

    @DecimalMin("0.01") @DecimalMax("99999.99")
    private BigDecimal price;

    @Past   @Future
    private LocalDateTime eventTime;

    @NotEmpty
    private List<@NotBlank String> tags;

    @Valid  // 级联校验嵌套对象
    private Address address;
}

// 分组校验
@Service
@Validated
public class UserService {
    public void create(@Validated(Group.Create.class) User user) { }
    public void update(@Validated(Group.Update.class) User user) { }
}

public interface Group {
    interface Create {}
    interface Update {}
}

public class User {
    @NotNull(groups = Group.Update.class)
    @Null(groups = Group.Create.class)
    private Long id;

    @NotBlank(groups = {Group.Create.class, Group.Update.class})
    private String name;
}
```

---

## 三、自定义注解

```java
import java.lang.annotation.*;

@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface LogAnnotation {

    String value() default "";
    String module() default "";
    int level() default 1;
    Class<?> clazz() default Object.class;
    String[] tags() default {};
    Status status() default Status.INFO;
    // @LogAnnotation("登录") 等价于 @LogAnnotation(value = "登录")
}

public enum Status { INFO, WARN, ERROR }

// 使用自定义注解
@RestController
public class DemoController {
    @LogAnnotation(value = "用户登录", module = "auth",
                   level = 2, tags = {"auth","login"}, status = Status.INFO)
    @PostMapping("/login")
    public String login(@RequestBody LoginRequest req) { return "success"; }
}

// 反射读取注解
public class AnnotationProcessor {
    public static void parseAnnotation(Class<?> clazz) {
        for (Method method : clazz.getDeclaredMethods()) {
            LogAnnotation ann = method.getAnnotation(LogAnnotation.class);
            if (ann != null) {
                System.out.println("值: " + ann.value());
                System.out.println("级别: " + ann.level());
            }
        }
    }
}
```

---

## 四、常用注解速查表

| 注解 | 来源 | 作用 |
|------|------|------|
| `@Override` | JDK | 检查方法重写 |
| `@Deprecated` | JDK | 标记过时 |
| `@SuppressWarnings` | JDK | 压制警告 |
| `@FunctionalInterface` | JDK | 函数式接口 |
| `@SafeVarargs` | JDK | 堆污染抑制 |
| `@SpringBootApplication` | Spring Boot | 启动类入口 |
| `@Configuration` | Spring | 配置类 |
| `@Bean` | Spring | 注册 Bean |
| `@Component` / `@Service` / `@Repository` / `@Controller` | Spring | 组件注册 |
| `@RestController` | Spring MVC | REST 控制器 |
| `@Autowired` | Spring | 依赖注入 |
| `@Qualifier` | Spring | 指定 Bean 名称 |
| `@Value` | Spring | 属性注入 |
| `@ConfigurationProperties` | Spring Boot | 批量属性绑定 |
| `@RequestMapping` 系列 | Spring MVC | 请求映射 |
| `@PathVariable` / `@RequestParam` / `@RequestBody` | Spring MVC | 参数绑定 |
| `@Transactional` | Spring | 声明式事务 |
| `@Async` / `@Scheduled` | Spring | 异步/定时 |
| `@Cacheable` / `@CacheEvict` / `@CachePut` | Spring | 缓存 |
| `@Valid` / `@Validated` | JSR / Spring | 参数校验 |
| `@ControllerAdvice` / `@ExceptionHandler` | Spring | 异常处理 |
| `@Aspect` / `@Before` / `@Around` | AspectJ | AOP 切面 |
| `@NotNull` / `@NotBlank` / `@Size` / `@Min` / `@Max` / `@Email` | JSR | Bean 校验 |
