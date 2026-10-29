---
title: 从自写 JDBC 到 Spring 事务、JdbcTemplate 与 MyBatis
date: 2026-10-29
category:
  - Java
tag:
  - Spring
  - 事务
  - MyBatis
---

# 从自写 JDBC 到 Spring 事务、JdbcTemplate 与 MyBatis

前面已经自己封装了 MySQL 连接与 SQL 执行。现在逐层替换重复代码：先标准化连接工厂，再使用 Spring JDBC 与事务，最后才引入 MyBatis。

<!-- more -->

## 1. 对照：第三方组件究竟替换什么

| 自己实现的部分 | 成熟组件 | 仍需自己负责 |
| --- | --- | --- |
| MysqlConnectionFactory | DataSource 实现、连接池 | 数据库地址、凭据与容量 |
| JdbcExecutor | JdbcTemplate | SQL、业务结果映射 |
| 手动 begin/commit/rollback | Spring 事务管理器 | 事务边界、隔离与业务一致性 |
| Repository 中 SQL 与映射 | MyBatis Mapper | SQL 设计、索引、业务语义 |
| 手动组装依赖 | Spring 容器 | 模块职责和依赖关系 |

MyBatis 并不是连接池，Spring 也不是数据库。引入框架之后仍要理解 Connection 和事务。

## 2. 先把连接入口改成 DataSource

本篇在第一篇容器 Maven 项目上增加与 spring-context 同版本的模块：

```xml
<dependency>
  <groupId>org.springframework</groupId><artifactId>spring-jdbc</artifactId>
  <version>${spring.version}</version>
</dependency>
```

保留第二篇 MySQL 驱动、环境变量、app_user 表和 User record。新代码放 `demo.tx`，不要同时加载旧 DatabaseConfig。以下用 DriverManagerDataSource 展示标准接口，它没有连接池，仅适合教学和测试；生产需要连接池实现。[Spring 连接管理](https://docs.spring.io/spring-framework/reference/6.2/data-access/jdbc/connections.html)

```java
package demo.tx;
import javax.sql.DataSource;
import org.springframework.context.annotation.*;
import org.springframework.core.env.Environment;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.datasource.*;
import org.springframework.transaction.PlatformTransactionManager;
import org.springframework.transaction.annotation.EnableTransactionManagement;
@Configuration(proxyBeanMethods = false)
@EnableTransactionManagement
public class TxConfig {
    @Bean
    public DataSource dataSource(Environment env) {
        return new DriverManagerDataSource(env.getRequiredProperty("DB_URL"),
            env.getRequiredProperty("DB_USER"), env.getRequiredProperty("DB_PASSWORD"));
    }
    @Bean
    public JdbcTemplate jdbcTemplate(DataSource dataSource) {
        return new JdbcTemplate(dataSource);
    }
    @Bean
    public PlatformTransactionManager transactionManager(DataSource dataSource) {
        return new DataSourceTransactionManager(dataSource);
    }
    @Bean
    public UserStore userStore(JdbcTemplate jdbc) { return new UserStore(jdbc); }
    @Bean
    public RegistrationService registrationService(UserStore store) {
        return new RegistrationService(store);
    }
}
```

## 3. JdbcTemplate 帮我们省掉什么

```java
package demo.tx;
import demo.User;
import org.springframework.jdbc.core.JdbcTemplate;
import java.util.Optional;
public class UserStore {
    private final JdbcTemplate jdbc;
    public UserStore(JdbcTemplate jdbc) { this.jdbc = jdbc; }
    public int add(String name) {
        return jdbc.update("INSERT INTO app_user(name) VALUES (?)", name);
    }
    public Optional<User> find(long id) {
        return jdbc.query("SELECT id, name FROM app_user WHERE id = ?",
            (rs, rowNum) -> new User(rs.getLong("id"), rs.getString("name")), id)
            .stream().findFirst();
    }
    public int count(String name) {
        return jdbc.queryForObject(
            "SELECT COUNT(*) FROM app_user WHERE name = ?", Integer.class, name);
    }
}
```

SQL 和 RowMapper 仍由我们提供，Template 负责资源、执行与异常转换，并与 Spring 事务管理协作。它会把 SQLException 转成 DataAccessException 体系，不需要每层都声明受检 SQL 异常。

## 4. 通过失败案例验证声明式事务

```java
package demo.tx;
import org.springframework.transaction.annotation.Transactional;
public class RegistrationService {
    private final UserStore users;
    public RegistrationService(UserStore users) { this.users = users; }
    @Transactional
    public void registerThenFail(String name) {
        users.add(name);
        throw new IllegalStateException("模拟后续业务失败");
    }
}
```

```java
package demo.tx;
import org.springframework.context.annotation.AnnotationConfigApplicationContext;
public class TxMain {
    public static void main(String[] args) {
        try (var context = new AnnotationConfigApplicationContext(TxConfig.class)) {
            var store = context.getBean(UserStore.class);
            String name = "tx-check-" + java.util.UUID.randomUUID();
            int before = store.count(name);
            try {
                context.getBean(RegistrationService.class).registerThenFail(name);
            } catch (IllegalStateException expected) {
                System.out.println(expected.getMessage());
            }
            int after = store.count(name);
            if (before != after) throw new AssertionError("回滚失败");
            System.out.println("回滚验证通过");
        }
    }
}
```

这要求 MySQL 表使用支持事务的存储引擎，例如默认 InnoDB。通过容器获取 Service 并从外部调用，才会经过事务代理。

### 4.1 实现机制

代理进入方法前，由事务管理器按传播行为获取或新建事务，把 JDBC 连接与当前执行线程的事务上下文关联。JdbcTemplate 通过 DataSourceUtils 获取连接，因此能使用同一事务连接。方法成功则提交，符合回滚规则的异常则回滚，最后清理资源。

这解释了为什么第二篇的 `DriverManager.getConnection()` 自写执行器不能仅靠 `@Transactional` 自动加入 Spring 事务。如果要保留自写执行模板，必须改用同一个 DataSource 并正确配对 DataSourceUtils.getConnection/releaseConnection 等事务感知机制。直接使用 JdbcTemplate 通常更可靠。

### 4.2 常见误区

- 默认回滚 RuntimeException 和 Error；受检异常需要按业务明确 `rollbackFor`。
- 方法内部捕获并吞掉异常，代理可能看见正常返回并提交。
- `this.registerThenFail(...)` 是自调用，默认代理模式下不触发新的拦截。
- 用 new 创建 Service、未启用事务基础设施、DAO 使用不同 DataSource 都会造成失效。
- `readOnly=true` 通常是事务提示与优化标记，不是通用写保护。
- 新线程不会自动继承线程绑定的事务上下文。

### 4.3 传播与隔离

REQUIRED 表示有事务则加入，没有则创建；REQUIRES_NEW 表示挂起外层并建立独立事务，需要评估连接池容量；NESTED 通常依赖 JDBC 保存点与管理器支持。隔离级别控制并发可见性，不替代唯一约束、乐观锁和正确 SQL。

事务边界通常放在完成一个业务动作的 Service 方法，避免将慢网络请求与长时间等待放进持有数据库连接的事务。

## 5. XML 如何开启事务

这是 TxConfig 的替代配置思路，沿用上一节的类：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:context="http://www.springframework.org/schema/context"
       xmlns:tx="http://www.springframework.org/schema/tx"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xsi:schemaLocation="
         http://www.springframework.org/schema/beans https://www.springframework.org/schema/beans/spring-beans.xsd
         http://www.springframework.org/schema/context https://www.springframework.org/schema/context/spring-context.xsd
         http://www.springframework.org/schema/tx https://www.springframework.org/schema/tx/spring-tx.xsd">
  <context:property-placeholder/>
  <bean id="dataSource" class="org.springframework.jdbc.datasource.DriverManagerDataSource">
    <property name="url" value="${DB_URL}"/>
    <property name="username" value="${DB_USER}"/>
    <property name="password" value="${DB_PASSWORD}"/>
  </bean>
  <bean id="jdbcTemplate" class="org.springframework.jdbc.core.JdbcTemplate">
    <constructor-arg ref="dataSource"/>
  </bean>
  <bean id="transactionManager" class="org.springframework.jdbc.datasource.DataSourceTransactionManager">
    <constructor-arg ref="dataSource"/>
  </bean>
  <tx:annotation-driven transaction-manager="transactionManager"/>
  <bean id="userStore" class="demo.tx.UserStore">
    <constructor-arg ref="jdbcTemplate"/>
  </bean>
  <bean id="registrationService" class="demo.tx.RegistrationService">
    <constructor-arg ref="userStore"/>
  </bean>
</beans>
```

保存为 tx.xml，以 ClassPathXmlApplicationContext 加载。`tx:annotation-driven` 等价于启用注解驱动事务基础设施，不是删除 Service 上的 @Transactional。若用纯 XML 的 `<tx:advice>` 配合 `<aop:config>`，还需相应 AOP/AspectJ 支持，不能只复制事务属性片段就认为已生效。

## 6. 理解重复代码之后，再引入 MyBatis

JdbcTemplate 仍需要每个查询写 SQL、绑定参数和结果映射。MyBatis 提供 Mapper 接口与 SQL 映射、动态 SQL 和对象映射，适合需要掌控 SQL 的项目。它没有消除 SQL，也不会自动替我们解决慢查询。

本系列 Spring 6 使用 MyBatis-Spring 3.0.x 兼容线；Spring 7 应重新核对对应兼容线，不直接套用本文依赖。教学基线可加入：

```xml
<dependency>
  <groupId>org.mybatis</groupId><artifactId>mybatis</artifactId><version>3.5.19</version>
</dependency>
<dependency>
  <groupId>org.mybatis</groupId><artifactId>mybatis-spring</artifactId><version>3.0.4</version>
</dependency>
```

新建 `demo.mapper.UserMapper`，复用 `demo.User` record：

```java
package demo.mapper;
import demo.User;
import org.apache.ibatis.annotations.*;
public interface UserMapper {
    @Select("SELECT id, name FROM app_user WHERE id = #{id}")
    @ConstructorArgs({
        @Arg(column = "id", javaType = long.class, id = true),
        @Arg(column = "name", javaType = String.class)
    })
    User find(@Param("id") long id);
    @Insert("INSERT INTO app_user(name) VALUES (#{name})")
    int add(@Param("name") String name);
}
```

新建配置，导入 TxConfig 从而复用同一个 DataSource 与事务管理器：

```java
package demo.mybatis;
import javax.sql.DataSource;
import org.apache.ibatis.session.SqlSessionFactory;
import org.mybatis.spring.SqlSessionFactoryBean;
import org.mybatis.spring.annotation.MapperScan;
import org.springframework.context.annotation.*;
@Configuration(proxyBeanMethods = false)
@Import(demo.tx.TxConfig.class)
@MapperScan("demo.mapper")
public class MyBatisConfig {
    @Bean
    public SqlSessionFactory sqlSessionFactory(DataSource dataSource) throws Exception {
        var factory = new SqlSessionFactoryBean();
        factory.setDataSource(dataSource);
        return factory.getObject();
    }
}
```

使用 AnnotationConfigApplicationContext(MyBatisConfig.class)，取出 UserMapper 查询。没有匹配行时该返回类型得到 null；Service 可以用 Optional.ofNullable 包装。MapperScan 为接口注册代理，调用代理后才会查找 SQL、绑定参数并执行，不是接口凭空有了手写实现类。

如果切换到 MyBatis 持久化，把 Mapper 注入 Service 并保持事务注解在 Service 边界。MyBatis-Spring 参与 Spring 事务，不要再手动对受管 SqlSession 调用 commit/rollback/close。[MyBatis-Spring 事务说明](https://mybatis.org/spring/transactions.html)

## 7. MyBatis XML 与 Spring XML 的关系

上节使用注解 SQL。若更偏好 XML，把 find 方法的 `@Select` 与 `@ConstructorArgs` 移除，保留 `@Param`，在 `src/main/resources/mappers/UserMapper.xml` 写：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE mapper PUBLIC "-//mybatis.org//DTD Mapper 3.0//EN"
  "https://mybatis.org/dtd/mybatis-3-mapper.dtd">
<mapper namespace="demo.mapper.UserMapper">
  <resultMap id="userResult" type="demo.User">
    <constructor>
      <idArg column="id" javaType="_long"/>
      <arg column="name" javaType="java.lang.String"/>
    </constructor>
  </resultMap>
  <select id="find" resultMap="userResult">
    SELECT id, name FROM app_user WHERE id = #{id}
  </select>
</mapper>
```

在 MyBatisConfig 的 factory.getObject() 之前添加：

```java
factory.setMapperLocations(
    new org.springframework.core.io.support.PathMatchingResourcePatternResolver()
        .getResources("classpath*:mappers/**/*.xml"));
```

namespace 对应接口完整名称，select id 对应方法名称。同一个方法不要同时用注解 SQL 和 XML 重复声明。`#{}` 做参数绑定，`${}` 是文本替换，不要用于不受信任的输入。

Spring XML 如果要注册相同工厂，其核心是：

```xml
<!-- 放进已有 beans 根元素；dataSource 为已注册的数据源 -->
<bean id="sqlSessionFactory" class="org.mybatis.spring.SqlSessionFactoryBean">
  <property name="dataSource" ref="dataSource"/>
  <property name="mapperLocations" value="classpath*:mappers/**/*.xml"/>
</bean>
<bean class="org.mybatis.spring.mapper.MapperScannerConfigurer">
  <property name="basePackage" value="demo.mapper"/>
  <property name="sqlSessionFactoryBeanName" value="sqlSessionFactory"/>
</bean>
```

Spring XML 负责造对象和连依赖；Mapper XML 负责 SQL 与映射。`SqlSessionFactoryBean` 是 Spring FactoryBean：通过 Bean 名通常取得它生产的 SqlSessionFactory，而不是工厂对象自身。

## 8. 连接池为什么放到最后理解

学习版每次连接都走 DriverManager；生产通常把 DataSource 换成 HikariCP 等连接池。上层 JdbcTemplate、Mapper、Service 不必因此重写。配置池大小需要结合数据库连接上限、请求耗时与应用实例数，不能机械调大。

连接池的 close 通常意味着归还连接，池自身在应用停止时关闭。把池注册成 Bean 并配置销毁方法；不要在一次请求结束时销毁整个池。框架解耦的实际收益就在于：变更连接实现而不改变业务接口。

## 9. 建议的练习顺序与验证边界

1. 用 DriverManager 完成一次查询，确认理解三个 JDBC 资源。
2. 运行第二篇 ConnectionFactory、JdbcExecutor、Repository。
3. 分别用 XML 与 @Bean 装配，确认返回相同数据。
4. 用共享连接手动制造失败，观察回滚。
5. 使用本篇 TxMain 验证 Spring 事务回滚。
6. 用 MyBatis 注解 SQL 查询，再换成 Mapper XML。
7. 最后将 Service 接入 MVC，返回真实数据库结果。

本系列完整 Java 类已提取到临时 Maven 项目并通过 Java 17 编译；数据库与 Tomcat 的运行步骤需按本机环境执行，尚未连接真实 MySQL 完成集成测试。实际项目还需测试事务回滚、重复键、网络中断、并发与资源释放。

- [Spring 声明式事务](https://docs.spring.io/spring-framework/reference/6.2/data-access/transaction/declarative.html)
- [Spring JdbcTemplate](https://docs.spring.io/spring-framework/reference/6.2/data-access/jdbc/core.html)
- [MyBatis-Spring](https://mybatis.org/spring/)
- [MyBatis XML 映射](https://mybatis.org/mybatis-3/sqlmap-xml.html)
