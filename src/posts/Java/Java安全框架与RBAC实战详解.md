# Java 安全框架与 RBAC 实战详解

> 面向 Spring Security、Apache Shiro、Sa-Token 的选型、内部架构、快速上手与深度定制。示例以 Java 17、Spring Boot 3.x、Spring Security 6.x 为主；Shiro 与 Sa-Token API 会注明常见版本差异。

## 目录

1. [先建立正确的安全模型](#1-先建立正确的安全模型)
2. [三套框架如何选](#2-三套框架如何选)
3. [Spring Security 内部架构](#3-spring-security-内部架构)
4. [Apache Shiro 内部架构](#4-apache-shiro-内部架构)
5. [Sa-Token 内部架构](#5-sa-token-内部架构)
6. [统一 RBAC 数据模型](#6-统一-rbac-数据模型)
7. [Spring Security 快速上手](#7-spring-security-快速上手)
8. [Shiro 快速上手](#8-shiro-快速上手)
9. [Sa-Token 快速上手](#9-sa-token-快速上手)
10. [定制认证与授权](#10-定制认证与授权)
11. [搭建可维护的 RBAC 服务](#11-搭建可维护的-rbac-服务)
12. [安全、性能与运维](#12-安全性能与运维)
13. [常见问题与排错](#13-常见问题与排错)
14. [落地检查清单](#14-落地检查清单)

---

## 1. 先建立正确的安全模型

### 1.1 认证、授权、会话不是一回事

- **认证（Authentication）**：你是谁？例如账号密码、短信、OIDC、客户端证书。
- **授权（Authorization）**：你能做什么？例如 `user:read`、`order:refund`。
- **会话（Session）**：如何在后续请求中记住你？例如 HttpSession、Cookie、JWT、Redis Token。
- **审计（Audit）**：谁在什么时候以什么结果访问了什么资源。

框架通常只负责通用管线，用户、角色、权限、组织、数据范围仍由业务系统定义。不要把“登录成功”误认为“已经完成 RBAC”。

### 1.2 RBAC 基础与扩展

基础 RBAC 关系为：`用户 -> 角色 -> 权限`。实际项目还需要：

```text
用户 ──< 用户角色 >── 角色 ──< 角色权限 >── 权限
  │                      │
  └── 组织/租户            └── 菜单、接口、按钮、数据范围
```

建议把权限表示为稳定的字符串（如 `sys:user:list`），把 URL 和 HTTP 方法作为资源属性，而不是直接把 URL 当权限名。前端菜单权限与后端接口权限必须分开校验，后端永远是最终边界。

### 1.3 一个请求的标准生命周期

```text
客户端请求
  -> Token/Cookie 解析
  -> 认证过滤器/拦截器
  -> 加载用户身份与权限
  -> 创建 SecurityContext/Subject/登录上下文
  -> URL 规则匹配
  -> 方法级授权（角色、权限、数据权限）
  -> Controller/Service
  -> 异常转换、审计、清理上下文
```

登录只发生在建立会话的请求；之后每次请求都应重新验证会话有效性、过期时间、注销状态和必要的设备/租户约束。

---

## 2. 三套框架如何选

| 维度 | Spring Security | Apache Shiro | Sa-Token |
|---|---|---|---|
| 定位 | Spring 生态的完整安全基础设施 | 独立、轻量、概念直观 | 面向业务开发的轻量权限认证框架 |
| 核心模型 | `SecurityContext`、Filter、Provider、决策器 | `Subject`、SecurityManager、Realm、Filter | `StpUtil`、登录会话、全局侦听器、注解 |
| OAuth2/OIDC | 生态最完整，资源服务器、客户端、授权服务器能力成熟 | 需组合其他组件 | 有集成模块，复杂协议仍建议使用专用 IdP |
| Spring Boot 集成 | 深度集成，默认配置多 | 需要显式配置 | Starter 上手快 |
| 学习成本 | 高，但扩展点标准 | 中，架构清晰 | 低，业务 API 简洁 |
| 适合场景 | 企业统一认证、微服务、OAuth2、复杂合规 | 非 Spring 项目、传统单体、希望掌控链路 | 快速搭建后台、前后端分离、国产 Spring 项目 |
| 风险点 | 配置错误可能造成规则覆盖/放行 | 组件与 Web 生态需要自行组合 | 复杂 OAuth2、细粒度数据权限需自行设计 |

经验建议：已经使用 Spring Cloud/OAuth2 的团队优先 Spring Security；传统单体且需要简单 Realm 可考虑 Shiro；希望快速完成登录、注解鉴权、踢人下线且协议复杂度不高可选 Sa-Token。不要在同一请求链中同时启用三套框架的认证职责。

---

## 3. Spring Security 内部架构

### 3.1 核心对象关系

```text
Servlet 容器
  -> DelegatingFilterProxy
  -> FilterChainProxy
  -> SecurityFilterChain（按请求匹配的过滤器链）
  -> 认证过滤器（BearerTokenAuthenticationFilter 等）
  -> AuthenticationManager
  -> AuthenticationProvider
  -> UserDetailsService / JwtDecoder / 外部 IdP
  -> SecurityContextHolder
  -> AuthorizationFilter / MethodInterceptor
```

- `DelegatingFilterProxy`：把 Servlet Filter 委托给 Spring Bean。
- `FilterChainProxy`：选择匹配当前请求的 `SecurityFilterChain`。
- `SecurityFilterChain`：一组有顺序的过滤器；顺序直接影响行为。
- `Authentication`：包含主体、凭据、权限与认证状态。
- `AuthenticationManager`：认证入口，默认实现 `ProviderManager`，按顺序尝试多个 Provider。
- `AuthenticationProvider`：真正执行某类凭据认证。
- `SecurityContextHolder`：线程级保存当前认证上下文，请求结束由过滤器清理。
- `AuthorizationManager`：对请求或方法作最终授权判断。

### 3.2 认证流程（用户名密码）

1. `UsernamePasswordAuthenticationFilter` 从登录请求构造未认证的 `UsernamePasswordAuthenticationToken`。
2. 调用 `AuthenticationManager.authenticate`。
3. `DaoAuthenticationProvider` 调用 `UserDetailsService.loadUserByUsername`。
4. 使用 `PasswordEncoder.matches` 校验密码。
5. 成功后返回已认证 Token，失败抛出 `AuthenticationException`。
6. `SecurityContextRepository` 保存上下文（Session 或显式无状态策略）。
7. 成功/失败处理器生成 JSON、重定向或 JWT。

密码永远只存自适应哈希（BCrypt、Argon2），不能使用 MD5、SHA-1 或明文。

### 3.3 授权流程

- URL 级：`AuthorizationFilter` 读取 `SecurityContext`，将 `Authentication` 交给 `AuthorizationManager<RequestAuthorizationContext>`。
- 方法级：`@PreAuthorize` 等注解由 `MethodSecurityInterceptor` 解析为表达式或 `AuthorizationManager`。
- 决策结果为允许则继续调用；拒绝抛出 `AccessDeniedException`，匿名用户通常转换为 401，已认证但无权限为 403。

Spring Security 6 推荐 `authorizeHttpRequests`，不要继续使用已弃用的 `authorizeRequests`。

### 3.4 常用扩展点

| 需求 | 扩展点 |
|---|---|
| 从数据库查用户 | `UserDetailsService` |
| 多种登录方式 | 自定义 `AuthenticationProvider` 或多个 Provider |
| JWT 校验 | `JwtDecoder`、`JwtAuthenticationConverter` |
| 统一异常 JSON | `AuthenticationEntryPoint`、`AccessDeniedHandler` |
| URL 动态权限 | 自定义 `AuthorizationManager` |
| 方法级表达式 | `@EnableMethodSecurity`、自定义 Bean 方法 |
| 登录成功审计 | `AuthenticationSuccessHandler`、事件监听器 |
| 踢人/并发会话 | `SessionRegistry`、`SessionAuthenticationStrategy` |
| 请求前后通用逻辑 | `OncePerRequestFilter` |

---

## 4. Apache Shiro 内部架构

### 4.1 核心对象关系

```text
请求
  -> ShiroFilter（FilterChainResolver）
  -> Subject（当前访问者）
  -> SecurityManager
  -> Authenticator（认证）
  -> Authorizer（授权）
  -> Realm（从业务源取得身份、角色、权限）
  -> SessionManager / SessionDAO
```

- `Subject`：对当前用户的门面，提供 `login`、`logout`、`hasRole`、`isPermitted`。
- `SecurityManager`：核心协调器，连接 Authenticator、Authorizer、SessionManager、CacheManager。
- `Realm`：最关键的业务扩展点，`doGetAuthenticationInfo` 负责认证，`doGetAuthorizationInfo` 负责授权。
- `Subject` 默认通过线程绑定保存，Web 场景由过滤器创建和清理。

### 4.2 认证与授权流程

`Subject.login(token)` 将 Token 交给 `Authenticator`；常用 `ModularRealmAuthenticator` 遍历 Realm。Realm 返回 `AuthenticationInfo`，内部凭证匹配器完成密码校验。授权时 `Authorizer` 调用 Realm 获取 `AuthorizationInfo`（角色和权限），结果可进入缓存。

### 4.3 Filter 链与注解

```ini
[urls]
/login = anon
/public/** = anon
/admin/** = authc
/admin/user/delete = perms[sys:user:delete]
```

`authc`、`anon`、`roles[...]`、`perms[...]` 是常用过滤器。方法级可启用 `@RequiresAuthentication`、`@RequiresRoles`、`@RequiresPermissions`。Shiro 2.x 的 Jakarta 命名空间与 Spring Boot 3 兼容性需确认所用 starter；老项目常见的是 `javax` 依赖。

---

## 5. Sa-Token 内部架构

### 5.1 核心链路

```text
请求
  -> SaServletFilter / SaInterceptor
  -> SaTokenContext（Web 上下文抽象）
  -> StpLogic（登录、Token、注销、踢人）
  -> SaTokenDao（Token/Session 持久化）
  -> StpInterface（获取角色、权限）
  -> 注解鉴权 / 路由拦截
```

- `StpUtil`：默认账号体系的静态门面。
- `StpLogic`：实际执行登录、校验 Token、会话续期、踢人下线。
- `SaTokenDao`：默认内存实现，生产环境切 Redis，并设置序列化、超时和命名空间。
- `StpInterface`：业务系统实现 `getPermissionList` 与 `getRoleList`。
- 注解如 `@SaCheckLogin`、`@SaCheckPermission`、`@SaCheckRole` 由拦截器解析。

Sa-Token 的优势是登录 API 与常见业务动作简单；复杂的 OAuth2/OIDC、JWT 密钥轮换、跨语言验证仍应优先采用成熟的认证中心或 Spring Security 资源服务器。

---

## 6. 统一 RBAC 数据模型

### 6.1 表结构

```sql
create table sys_user (
  id bigint primary key,
  username varchar(64) not null unique,
  password_hash varchar(255) not null,
  status tinyint not null default 1,
  tenant_id bigint not null,
  version int not null default 0,
  created_at timestamp not null,
  updated_at timestamp not null
);

create table sys_role (
  id bigint primary key,
  code varchar(64) not null,
  name varchar(100) not null,
  tenant_id bigint not null,
  status tinyint not null default 1,
  unique (tenant_id, code)
);

create table sys_permission (
  id bigint primary key,
  code varchar(128) not null unique,
  name varchar(100) not null,
  resource_type varchar(20) not null, -- API/MENU/BUTTON/DATA
  http_method varchar(10),
  path_pattern varchar(255),
  status tinyint not null default 1
);

create table sys_user_role (user_id bigint not null, role_id bigint not null,
  primary key (user_id, role_id));
create table sys_role_permission (role_id bigint not null, permission_id bigint not null,
  primary key (role_id, permission_id));
```

生产设计还应增加组织关系、角色继承（可选）、数据范围规则、授权变更版本号和审计表。所有关联表建立外键或由应用层保证引用完整性，并建立 `tenant_id`、关联键、状态字段索引。

### 6.2 权限命名约定

推荐：`资源:动作[:范围]`，例如 `sys:user:read`、`sys:user:create`、`order:refund`。角色使用稳定 code（`SUPER_ADMIN`），显示名称可变。通配权限如 `sys:user:*` 只能由授权决策器解释，不要在 SQL 中用模糊匹配替代明确规则。

### 6.3 JWT 与数据库权限的取舍

- JWT 中放少量稳定声明（用户 ID、租户 ID、会话 ID、权限版本），避免把完整权限列表塞进 Token。
- 权限变更要立即生效时，用 `jti`/会话 ID 黑名单或权限版本校验；纯 JWT 在过期前天然无法撤销。
- Redis 会话适合踢人、并发控制和权限实时变更；无状态 JWT 适合服务间验证但运维复杂度更高。

---

## 7. Spring Security 快速上手

### 7.1 依赖

```xml
<dependency>
  <groupId>org.springframework.boot</groupId>
  <artifactId>spring-boot-starter-security</artifactId>
</dependency>
<dependency>
  <groupId>org.springframework.boot</groupId>
  <artifactId>spring-boot-starter-validation</artifactId>
</dependency>
```

### 7.2 数据库用户与密码编码

```java
@Service
class DbUserDetailsService implements UserDetailsService {
    private final UserMapper mapper;
    DbUserDetailsService(UserMapper mapper) { this.mapper = mapper; }

    @Override
    public UserDetails loadUserByUsername(String username) {
        var u = mapper.findEnabledByUsername(username)
            .orElseThrow(() -> new UsernameNotFoundException(username));
        var authorities = mapper.findPermissionCodes(u.id()).stream()
            .map(SimpleGrantedAuthority::new).toList();
        return User.withUsername(u.username())
            .password(u.passwordHash()).authorities(authorities).build();
    }
}
```

```java
@Bean
PasswordEncoder passwordEncoder() {
    return Argon2PasswordEncoder.defaultsForSpringSecurity_v5_8();
}
```

### 7.3 Session 模式配置

```java
@Configuration
@EnableMethodSecurity
class SecurityConfig {
  @Bean
  SecurityFilterChain api(HttpSecurity http) throws Exception {
    http
      .csrf(csrf -> csrf.disable()) // 仅在纯 Token API；Cookie 会话不要盲目关闭
      .sessionManagement(s -> s.sessionCreationPolicy(SessionCreationPolicy.IF_REQUIRED))
      .authorizeHttpRequests(a -> a
        .requestMatchers("/auth/login", "/error", "/health").permitAll()
        .requestMatchers(HttpMethod.GET, "/api/users/**").hasAuthority("sys:user:read")
        .anyRequest().authenticated())
      .formLogin(f -> f.disable())
      .httpBasic(b -> b.disable())
      .exceptionHandling(e -> e
        .authenticationEntryPoint((req, res, ex) -> writeJson(res, 401, "UNAUTHENTICATED"))
        .accessDeniedHandler((req, res, ex) -> writeJson(res, 403, "FORBIDDEN")));
    return http.build();
  }
}
```

`csrf.disable()` 只适用于不依赖浏览器自动携带 Cookie 的 Bearer Token API；如果使用 Session Cookie，应保留 CSRF 或采用等价的双提交/自定义校验。

### 7.4 方法级 RBAC

```java
@PreAuthorize("hasAuthority('sys:user:read')")
public Page<UserView> listUsers(...) { ... }

@PreAuthorize("hasRole('ADMIN') or @permissionEvaluator.canEditUser(authentication, #id)")
public void updateUser(long id, UserCommand command) { ... }
```

把复杂数据权限放在 Bean（如 `permissionEvaluator`）中，避免在 SpEL 中拼接 SQL。方法级鉴权要覆盖 Service 层，否则内部调用或异步任务可能绕过 Controller 规则。

### 7.5 JWT 资源服务器

```java
http.oauth2ResourceServer(oauth -> oauth
  .jwt(jwt -> jwt.jwtAuthenticationConverter(customConverter())));
```

验证 `iss`、`aud`、签名算法和时钟偏差；JWK 公钥应缓存并支持轮换。权限 Claim 到 `GrantedAuthority` 的转换必须统一前缀约定，例如直接使用 `sys:user:read`，不要一部分代码用 `ROLE_` 一部分不用。

---

## 8. Shiro 快速上手

### 8.1 Realm 实现

```java
public class RbacRealm extends AuthorizingRealm {
  private final UserService users;

  @Override
  protected AuthenticationInfo doGetAuthenticationInfo(AuthenticationToken token) {
    String username = (String) token.getPrincipal();
    User u = users.findEnabled(username)
        .orElseThrow(() -> new UnknownAccountException(username));
    return new SimpleAuthenticationInfo(u.getUsername(), u.getPasswordHash(),
        ByteSource.Util.bytes(u.getSalt()), getName());
  }

  @Override
  protected AuthorizationInfo doGetAuthorizationInfo(PrincipalCollection principals) {
    Long userId = (Long) principals.getPrimaryPrincipal();
    SimpleAuthorizationInfo info = new SimpleAuthorizationInfo();
    info.setRoles(users.roleCodes(userId));
    info.setStringPermissions(users.permissionCodes(userId));
    return info;
  }
}
```

配合 `HashedCredentialsMatcher`（或项目认可的 Argon2 matcher）完成凭证校验；不要把数据库密码当作明文 `SimpleCredentialsMatcher` 输入。

### 8.2 Spring Boot 配置要点

注册 `DefaultWebSecurityManager`、Realm、`ShiroFilterFactoryBean`，将 `/login` 和静态资源设为 `anon`，管理 API 设为 `authc`，敏感操作再用注解或程序式权限校验。缓存使用 Redis 时配置 `CacheManager`，并在角色/权限变更后显式清理对应主体缓存。

### 8.3 程序式调用

```java
Subject subject = SecurityUtils.getSubject();
subject.login(new UsernamePasswordToken(username, password));
if (!subject.isPermitted("sys:user:delete")) {
    throw new AuthorizationException("no permission");
}
subject.logout();
```

---

## 9. Sa-Token 快速上手

### 9.1 依赖与配置

```xml
<dependency>
  <groupId>cn.dev33</groupId>
  <artifactId>sa-token-spring-boot3-starter</artifactId>
  <version>${sa-token.version}</version>
</dependency>
<dependency>
  <groupId>cn.dev33</groupId>
  <artifactId>sa-token-redis-jackson</artifactId>
</dependency>
```

```yaml
sa-token:
  token-name: Authorization
  token-prefix: Bearer
  timeout: 7200
  active-timeout: 1800
  is-concurrent: false
  is-share: false
  is-log: true
```

### 9.2 登录、过滤器与权限接口

```java
// 登录成功后返回 Token
StpUtil.login(user.getId(), new SaLoginModel().setDevice("web"));
String token = StpUtil.getTokenValue();

@Component
class RbacStpInterface implements StpInterface {
  public List<String> getPermissionList(Object loginId, String loginType) {
    return permissionService.codes(Long.valueOf(loginId.toString()));
  }
  public List<String> getRoleList(Object loginId, String loginType) {
    return roleService.codes(Long.valueOf(loginId.toString()));
  }
}
```

```java
@SaCheckPermission("sys:user:delete")
@DeleteMapping("/api/users/{id}")
public void delete(@PathVariable long id) { ... }
```

全局 `SaServletFilter` 用于排除登录、健康检查等路径；Redis DAO 用于多实例共享会话。登录类型（`loginType`）可用于区分后台管理员、移动端用户等账号体系。

---

## 10. 定制认证与授权

### 10.1 多因素认证（MFA）

密码认证成功后不要立即建立高权限会话。先创建短期 `MFA_PENDING` 会话，校验 TOTP/短信/硬件密钥后再提升为完全会话。MFA 状态、尝试次数、锁定时间放 Redis，并绑定设备与 IP 风险策略。

### 10.2 动态权限加载与缓存失效

建议缓存键按 `tenant:user:permissionVersion` 组织，用户、角色、权限任一变更时递增版本并删除缓存。不要只清应用本地缓存；集群要使用 Redis Pub/Sub、消息队列或统一版本检查。

### 10.3 自定义 URL 决策器

统一保存 `HTTP_METHOD + PATH_PATTERN -> permissionCode`。匹配顺序应为“精确路径 > 模板路径 > 通配路径”，拒绝默认优先。对于 `GET /users/{id}` 这类资源，URL 权限只能判断动作，资源归属必须在 Service 中再次检查。

### 10.4 数据权限

将数据范围抽象成策略：`ALL`、`TENANT`、`ORG_AND_CHILDREN`、`SELF`、`CUSTOM`。在 Repository/查询构造器层注入条件，并禁止从客户端直接提交 `tenant_id`、`owner_id` 等隔离字段。SQL 拼接必须使用参数绑定和白名单列名。

### 10.5 自定义异常与响应

统一返回：

```json
{"code":"AUTH_REQUIRED","message":"请先登录","requestId":"..."}
```

401 表示缺少或无效身份，403 表示身份有效但无权。不要把“用户不存在”“密码错误”区分返回给外部，避免账号枚举；内部日志可保留原因和 traceId。

### 10.6 单点登录与第三方身份

OAuth2/OIDC 中，认证交给授权服务器，业务服务作为 Resource Server 验证 Access Token。只信任签名、`iss`、`aud`、过期时间和 nonce/state 校验通过的声明；不要仅凭前端传来的用户 ID 建立身份。注销要区分本地会话注销、IdP 会话注销和 Token 撤销。

---

## 11. 搭建可维护的 RBAC 服务

### 11.1 推荐模块

```text
security/
  SecurityConfig
  AuthenticationController
  CurrentUser
  PermissionAuthorizationManager
rbac/
  UserService / RoleService / PermissionService
  UserMapper / RoleMapper / PermissionMapper
  DataScopeEvaluator
audit/
  AuditEventListener / AuditRepository
```

框架适配代码放 `security`，业务授权规则放 `rbac`，避免 Controller 直接调用框架静态 API。用 `CurrentUser` 统一取得当前主体，便于测试和未来替换框架。

### 11.2 授权管理接口

至少提供：用户分页、启停用、重置密码、分配角色；角色 CRUD、分配权限；权限资源同步；用户会话查询、强制下线；授权变更审计。所有写操作需要二次确认或 MFA，并记录操作者、目标、前后值、IP、结果。

### 11.3 测试策略

- 单元测试：密码编码、权限合并、通配符匹配、数据范围策略。
- 集成测试：匿名/已登录/无权限三种响应；角色变更后的缓存失效；租户隔离。
- 安全测试：越权 IDOR、路径大小写绕过、方法覆盖、JWT 算法降级、CSRF、重放。
- 契约测试：前端菜单权限与后端权限 code 的一致性。

Spring Security 可用 `@WithMockUser` 和 MockMvc；Shiro 使用 `Subject` 测试工具；Sa-Token 使用测试环境 DAO。测试不能只验证前端按钮是否隐藏。

---

## 12. 安全、性能与运维

1. **密码**：Argon2id/BCrypt，设置合理 cost；登录失败限速与账号/IP 组合锁定。
2. **Token**：短有效期、轮换 Refresh Token、服务端撤销；禁止放 URL、日志和异常栈。
3. **Session**：固定会话攻击防护；登录成功后更换 Session ID；并发会话策略明确。
4. **Cookie**：`HttpOnly`、`Secure`、`SameSite`，生产全站 HTTPS。
5. **缓存**：Redis ACL、TLS、密钥空间隔离；缓存击穿时使用互斥锁或短 TTL。
6. **日志**：脱敏密码、Token、身份证和手机号；审计日志不可被普通管理员修改。
7. **性能**：一次请求只加载一次权限；权限查询用批量 SQL；避免 N+1；对动态决策器做本地短缓存并以版本号校验。
8. **监控**：登录成功率、失败率、锁定数、403 比例、Token 校验异常、Redis 延迟、权限缓存命中率。

---

## 13. 常见问题与排错

### 13.1 明明有权限却 403

检查权限字符串是否一致（大小写、前缀、空格）、是否加载了最新缓存、是否误用了 `hasRole`（它通常会自动添加 `ROLE_` 前缀）、方法安全是否启用、是否匹配了另一条更早的 Filter Chain。

### 13.2 登录成功但下一请求未登录

检查 SessionCreationPolicy、Cookie 域/路径、反向代理是否转发 `Set-Cookie`、JWT 是否真的附带 `Authorization: Bearer`、多实例是否共享 Redis/签名密钥，以及请求结束时是否错误清理了上下文。

### 13.3 权限修改不生效

确认数据库事务已提交、缓存键和租户维度正确、集群节点已收到失效通知；JWT 方案则确认权限版本或撤销机制存在。

### 13.4 放行规则被绕过

将 `permitAll`、`anon` 规则集中管理；禁止在 Controller 内仅依赖前端判断；对路径规范化、URL 编码、大小写、尾斜杠和 HTTP 方法做测试。

---

## 14. 落地检查清单

- [ ] 明确使用 Session、JWT 还是 Redis Token，并写出撤销方案。
- [ ] 用户、角色、权限、租户、组织和数据范围模型已评审。
- [ ] 所有敏感 API 有后端权限校验，Service 层不会被绕过。
- [ ] 密码使用 Argon2id/BCrypt，登录限速和 MFA 策略已启用。
- [ ] 401/403、登录失败、注销、踢人和权限变更均有统一响应与审计。
- [ ] 集群共享会话、缓存失效和密钥轮换方案已验证。
- [ ] 已覆盖越权、IDOR、CSRF、Token 重放、租户隔离和路径绕过测试。
- [ ] 已监控认证失败、403 激增、缓存命中率与异常登录行为。

### 最小决策

如果目标是快速搭建一个单体后台 RBAC：选 Sa-Token 或 Spring Security Session；如果未来需要 OAuth2/OIDC、微服务资源服务器和企业统一身份：直接选 Spring Security；如果已有 Shiro 体系且 Realm 设计成熟，不必为了替换框架而迁移，重点做好缓存、数据权限和审计。

---

## 15. 从零实现一个 RBAC 后端：完整落地流程

下面以“管理员管理用户”为例，把一次操作从数据库设计到框架校验串起来。推荐先实现一个垂直切片，再复制到角色、菜单和数据权限模块。

### 15.1 建议的工程结构

```text
src/main/java/com/example
├── common
│   ├── ApiResponse.java
│   ├── BusinessException.java
│   └── GlobalExceptionHandler.java
├── security
│   ├── SecurityConfig.java
│   ├── CurrentPrincipal.java
│   ├── LoginController.java
│   ├── JsonAuthenticationEntryPoint.java
│   ├── JsonAccessDeniedHandler.java
│   └── PermissionAuthorizationManager.java
├── rbac
│   ├── user/UserController.java
│   ├── user/UserService.java
│   ├── user/UserRepository.java
│   ├── role/RoleService.java
│   └── permission/PermissionService.java
└── audit
    ├── AuditAspect.java
    └── AuditRepository.java
```

Controller 只负责参数和 HTTP 语义，Service 负责业务约束与数据范围，Repository 负责持久化，security 只负责身份和通用授权。这样将来从 Shiro 切换到 Spring Security 时，RBAC 业务层无需大面积改动。

### 15.2 用户登录接口

```java
public record LoginCommand(
    @NotBlank String username,
    @NotBlank @Size(min = 8, max = 72) String password,
    String captcha,
    String deviceId) {}

@RestController
@RequestMapping("/auth")
class LoginController {
  private final AuthenticationManager authenticationManager;
  private final TokenService tokenService;

  @PostMapping("/login")
  ApiResponse<TokenView> login(@Valid @RequestBody LoginCommand command,
                               HttpServletRequest request) {
    var token = UsernamePasswordAuthenticationToken.unauthenticated(
        command.username(), command.password());
    var authentication = authenticationManager.authenticate(token);
    var result = tokenService.issue(authentication, command.deviceId(), request);
    return ApiResponse.ok(result);
  }
}
```

登录流程应在进入 `AuthenticationManager` 前完成验证码、登录频率、IP 风险检查；成功后记录登录事件，失败时只向客户端返回统一错误。`TokenService` 可以实现为 Session、JWT 或 Redis Token，Controller 不应感知细节。

### 15.3 当前用户对象

```java
public record CurrentPrincipal(long userId, long tenantId,
                               String username, Set<String> authorities) {
  boolean has(String permission) {
    return authorities.contains(permission)
        || authorities.contains(permission.substring(0, permission.lastIndexOf(':')) + ":*")
        || authorities.contains("*");
  }
}
```

不要在业务代码中到处调用 `SecurityContextHolder`、`StpUtil.getLoginId()` 或 `SecurityUtils.getSubject()`。封装一个 `CurrentPrincipalProvider`，在单元测试中可注入假用户，在异步任务中显式传递主体，避免线程上下文丢失。

### 15.4 用户管理 Service

```java
@Service
class UserService {
  private final UserRepository users;
  private final PermissionService permissions;

  @PreAuthorize("hasAuthority('sys:user:create')")
  @Transactional
  public long create(CreateUserCommand command, CurrentPrincipal operator) {
    if (users.existsByUsername(operator.tenantId(), command.username())) {
      throw new BusinessException("USERNAME_EXISTS");
    }
    var entity = UserEntity.create(operator.tenantId(), command.username(),
        passwordEncoder.encode(command.initialPassword()));
    users.insert(entity);
    audit("USER_CREATE", entity.id(), operator);
    return entity.id();
  }

  @PreAuthorize("hasAuthority('sys:user:update')")
  @Transactional
  public void update(long id, UpdateUserCommand command, CurrentPrincipal operator) {
    var target = users.findInTenant(id, operator.tenantId())
        .orElseThrow(() -> new NotFoundException("USER_NOT_FOUND"));
    dataScope.checkCanEdit(operator, target);
    target.changeProfile(command.nickname(), command.phone());
    users.update(target);
  }
}
```

关键点是**权限判断与租户过滤同时存在**。拥有 `sys:user:update` 只说明可以修改用户，不代表可以修改其他租户或其他组织的用户。

---

## 16. Spring Security 深度定制示例

### 16.1 自定义认证 Provider

适用于用户名 + 租户 + 密码、手机号 + 验证码等非标准登录。

```java
final class TenantPasswordAuthenticationToken
    extends AbstractAuthenticationToken {
  private final String username;
  private final String password;
  private final long tenantId;

  static TenantPasswordAuthenticationToken unauthenticated(
      long tenantId, String username, String password) {
    return new TenantPasswordAuthenticationToken(tenantId, username, password, false);
  }
  private final Object principal;
  private final Object credentials;
  private TenantPasswordAuthenticationToken(long tenantId, String u, String p,
                                            boolean authenticated) {
    super(null); this.tenantId = tenantId; this.username = u;
    this.password = p; this.principal = u; this.credentials = p;
    setAuthenticated(authenticated);
  }
  // getters omitted
}

@Component
class TenantPasswordProvider implements AuthenticationProvider {
  private final UserDetailsService users;
  private final PasswordEncoder encoder;

  public Authentication authenticate(Authentication input) {
    var token = (TenantPasswordAuthenticationToken) input;
    var user = users.loadUserByUsername(token.getName());
    if (!encoder.matches((String) token.getCredentials(), user.getPassword())) {
      throw new BadCredentialsException("bad credentials");
    }
    return new UsernamePasswordAuthenticationToken(user, null, user.getAuthorities());
  }
  public boolean supports(Class<?> type) {
    return TenantPasswordAuthenticationToken.class.isAssignableFrom(type);
  }
}
```

自定义 Token 不应把原始密码放入日志、异常和序列化对象；认证成功返回的 Authentication 必须带完整权限，并设置为已认证状态。

### 16.2 动态 AuthorizationManager

```java
@Component
class PermissionAuthorizationManager
    implements AuthorizationManager<RequestAuthorizationContext> {
  private final PermissionRuleCache cache;

  @Override
  public AuthorizationDecision check(Supplier<Authentication> auth,
                                      RequestAuthorizationContext context) {
    Authentication a = auth.get();
    if (a == null || !a.isAuthenticated()) return new AuthorizationDecision(false);
    String permission = cache.match(context.getRequest().getMethod(),
                                    context.getRequest().getRequestURI());
    return new AuthorizationDecision(permission == null
        || a.getAuthorities().stream().anyMatch(x -> x.getAuthority().equals(permission)));
  }
}
```

实际项目要补充：规则不存在时是放行还是拒绝、超级管理员如何处理、路径参数如何规范化、规则缓存如何发布失效事件。管理后台修改规则后，建议把 `permission_revision` 写入 Redis，过滤器每次只比较版本号，不必每次查全量权限。

### 16.3 前后端分离的无状态配置

```java
http
  .csrf(AbstractHttpConfigurer::disable)
  .sessionManagement(s -> s.sessionCreationPolicy(SessionCreationPolicy.STATELESS))
  .addFilterBefore(jwtFilter, UsernamePasswordAuthenticationFilter.class)
  .authorizeHttpRequests(a -> a
      .requestMatchers("/auth/**").permitAll()
      .anyRequest().authenticated());
```

JWT Filter 必须处理：没有 Header 时继续链路、Bearer 格式错误返回 401、签名/过期/受众校验失败返回 401、解析成功后设置上下文、请求结束不泄露上下文。不要在 Filter 里直接返回“无权限”，授权应由后面的 AuthorizationFilter 完成。

---

## 17. Shiro 深度定制示例

### 17.1 多 Realm 与认证策略

可以同时配置密码 Realm、短信 Realm、LDAP Realm。使用 `ModularRealmAuthenticator` 时要明确 `AtLeastOneSuccessfulStrategy`、`FirstSuccessfulStrategy` 或 `AllSuccessfulStrategy`，否则可能出现“密码 Realm 失败但短信 Realm 成功”的不可预期行为。

```java
DefaultWebSecurityManager manager = new DefaultWebSecurityManager();
manager.setRealms(List.of(passwordRealm, smsRealm));
manager.setAuthenticator(authenticatorWithStrategy());
manager.setSessionManager(redisSessionManager());
manager.setCacheManager(redisCacheManager());
```

### 17.2 自定义 Subject 工厂

需要把租户、设备、登录方式放进主体时，可自定义 `SubjectFactory` 或在 Principal 中使用不可变对象：`Principal(userId, tenantId, deviceId)`。不要把整个 User Entity 放入 Principal，以免密码、敏感字段被序列化进 Session。

### 17.3 自定义过滤器

实现 `AccessControlFilter` 时分别处理 `isAccessAllowed`、`onAccessDenied` 和登录跳转。REST API 不要默认重定向到 HTML 登录页，应返回 JSON 401。过滤器注册名必须和 URL 配置完全一致，过滤器链顺序要先完成身份恢复再做权限判断。

---

## 18. Sa-Token 深度定制示例

### 18.1 自定义登录逻辑与会话模型

```java
public void login(LoginCommand command) {
  User user = userService.verify(command.username(), command.password());
  SaLoginModel model = new SaLoginModel()
      .setDevice(command.deviceId())
      .setTimeout(Duration.ofHours(2).toSeconds())
      .setIsLastingCookie(false)
      .setExtra("tenantId", user.tenantId());
  StpUtil.login(user.id(), model);
}
```

`timeout` 是最长存活时间，`active-timeout` 是无操作超时；二者要根据后台、移动端、机器账号分别制定。机器账号不建议复用普通用户 Token，应使用专用登录类型和最小权限。

### 18.2 注解与路由双重鉴权

路由拦截器适合全局规则（登录、租户、黑名单），注解适合业务动作（删除、导出、审批）。高风险操作可组合：先 `@SaCheckPermission`，再在 Service 中执行资源所有权和数据范围判断。

### 18.3 事件监听

监听登录、注销、踢人、封禁和 Token 活跃事件，将审计写入消息队列或数据库。事件处理必须幂等，不能因为审计库短暂不可用而阻塞所有业务请求；关键安全事件则应采用可靠消息和重试。

---

## 19. 角色、权限、菜单、数据范围的关系

### 19.1 不要把菜单等同于权限

菜单决定“看见什么”，按钮权限决定“界面上能点什么”，API 权限决定“服务端允许什么”，数据范围决定“允许操作哪些记录”。一个菜单可以对应多个 API，一个 API 也可能被多个菜单使用。

```text
菜单：系统管理 / 用户管理
  ├── 按钮：新增 -> sys:user:create
  ├── 按钮：编辑 -> sys:user:update
  └── 页面加载 -> GET /api/users -> sys:user:read
```

### 19.2 角色继承

角色继承适用于“审计员继承只读权限”这类稳定关系，但要防止循环继承和权限爆炸。数据库保存 `role_parent`，加载时进行 DAG 校验；最多展开固定层数，或在发布角色时预计算闭包表。

### 19.3 数据范围实现方式

1. **Service 显式条件**：最透明，适合关键查询。
2. **MyBatis 拦截器**：代码少，但复杂 SQL、子查询和分页容易出错。
3. **数据库行级安全**：隔离强，但依赖数据库能力和连接上下文。
4. **独立授权服务**：跨系统统一，但有网络延迟和可用性成本。

建议关键写操作使用 Service 显式校验，普通列表查询再考虑拦截器或数据库行级策略。

---

## 20. 微服务与统一认证

### 20.1 网关不能替代服务鉴权

网关可以做 Token 初步校验、路由级粗粒度拦截和限流；每个业务服务仍要验证 Token 并执行自身权限与数据范围。内部调用应使用服务身份或短期下游 Token，不能仅凭网关转发的 `X-User-Id` 信任用户。

### 20.2 Token 传递与权限一致性

推荐 OIDC 授权服务器 + Spring Security Resource Server。Token 中保留 `sub`、`iss`、`aud`、`exp`、`jti`、`tenant_id` 和权限版本；服务本地缓存用户权限，版本变化时主动失效。跨服务传递用户上下文时使用标准 Header，并防止外部请求伪造内部 Header。

### 20.3 远程授权（PDP/PEP）

当权限规则需要统一治理，可拆分：PEP（业务服务中的执行点）、PDP（策略决策点）、PIP（用户、组织、资源属性提供者）。远程决策必须设置超时、熔断和默认拒绝策略，缓存只缓存明确允许且带版本的结果。

---

## 21. 可复制的测试用例矩阵

| 场景 | 预期 HTTP | 必测内容 |
|---|---:|---|
| 无 Token 访问保护接口 | 401 | 不泄露登录页 HTML |
| Token 过期 | 401 | 清理客户端会话，记录原因 |
| 已登录但无 API 权限 | 403 | 不调用 Service |
| 有权限但跨租户访问 | 404/403 | 不泄露目标是否存在 |
| 角色刚被撤销 | 403 | 缓存和 JWT 版本立即生效 |
| 用户被踢下线 | 401 | 所有节点都拒绝旧 Token |
| 管理员修改自身最高权限 | 拒绝或二次审批 | 防止锁死和权限自升级 |
| 导出大量数据 | 200/429 | 数据范围、限流、审计 |

对每个接口至少写“匿名、普通角色、目标角色、跨租户、禁用用户、过期会话”六类测试。安全测试应通过真实过滤器链，不要只调用 Controller 方法。

---

## 22. 版本迁移注意事项

### 22.1 Spring Security 5 到 6

- `WebSecurityConfigurerAdapter` 被移除，改为 `SecurityFilterChain` Bean。
- `authorizeRequests` 改为 `authorizeHttpRequests`。
- `antMatchers` 改为 `requestMatchers`。
- Spring Boot 3 使用 Jakarta 命名空间，旧的 `javax.servlet` 代码需迁移。
- 方法安全使用 `@EnableMethodSecurity`。

### 22.2 Shiro 1.x 到 2.x

核对 Jakarta Servlet、Spring Boot 3 starter、密码匹配器和 Web Filter 包名；不要直接复制旧博客中的 `javax` 配置。升级前先用集成测试锁定 401、403、Session 和 RememberMe 行为。

### 22.3 Sa-Token 版本升级

检查 starter 名称、Redis 序列化模块、Token 前缀、配置键和注解拦截器。升级时保留旧 Token 的兼容窗口，或主动使全量会话失效并通知用户重新登录。

---

## 23. 最终推荐方案

对于新建的 Spring Boot RBAC 后台，可以采用以下组合：

```text
OIDC/账号密码认证
        ↓
Spring Security 负责认证、JWT/Session 校验、401/403
        ↓
RBACService 负责角色权限与租户数据范围
        ↓
Redis 负责会话、权限版本、封禁与限流
        ↓
MySQL 负责用户、角色、权限、组织和审计持久化
```

若项目只需要简单的后台登录和注解权限，Sa-Token 可以降低接入成本；若已有 Shiro，围绕 Realm、缓存和数据权限做好治理即可。无论选择哪套框架，真正决定安全性的不是注解数量，而是：权限模型是否清晰、默认是否拒绝、数据范围是否在服务端强制执行、会话是否可撤销、变更是否可审计。
