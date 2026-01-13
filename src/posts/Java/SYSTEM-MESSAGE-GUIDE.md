---
cover: /assets/images/cover2.jpg
icon: pen-to-square
date: 2026-01-2
category:
  - 接入ai
tag:
  - ai
  - chatbot
star: true
sticky: true
---

# 系统消息(System Message)功能使用指南

## 功能概述

系统消息功能允许你为AI聊天机器人定义角色、行为规范和回答范围，从而控制AI的回复风格和内容范围。

## 配置说明

### 1. 配置文件设置

在 `application.yml` 中配置默认系统消息：

```yaml
deepseek:
  system:
    # 默认系统消息，定义AI的角色和行为
    message: "你是一个专业的AI助手，专门为用户提供技术支持和问题解答。请保持友好、专业的态度，如果遇到不确定的问题，请诚实地说明。你的回答应该准确、有用且易于理解。"
    # 是否启用系统消息
    enabled: true
```

### 2. 系统消息示例

#### 技术支持AI
```
你是一个专业的技术支持AI助手，专门回答编程、软件开发和技术相关的问题。
- 只回答与技术相关的问题
- 如果用户问非技术问题，请礼貌地引导回技术话题
- 提供准确、实用的技术解决方案
- 包含代码示例时请确保语法正确
```

#### 客服AI
```
你是一个友好的客服AI助手，专门为用户提供产品咨询和售后服务。
- 保持礼貌、耐心的服务态度
- 优先解决用户的产品相关问题
- 如遇无法解决的问题，建议联系人工客服
- 不讨论与产品无关的话题
```

#### 教育AI
```
你是一个专业的教育AI助手，专门帮助学生学习和答疑。
- 采用启发式教学方法，引导学生思考
- 提供详细的解题步骤和原理解释
- 鼓励学生独立思考，不直接给出答案
- 只回答学习相关的问题
```

## API接口

### 1. 获取当前系统消息

**接口**: `GET /api/chat/system-message`

**响应示例**:
```json
{
  "success": true,
  "enabled": true,
  "message": "你是一个专业的AI助手，专门为用户提供技术支持和问题解答..."
}
```

### 2. 设置系统消息

**接口**: `POST /api/chat/system-message`

**参数**:
- `message` (必需): 系统消息内容
- `enabled` (可选): 是否启用，默认true

**请求示例**:
```
POST /api/chat/system-message
Content-Type: application/x-www-form-urlencoded

message=你是一个专业的技术支持AI，只回答编程相关问题&enabled=true
```

**响应示例**:
```json
{
  "success": true,
  "message": "系统消息设置成功",
  "enabled": true,
  "systemMessage": "你是一个专业的技术支持AI，只回答编程相关问题"
}
```

### 3. 重置系统消息

**接口**: `POST /api/chat/system-message/reset`

**响应示例**:
```json
{
  "success": true,
  "message": "系统消息已重置为默认设置"
}
```

## 使用场景

### 1. 限制回答范围
```
你是一个专门回答Java编程问题的AI助手。
- 只回答Java相关的编程问题
- 不回答其他编程语言的问题
- 不讨论非编程相关的话题
- 如果用户问其他问题，请引导到Java编程话题
```

### 2. 定义回答风格
```
你是一个幽默风趣的AI助手。
- 在回答问题时适当使用幽默
- 保持轻松愉快的对话氛围
- 但在涉及严肃问题时要保持专业
- 用生动的比喻来解释复杂概念
```

### 3. 角色扮演
```
你是一个资深的软件架构师，拥有15年的开发经验。
- 从架构师的角度分析问题
- 考虑系统的可扩展性、性能和维护性
- 提供最佳实践建议
- 分享实际项目经验
```

## 实现原理

### 1. 消息构建顺序
```java
// 对话历史构建顺序：
1. System Message (系统消息) - 定义AI角色
2. Historical Messages (历史消息) - 上下文对话
3. User Message (用户消息) - 当前问题
```

### 2. 代码实现
```java
private List<DeepSeekChatRequest.Message> buildChatHistory(String sessionId) {
    List<DeepSeekChatRequest.Message> history = new ArrayList<>();
    
    // 1. 添加系统消息
    if (systemMessageEnabled && systemMessage != null) {
        history.add(new DeepSeekChatRequest.Message("system", systemMessage));
    }
    
    // 2. 添加历史对话
    List<ChatMessage> dbMessages = messageRepository.findMessagesBySessionId(sessionId);
    for (ChatMessage msg : dbMessages) {
        history.add(new DeepSeekChatRequest.Message(msg.getRole().name(), msg.getContent()));
    }
    
    return history;
}
```

## 最佳实践

### 1. 系统消息编写原则
- **明确具体**: 清楚说明AI的角色和职责
- **设定边界**: 明确什么能回答，什么不能回答
- **定义风格**: 指定回答的语气和风格
- **提供指导**: 给出具体的行为指导原则

### 2. 常用模板

#### 专业领域限制
```
你是一个专门回答[领域]问题的AI助手。
- 只回答与[领域]相关的问题
- 如果用户询问其他领域的问题，请礼貌地说明你只能回答[领域]相关问题
- 提供准确、专业的[领域]知识和建议
- 如果不确定答案，请诚实说明并建议咨询专业人士
```

#### 企业客服模板
```
你是[公司名称]的AI客服助手。
- 专门为用户提供[产品/服务]相关的咨询和支持
- 保持友好、专业的服务态度
- 优先解决用户的产品使用问题
- 如遇复杂问题，建议用户联系人工客服：[联系方式]
- 不讨论竞争对手或其他公司的产品
```

### 3. 动态调整策略
```java
// 根据不同场景动态设置系统消息
public void setContextualSystemMessage(String context) {
    switch (context) {
        case "technical_support":
            setSystemMessage("你是技术支持AI...", true);
            break;
        case "sales":
            setSystemMessage("你是销售顾问AI...", true);
            break;
        case "education":
            setSystemMessage("你是教育AI助手...", true);
            break;
        default:
            resetSystemMessage();
    }
}
```

## 测试建议

### 1. 功能测试
1. 设置系统消息后，测试AI是否按照指定角色回答
2. 测试边界情况，询问系统消息限制范围外的问题
3. 验证系统消息的持久性（重启后是否保持）

### 2. 效果验证
```bash
# 1. 设置技术支持AI
curl -X POST "http://localhost:8080/api/chat/system-message" \
  -d "message=你是技术支持AI，只回答编程问题&enabled=true"

# 2. 测试技术问题
curl -X POST "http://localhost:8080/api/chat/completion" \
  -d "message=如何使用Spring Boot&sessionId=test"

# 3. 测试非技术问题
curl -X POST "http://localhost:8080/api/chat/completion" \
  -d "message=今天天气怎么样&sessionId=test"
```

## 注意事项

1. **消息长度**: 系统消息会占用token，建议控制在200字以内
2. **优先级**: 系统消息优先级最高，会影响所有后续对话
3. **持久性**: 运行时设置的系统消息重启后会恢复为配置文件中的默认值
4. **性能影响**: 每次对话都会包含系统消息，会略微增加API调用成本

## 故障排除

### 1. 系统消息不生效
- 检查 `enabled` 参数是否为 `true`
- 确认消息内容不为空
- 查看日志确认系统消息是否正确添加到对话历史

### 2. AI回答不符合预期
- 检查系统消息是否足够明确和具体
- 考虑增加更详细的行为指导
- 测试不同的系统消息表述方式

### 3. 配置不生效
- 确认配置文件语法正确
- 检查配置文件路径和加载情况
- 重启应用使配置生效

通过合理使用系统消息功能，你可以创建出专业、有针对性的AI助手，为用户提供更好的服务体验！