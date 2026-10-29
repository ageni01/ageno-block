---
title: 从零封装 MySQL 连接与 JDBC，再交给 Spring 管理
date: 2026-10-29
category:
  - Java
tag:
  - JDBC
  - MySQL
  - Spring
---

# 从零封装 MySQL 连接与 JDBC，再交给 Spring 管理

本文先不用 MyBatis，也不用 JdbcTemplate。先弄清连接、SQL、参数、结果映射和关闭资源各由谁负责，再理解成熟框架替我们承担了哪些工作。前置阅读：[Spring 容器](./01-spring-container.md)。

<!-- more -->

## 1. 先分清六层职责

```text
Controller：接收请求
Service：业务规则与事务边界
Repository/DAO：某个业务对象的持久化操作
JdbcExecutor：执行 SQL、绑定参数、遍历结果
ConnectionFactory/DataSource：提供连接
MySQL JDBC 驱动：实现 JDBC 接口并与数据库通信
```

“自己封装连接”通常是封装 JDBC 驱动的使用方式，不是重写 MySQL 网络协议。`Connection`、`PreparedStatement`、`ResultSet` 是 JDBC 标准接口，MySQL Connector/J 提供具体实现。现代 JDBC 驱动通常可自动注册，一般不再需要手动 `Class.forName("com.mysql.cj.jdbc.Driver")`。[Connector/J 官方说明](https://dev.mysql.com/doc/connector-j/en/connector-j-overview.html)

## 2. 准备数据库

安装并启动本地 MySQL 8.x。以下 SQL 以管理员执行，示例账号仅用于本机学习；替换密码，不要提交真实凭据：

```sql
CREATE DATABASE spring_study CHARACTER SET utf8mb4;
CREATE USER 'spring_student'@'localhost' IDENTIFIED BY 'replace-with-local-password';
GRANT SELECT, INSERT, UPDATE, DELETE ON spring_study.* TO 'spring_student'@'localhost';
USE spring_study;
CREATE TABLE app_user (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL
);
INSERT INTO app_user(name) VALUES ('ageno');
```

在第一篇 Maven 项目的 dependencies 内添加以下教学基线版本，升级时核对驱动与数据库兼容性：

```xml
<dependency>
  <groupId>com.mysql</groupId><artifactId>mysql-connector-j</artifactId>
  <version>9.2.0</version>
</dependency>
```

终端设置变量后，从同一终端启动程序；IDE 运行则在 Run Configuration 中设置环境变量：

```bash
export DB_URL='jdbc:mysql://localhost:3306/spring_study?connectionTimeZone=UTC'
export DB_USER='spring_student'
export DB_PASSWORD='replace-with-local-password'
```

URL 中的参数用于驱动配置；不要为消除连接错误随意关闭 TLS 或认证保护。远程生产连接应按部署环境配置证书与 TLS，本文只演示本地数据库。

## 3. 最原始的 JDBC 查询

```java
try (var connection = java.sql.DriverManager.getConnection(
         System.getenv("DB_URL"), System.getenv("DB_USER"), System.getenv("DB_PASSWORD"));
     var statement = connection.prepareStatement(
         "SELECT id, name FROM app_user WHERE id = ?")) {
    statement.setLong(1, 1L);
    try (var resultSet = statement.executeQuery()) {
        while (resultSet.next()) {
            System.out.println(resultSet.getLong("id") + ":" + resultSet.getString("name"));
        }
    }
}
```

参数下标从 1 开始。使用 `?` 和参数绑定，不把用户输入拼进 SQL。参数占位符不能替代表名、列名与排序关键字，这些动态内容需要白名单。try-with-resources 按逆序关闭 ResultSet、Statement、Connection，即使发生异常也会尝试释放资源。

如果每个 DAO 都复制这段代码，很快就会出现忘记关闭、结果映射重复、异常被吞掉等问题。

## 4. 第一次封装：只解决获取连接

下面各 public 类型都放入 `src/main/java/demo/` 对应文件，并保留 `package demo;`。这个阶段只使用 JDK JDBC 和 MySQL 驱动。

```java
package demo;
import java.sql.Connection;
import java.sql.SQLException;
@FunctionalInterface
public interface ConnectionFactory {
    Connection open() throws SQLException;
}
```

```java
package demo;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Objects;
public final class MysqlConnectionFactory implements ConnectionFactory {
    private final String url;
    private final String username;
    private final String password;
    public MysqlConnectionFactory(String url, String username, String password) {
        this.url = Objects.requireNonNull(url, "DB_URL 未设置");
        this.username = Objects.requireNonNull(username, "DB_USER 未设置");
        this.password = Objects.requireNonNull(password, "DB_PASSWORD 未设置");
    }
    @Override
    public Connection open() throws SQLException {
        return DriverManager.getConnection(url, username, password);
    }
}
```

这个类保存连接参数，每次 open 都创建新连接。不要用一个 static Connection 给所有请求共享：连接有事务、autoCommit 等会话状态，并发使用会混乱。它也不是连接池，不适合高并发生产环境。

## 5. 第二次封装：提取执行模板和结果映射

先让“怎么执行”固定下来，“怎么把一行变成对象”交给调用者。这是模板与回调的组合。

```java
package demo;
import java.sql.ResultSet;
import java.sql.SQLException;
@FunctionalInterface
public interface RowMapper<T> {
    T map(ResultSet resultSet) throws SQLException;
}
```

```java
package demo;
import java.sql.*;
import java.util.ArrayList;
import java.util.List;
public final class JdbcExecutor {
    private final ConnectionFactory factory;
    public JdbcExecutor(ConnectionFactory factory) { this.factory = factory; }

    public <T> List<T> query(String sql, RowMapper<T> mapper, Object... args)
            throws SQLException {
        try (Connection connection = factory.open();
             PreparedStatement statement = connection.prepareStatement(sql)) {
            bind(statement, args);
            try (ResultSet resultSet = statement.executeQuery()) {
                List<T> rows = new ArrayList<>();
                while (resultSet.next()) rows.add(mapper.map(resultSet));
                return rows;
            }
        }
    }

    public int update(String sql, Object... args) throws SQLException {
        try (Connection connection = factory.open();
             PreparedStatement statement = connection.prepareStatement(sql)) {
            bind(statement, args);
            return statement.executeUpdate();
        }
    }

    private static void bind(PreparedStatement statement, Object[] args)
            throws SQLException {
        for (int i = 0; i < args.length; i++) statement.setObject(i + 1, args[i]);
    }
}
```

这里 `setObject` 足以演示常见参数，复杂类型、明确 SQL NULL 类型、时间与批处理需要更细的绑定策略。大结果集也不应总装入 List。异常继续抛出，不能假装查询失败等于“没有数据”。

## 6. 把 SQL 放回业务 Repository

```java
package demo;
public record User(long id, String name) { }
```

```java
package demo;
import java.sql.SQLException;
import java.util.List;
import java.util.Optional;
public final class JdbcUserRepository {
    private final JdbcExecutor jdbc;
    public JdbcUserRepository(JdbcExecutor jdbc) { this.jdbc = jdbc; }
    public Optional<User> find(long id) throws SQLException {
        List<User> rows = jdbc.query(
            "SELECT id, name FROM app_user WHERE id = ?",
            rs -> new User(rs.getLong("id"), rs.getString("name")), id);
        return rows.stream().findFirst();
    }
    public int add(String name) throws SQLException {
        return jdbc.update("INSERT INTO app_user(name) VALUES (?)", name);
    }
}
```

手动装配并查询：

```java
package demo;
public class JdbcMain {
    public static void main(String[] args) throws Exception {
        var factory = new MysqlConnectionFactory(System.getenv("DB_URL"),
            System.getenv("DB_USER"), System.getenv("DB_PASSWORD"));
        var repository = new JdbcUserRepository(new JdbcExecutor(factory));
        System.out.println(repository.find(1));
    }
}
```

预期查询到 `Optional[User[id=1, name=ageno]]`，以真实数据为准。这时我们已经完成无 Spring 的数据库访问封装。

## 7. 第三步：通过 @Bean 交给 Spring

```java
package demo;
import org.springframework.context.annotation.*;
import org.springframework.core.env.Environment;
@Configuration(proxyBeanMethods = false)
public class DatabaseConfig {
    @Bean
    public ConnectionFactory connectionFactory(Environment env) {
        return new MysqlConnectionFactory(env.getRequiredProperty("DB_URL"),
            env.getRequiredProperty("DB_USER"), env.getRequiredProperty("DB_PASSWORD"));
    }
    @Bean
    public JdbcExecutor jdbcExecutor(ConnectionFactory factory) {
        return new JdbcExecutor(factory);
    }
    @Bean
    public JdbcUserRepository userRepository(JdbcExecutor executor) {
        return new JdbcUserRepository(executor);
    }
}
```

入口改成：

```java
try (var context = new org.springframework.context.annotation.AnnotationConfigApplicationContext(
        demo.DatabaseConfig.class)) {
    System.out.println(context.getBean(demo.JdbcUserRepository.class).find(1));
}
```

这个片段放到声明 `throws Exception` 的 main 方法中。业务类没有为了 Spring 改成特殊父类，改变的是装配入口。

## 8. 相同关系的 XML 配置

保存为 `src/main/resources/database.xml`：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:context="http://www.springframework.org/schema/context"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xsi:schemaLocation="
         http://www.springframework.org/schema/beans https://www.springframework.org/schema/beans/spring-beans.xsd
         http://www.springframework.org/schema/context https://www.springframework.org/schema/context/spring-context.xsd">
  <context:property-placeholder/>
  <bean id="connectionFactory" class="demo.MysqlConnectionFactory">
    <constructor-arg value="${DB_URL}"/>
    <constructor-arg value="${DB_USER}"/>
    <constructor-arg value="${DB_PASSWORD}"/>
  </bean>
  <bean id="jdbcExecutor" class="demo.JdbcExecutor">
    <constructor-arg ref="connectionFactory"/>
  </bean>
  <bean id="userRepository" class="demo.JdbcUserRepository">
    <constructor-arg ref="jdbcExecutor"/>
  </bean>
</beans>
```

用 `ClassPathXmlApplicationContext("database.xml")` 加载即可。环境变量由 Spring Environment 解析，`property-placeholder` 负责替换 XML 中的占位符。不要同时加载本节 XML 和上一节 DatabaseConfig 来重复注册同一批对象。

## 9. 自己封装后，最重要的缺口是事务

现在每个 query/update 都打开并关闭自己的连接，默认通常为 autoCommit。连续调用两次 update 并不构成一个事务；第一步成功、第二步失败会留下部分结果。

手动事务必须让多个 SQL 共用同一个 Connection：

```java
try (var connection = factory.open()) {
    connection.setAutoCommit(false);
    try {
        try (var a = connection.prepareStatement(
                "UPDATE account SET balance = balance - ? WHERE id = ?");
             var b = connection.prepareStatement(
                "UPDATE account SET balance = balance + ? WHERE id = ?")) {
            a.setBigDecimal(1, amount); a.setLong(2, fromId);
            b.setBigDecimal(1, amount); b.setLong(2, toId);
            if (a.executeUpdate() != 1 || b.executeUpdate() != 1)
                throw new java.sql.SQLException("账户不存在");
        }
        connection.commit();
    } catch (Exception failure) {
        try { connection.rollback(); }
        catch (java.sql.SQLException rollbackFailure) { failure.addSuppressed(rollbackFailure); }
        throw failure;
    }
}
```

这是展示共享连接的片段，`factory`、amount、fromId、toId 由调用方提供，`account` 表需另建；不构成完整转账系统，还需要余额约束、金额校验与并发控制。这里使用新建后直接关闭的连接；若管理的是复用连接，还必须按连接池契约恢复状态。

封装事务时可以让回调接收 Connection，或引入事务连接上下文；不能简单在 Service 加一个注解后仍任由 DAO 打开全新连接。下一篇事务文章会使用 Spring 的标准 DataSource 与 JdbcTemplate 解决这个问题。

## 10. 从自定义接口走向标准接口

ConnectionFactory 是我们定义的最小接口，`DataSource` 是 JDBC 标准连接工厂接口。改用 DataSource 后可以方便接入连接池、Spring 事务、JdbcTemplate 与 MyBatis。

连接池管理连接复用与数量；JdbcTemplate 管执行模板；MyBatis 管 SQL 到方法和对象的映射。这三者不是相互替代的东西。先理解这些边界，再引入第三方组件，就知道每个组件在替换哪一层。

## 11. 验证与排错

依次运行 SQL、设置环境变量、运行 JdbcMain、运行 Spring Java 配置入口、运行 XML 配置入口。后三者应读取到相同数据。修改 id 为不存在的值应返回 Optional.empty；关闭数据库应抛连接异常，而不是返回空集合。

`No suitable driver` 检查驱动依赖和 URL；`Access denied` 检查账户、来源主机和密码；`Unknown database` 检查是否执行初始化 SQL。不要把密码或包含凭据的完整连接信息打印到日志。

- [MySQL Connector/J 文档](https://dev.mysql.com/doc/connector-j/en/)
- [Spring 数据库连接管理](https://docs.spring.io/spring-framework/reference/6.2/data-access/jdbc/connections.html)
- 下一篇：[Spring MVC 请求处理与 XML 配置](./03-spring-mvc.md)
