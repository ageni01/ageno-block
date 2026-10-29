---
title: Spring 详解：从手动装配到 XML、Bean 与容器原理
date: 2026-10-29
category:
  - Java
tag:
  - Spring
  - IoC
  - Bean
---

# Spring 详解：从手动装配到 XML、Bean 与容器原理

本系列按“遇到问题 → 自己实现 → 理解抽象 → 使用框架”的顺序学习。先读本文，再读 [MySQL 与 JDBC 封装](./02-jdbc-mysql.md)、[Spring MVC](./03-spring-mvc.md)，最后读 [事务与 MyBatis](./04-transactions-mybatis.md)。

<!-- more -->

## 1. 版本和边界

本文采用 Java 17、Spring Framework **6.2.19** 的 API 风格，Web 部分使用 Jakarta Servlet 与 Tomcat 10.1。选择 6.2 是为了与大量现有项目衔接，不表示它是最新主版本。Spring 7 的环境与依赖要求需另按官方迁移说明核对。全部 Spring 模块保持同一版本；Servlet 导包用 `jakarta.servlet`，不要混用旧教程的 `javax.servlet`。JDBC 的 `javax.sql.DataSource` 属于 Java SE，名字没有跟着 Servlet 改成 jakarta。

本系列没有使用 Spring Boot。Spring Framework 提供基础能力；Spring MVC 是其中的 Servlet Web 框架；Spring Boot 提供自动配置、启动与部署便利；MyBatis 负责 SQL 映射。它们各有职责。

## 2. 不用 Spring，程序怎么写

不使用框架也能写分层程序、依赖注入和事务。常见方式包括普通 Java 工厂、Servlet/JSP、JDBC、JNDI 数据源，以及 EJB 等技术。这里用一个简化演进说明设计动机，并不意味着历史上所有项目都经历了相同阶段。

最直接的代码：

```java
class UserService {
    private final MysqlUserRepository repository = new MysqlUserRepository();
}
```

问题是 Service 负责业务的同时，还决定了 Repository 的实现和创建方式。如果 Repository 又需要数据库地址、密码和连接工厂，创建逻辑会不断向上扩散。测试想换成内存实现，也要修改业务类。

第一步是抽接口并由构造器接收依赖：

```java
interface UserRepository {
    String findName(long id);
}

class UserService {
    private final UserRepository repository;
    UserService(UserRepository repository) { this.repository = repository; }
    String name(long id) { return repository.findName(id); }
}
```

第二步，在入口统一装配：

```java
UserRepository repository = id -> "测试用户";
UserService service = new UserService(repository);
System.out.println(service.name(1));
```

这已经是依赖注入（DI）。控制反转（IoC）体现为：业务对象把依赖的创建和选择交给外部。Spring 把这个装配入口扩展成一个可配置、可扩展的对象容器，还处理生命周期、作用域、代理等问题。[官方容器说明](https://docs.spring.io/spring-framework/reference/6.2/core/beans.html)

## 3. Spring 各部分提供什么

| 能力 | 解决的问题 | 典型入口 |
| --- | --- | --- |
| IoC/DI | 统一创建对象、连接依赖、管理生命周期 | ApplicationContext、BeanFactory |
| AOP | 把事务、审计等跨业务逻辑放到调用边界 | 代理、切面、拦截器链 |
| 事务 | 统一事务边界和传播规则 | PlatformTransactionManager、@Transactional |
| JDBC | 减少资源管理、结果映射和异常处理重复 | JdbcTemplate、DataSourceUtils |
| Web MVC | 把 HTTP 请求映射到业务方法与响应 | DispatcherServlet |
| 资源与配置 | 读取 classpath 资源、环境参数、配置文件 | Resource、Environment |
| 事件与国际化 | 容器内事件协作、消息文本管理 | ApplicationEventPublisher、MessageSource |
| 测试集成 | 为测试加载上下文、测试请求处理 | Spring Test、MockMvc |

Spring 的优势来自这些机制共享同一个容器与扩展点，不是因为 DI 或代理只能由 Spring 实现。

## 4. Bean 到底是什么

Bean 是交给 Spring 容器管理的对象。普通 Java 对象可以成为 Bean，不要求继承特殊父类，也不等于数据库实体。

- `new UserService(...)` 创建的是普通对象；容器不会自动替它执行完整的管理流程。
- XML 的 `<bean>`、配置类的 `@Bean`、扫描到的 `@Component` 都能提供 Bean 定义。
- BeanDefinition 是“怎么创建”的元数据，Bean 实例才是真正运行的对象。
- `BeanFactory` 是核心工厂接口；`ApplicationContext` 在其上提供事件、资源、国际化和更完整的启动流程。

默认 singleton 表示同一个容器内、同一个 Bean 定义通常对应一个实例，不是整个 JVM 只能有一个对象。singleton 不保证线程安全：不要在共享 Service 中存当前请求的用户名、Connection 等可变状态。

## 5. 第一份可以运行的容器配置

创建 Maven 项目，在 `pom.xml` 中放入：

```xml
<project xmlns="http://maven.apache.org/POM/4.0.0"
         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
  <groupId>demo</groupId><artifactId>spring-study</artifactId><version>1.0</version>
  <properties>
    <maven.compiler.release>17</maven.compiler.release>
    <project.build.sourceEncoding>UTF-8</project.build.sourceEncoding>
    <spring.version>6.2.19</spring.version>
  </properties>
  <dependencies>
    <dependency>
      <groupId>org.springframework</groupId><artifactId>spring-context</artifactId>
      <version>${spring.version}</version>
    </dependency>
  </dependencies>
  <build><plugins>
    <plugin>
      <groupId>org.apache.maven.plugins</groupId><artifactId>maven-compiler-plugin</artifactId>
      <version>3.13.0</version>
    </plugin>
  </plugins></build>
</project>
```

下面每个 public 类放入独立同名文件，目录为 `src/main/java/demo/`：

```java
package demo;
public class GreetingService {
    private final String prefix;
    public GreetingService(String prefix) { this.prefix = prefix; }
    public String hello(String name) { return prefix + name; }
}
```

### 5.1 XML 配置：把装配关系写在文件里

在 `src/main/resources/applicationContext.xml` 中写：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xsi:schemaLocation="http://www.springframework.org/schema/beans https://www.springframework.org/schema/beans/spring-beans.xsd">
  <bean id="greetingService" class="demo.GreetingService">
    <constructor-arg value="你好，"/>
  </bean>
</beans>
```

```java
package demo;
import org.springframework.context.support.ClassPathXmlApplicationContext;
public class XmlMain {
    public static void main(String[] args) {
        try (var context = new ClassPathXmlApplicationContext("applicationContext.xml")) {
            System.out.println(context.getBean(GreetingService.class).hello("ageno"));
        }
    }
}
```

用 IDE 运行 `XmlMain`，预期输出 `你好，ageno`。`class` 必须是完整类名。`value` 是字面值，`ref` 是引用其他 Bean：

```xml
<!-- 示意：前提是这两个类与构造器已经存在 -->
<bean id="userRepository" class="demo.MysqlUserRepository"/>
<bean id="userService" class="demo.UserService">
  <constructor-arg ref="userRepository"/>
</bean>
```

`<property name="url" value="..."/>` 则调用 `setUrl(...)`，不是任意修改私有字段。集合用 `<list>`、`<set>`、`<map>`；其他配置文件用 `<import resource="database.xml"/>`。只有需要时才引入相应命名空间，不必复制一大份万能 XML。

### 5.2 Java 配置：通过方法声明 Bean

```java
package demo;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
@Configuration(proxyBeanMethods = false)
public class AppConfig {
    @Bean
    public GreetingService greetingService() {
        return new GreetingService("你好，");
    }
}
```

```java
package demo;
import org.springframework.context.annotation.AnnotationConfigApplicationContext;
public class JavaMain {
    public static void main(String[] args) {
        try (var context = new AnnotationConfigApplicationContext(AppConfig.class)) {
            System.out.println(context.getBean(GreetingService.class).hello("ageno"));
        }
    }
}
```

`@Bean` 方法名默认就是 Bean 名。返回值由容器管理，特别适合第三方类与需要自定义构造过程的对象。Bean 之间优先用方法参数注入：

```java
@Bean
public UserService userService(UserRepository repository) {
    return new UserService(repository);
}
```

上面是装配示意，需要另定义 public 的 UserRepository 与 UserService。使用 `proxyBeanMethods = false` 时直接调用另一个 `@Bean` 方法是普通 Java 调用，可能创建额外对象；方法参数注入能清楚表达容器依赖。

### 5.3 扫描注解：让应用类声明角色

`@Component` 表示通用组件，`@Service` 表示业务组件，`@Repository` 表示持久化组件，`@Controller` 表示 Web 控制器。它们需要配合 `@ComponentScan` 或 XML 的 `<context:component-scan>` 才会被发现。

单一构造器通常不必写 `@Autowired`。同类型有多个 Bean 时使用 `@Qualifier("名称")` 或指定 `@Primary`，不要依赖模糊猜测。

```java
@Configuration
@ComponentScan("demo.service")
class ScanConfig { }
```

XML 中扫描需要声明 `xmlns:context="http://www.springframework.org/schema/context"` 并在 schemaLocation 添加对应的 `spring-context.xsd`。XML 和 Java 配置可以共存，Java 侧用 `@ImportResource("classpath:applicationContext.xml")`；避免同一个类既扫描又手动注册出两套实例。

## 6. 外部配置、作用域和生命周期

数据库密码等配置从环境变量或外部配置读取。普通 Spring 不会因为放了 `application.yml` 就自动按 Boot 的规则装配数据库。Java 配置中可以注入 `Environment` 并调用 `getRequiredProperty("DB_URL")`。

| 作用域 | 实例范围 | 注意 |
| --- | --- | --- |
| singleton | 每容器每定义 | 默认，注意并发 |
| prototype | 每次向容器请求 | 注入 singleton 后不会每次调用自动更新 |
| request | 每次 HTTP 请求 | 需要 Web 上下文 |
| session | 每个 HTTP 会话 | 注意内存与并发 |

对 prototype 动态获取可注入 `ObjectProvider<T>`。初始化与销毁可以用 `@Bean(initMethod="start", destroyMethod="close")`，或 XML 的 `init-method`/`destroy-method`。容器不会自动替 prototype 完整管理销毁。资源拥有者负责关闭资源，避免多个组件重复关闭共享数据源。

## 7. 容器的独特能力怎样设计出来

这是便于理解的机制模型，不是 Spring 源码的完整重写：

1. 读取 XML、配置类或扫描结果，形成 BeanDefinition 注册表。
2. 根据构造器和依赖图决定创建顺序，递归解析需要的 Bean。
3. 调用构造器，注入属性，执行相关生命周期回调。
4. 允许 BeanPostProcessor 在初始化前后处理实例，例如返回代理。
5. 把最终对外暴露的对象存入相应作用域，结束时按规则销毁。

核心思想是“元数据 + 工厂 + 缓存 + 扩展点”。`BeanFactoryPostProcessor` 修改定义，`BeanPostProcessor` 处理实例；两者作用阶段不同。

一个最小手写工厂可以是 `Map<Class<?>, Supplier<?>>` 加单例缓存。继续增加构造器解析、循环依赖检测、生命周期和代理，复杂度就迅速增加。Spring 的价值在于把这些边界处理系统化。构造器循环依赖通常说明职责需要重新划分，不应把容器的某些循环依赖处理机制当作常规设计方案。

## 8. AOP：为什么一个普通方法会多出事务和日志

将 Service 包上一层代理：

```text
调用者 → 代理：前置处理 → 目标方法 → 代理：提交/异常处理 → 调用者
```

接口代理可基于 JDK 动态代理，类代理可基于子类。Spring AOP 默认围绕方法执行工作，不是任意代码位置都能拦截。`final` 方法无法被子类代理覆盖，私有方法也不是常规代理入口。

`this.otherMethod()` 没有再次经过外部代理，因此默认代理模式下自调用的 `@Transactional` 常常不生效。注解只是元数据，还需要启用相关基础设施、注册 Bean，并通过代理调用。

## 9. 常见问题

- 找不到 Bean：检查扫描路径、配置类是否加载、是否只是自己 new 了对象。
- 多个候选：明确 Qualifier 或 Primary，不要随意删掉必要实现。
- 配置值没读取：检查属性源，区分普通 Spring 与 Boot。
- 单例数据串请求：移除共享对象中的请求状态；局部变量通常更适合。
- XML 与 Java 配置同时生效出重复对象：确定唯一装配入口。

## 10. 参考与下一步

以上示例以教学为目的；代码分为可运行最小容器例子与明确标注的局部装配片段。数据库运行环境在下一篇设置。

- [Spring 6.2 IoC 容器](https://docs.spring.io/spring-framework/reference/6.2/core/beans.html)
- [Java 配置](https://docs.spring.io/spring-framework/reference/6.2/core/beans/java.html)
- [AOP 代理机制](https://docs.spring.io/spring-framework/reference/6.2/core/aop/proxying.html)
- 下一篇：[从 JDBC 手动连接 MySQL 到自己的数据库访问封装](./02-jdbc-mysql.md)
