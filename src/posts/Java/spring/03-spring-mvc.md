---
title: Spring MVC 详解：从 Servlet 到 DispatcherServlet、接口与 XML 配置
date: 2026-10-29
category:
  - Java
tag:
  - Spring MVC
  - Servlet
  - Web
---

# Spring MVC 详解：从 Servlet 到 DispatcherServlet、接口与 XML 配置

Spring 管理对象和基础设施，Spring MVC 在此基础上组织 HTTP 请求处理。本篇用一个不依赖数据库的最小 Web 应用先验证 MVC，再说明如何接入前一篇的 Repository。

<!-- more -->

## 1. 没有 Spring MVC 时怎么写

Servlet 容器接收 HTTP 请求，开发者注册 Servlet，在 doGet/doPost 中读取参数、调用业务、设置状态码和编码、输出 HTML 或 JSON。JSP 可以负责渲染，Filter 处理请求级通用逻辑。这套方式现在仍可使用。

```java
package demo.legacy;
import jakarta.servlet.annotation.WebServlet;
import jakarta.servlet.http.*;
import java.io.IOException;
@WebServlet("/legacy/hello")
public class HelloServlet extends HttpServlet {
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
            throws IOException {
        String name = request.getParameter("name");
        if (name == null || name.isBlank()) {
            response.sendError(400, "name is required");
            return;
        }
        response.setContentType("text/plain;charset=UTF-8");
        response.getWriter().write("Hello, " + name);
    }
}
```

如果接口越来越多，每个 Servlet 都要重复参数转换、校验、错误输出和序列化。我们可以自己写一个中央 Servlet，维护 URL 到处理器的 Map，再让处理器只返回业务结果。继续抽离参数绑定、结果转换、异常处理，就接近 MVC 框架的结构。

Spring MVC 不是替代 Tomcat：Tomcat 实现 Servlet 容器，DispatcherServlet 是其中运行的一个 Servlet。

## 2. MVC 与完整请求链

- Model：交给视图的数据及相关模型，不等于只能指数据库实体。
- View：HTML/JSP 等展示层。
- Controller：接收输入、调用业务、决定输出。

REST 接口通常直接返回 JSON，但仍使用 Spring MVC 的请求调度基础设施。

```text
HTTP → Tomcat → Filter 链 → DispatcherServlet
  → HandlerMapping：找到控制器方法与拦截器
  → HandlerAdapter：调用方法
      → 参数解析、类型转换、必要的校验
      → Controller → Service → Repository
  → 返回值处理
      → HttpMessageConverter 写 JSON 等响应
      或 → ViewResolver 找视图 → 渲染 Model
  → HTTP 响应
异常 → HandlerExceptionResolver → 异常处理方法/错误响应
```

这体现 Front Controller、策略接口、适配器和责任链等设计。DispatcherServlet 保留公共算法，各种组件负责变化的部分，因此新增参数类型或返回格式可以扩展相应策略，不必重写整个中央 Servlet。[官方 DispatcherServlet 说明](https://docs.spring.io/spring-framework/reference/6.2/web/webmvc/mvc-servlet.html)

## 3. 最小项目与版本

新建独立 Maven 项目 `mvc-study`。下面使用 Java 17、Spring 6.2.19、Tomcat 10.1（Servlet 6.0），不使用 Boot。按此固定组合学习，后续再整体升级。

```xml
<project xmlns="http://maven.apache.org/POM/4.0.0"
         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
  <groupId>demo</groupId><artifactId>mvc-study</artifactId><version>1.0</version>
  <packaging>war</packaging>
  <properties>
    <maven.compiler.release>17</maven.compiler.release>
    <project.build.sourceEncoding>UTF-8</project.build.sourceEncoding>
    <spring.version>6.2.19</spring.version>
  </properties>
  <dependencies>
    <dependency>
      <groupId>org.springframework</groupId><artifactId>spring-webmvc</artifactId>
      <version>${spring.version}</version>
    </dependency>
    <dependency>
      <groupId>jakarta.servlet</groupId><artifactId>jakarta.servlet-api</artifactId>
      <version>6.0.0</version><scope>provided</scope>
    </dependency>
    <dependency>
      <groupId>com.fasterxml.jackson.core</groupId><artifactId>jackson-databind</artifactId>
      <version>2.18.3</version>
    </dependency>
  </dependencies>
  <build>
    <finalName>mvc-study</finalName>
    <plugins>
      <plugin>
        <groupId>org.apache.maven.plugins</groupId><artifactId>maven-compiler-plugin</artifactId>
        <version>3.13.0</version>
      </plugin>
      <plugin>
        <groupId>org.apache.maven.plugins</groupId><artifactId>maven-war-plugin</artifactId>
        <version>3.4.0</version>
        <configuration><failOnMissingWebXml>false</failOnMissingWebXml></configuration>
      </plugin>
    </plugins>
  </build>
</project>
```

这里的 Jackson 版本是教学依赖基线，生产项目应跟随组织维护的 BOM 与安全更新策略。Spring MVC 的 JSON 输出需要消息转换器及相应序列化库，单有 `@RestController` 并不凭空实现 JSON 序列化。

## 4. Java 方式注册 DispatcherServlet

目录结构：

```text
src/main/java/demo/web/
  WebInitializer.java
  WebConfig.java
  HelloController.java
```

```java
package demo.web;
import org.springframework.web.servlet.support.AbstractAnnotationConfigDispatcherServletInitializer;
public class WebInitializer
        extends AbstractAnnotationConfigDispatcherServletInitializer {
    @Override protected Class<?>[] getRootConfigClasses() { return null; }
    @Override protected Class<?>[] getServletConfigClasses() {
        return new Class<?>[] { WebConfig.class };
    }
    @Override protected String[] getServletMappings() { return new String[] { "/" }; }
}
```

```java
package demo.web;
import org.springframework.context.annotation.*;
import org.springframework.web.servlet.config.annotation.*;
@Configuration
@EnableWebMvc
@ComponentScan("demo.web")
public class WebConfig implements WebMvcConfigurer {
    @Override
    public void addResourceHandlers(ResourceHandlerRegistry registry) {
        registry.addResourceHandler("/assets/**").addResourceLocations("/assets/");
    }
}
```

```java
package demo.web;
import org.springframework.web.bind.annotation.*;
@RestController
@RequestMapping("/api")
public class HelloController {
    public record Greeting(String message) { }
    @GetMapping("/hello")
    public Greeting hello(@RequestParam(name = "name", defaultValue = "ageno") String name) {
        return new Greeting("Hello, " + name);
    }
}
```

Servlet 容器通过初始化扩展机制发现 WebInitializer，后者建立 Web 上下文并注册 DispatcherServlet。`@EnableWebMvc` 注册常用 MVC 组件；`@ComponentScan` 扫描控制器，两者职责不同。

## 5. 构建与部署验证

执行：

```bash
mvn clean package
```

将 `target/mvc-study.war` 复制到 Tomcat 10.1 的 `webapps/`，启动 Tomcat；确保 Tomcat 使用 Java 17。默认 context path 由 WAR 文件名决定：

```bash
curl -i 'http://localhost:8080/mvc-study/api/hello?name=ageno'
```

预期 HTTP 200，JSON 内容为 `{"message":"Hello, ageno"}`。容器端口或部署方式不同时相应修改 URL。项目没有 Boot 插件，不能直接假定 `java -jar mvc-study.war` 可以运行。

静态文件放 `src/main/webapp/assets/`，例如 test.txt，访问 `/mvc-study/assets/test.txt`。不要将 `/` 映射随意改成 `/*`，否则可能影响 JSP 等容器处理路径。

## 6. Controller 常用功能

| 注解/类型 | 用法 | 示例 |
| --- | --- | --- |
| @PathVariable | 路径参数 | `/users/{id}` |
| @RequestParam | 查询参数或表单参数 | `?page=1` |
| @RequestBody | 用消息转换器读取请求体 | JSON DTO |
| @RequestHeader | 读取请求头 | Accept-Language |
| @CookieValue | 读取 Cookie | 偏好设置 |
| @ModelAttribute | 表单/模型绑定 | 搜索条件 DTO |
| ResponseEntity | 控制响应状态、头、体 | 201 Created |
| @ResponseBody | 将返回值写入响应体 | JSON 对象 |

`@RestController` 相当于控制器角色加默认响应体语义。普通 `@Controller` 返回 String 时通常作为视图名，`@RestController` 返回 String 时通常作为文本响应。

以下类可直接加入最小项目，展示路径参数、请求体和统一异常处理；它不持久化数据，只用于验证 HTTP 协议行为：

```java
package demo.web;
import org.springframework.http.*;
import org.springframework.web.bind.annotation.*;
import java.net.URI;
@RestController
@RequestMapping("/api/users")
public class UserController {
    public record CreateUser(String name) { }
    public record UserView(long id, String name) { }
    @GetMapping("/{id}")
    public UserView find(@PathVariable("id") long id) {
        if (id != 1) throw new UserNotFoundException();
        return new UserView(1, "ageno");
    }
    @PostMapping
    public ResponseEntity<UserView> create(@RequestBody CreateUser input) {
        if (input.name() == null || input.name().isBlank() || input.name().length() > 100)
            throw new IllegalArgumentException("name 长度必须是 1–100");
        return ResponseEntity.created(URI.create("./users/1"))
            .body(new UserView(1, input.name()));
    }
}
```

```java
package demo.web;
public class UserNotFoundException extends RuntimeException { }
```

```java
package demo.web;
import org.springframework.http.*;
import org.springframework.web.bind.annotation.*;
@RestControllerAdvice
public class ApiErrors {
    @ExceptionHandler(UserNotFoundException.class)
    public ResponseEntity<ProblemDetail> notFound() {
        return ResponseEntity.status(404).body(
            ProblemDetail.forStatusAndDetail(HttpStatus.NOT_FOUND, "用户不存在"));
    }
    @ExceptionHandler(IllegalArgumentException.class)
    public ResponseEntity<ProblemDetail> badInput(IllegalArgumentException failure) {
        return ResponseEntity.badRequest().body(
            ProblemDetail.forStatusAndDetail(HttpStatus.BAD_REQUEST, failure.getMessage()));
    }
}
```

```bash
curl -i 'http://localhost:8080/mvc-study/api/users/2'
curl -i -X POST 'http://localhost:8080/mvc-study/api/users' \
  -H 'Content-Type: application/json' -d '{"name":"ageno"}'
```

预期分别为 404 和 201。这里返回的是演示对象，不应把响应误认为已经插入 MySQL；接入 Repository 后必须返回实际生成的主键与真实资源位置。

## 7. 参数校验、异常与安全边界

实际 DTO 可以使用 `@NotBlank`、`@Size`，在参数上使用 `@Valid @RequestBody`。这需要 Jakarta Validation API 和实现，例如兼容的 Hibernate Validator 版本；只有注解而没有校验基础设施不够。本篇为保持最小依赖采用手动校验。

JSON 格式错误、类型转换失败、缺少必要参数与业务错误不是同一类异常。可以继承 `ResponseEntityExceptionHandler` 统一处理框架标准异常，再补充业务异常。不要捕获所有 Exception 后一律返回 HTTP 200，也不要把 SQL、堆栈、密码原样放进客户端响应。

Filter 适合 Servlet 请求级处理；HandlerInterceptor 在 MVC 的 Handler 调度范围工作，适合计时等功能；AOP 作用于 Bean 方法。权限认证不要仅靠一个拦截器拼凑，完整应用应使用安全框架并验证覆盖范围。

CORS 是浏览器跨源访问策略，不是登录认证。可以通过 WebMvcConfigurer 的 `addCorsMappings` 配置指定来源，携带凭据时不要用任意来源通配符。

## 8. XML 方式配置相同 Web 应用

本节是替代方案：移除 WebInitializer 与 WebConfig，避免重复注册 DispatcherServlet 或两套 MVC 配置。Controller 与 ApiErrors 保持原样。

创建 `src/main/webapp/WEB-INF/web.xml`：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<web-app xmlns="https://jakarta.ee/xml/ns/jakartaee"
         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="https://jakarta.ee/xml/ns/jakartaee https://jakarta.ee/xml/ns/jakartaee/web-app_6_0.xsd"
         version="6.0">
  <servlet>
    <servlet-name>mvc</servlet-name>
    <servlet-class>org.springframework.web.servlet.DispatcherServlet</servlet-class>
    <init-param>
      <param-name>contextConfigLocation</param-name>
      <param-value>/WEB-INF/spring-mvc.xml</param-value>
    </init-param>
    <load-on-startup>1</load-on-startup>
  </servlet>
  <servlet-mapping>
    <servlet-name>mvc</servlet-name><url-pattern>/</url-pattern>
  </servlet-mapping>
</web-app>
```

创建 `src/main/webapp/WEB-INF/spring-mvc.xml`：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:context="http://www.springframework.org/schema/context"
       xmlns:mvc="http://www.springframework.org/schema/mvc"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xsi:schemaLocation="
         http://www.springframework.org/schema/beans https://www.springframework.org/schema/beans/spring-beans.xsd
         http://www.springframework.org/schema/context https://www.springframework.org/schema/context/spring-context.xsd
         http://www.springframework.org/schema/mvc https://www.springframework.org/schema/mvc/spring-mvc.xsd">
  <context:component-scan base-package="demo.web"/>
  <mvc:annotation-driven/>
  <mvc:resources mapping="/assets/**" location="/assets/"/>
</beans>
```

重新打包与部署，相同 URL 应有相同响应。`web.xml` 属于 Servlet 容器配置，`spring-mvc.xml` 属于 Spring 配置，MyBatis Mapper XML 则属于 SQL 映射；三者不能互相替代。

## 9. HTML/JSP 视图怎么接

下面是扩展示意：

```java
@Controller
class PageController {
    @GetMapping("/welcome")
    String welcome(org.springframework.ui.Model model) {
        model.addAttribute("name", "ageno");
        return "welcome";
    }
}
```

配置 `InternalResourceViewResolver`，prefix 为 `/WEB-INF/views/`、suffix 为 `.jsp`，容器据此渲染 `/WEB-INF/views/welcome.jsp`。JSP 依赖容器的 JSP 引擎；使用 JSTL 时另加与 Jakarta 版本兼容的依赖。把视图放 WEB-INF 可避免用户直接绕过 Controller 访问。

## 10. 和数据库容器如何组合

小型应用可以在一个 Web 上下文通过 `@Import(DatabaseConfig.class)` 组合业务与数据库配置。较大传统应用常用根上下文放 Service、Repository、DataSource，DispatcherServlet 子上下文放 Controller 和 MVC 组件。子上下文可以访问父上下文，父上下文不能反向访问子上下文。

Java 初始化器中 `getRootConfigClasses()` 返回根配置，`getServletConfigClasses()` 返回 WebConfig。两边不要扫描同一批所有组件，避免 Service 重复创建导致事务等配置只对其中一套实例生效。

Controller 只负责 HTTP 输入输出，Service 负责业务，Repository 负责 SQL。把第二篇 Repository 注入 Service 后，应在边界把 SQLException 转成适当的应用异常，记录服务端日志并输出合理状态码。下一篇将改用 Spring JDBC 标准异常与事务管理。

## 11. 为什么可以扩展那么多行为

- HandlerMapping：回答“这个请求交给谁”。注解映射只是其中一种策略。
- HandlerAdapter：统一调度不同形态的处理器。
- HandlerMethodArgumentResolver：把请求解析成方法参数。
- HttpMessageConverter：按类型与媒体类型读取/写出请求体和响应体。
- HandlerExceptionResolver：把异常转为 HTTP 响应或视图。
- ViewResolver：把逻辑视图名解析成 View。

注解在启动时被扫描为映射元数据；请求到来时通过已注册的策略查找、绑定和调用。它不是每次都凭空理解 Java 方法，也不是所有功能都由反射一个机制完成。

## 12. 排错顺序与参考

404 先检查 context path、DispatcherServlet 映射和 Controller 扫描；405 检查 HTTP 方法；400 检查参数和 JSON；415 检查 Content-Type 与转换器；406 检查 Accept 与输出类型。类加载报 javax/jakarta 错误时，先确认 Spring 与 Tomcat 的代际兼容性。

- [Spring MVC 6.2](https://docs.spring.io/spring-framework/reference/6.2/web/webmvc.html)
- [MVC Java/XML 配置](https://docs.spring.io/spring-framework/reference/6.2/web/webmvc/mvc-config.html)
- [请求处理方法](https://docs.spring.io/spring-framework/reference/6.2/web/webmvc/mvc-controller/ann-methods.html)
- 下一篇：[从自写 JDBC 到 Spring 事务与 MyBatis](./04-transactions-mybatis.md)
