# Logback 与 Log4j2 完整配置指南

---

## 一、Logback

### 1.1 Maven 依赖

```xml
<!-- Logback Core -->
<dependency>
    <groupId>ch.qos.logback</groupId>
    <artifactId>logback-core</artifactId>
    <version>1.5.12</version>
</dependency>

<!-- Logback Classic (包含 SLF4J API) -->
<dependency>
    <groupId>ch.qos.logback</groupId>
    <artifactId>logback-classic</artifactId>
    <version>1.5.12</version>
</dependency>

<!-- SLF4J API -->
<dependency>
    <groupId>org.slf4j</groupId>
    <artifactId>slf4j-api</artifactId>
    <version>2.0.16</version>
</dependency>
```

### 1.2 logback.xml 完整配置

```xml
<?xml version="1.0" encoding="UTF-8"?>
<configuration scan="true" scanPeriod="30 seconds">

    <!-- ======================== 变量定义 ======================== -->
    <property name="LOG_HOME" value="./logs"/>
    <property name="APP_NAME" value="my-app"/>
    <property name="LOG_PATTERN"
              value="%d{yyyy-MM-dd HH:mm:ss.SSS} [%thread] %-5level %logger{36} - %msg%n"/>

    <!-- ======================== Console Appender ======================== -->
    <appender name="CONSOLE" class="ch.qos.logback.core.ConsoleAppender">
        <encoder>
            <pattern>%d{HH:mm:ss.SSS} [%thread] %-5level %logger{36} - %msg%n</pattern>
            <charset>UTF-8</charset>
        </encoder>
    </appender>

    <!-- ======================== Rolling File Appender ======================== -->
    <!-- 按大小滚动，最大保留 30 天 -->
    <appender name="ROLLING_FILE" class="ch.qos.logback.core.rolling.RollingFileAppender">
        <file>${LOG_HOME}/${APP_NAME}.log</file>
        <rollingPolicy class="ch.qos.logback.core.rolling.SizeAndTimeBasedRollingPolicy">
            <!-- 按日期和大小分包，单文件最大 100MB -->
            <fileNamePattern>${LOG_HOME}/${APP_NAME}.%d{yyyy-MM-dd}.%i.log</fileNamePattern>
            <maxFileSize>100MB</maxFileSize>
            <!-- 最大保留天数 -->
            <maxHistory>30</maxHistory>
            <!-- 所有日志文件总大小上限 -->
            <totalSizeCap>5GB</totalSizeCap>
        </rollingPolicy>
        <encoder>
            <pattern>${LOG_PATTERN}</pattern>
            <charset>UTF-8</charset>
        </encoder>
    </appender>

    <!-- ======================== ERROR 独立文件 ======================== -->
    <appender name="ERROR_FILE" class="ch.qos.logback.core.rolling.RollingFileAppender">
        <file>${LOG_HOME}/${APP_NAME}-error.log</file>
        <!-- 只记录 ERROR 及以上 -->
        <filter class="ch.qos.logback.classic.filter.ThresholdFilter">
            <level>ERROR</level>
        </filter>
        <rollingPolicy class="ch.qos.logback.core.rolling.SizeAndTimeBasedRollingPolicy">
            <fileNamePattern>${LOG_HOME}/${APP_NAME}-error.%d{yyyy-MM-dd}.%i.log</fileNamePattern>
            <maxFileSize>50MB</maxFileSize>
            <maxHistory>90</maxHistory>
            <totalSizeCap>2GB</totalSizeCap>
        </rollingPolicy>
        <encoder>
            <pattern>${LOG_PATTERN}</pattern>
            <charset>UTF-8</charset>
        </encoder>
    </appender>

    <!-- ======================== Logger 配置 ======================== -->
    <!-- 项目自身的包 -->
    <logger name="com.mycompany" level="DEBUG" additivity="false">
        <appender-ref ref="ROLLING_FILE"/>
        <appender-ref ref="ERROR_FILE"/>
    </logger>

    <!-- 第三方框架 -->
    <logger name="org.springframework" level="WARN"/>
    <logger name="org.hibernate" level="WARN"/>
    <logger name="com.zaxxer.hikari" level="INFO"/>

    <!-- Root Logger -->
    <root level="INFO">
        <appender-ref ref="CONSOLE"/>
        <appender-ref ref="ROLLING_FILE"/>
        <appender-ref ref="ERROR_FILE"/>
    </root>

</configuration>
```

### 1.3 指定 Logback 配置文件的方式

| 方式 | 说明 |
|------|------|
| `logback.xml` | 默认路径，classpath 根目录下自动加载 |
| `logback-spring.xml` | Spring Boot 专用，支持 `<springProfile>` |
| `-Dlogback.configurationFile=path` | JVM 参数指定任意路径 |
| `ContextInitializer` | 编程方式指定 |

**Spring Boot 中使用 logback-spring.xml（推荐）：**

```xml
<!-- 不同环境加载不同配置 -->
<springProfile name="dev">
    <root level="DEBUG">
        <appender-ref ref="CONSOLE"/>
    </root>
</springProfile>

<springProfile name="prod">
    <root level="INFO">
        <appender-ref ref="ROLLING_FILE"/>
        <appender-ref ref="ERROR_FILE"/>
    </root>
</springProfile>
```

**application.yml 指定配置文件路径：**

```yaml
logging:
  config: classpath:logback-prod.xml
```

**JVM 启动参数指定：**

```bash
java -Dlogback.configurationFile=/etc/myapp/logback.xml -jar app.jar
```

---

## 二、Log4j2

### 2.1 Maven 依赖

```xml
<!-- Log4j2 API -->
<dependency>
    <groupId>org.apache.logging.log4j</groupId>
    <artifactId>log4j-api</artifactId>
    <version>2.24.1</version>
</dependency>

<!-- Log4j2 Core -->
<dependency>
    <groupId>org.apache.logging.log4j</groupId>
    <artifactId>log4j-core</artifactId>
    <version>2.24.1</version>
</dependency>

<!-- Log4j2 与 SLF4J 桥接 (使用 Logback 时不需要) -->
<dependency>
    <groupId>org.apache.logging.log4j</groupId>
    <artifactId>log4j-slf4j2-impl</artifactId>
    <version>2.24.1</version>
</dependency>

<!-- Log4j2 Web (如需要) -->
<dependency>
    <groupId>org.apache.logging.log4j</groupId>
    <artifactId>log4j-web</artifactId>
    <version>2.24.1</version>
</dependency>
```

### 2.2 log4j2.xml 完整配置

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Configuration status="WARN" monitorInterval="30">

    <!-- ======================== 变量定义 ======================== -->
    <Properties>
        <Property name="LOG_HOME">./logs</Property>
        <Property name="APP_NAME">my-app</Property>
        <Property name="LOG_PATTERN">%d{yyyy-MM-dd HH:mm:ss.SSS} [%t] %-5level %logger{36} - %msg%n</Property>
    </Properties>

    <!-- ======================== Appenders ======================== -->
    <Appenders>
        <!-- 控制台 -->
        <Console name="Console" target="SYSTEM_OUT">
            <PatternLayout pattern="%d{HH:mm:ss.SSS} [%t] %-5level %logger{36} - %msg%n"
                           charset="UTF-8"/>
        </Console>

        <!-- 普通滚动文件 - 按大小 + 时间分包 -->
        <RollingFile name="RollingFile"
                     fileName="${LOG_HOME}/${APP_NAME}.log"
                     filePattern="${LOG_HOME}/${APP_NAME}.%d{yyyy-MM-dd}.%i.log">
            <PatternLayout pattern="${LOG_PATTERN}" charset="UTF-8"/>
            <Policies>
                <!-- 单文件超过 100MB 触发滚动 -->
                <SizeBasedTriggeringPolicy size="100 MB"/>
                <!-- 每天午夜触发滚动 -->
                <TimeBasedTriggeringPolicy interval="1" modulate="true"/>
            </Policies>
            <!-- 最多保留 30 天，最多 30 个文件，总大小 5GB -->
            <DefaultRolloverStrategy max="30">
                <Delete basePath="${LOG_HOME}" maxDepth="1">
                    <IfFileName glob="${APP_NAME}.*.log"/>
                    <IfLastModified age="30d"/>
                    <IfAccumulatedFileSize exceeds="5 GB"/>
                </Delete>
            </DefaultRolloverStrategy>
        </RollingFile>

        <!-- ERROR 独立文件 -->
        <RollingFile name="ErrorFile"
                     fileName="${LOG_HOME}/${APP_NAME}-error.log"
                     filePattern="${LOG_HOME}/${APP_NAME}-error.%d{yyyy-MM-dd}.%i.log">
            <PatternLayout pattern="${LOG_PATTERN}" charset="UTF-8"/>
            <!-- 只接收 ERROR 级别 -->
            <ThresholdFilter level="ERROR" onMatch="ACCEPT" onMismatch="DENY"/>
            <Policies>
                <SizeBasedTriggeringPolicy size="50 MB"/>
                <TimeBasedTriggeringPolicy interval="1" modulate="true"/>
            </Policies>
            <DefaultRolloverStrategy max="30">
                <Delete basePath="${LOG_HOME}" maxDepth="1">
                    <IfFileName glob="${APP_NAME}-error.*.log"/>
                    <IfLastModified age="90d"/>
                    <IfAccumulatedFileSize exceeds="2 GB"/>
                </Delete>
            </DefaultRolloverStrategy>
        </RollingFile>
    </Appenders>

    <!-- ======================== Loggers ======================== -->
    <Loggers>
        <!-- 项目自身 -->
        <Logger name="com.mycompany" level="DEBUG" additivity="false">
            <AppenderRef ref="RollingFile"/>
            <AppenderRef ref="ErrorFile"/>
        </Logger>

        <!-- 第三方框架 -->
        <Logger name="org.springframework" level="WARN" additivity="false">
            <AppenderRef ref="Console"/>
        </Logger>
        <Logger name="org.hibernate" level="WARN" additivity="false">
            <AppenderRef ref="Console"/>
        </Logger>
        <Logger name="com.zaxxer.hikari" level="INFO" additivity="false">
            <AppenderRef ref="Console"/>
        </Logger>

        <!-- Root -->
        <Root level="INFO">
            <AppenderRef ref="Console"/>
            <AppenderRef ref="RollingFile"/>
            <AppenderRef ref="ErrorFile"/>
        </Root>
    </Loggers>

</Configuration>
```

### 2.3 指定 Log4j2 配置文件的方式

| 优先级 | 方式 | 说明 |
|--------|------|------|
| 最高 | `log4j2.configurationFile` 系统属性 | `-Dlog4j2.configurationFile=path` |
| 高 | `LOG4J_CONFIGURATION_FILE` 环境变量 | `export LOG4J_CONFIGURATION_FILE=path` |
| 默认 | classpath 扫描 | 按顺序查找：`log4j2-test.xml` → `log4j2-test.json` → `log4j2-test.yaml` → `log4j2.xml` → `log4j2.json` → `log4j2.yaml` |

**application.yml 指定：**

```yaml
logging:
  config: classpath:log4j2-prod.xml
```

**Spring Boot 多环境 profile 方式：**

```xml
<Configuration status="WARN">
    <SpringProfile name="dev">
        <Appenders>
            <Console name="Console" target="SYSTEM_OUT">
                <PatternLayout pattern="%d{HH:mm:ss} %-5level %logger{36} - %msg%n"/>
            </Console>
        </Appenders>
        <Loggers>
            <Root level="DEBUG">
                <AppenderRef ref="Console"/>
            </Root>
        </Loggers>
    </SpringProfile>

    <SpringProfile name="prod">
        <!-- 生产环境配置 -->
    </SpringProfile>
</Configuration>
```

---

## 三、日志级别对照

| 级别 | Logback | Log4j2 | 说明 |
|------|---------|--------|------|
| TRACE | ✅ | ✅ | 最细粒度追踪 |
| DEBUG | ✅ | ✅ | 调试信息 |
| INFO | ✅ | ✅ | 一般信息（默认） |
| WARN | ✅ | ✅ | 警告 |
| ERROR | ✅ | ✅ | 错误 |
| FATAL | ❌ | ✅ | 致命错误 |
| OFF | ✅ | ✅ | 关闭所有日志 |

**级别从低到高：** `TRACE < DEBUG < INFO < WARN < ERROR < FATAL < OFF`

---

## 四、日志分包策略详解

### 4.1 常见分包方式

```
logs/
├── my-app.log                        # 当前日志
├── my-app.2026-08-14.0.log           # 按日期+序号
├── my-app.2026-08-14.1.log           # 同一天第二个文件
├── my-app-error.log                  # 错误日志
├── my-app-error.2026-08-14.0.log
└── archive/                          # 归档目录（可选）
```

### 4.2 Logback SizeAndTimeBasedRollingPolicy 参数

```xml
<rollingPolicy class="ch.qos.logback.core.rolling.SizeAndTimeBasedRollingPolicy">
    <!-- 文件名模式: 日期 + 索引 -->
    <fileNamePattern>${LOG_HOME}/${APP_NAME}.%d{yyyy-MM-dd}.%i.log</fileNamePattern>

    <!-- 单文件最大体积 -->
    <maxFileSize>100MB</maxFileSize>

    <!-- 最大保留天数 -->
    <maxHistory>30</maxHistory>

    <!-- 所有日志文件总大小上限 (可选) -->
    <totalSizeCap>5GB</totalSizeCap>

    <!-- 清理周期，默认每12小时检查一次 -->
    <cleanHistoryOnStart>false</cleanHistoryOnStart>
</rollingPolicy>
```

### 4.3 Log4j2 DefaultRolloverStrategy 参数

```xml
<DefaultRolloverStrategy>
    <!-- 最大文件序号 -->
    <max>30</max>

    <!-- 删除策略 -->
    <Delete basePath="${LOG_HOME}" maxDepth="1">
        <!-- 按文件名匹配 -->
        <IfFileName glob="${APP_NAME}.*.log"/>
        <!-- 按最后修改时间删除 -->
        <IfLastModified age="30d"/>
        <!-- 按累计大小删除 -->
        <IfAccumulatedFileSize exceeds="5 GB"/>
        <!-- 按累计文件数删除 -->
        <IfAccumulatedFileCount exceeds="100"/>
    </Delete>
</DefaultRolloverStrategy>
```

---

## 五、Logback vs Log4j2 对比

| 特性 | Logback | Log4j2 |
|------|---------|--------|
| 性能 | 良好 | 更优（异步日志） |
| 配置热加载 | `scan="true"` | `monitorInterval="30"` |
| 异步支持 | `AsyncAppender` | 内置异步 Logger |
| 延迟加载 | ❌ | ✅ |
| 插件系统 | 有限 | ✅ 丰富 |
| 过滤器 | 简单 | 灵活（组合过滤器） |
| 垃圾回收 | 有 GC 压力 | 无 GC 设计 |
| 社区 | SLF4J 生态 | Apache 生态 |

### 5.1 Logback 异步配置

```xml
<appender name="ASYNC" class="ch.qos.logback.classic.AsyncAppender">
    <appender-ref ref="ROLLING_FILE"/>
    <!-- 队列大小 -->
    <queueSize>512</queueSize>
    <!-- 队列满时是否丢弃 TRACE/DEBUG -->
    <discardingThreshold>0</discardingThreshold>
    <!-- 是否 neverBlock -->
    <neverBlock>false</neverBlock>
</appender>
```

### 5.2 Log4j2 异步配置

```xml
<!-- 方式1: 全局异步 (推荐) -->
<!-- 在 classpath 下创建 log4j2.component.properties -->
<!-- log4j2.contextSelector=org.apache.logging.log4j.core.async.AsyncLoggerContextSelector -->

<!-- 方式2: 指定 Logger 异步 -->
<Logger name="com.mycompany" level="DEBUG" includeLocation="false" additivity="false">
    <AsyncAppender ref="RollingFile"/>
</Logger>
```

---

## 六、常见问题

### 6.1 日志文件不滚动

- 检查 `maxFileSize` 和 `TimeBasedTriggeringPolicy` 配置
- 确认日志目录写入权限
- Log4j2 检查 `DefaultRolloverStrategy` 配置

### 6.2 配置文件不生效

- Logback: 确认文件名是 `logback.xml`（不是 `log4j.xml`）
- Log4j2: 确认文件名是 `log4j2.xml`（不是 `log4j.xml`）
- 检查 classpath 优先级和 JVM 参数

### 6.3 中文乱码

```xml
<!-- Logback -->
<charset>UTF-8</charset>

<!-- Log4j2 -->
<charset>UTF-8</charset>
```

### 6.4 日志文件过大

- 减小 `maxFileSize`
- 减小 `maxHistory` 或 `totalSizeCap`
- 配置 `Delete` 策略自动清理

---

## 七、最佳实践

1. **生产环境使用异步日志**，避免 I/O 阻塞
2. **ERROR 日志单独分文件**，便于监控和告警
3. **配置热加载**，修改配置无需重启
4. **统一日志格式**，便于 ELK 收集和分析
5. **设置合理的保留策略**，避免磁盘占满
6. **使用 SLF4J 门面**，方便切换实现框架
7. **敏感信息脱敏**，不要在日志中输出密码、token 等
