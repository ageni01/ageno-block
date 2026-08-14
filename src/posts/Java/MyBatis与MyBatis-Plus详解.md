# MyBatis 与 MyBatis-Plus 详解

> 涵盖：XML 映射、注解 SQL、MyBatis-Plus 增强、分页、多租户、自动填充、逻辑删除、数据权限

---

## 目录

1. [MyBatis 核心架构](#1-mybatis-核心架构)
2. [XML 映射文件](#2-xml-映射文件)
3. [注解 SQL](#3-注解-sql)
4. [MyBatis-Plus 快速开始](#4-mybatis-plus-快速开始)
5. [MyBatis-Plus 核心功能](#5-mybatis-plus-核心功能)
6. [多租户（Tenant）](#6-多租户tenant)
7. [自动填充（MetaObjectHandler）](#7-自动填充metaobjecthandler)
8. [逻辑删除](#8-逻辑删除)
9. [数据权限](#9-数据权限)
10. [分页](#10-分页)
11. [动态 SQL 大全](#11-动态-sql-大全)
12. [插件机制与拦截器](#12-插件机制与拦截器)
13. [最佳实践与性能优化](#13-最佳实践与性能优化)

---

## 1. MyBatis 核心架构

```
                    ┌─────────────┐
                    │  SqlSession │
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
         ┌────┴───┐  ┌────┴────┐  ┌───┴────┐
         │Executor│  │Mapped   │  │SqlSession│
         │        │  │Statement│  │Factory   │
         └───┬────┘  └─────────┘  └────────┘
             │
     ┌───────┼───────────┐
     │       │           │
┌────┴──┐ ┌──┴───┐ ┌────┴────┐
│Stmt-  │ │Param-│ │ResultSet│
│Handler│ │Handler│ │Handler  │
└───────┘ └──────┘ └─────────┘
     │
     ▼
  JDBC / 数据库
```

### 核心组件

| 组件 | 职责 |
|------|------|
| `SqlSessionFactory` | 解析配置，创建 SqlSession |
| `SqlSession` | 数据库会话（增删改查入口） |
| `Executor` | 执行器（Simple / Reuse / Batch） |
| `MappedStatement` | 封装 SQL 配置（XML 或注解解析而来） |
| `StatementHandler` | JDBC Statement 处理 |
| `ParameterHandler` | 参数处理 |
| `ResultSetHandler` | 结果集映射 |
| `Interceptor` | 四组件拦截（可扩展插件） |

---

## 2. XML 映射文件

### 2.1 完整示例

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE mapper PUBLIC "-//mybatis.org//DTD Mapper 3.0//EN"
    "http://mybatis.org/dtd/mybatis-3-mapper.dtd">

<mapper namespace="com.example.mapper.UserMapper">

    <!-- 结果映射 -->
    <resultMap id="userMap" type="User">
        <id     column="id"       property="id" />
        <result column="user_name" property="userName" />
        <result column="password"  property="password" />
        <result column="email"     property="email" />
        <result column="tenant_id" property="tenantId" />
        <result column="deleted"   property="deleted" />
        <result column="create_time" property="createTime" />
        <result column="update_time" property="updateTime" />
        <!-- 关联映射 -->
        <association property="dept"   javaType="Dept"    column="dept_id"
                     select="com.example.mapper.DeptMapper.selectById" />
        <collection property="roles"  ofType="Role"
                     select="com.example.mapper.RoleMapper.selectByUserId" />
    </resultMap>

    <!-- CRUD -->
    <select id="selectById" resultMap="userMap">
        SELECT * FROM t_user WHERE id = #{id}
    </select>

    <insert id="insert" useGeneratedKeys="true" keyProperty="id">
        INSERT INTO t_user (user_name, password, email, tenant_id)
        VALUES (#{userName}, #{password}, #{email}, #{tenantId})
    </insert>

    <update id="updateById">
        UPDATE t_user
        SET user_name = #{userName}, password = #{password}
        WHERE id = #{id}
    </update>

    <delete id="deleteById">
        DELETE FROM t_user WHERE id = #{id}
    </delete>

    <!-- 动态 SQL -->
    <select id="selectByCondition" resultMap="userMap">
        SELECT * FROM t_user
        <where>
            <if test="userName != null and userName != ''">
                AND user_name LIKE CONCAT('%', #{userName}, '%')
            </if>
            <if test="email != null and email != ''">
                AND email = #{email}
            </if>
            <if test="tenantId != null">
                AND tenant_id = #{tenantId}
            </if>
        </where>
        ORDER BY create_time DESC
        LIMIT #{offset}, #{limit}
    </select>

</mapper>
```

### 2.2 XML 标签大全

| 标签 | 用途 |
|------|------|
| `<select>` | 查询 |
| `<insert>` | 插入 |
| `<update>` | 更新 |
| `<delete>` | 删除 |
| `<resultMap>` | 结果映射 |
| `<sql>` | SQL 片段 |
| `<include>` | 引用 SQL 片段 |
| `<where>` | 自动处理 WHERE + 去除首个 AND/OR |
| `<set>` | 自动处理 SET + 去除末尾逗号 |
| `<if>` | 条件判断 |
| `<choose>/<when>/<otherwise>` | switch-case |
| `<trim>` | 自定义前后缀处理 |
| `<foreach>` | 集合遍历 |
| `<bind>` | 绑定变量 |
| `<selectKey>` | 自增主键回写（Oracle 等） |

### 2.3 #{} vs ${}

```xml
<!-- #{} : 预编译参数（推荐），防 SQL 注入 -->
SELECT * FROM t_user WHERE id = #{id}
-- 生成: SELECT * FROM t_user WHERE id = ?

<!-- ${} : 字符串替换（有注入风险），仅用于表名/列名/排序等 -->
SELECT * FROM ${tableName} ORDER BY ${orderColumn}
-- ⚠️ 不要用于用户输入的值
```

### 2.4 sql 片段

```xml
<sql id="baseColumns">
    id, user_name, password, email, tenant_id,
    create_time, update_time, deleted
</sql>

<sql id="baseWhere">
    <where>
        <if test="tenantId != null">
            AND tenant_id = #{tenantId}
        </if>
        <if test="deleted != null">
            AND deleted = #{deleted}
        </if>
    </where>
</sql>

<select id="selectAll" resultMap="userMap">
    SELECT <include refid="baseColumns" />
    FROM t_user
    <include refid="baseWhere" />
</select>
```

---

## 3. 注解 SQL

### 3.1 基本注解

```java
public interface UserMapper {

    @Select("SELECT * FROM t_user WHERE id = #{id}")
    User selectById(Long id);

    @Insert("INSERT INTO t_user (user_name, password, email, tenant_id) " +
            "VALUES (#{userName}, #{password}, #{email}, #{tenantId})")
    @Options(useGeneratedKeys = true, keyProperty = "id")
    int insert(User user);

    @Update("UPDATE t_user SET user_name = #{userName} WHERE id = #{id}")
    int updateById(User user);

    @Delete("DELETE FROM t_user WHERE id = #{id}")
    int deleteById(Long id);
}
```

### 3.2 注解 + XML 混合

```java
// 复杂的 SQL 写在 XML，简单的用注解
public interface OrderMapper {

    // 简单查询用注解
    @Select("SELECT * FROM t_order WHERE order_no = #{orderNo}")
    Order findByOrderNo(String orderNo);

    // 复杂查询引用 XML
    List<Order> selectByCondition(@Param("userId") Long userId,
                                  @Param("status") Integer status,
                                  @Param("startTime") Date startTime);
}
```

### 3.3 高级注解

```java
// @Results = XML resultMap
@Results(id = "userMap", value = {
    @Result(property = "id",       column = "id",         id = true),
    @Result(property = "userName", column = "user_name"),
    @Result(property = "password", column = "password"),
    @Result(property = "tenantId", column = "tenant_id"),
    @Result(property = "dept",     column = "dept_id",
            one = @One(select = "com.example.mapper.DeptMapper.selectById")),
    @Result(property = "roles",    column = "id",
            many = @Many(select = "com.example.mapper.RoleMapper.selectByUserId"))
})
@Select("SELECT * FROM t_user WHERE id = #{id}")
User selectById(Long id);

// 复用 @Results
@ResultMap("userMap")
@Select("SELECT * FROM t_user WHERE user_name = #{userName}")
User findByUserName(String userName);
```

### 3.4 注解动态 SQL（@SelectProvider）

```java
// 适用：简单动态 SQL，复杂还是用 XML
@SelectProvider(type = UserSqlProvider.class, method = "selectByCondition")
List<User> selectByCondition(UserQuery query);

public class UserSqlProvider {

    public String selectByCondition(UserQuery query) {
        return new SQL() {{
            SELECT("*");
            FROM("t_user");
            if (query.getUserName() != null) {
                WHERE("user_name LIKE CONCAT('%', #{userName}, '%')");
            }
            if (query.getEmail() != null) {
                WHERE("email = #{email}");
            }
            if (query.getTenantId() != null) {
                WHERE("tenant_id = #{tenantId}");
            }
            ORDER_BY("create_time DESC");
        }}.toString();
    }
}
```

---

## 4. MyBatis-Plus 快速开始

### 4.1 依赖

```xml
<dependency>
    <groupId>com.baomidou</groupId>
    <artifactId>mybatis-plus-boot-starter</artifactId>
    <version>3.5.5</version>
</dependency>
```

### 4.2 基类 & Mapper

```java
// ========== 实体类 ==========
@Data
@TableName("t_user")
public class User {

    @TableId(type = IdType.AUTO)
    private Long id;

    @TableField("user_name")
    private String userName;

    private String password;
    private String email;

    @TableField(fill = FieldFill.INSERT)
    private Long tenantId;

    @TableField(fill = FieldFill.INSERT)
    private Date createTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private Date updateTime;

    @TableLogic
    @TableField("deleted")
    private Integer deleted;
}

// ========== Mapper ==========
// 继承 BaseMapper 即拥有 CRUD + 分页
public interface UserMapper extends BaseMapper<User> {
    // 自定义方法
    @Select("SELECT * FROM t_user WHERE email = #{email}")
    User findByEmail(String email);
}

// ========== Service ==========
// IService 提供了大量模板方法
public interface UserService extends IService<User> {
    User findByEmail(String email);
}

@Service
public class UserServiceImpl extends ServiceImpl<UserMapper, User>
        implements UserService {
    @Override
    public User findByEmail(String email) {
        return baseMapper.findByEmail(email);
    }
}
```

### 4.3 无需写 SQL 的 CRUD

```java
@RestController
@RequestMapping("/users")
public class UserController {

    @Autowired
    private UserService userService;

    // 增
    @PostMapping
    public boolean add(@RequestBody User user) {
        return userService.save(user);
    }

    // 批量
    @PostMapping("/batch")
    public boolean addBatch(@RequestBody List<User> users) {
        return userService.saveBatch(users);
    }

    // 删
    @DeleteMapping("/{id}")
    public boolean remove(@PathVariable Long id) {
        return userService.removeById(id);
    }

    // 改
    @PutMapping
    public boolean update(@RequestBody User user) {
        return userService.updateById(user);
    }

    // 查（单条）
    @GetMapping("/{id}")
    public User get(@PathVariable Long id) {
        return userService.getById(id);
    }

    // 查（条件）
    @GetMapping
    public List<User> list(@RequestParam(required = false) String userName) {
        return userService.list(
            new LambdaQueryWrapper<User>()
                .like(userName != null, User::getUserName, userName)
                .orderByDesc(User::getCreateTime)
        );
    }

    // 分页
    @GetMapping("/page")
    public Page<User> page(@RequestParam int page, @RequestParam int size) {
        return userService.page(new Page<>(page, size));
    }
}
```

---

## 5. MyBatis-Plus 核心功能

### 5.1 条件构造器（Wrapper）

| Wrapper 类 | 用途 |
|------------|------|
| `QueryWrapper<T>` | 查询条件（字符串字段名） |
| `LambdaQueryWrapper<T>` | 查询条件（Lambda，编译期校验） |
| `UpdateWrapper<T>` | 更新条件（可指定 SET 子句） |
| `LambdaUpdateWrapper<T>` | 更新条件（Lambda 版） |

```java
// ========== LambdaQueryWrapper（推荐） ==========
List<User> users = userService.list(
    new LambdaQueryWrapper<User>()
        .eq(User::getTenantId, 100L)                    // =
        .ne(User::getDeleted, 1)                        // <>
        .like(User::getUserName, "张")                   // LIKE
        .likeLeft(User::getEmail, "@qq.com")            // LIKE '%...'
        .likeRight(User::getPhone, "138")               // LIKE '...%'
        .ge(User::getCreateTime, startTime)             // >=
        .lt(User::getCreateTime, endTime)               // <
        .in(User::getStatus, 1, 2, 3)                   // IN
        .notIn(User::getRole, "admin", "guest")         // NOT IN
        .isNull(User::getEmail)                         // IS NULL
        .isNotNull(User::getPhone)                      // IS NOT NULL
        .between(User::getAge, 18, 60)                  // BETWEEN
        .notBetween(User::getAge, 0, 17)                // NOT BETWEEN
        .apply("date_format(create_time, '%Y-%m-%d') = {0}", today)
        .last("LIMIT 10")                               // 追加
        .orderByAsc(User::getSort)                      // ORDER BY ASC
        .orderByDesc(User::getCreateTime)               // ORDER BY DESC
);

// 嵌套条件
List<User> users = userService.list(
    new LambdaQueryWrapper<User>()
        .and(w -> w.eq(User::getStatus, 1)
                   .or().eq(User::getStatus, 2))
        .or(w -> w.eq(User::getRole, "admin")
                  .eq(User::getTenantId, 100L))
);

// ========== UpdateWrapper ==========
userService.update(
    new LambdaUpdateWrapper<User>()
        .set(User::getEmail, "new@email.com")
        .set(User::getPassword, "newpwd")
        .eq(User::getId, 1L)
);

// 带条件的更新（实体 + wrapper）
User user = new User();
user.setEmail("test@email.com");
userService.update(user,
    new LambdaUpdateWrapper<User>()
        .eq(User::getId, 1L)
        .eq(User::getTenantId, 100L)
);

// ========== QueryWrapper（不推荐但兼容） ==========
QueryWrapper<User> qw = new QueryWrapper<>();
qw.eq("tenant_id", 100L)
  .like("user_name", "张")
  .orderByDesc("create_time");
```

### 5.2 常用注解

```java
@Data
@TableName("t_user")                     // 表名映射
@KeySequence("seq_user")                 // 序列（Oracle）
public class User {

    @TableId(type = IdType.AUTO)         // 主键策略
    private Long id;

    @TableField("user_name")             // 字段映射
    private String userName;

    @TableField(exist = false)           // 非数据库字段
    private String extraInfo;

    @TableField(fill = FieldFill.INSERT) // 自动填充
    private Date createTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private Date updateTime;

    @TableLogic                          // 逻辑删除
    private Integer deleted;

    @Version                             // 乐观锁
    private Integer version;
}
```

### 5.3 主键策略

| 策略 | 说明 |
|------|------|
| `IdType.AUTO` | 数据库自增 |
| `IdType.NONE` | 未设置（需手动赋值） |
| `IdType.INPUT` | 手动输入 |
| `IdType.ASSIGN_ID` | 雪花算法（默认） |
| `IdType.ASSIGN_UUID` | UUID |
| `IdType.SEQUENCE` | 序列（Oracle/PostgreSQL） |

---

## 6. 多租户（Tenant）

### 6.1 什么是多租户

```
多租户三种方案：

1. 独立数据库 ── 隔离最彻底，成本最高
   租户A: db_a               租户B: db_b
   ┌─────────┐               ┌─────────┐
   │ t_user  │               │ t_user  │
   └─────────┘               └─────────┘

2. 共享数据库，独立 Schema ── 中等隔离
   租户A: public.t_user      租户B: public.t_user

3. 共享表，通过 tenant_id 区分 ── 隔离最弱，成本最低（最常用）
   t_user
   ┌────┬───────┬───────────┐
   │ id │ name  │ tenant_id │
   ├────┼───────┼───────────┤
   │ 1  │ 张三   │ 100       │
   │ 2  │ 李四   │ 200       │
   └────┴───────┴───────────┘
```

### 6.2 MyBatis-Plus 租户拦截器

```java
@Configuration
public class MybatisPlusConfig {

    @Bean
    public MybatisPlusInterceptor mybatisPlusInterceptor() {
        MybatisPlusInterceptor interceptor = new MybatisPlusInterceptor();

        // 租户插件（自动为 SQL 追加 tenant_id = ?）
        interceptor.addInnerInterceptor(new TenantLineInnerInterceptor(
            new TenantLineHandler() {
                @Override
                public String getTenantIdColumn() {
                    return "tenant_id";  // 列名
                }

                @Override
                public Expression getTenantId() {
                    // 从当前上下文获取租户 ID
                    Long tenantId = TenantContext.getCurrentTenantId();
                    if (tenantId == null) {
                        throw new RuntimeException("tenantId not found");
                    }
                    return new LongValue(tenantId);
                }

                @Override
                public boolean ignoreTable(String tableName) {
                    // 忽略某些表（如系统表、字典表）
                    return tableName.startsWith("sys_")
                        || tableName.equals("t_dict")
                        || tableName.equals("t_config");
                }
            }
        ));

        return interceptor;
    }
}
```

### 6.3 租户上下文

```java
// ========== 租户上下文（ThreadLocal） ==========
public class TenantContext {
    private static final ThreadLocal<Long> TENANT_HOLDER = new ThreadLocal<>();

    public static void setCurrentTenantId(Long tenantId) {
        TENANT_HOLDER.set(tenantId);
    }

    public static Long getCurrentTenantId() {
        return TENANT_HOLDER.get();
    }

    public static void clear() {
        TENANT_HOLDER.remove();
    }
}

// ========== 过滤器：从请求中解析租户 ==========
@Component
public class TenantFilter implements Filter {
    @Override
    public void doFilter(ServletRequest request, ServletResponse response,
                         FilterChain chain) throws IOException, ServletException {
        HttpServletRequest req = (HttpServletRequest) request;
        String tenantId = req.getHeader("X-Tenant-Id");  // 从 Header 读
        if (tenantId != null) {
            TenantContext.setCurrentTenantId(Long.parseLong(tenantId));
        }
        try {
            chain.doFilter(request, response);
        } finally {
            TenantContext.clear();  // ⚠️ 必须清理
        }
    }
}

// ========== 忽略租户的场景 ==========
// 登录接口无需租户（登录前还不知道租户）
@Bean
public TenantLineHandler tenantLineHandler() {
    return new TenantLineHandler() {
        @Override
        public boolean ignoreTable(String tableName) {
            return tableName.equals("sys_user")  // 系统用户表
                || tableName.equals("t_tenant")  // 租户表
                || tableName.equals("t_dict");
        }

        @Override
        public Expression getTenantId() {
            return new LongValue(TenantContext.getCurrentTenantId());
        }
    };
}

// 方法级别忽略租户
@InterceptorIgnore(tenantLine = "true")
@Select("SELECT * FROM t_dict WHERE type = #{type}")
List<Dict> selectDictByType(String type);

// 或写在 Mapper 上
@InterceptorIgnore(tenantLine = "true")
public interface DictMapper extends BaseMapper<Dict> {
}
```

### 6.4 拦截后 SQL 示例

```sql
-- 原始 SQL
SELECT * FROM t_user WHERE status = 1

-- 拦截后（自动追加 AND tenant_id = 100）
SELECT * FROM t_user WHERE status = 1 AND tenant_id = 100

-- 插入拦截
-- 原始
INSERT INTO t_user (user_name, email) VALUES ('张三', 'z@email.com')
-- 拦截后（自动补 tenant_id 列和值）
INSERT INTO t_user (user_name, email, tenant_id) VALUES ('张三', 'z@email.com', 100)

-- 更新拦截
UPDATE t_user SET email = 'new@email.com' WHERE id = 1
-- 拦截后
UPDATE t_user SET email = 'new@email.com' WHERE id = 1 AND tenant_id = 100

-- 删除拦截
DELETE FROM t_user WHERE id = 1
-- 拦截后
DELETE FROM t_user WHERE id = 1 AND tenant_id = 100
```

### 6.5 共享租户表数据（多租户读取某些共有数据）

```java
// 需求：所有租户都能读到的公共数据（tenant_id = 0）
// 方案：在 ignoreTable 中放行，或自定义 SQL 时 union
@Select("SELECT * FROM t_global_config WHERE tenant_id = 0 " +
        "OR tenant_id = #{tenantId}")
List<Config> selectConfigs(@Param("tenantId") Long tenantId);

// 或在 TenantLineHandler 中特殊处理
public class CustomTenantHandler implements TenantLineHandler {
    @Override
    public Expression getTenantId() {
        return new LongValue(TenantContext.getCurrentTenantId());
    }

    @Override
    public boolean ignoreTable(String tableName) {
        return tableName.startsWith("sys_")
            || tableName.startsWith("global_");
    }
}
```

---

## 7. 自动填充（MetaObjectHandler）

### 7.1 通用填充器

```java
@Component
public class MyMetaObjectHandler implements MetaObjectHandler {

    @Override
    public void insertFill(MetaObject metaObject) {
        // 新增时自动填充
        this.strictInsertFill(metaObject, "createTime", Date.class, new Date());
        this.strictInsertFill(metaObject, "updateTime", Date.class, new Date());
        this.strictInsertFill(metaObject, "deleted", Integer.class, 0);
        // 自动填充租户
        this.strictInsertFill(metaObject, "tenantId", Long.class,
                TenantContext.getCurrentTenantId());
        // 自动填充创建人
        this.strictInsertFill(metaObject, "createBy", String.class,
                UserContext.getCurrentUserId());
    }

    @Override
    public void updateFill(MetaObject metaObject) {
        // 更新时自动填充
        this.strictUpdateFill(metaObject, "updateTime", Date.class, new Date());
        this.strictUpdateFill(metaObject, "updateBy", String.class,
                UserContext.getCurrentUserId());
    }
}
```

### 7.2 实体类配置

```java
@Data
public class BaseEntity {

    @TableField(fill = FieldFill.INSERT)
    private Long tenantId;

    @TableField(fill = FieldFill.INSERT)
    private Date createTime;

    @TableField(fill = FieldFill.INSERT)
    private String createBy;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private Date updateTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private String updateBy;

    @TableField(fill = FieldFill.INSERT)
    @TableLogic
    private Integer deleted;
}

// 实体继承基类即可
@Data
@TableName("t_user")
public class User extends BaseEntity {
    @TableId(type = IdType.AUTO)
    private Long id;
    private String userName;
    private String email;
}
```

---

## 8. 逻辑删除

### 8.1 配置

```yaml
# application.yml
mybatis-plus:
  global-config:
    db-config:
      logic-delete-field: deleted     # 全局逻辑删除字段名
      logic-delete-value: 1           # 删除值
      logic-not-delete-value: 0       # 未删除值
```

### 8.2 使用

```java
@Data
@TableName("t_user")
public class User {
    @TableId
    private Long id;

    @TableLogic                          // 标注逻辑删除字段
    private Integer deleted;
}

// ⚠️ 逻辑删除后，MyBatis-Plus 自动拦截：
//   DELETE → UPDATE t_user SET deleted = 1 WHERE id = ?
//   SELECT → 自动追加 AND deleted = 0
//   UPDATE → 自动追加 AND deleted = 0

// 查询时忽略逻辑删除
// 方式一：手动写 SQL（绕过拦截）
@Select("SELECT * FROM t_user WHERE id = #{id}")
User selectAnyById(Long id);

// 方式二：注解忽略
@InterceptorIgnore(illegalSql = "true")
List<User> listAllWithDeleted();
```

### 8.3 SQL 拦截效果

```sql
-- delete
DELETE FROM t_user WHERE id = 1
-- 被拦截为:
UPDATE t_user SET deleted = 1 WHERE id = 1 AND deleted = 0

-- select
SELECT * FROM t_user WHERE status = 1
-- 被拦截为:
SELECT * FROM t_user WHERE status = 1 AND deleted = 0

-- update
UPDATE t_user SET email = 'new@email.com' WHERE id = 1
-- 被拦截为:
UPDATE t_user SET email = 'new@email.com' WHERE id = 1 AND deleted = 0
```

---

## 9. 数据权限

### 9.1 自定义数据权限拦截器

```java
// 需求：不同角色看到不同数据
// - 管理员：所有数据
// - 部门经理：本部门数据
// - 普通员工：自己的数据

@Component
public class DataScopeInterceptor implements InnerInterceptor {

    @Override
    public void beforeQuery(Executor executor, MappedStatement ms,
                            Object parameter, RowBounds rowBounds,
                            ResultHandler resultHandler, BoundSql boundSql) {
        // 获取当前用户信息
        User currentUser = UserContext.getCurrentUser();

        // 管理员不拦截
        if (currentUser.isAdmin()) return;

        // 获取原始 SQL
        String originalSql = boundSql.getSql();
        String newSql;

        switch (currentUser.getRole()) {
            case "dept_manager":
                newSql = "SELECT * FROM (" + originalSql + ") tmp " +
                         "WHERE dept_id = " + currentUser.getDeptId();
                break;
            case "employee":
                newSql = "SELECT * FROM (" + originalSql + ") tmp " +
                         "WHERE create_by = " + currentUser.getId();
                break;
            default:
                newSql = originalSql;
        }

        // 通过反射修改 BoundSql 中的 SQL
        ReflectionUtil.setFieldValue(boundSql, "sql", newSql);
    }
}
```

### 9.2 使用 MyBatis-Plus 拦截器链

```java
@Configuration
public class MybatisPlusConfig {

    @Bean
    public MybatisPlusInterceptor mybatisPlusInterceptor() {
        MybatisPlusInterceptor interceptor = new MybatisPlusInterceptor();

        // 顺序：数据权限 → 多租户 → 分页
        // 数据权限（内层 SELECT + 外层 WHERE）
        interceptor.addInnerInterceptor(new DataScopeInnerInterceptor());

        // 多租户
        interceptor.addInnerInterceptor(new TenantLineInnerInterceptor(
                new CustomTenantHandler()));

        // 分页
        interceptor.addInnerInterceptor(new PaginationInnerInterceptor(
                DbType.MYSQL));

        return interceptor;
    }
}
```

---

## 10. 分页

### 10.1 配置

```java
@Configuration
public class MybatisPlusConfig {

    @Bean
    public MybatisPlusInterceptor mybatisPlusInterceptor() {
        MybatisPlusInterceptor interceptor = new MybatisPlusInterceptor();

        // 分页插件（必须加在拦截器链中）
        interceptor.addInnerInterceptor(
            new PaginationInnerInterceptor(DbType.MYSQL));

        // 最大单页限制（防止拖库）
        PaginationInnerInterceptor pagination =
            new PaginationInnerInterceptor(DbType.MYSQL);
        pagination.setMaxLimit(500L);
        interceptor.addInnerInterceptor(pagination);

        return interceptor;
    }
}
```

### 10.2 使用

```java
// ========== Service 层 ==========
public Page<User> pageUsers(int pageNum, int pageSize, String userName) {
    Page<User> page = new Page<>(pageNum, pageSize);

    return userService.page(page,
        new LambdaQueryWrapper<User>()
            .like(userName != null, User::getUserName, userName)
            .orderByDesc(User::getCreateTime)
    );
}

// ========== Controller 层 ==========
@GetMapping("/page")
public Result<Page<User>> page(
        @RequestParam(defaultValue = "1") int page,
        @RequestParam(defaultValue = "20") int size,
        @RequestParam(required = false) String userName) {

    Page<User> result = userService.pageUsers(page, size, userName);

    // 返回给前端
    return Result.success(result);
    // result 包含:
    //   result.getRecords()       // 当前页数据
    //   result.getTotal()         // 总记录数
    //   result.getPages()         // 总页数
    //   result.getCurrent()       // 当前页
    //   result.getSize()          // 每页大小
}

// ========== XML 自定义分页 ==========
// Mapper
Page<User> selectUserPage(Page<?> page, @Param("userName") String userName);

// XML
<select id="selectUserPage" resultMap="userMap">
    SELECT * FROM t_user
    <where>
        <if test="userName != null and userName != ''">
            AND user_name LIKE CONCAT('%', #{userName}, '%')
        </if>
    </where>
    ORDER BY create_time DESC
</select>
<!-- ⚠️ 传入 Page 参数时，MyBatis-Plus 自动拦截追加 LIMIT/COUNT -->
<!-- 不需要手动写 LIMIT -->
```

### 10.3 分页拦截后的 SQL

```sql
-- 原始 SQL（Page 参数传入）
SELECT * FROM t_user WHERE tenant_id = 100 ORDER BY create_time DESC

-- 拦截后（自动执行两条 SQL）
-- 1. 总数查询
SELECT COUNT(*) FROM t_user WHERE tenant_id = 100

-- 2. 分页查询
SELECT * FROM t_user WHERE tenant_id = 100 ORDER BY create_time DESC LIMIT 0, 20
```

---

## 11. 动态 SQL 大全

### 11.1 if

```xml
<if test="userName != null and userName != ''">
    AND user_name LIKE CONCAT('%', #{userName}, '%')
</if>

<!-- 多个条件 -->
<if test="status != null">
    AND status IN
    <foreach collection="status" item="s" open="(" separator="," close=")">
        #{s}
    </foreach>
</if>
```

### 11.2 choose (switch-case)

```xml
<choose>
    <when test="sortField == 'name'">
        ORDER BY user_name
    </when>
    <when test="sortField == 'time'">
        ORDER BY create_time
    </when>
    <otherwise>
        ORDER BY id
    </otherwise>
</choose>
```

### 11.3 where / set / trim

```xml
<!-- where: 自动处理 AND/OR 前缀 -->
<where>
    <if test="userName != null">AND user_name LIKE ...</if>
    <if test="email != null">   AND email = ...</if>
</where>

<!-- set: 自动处理 SET 后缀逗号 -->
<update>
    UPDATE t_user
    <set>
        <if test="userName != null">user_name = #{userName},</if>
        <if test="email != null">   email = #{email},</if>
    </set>
    WHERE id = #{id}
</update>

<!-- trim: 自定义前后缀 -->
<trim prefix="SET" suffixOverrides=",">
    <if test="userName != null">user_name = #{userName},</if>
    <if test="email != null">   email = #{email},</if>
</trim>

<trim prefix="WHERE" prefixOverrides="AND |OR ">
    <if test="userName != null">AND user_name LIKE ...</if>
</trim>
```

### 11.4 foreach

```xml
<!-- IN 查询 -->
SELECT * FROM t_user WHERE id IN
<foreach collection="ids" item="id" open="(" separator="," close=")">
    #{id}
</foreach>

<!-- 批量插入 -->
INSERT INTO t_user (user_name, email, tenant_id) VALUES
<foreach collection="list" item="user" separator=",">
    (#{user.userName}, #{user.email}, #{user.tenantId})
</foreach>

<!-- 批量更新（MySQL） -->
<foreach collection="list" item="user" separator=";">
    UPDATE t_user SET user_name = #{user.userName}
    WHERE id = #{user.id}
</foreach>
```

### 11.5 bind

```xml
<!-- 声明变量 -->
<bind name="likeName" value="'%' + userName + '%'" />
SELECT * FROM t_user WHERE user_name LIKE #{likeName}

<bind name="offset" value="(pageNum - 1) * pageSize" />
<bind name="limit"  value="pageSize" />
SELECT * FROM t_user LIMIT #{offset}, #{limit}
```

### 11.6 注解动态 SQL（MyBatis-Plus 风格）

```java
// 使用 @Select + 条件构造器（MBD 自动处理）
// 不需要写 Provider，直接用 Wrapper
@Select("SELECT * FROM t_user ${ew.customSqlSegment}")
List<User> selectByWrapper(@Param(Constants.WRAPPER) Wrapper<User> wrapper);

// 调用
userMapper.selectByWrapper(
    new LambdaQueryWrapper<User>()
        .eq(User::getTenantId, 100L)
        .like(User::getUserName, "张")
        .orderByDesc(User::getCreateTime)
);
```

---

## 12. 插件机制与拦截器

### 12.1 MyBatis 四大拦截点

```java
@Intercepts({
    @Signature(
        type = Executor.class,          // 执行器
        method = "update",
        args = {MappedStatement.class, Object.class}
    ),
    @Signature(
        type = Executor.class,
        method = "query",
        args = {MappedStatement.class, Object.class,
                RowBounds.class, ResultHandler.class}
    ),
    @Signature(
        type = StatementHandler.class,  // 语句处理器
        method = "prepare",
        args = {Connection.class, Integer.class}
    ),
    @Signature(
        type = ParameterHandler.class,  // 参数处理器
        method = "setParameters",
        args = {PreparedStatement.class}
    ),
    @Signature(
        type = ResultSetHandler.class,  // 结果处理器
        method = "handleResultSets",
        args = {Statement.class}
    )
})
public class MyInterceptor implements Interceptor {

    @Override
    public Object intercept(Invocation invocation) throws Throwable {
        // 拦截逻辑
        Object target = invocation.getTarget();
        Method method = invocation.getMethod();
        Object[] args = invocation.getArgs();

        // 获取 SQL
        if (target instanceof StatementHandler) {
            StatementHandler sh = (StatementHandler) target;
            String sql = sh.getBoundSql().getSql();
            System.out.println("SQL: " + sql);
        }

        return invocation.proceed();
    }

    @Override
    public Object plugin(Object target) {
        return Plugin.wrap(target, this);
    }

    @Override
    public void setProperties(Properties properties) {
        // 读取配置参数
    }
}
```

### 12.2 MyBatis-Plus 内置拦截器

| 拦截器 | 用途 |
|--------|------|
| `PaginationInnerInterceptor` | 分页 |
| `TenantLineInnerInterceptor` | 多租户 |
| `DynamicTableNameInnerInterceptor` | 动态表名 |
| `OptimisticLockerInnerInterceptor` | 乐观锁 |
| `IllegalSQLInnerInterceptor` | SQL 性能校验 |
| `BlockAttackInnerInterceptor` | 阻止全表更新/删除 |

```java
@Bean
public MybatisPlusInterceptor mybatisPlusInterceptor() {
    MybatisPlusInterceptor interceptor = new MybatisPlusInterceptor();

    // 多租户
    interceptor.addInnerInterceptor(new TenantLineInnerInterceptor(...));

    // 分页
    interceptor.addInnerInterceptor(new PaginationInnerInterceptor(DbType.MYSQL));

    // 乐观锁
    interceptor.addInnerInterceptor(new OptimisticLockerInnerInterceptor());

    // 防止全表更新/删除
    interceptor.addInnerInterceptor(new BlockAttackInnerInterceptor());

    // SQL 性能校验（慢 SQL 告警）
    IllegalSQLInnerInterceptor illegal = new IllegalSQLInnerInterceptor();
    // 可配置
    interceptor.addInnerInterceptor(illegal);

    return interceptor;
}
```

### 12.3 乐观锁

```java
// 1. 实体字段加 @Version
@Data
public class User {
    @Version
    private Integer version;
}

// 2. 使用方式（自动处理 version +1）
User user = userService.getById(1L);
user.setEmail("new@email.com");
userService.updateById(user);
// 执行的 SQL:
// UPDATE t_user SET email = 'new@email.com', version = version + 1
// WHERE id = 1 AND version = 旧version
// 返回影响的记录数 = 0 则说明被并发修改
```

### 12.4 动态表名

```java
// 按年月分表
@Bean
public MybatisPlusInterceptor mybatisPlusInterceptor() {
    MybatisPlusInterceptor interceptor = new MybatisPlusInterceptor();

    DynamicTableNameInnerInterceptor dynamic = new DynamicTableNameInnerInterceptor();
    dynamic.setTableNameHandlerMap(new HashMap<String, TableNameHandler>() {{
        put("t_order", (sql, tableName) -> {
            // 根据当前时间决定表名
            String ym = LocalDate.now().format(DateTimeFormatter.ofPattern("yyyyMM"));
            return "t_order_" + ym;
        });
    }});

    interceptor.addInnerInterceptor(dynamic);
    return interceptor;
}
```

---

## 13. 最佳实践与性能优化

### 13.1 项目结构推荐

```
com.example
├── config
│   └── MybatisPlusConfig.java          // MP 配置
├── interceptor
│   ├── TenantContext.java               // 租户上下文
│   ├── TenantFilter.java                // 租户过滤器
│   └── DataScopeInterceptor.java        // 数据权限
├── handler
│   └── MyMetaObjectHandler.java         // 自动填充
├── entity
│   └── User.java
├── mapper
│   └── UserMapper.java
├── service
│   ├── UserService.java
│   └── impl
│       └── UserServiceImpl.java
└── controller
    └── UserController.java
```

### 13.2 最佳实践清单

```markdown
1. 【强制】表名、字段名用下划线命名，实体用驼峰（开启 map-underscore-to-camel-case）
2. 【强制】禁用 ${} 拼接用户输入（SQL 注入风险）
3. 【推荐】复杂 SQL 用 XML，简单 CRUD 用 MyBatis-Plus 注解
4. 【推荐】统一继承 BaseEntity（id, tenant_id, create_time, update_time, deleted）
5. 【强制】逻辑删除字段用 deleted（0=正常, 1=删除），区分业务状态
6. 【推荐】使用 LambdaQueryWrapper 代替 QueryWrapper（编译期校验）
7. 【强制】分页参数校验：最大 limit 限制，防止拖库
8. 【强制】批量操作在 Service 层加事务 @Transactional
9. 【推荐】Mapper 层只负责 SQL，业务逻辑在 Service 层
10.【推荐】多租户 + 数据权限优先用拦截器，而非在每个 SQL 手写
```

### 13.3 性能优化

```yaml
# application.yml
mybatis-plus:
  configuration:
    # 驼峰命名
    map-underscore-to-camel-case: true
    # 二级缓存（生产按需开启，默认 false）
    cache-enabled: false
    # 延迟加载（慎用，N+1 问题）
    lazy-loading-enabled: false
    # 查询超时（秒）
    default-statement-timeout: 30
    # 日志
    log-impl: org.apache.ibatis.logging.slf4j.Slf4jImpl
  global-config:
    db-config:
      # 全局逻辑删除
      logic-delete-field: deleted
      logic-delete-value: 1
      logic-not-delete-value: 0
      # ID 策略
      id-type: assign_id
      # 表前缀
      table-prefix: t_
```

### 13.4 常见问题

| 问题 | 原因 | 解决 |
|------|------|------|
| 分页不生效 | 没加 PaginationInnerInterceptor | 配置拦截器 |
| 租户 ID 没自动加 | 自动填充没配 / 拦截器没配 | 配 MetaObjectHandler / TenantLineInnerInterceptor |
| 逻辑删除后还能查到 | 没配 @TableLogic / 没加拦截器 | 配全局或注解 |
| N+1 查询 | association/collection 延迟加载 | 用 JOIN 代替子查询 / @Sql 联表 |
| 批量插入慢 | 逐条 insert | saveBatch / rewriteBatchedStatements=true |
| 更新丢失 | 并发覆盖 | 乐观锁 @Version |
| 全表更新 | 忘加 WHERE | BlockAttackInnerInterceptor |
| 字段不存在 | 实体 vs 数据库不一致 | @TableField(exist = false) |

### 13.5 性能优化点

```java
// 1. 批量插入（JDBC URL 加 rewriteBatchedStatements）
// jdbc:mysql://localhost:3306/db?rewriteBatchedStatements=true
userService.saveBatch(userList); // 默认分批 1000 条

// 2. 只查需要的列（避免 SELECT *）
@Select("SELECT id, user_name, email FROM t_user WHERE id = #{id}")
User selectById(Long id);

// 3. 分页优化（先 count 再 limit）
Page<User> page = userService.page(new Page<>(1, 20), wrapper);
// 如果不需要 count，用
page.setSearchCount(false);

// 4. 超大分页优化
// ❌ LIMIT 100000, 20
// ✅ 子查询分页
@Select("SELECT * FROM t_user WHERE id > #{lastId} ORDER BY id LIMIT 20")
List<User> pageByCursor(@Param("lastId") Long lastId);

// 5. 连接池配置
//    HikariCP 默认已够用，注意 maxPoolSize 不要过大
spring.datasource.hikari.maximum-pool-size=20
spring.datasource.hikari.minimum-idle=5
```

---

## 附录：常用代码片段

### 完整 BaseEntity

```java
@Data
public class BaseEntity implements Serializable {

    @TableId(type = IdType.ASSIGN_ID)
    private Long id;

    @TableField(fill = FieldFill.INSERT)
    private Long tenantId;

    @TableField(fill = FieldFill.INSERT)
    private Date createTime;

    @TableField(fill = FieldFill.INSERT)
    private String createBy;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private Date updateTime;

    @TableField(fill = FieldFill.INSERT_UPDATE)
    private String updateBy;

    @TableLogic
    @TableField(fill = FieldFill.INSERT)
    private Integer deleted;
}
```

### 条件构造器工具类

```java
public class QueryBuilder {

    public static <T> LambdaQueryWrapper<T> eq(String field, Object value) {
        return new LambdaQueryWrapper<T>().eq(
                SFunctionUtil.resolve(field), value);
    }

    public static <T> LambdaQueryWrapper<T> like(String field, Object value) {
        return new LambdaQueryWrapper<T>().like(
                SFunctionUtil.resolve(field), value);
    }

    // 快速构建分页
    public static <T> Page<T> page(int pageNum, int pageSize) {
        return new Page<>(pageNum, pageSize);
    }
}
```

### yml 完整配置

```yaml
spring:
  datasource:
    url: jdbc:mysql://localhost:3306/db?useUnicode=true&characterEncoding=utf-8&serverTimezone=Asia/Shanghai&rewriteBatchedStatements=true
    username: root
    password: root
    driver-class-name: com.mysql.cj.jdbc.Driver
    hikari:
      maximum-pool-size: 20
      minimum-idle: 5
      idle-timeout: 300000
      connection-timeout: 30000
      max-lifetime: 1200000

mybatis-plus:
  configuration:
    map-underscore-to-camel-case: true
    cache-enabled: false
    default-statement-timeout: 30
    log-impl: org.apache.ibatis.logging.slf4j.Slf4jImpl
  global-config:
    db-config:
      id-type: assign_id
      table-prefix: t_
      logic-delete-field: deleted
      logic-delete-value: 1
      logic-not-delete-value: 0
  mapper-locations: classpath*:mapper/**/*.xml
  type-aliases-package: com.example.entity
```

---

> **参考**: [MyBatis-Plus 官方文档](https://baomidou.com)、[MyBatis 官方文档](https://mybatis.org/mybatis-3/zh/)、《MyBatis 从入门到精通》
