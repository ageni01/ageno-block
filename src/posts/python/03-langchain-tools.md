---
title: "LangChain Tools — 给大模型编写工具"

category:
  - AI 应用
tag:
  - AI 应用
---
# LangChain Tools — 给大模型编写工具

> **这是搭建智能体的核心环节。** Tools 是大模型连接外部世界的"手"——有了 Tools，LLM 才能查天气、算数学、读写文件、调用 API、执行代码。

## 目录

1. [为什么需要 Tools？](#1-为什么需要-tools)
2. [Tool 的基本概念](#2-tool-的基本概念)
3. [@tool 装饰器](#3-tool-装饰器)
4. [StructuredTool](#4-structuredtool)
5. [Tool Calling（函数调用）机制](#5-tool-calling函数调用机制)
6. [完整流程详解](#6-完整流程详解)
7. [实战：多个 Tools 组合](#7-实战多个-tools-组合)
8. [高级：Tool 错误处理与重试](#8-高级tool-错误处理与重试)

---

## 1. 为什么需要 Tools？

**大模型的局限：**

| 能力 | LLM 自身 | 加 Tools 后 |
|------|----------|------------|
| 数学计算 | 经常算错 | 调用计算器工具 ✅ |
| 实时数据 | 知识截止到训练日期 | 可以搜索/查 API ✅ |
| 执行代码 | 只能"想象"结果 | 真正运行并返回结果 ✅ |
| 访问文件 | ❌ | 读写文件 ✅ |
| 调用 API | ❌ | 发起 HTTP 请求 ✅ |

**核心思想：** LLM 决定"做什么"，Tools 负责"怎么做"。

## 2. Tool 的基本概念

一个 Tool（工具）本质上是一个函数 + 元数据：

```python
from langchain_core.tools import BaseTool

# 一个 Tool 包含：
# - name:        工具名称（LLM 通过名称选择调用哪个工具）
# - description: 工具描述（LLM 根据描述理解工具用途）
# - args:        参数列表（LLM 知道需要传入什么参数）
# - func:        实际执行的函数
```

**LLM 调用 Tool 的完整流程：**

```
用户提问："北京的天气怎么样？"
     │
     ▼
┌─────────────────────────┐
│  1. LLM 收到问题         │
│  2. LLM 思考：我需要     │
│     查天气工具           │
│  3. LLM 返回：           │
│     Tool Call 指令       │  ← 关键！LLM 并不执行，只是"说要调用"
│     get_weather(         │
│       city="北京"        │
│     )                    │
└─────────┬───────────────┘
          │
          ▼
┌─────────────────────────┐
│  4. 我们的代码收到       │
│     Tool Call           │
│  5. 执行 get_weather()  │  ← 真正执行的是我们的代码
│  6. 得到结果：           │
│     "22°C，晴"           │
└─────────┬───────────────┘
          │
          ▼
┌─────────────────────────┐
│  7. 把结果送回 LLM       │
│  8. LLM 根据结果         │
│     组织自然语言回答      │
│  9. "北京今天22°C，晴"   │
└─────────────────────────┘
```

## 3. @tool 装饰器

### 3.1 最简单的工具

```python
from langchain_core.tools import tool

# ============================================================
# @tool 装饰器：把普通函数变成 Tool
#   - 函数名 → Tool name
#   - 函数文档字符串 → Tool description
#   - 类型注解 → Tool 参数 schema
# ============================================================
@tool
def get_current_time() -> str:
    """
    获取当前的北京时间。
    当你需要知道现在几点了，可以使用这个工具。
    """
    from datetime import datetime, timezone, timedelta
    tz = timezone(timedelta(hours=8))
    return datetime.now(tz).strftime("%Y-%m-%d %H:%M:%S")

# 查看 Tool 的自动生成信息
print(get_current_time.name)        # "get_current_time"
print(get_current_time.description) # "获取当前的北京时间。..."
print(get_current_time.args)        # {"type": "object", "properties": {}, "required": []}
```

### 3.2 带参数的工具

```python
@tool
def calculate(expression: str) -> str:
    """
    计算数学表达式的结果。
    当你需要进行精确数学计算时使用。
    
    参数:
        expression: 数学表达式，如 "2 + 3 * 4" 或 "sqrt(16)"
    """
    import math
    # 安全起见，用 eval 但限制可用函数
    allowed_names = {
        k: v for k, v in math.__dict__.items()
        if not k.startswith("__")
    }
    allowed_names["abs"] = abs
    try:
        result = eval(expression, {"__builtins__": {}}, allowed_names)
        return str(result)
    except Exception as e:
        return f"计算错误：{e}"

# 查看 Tool 参数
print(calculate.args)
# → {"type": "object", "properties": {"expression": {"type": "string"}}, "required": ["expression"]}
```

### 3.3 带类型注解的工具（参数更精确）

```python
from typing import Optional

@tool
def search_products(
    keyword: str,
    category: Optional[str] = None,
    max_price: Optional[float] = None,
    page: int = 1,
) -> str:
    """
    搜索商品信息。
    
    参数:
        keyword:    搜索关键词（必填）
        category:   商品分类（可选）
        max_price:  最高价格（可选）
        page:       页码，默认第1页（可选，默认1）
    """
    # 模拟搜索结果
    results = [
        {"name": f"{keyword} 商品A", "price": 99.0},
        {"name": f"{keyword} 商品B", "price": 199.0},
    ]
    # 如果有筛选条件，在这里逻辑处理...
    return f"找到 {len(results)} 个商品: {results}"

print(search_products.args)
# → {
#   "type": "object",
#   "properties": {
#     "keyword": {"type": "string"},
#     "category": {"type": "string"},
#     "max_price": {"type": "number"},
#     "page": {"type": "integer", "default": 1}
#   },
#   "required": ["keyword"]
# }
```

### 3.4 从类方法创建 Tool（需要 self 的工具）

```python
@tool
def send_email(
    to: str,
    subject: str,
    body: str,
) -> str:
    """
    发送电子邮件。
    当你需要给用户发送邮件时使用。
    
    参数:
        to:      收件人邮箱地址
        subject: 邮件主题
        body:    邮件正文
    """
    # 实际的邮件发送逻辑
    print(f"📧 正在发送邮件到 {to}...")
    # smtp.sendmail(...)
    return f"邮件已成功发送到 {to}"
```

## 4. StructuredTool

如果需要更多控制（如自定义错误处理、回调），使用 `StructuredTool`：

```python
from langchain_core.tools import StructuredTool
from pydantic import BaseModel, Field

# 定义参数模型（更精确的参数校验）
class WeatherInput(BaseModel):
    city: str = Field(description="城市名称，如 北京、上海、广州")
    days: int = Field(default=1, description="预报天数，默认为1")

def _get_weather(city: str, days: int = 1) -> str:
    """实际的天气查询逻辑"""
    import random
    weathers = ["晴 ☀️", "多云 ⛅", "阴 🌥️", "小雨 🌦️", "大雨 🌧️"]
    result = []
    for d in range(days):
        weather = random.choice(weathers)
        temp = random.randint(15, 35)
        result.append(f"第{d+1}天：{weather} {temp}°C")
    return "\n".join(result)

# StructuredTool：更精细地定义 Tool
weather_tool = StructuredTool.from_function(
    func=_get_weather,           # 实际执行函数
    name="get_weather",          # Tool 名称
    description="""              # Tool 描述（越详细，LLM 越准确调用）
    查询指定城市的天气预报。
    支持国内主要城市。
    """,
    args_schema=WeatherInput,    # Pydantic 模型（参数校验）
    return_direct=False,         # True = 直接将结果返回给用户，不走 LLM
)
```

## 5. Tool Calling（函数调用）机制

### 5.1 核心原理

Tool Calling 是 **LLM 的原生能力**（OpenAI 2023 年引入），不是 LangChain 发明的。流程：

```
LLM 的回复不再是纯文本，而可能包含：
{
  "content": null,                              ← 没有文本回复
  "tool_calls": [                                ← LLM 要调用工具
    {
      "id": "call_xxx",
      "function": {
        "name": "get_weather",                   ← 调用 get_weather
        "arguments": "{\"city\": \"北京\"}"       ← 参数
      }
    }
  ]
}
```

### 5.2 基础调用：bind_tools

```python
from langchain_openai import ChatOpenAI
from langchain_core.tools import tool

# 1️⃣ 定义工具
@tool
def get_weather(city: str) -> str:
    """查询城市天气"""
    return f"{city} 的天气：晴，25°C"

@tool
def calculate(expression: str) -> str:
    """计算数学表达式"""
    return str(eval(expression))

# 2️⃣ 将工具绑定到 LLM
# bind_tools: 把 Tool 的 schema 注册到 LLM，
# 让 LLM 知道"可以用这些工具"
llm = ChatOpenAI(model="deepseek-chat", temperature=0)
llm_with_tools = llm.bind_tools([get_weather, calculate])

# 3️⃣ 调用 LLM
response = llm_with_tools.invoke("北京的天气怎么样？")
print(response.content)          # → None（没有文本回复）
print(response.tool_calls)       # → [ToolCall 列表]

# tool_calls 的结构：
# [
#   ToolCall(
#     name="get_weather",
#     args={"city": "北京"},
#     id="call_abc123",
#     type="tool_call",
#   )
# ]
```

### 5.3 手动处理 Tool Calling

```python
# 4️⃣ 检查是否有 tool_calls，并执行
if response.tool_calls:
    for tool_call in response.tool_calls:
        tool_name = tool_call["name"]
        tool_args = tool_call["args"]
        
        print(f"🤖 LLM 要调用: {tool_name}({tool_args})")
        
        # 分发到对应的 Python 函数
        if tool_name == "get_weather":
            result = get_weather.invoke(tool_args)
        elif tool_name == "calculate":
            result = calculate.invoke(tool_args)
        else:
            result = f"未知工具: {tool_name}"
        
        print(f"🔧 执行结果: {result}")
else:
    print(f"🤖 LLM 直接回答: {response.content}")
```

## 6. 完整流程详解

这是 **AI Agent 最核心的循环**：

```python
import os
from dotenv import load_dotenv
from langchain_openai import ChatOpenAI
from langchain_core.tools import tool

load_dotenv()

# ============================================================
# 完整的 Tool Calling 流程
# ============================================================

# ---------- 1. 定义工具 ----------
@tool
def get_weather(city: str) -> str:
    """
    查询指定城市的当前天气。
    当你需要了解某个城市的天气时使用。
    """
    # 模拟天气查询
    weather_data = {
        "北京": "晴，25°C，空气质量良",
        "上海": "多云，28°C，湿度较高",
        "广州": "小雨，30°C，体感闷热",
        "深圳": "晴，29°C，适合户外活动",
    }
    return weather_data.get(city, f"暂无 {city} 的天气数据")


@tool
def get_city_info(city: str) -> str:
    """
    查询城市基本信息，包括人口、面积、特色等。
    当用户想了解某个城市时使用。
    """
    info = {
        "北京": "首都，常住人口2188万，面积16410平方公里，故宫、长城所在地",
        "上海": "直辖市，常住人口2475万，面积6340平方公里，经济金融中心",
        "广州": "广东省会，常住人口1873万，面积7434平方公里，美食之都",
    }
    return info.get(city, f"暂无 {city} 的信息")


@tool
def calculate(expression: str) -> str:
    """
    计算数学表达式。输入应为合法的 Python 数学表达式。
    """
    import math
    allowed = {k: v for k, v in math.__dict__.items() if not k.startswith("_")}
    allowed["abs"] = abs
    try:
        return str(eval(expression, {"__builtins__": {}}, allowed))
    except Exception as e:
        return f"计算失败：{e}"


# ---------- 2. 初始化 LLM 并绑定工具 ----------
llm = ChatOpenAI(model="deepseek-chat", temperature=0)
tools = [get_weather, get_city_info, calculate]
llm_with_tools = llm.bind_tools(tools)


# ---------- 3. 核心循环：思考 → 行动 → 观察 ----------
def run_agent(query: str, max_iterations: int = 5):
    """
    AI Agent 的核心执行逻辑。
    
    每次循环：
      1. LLM 思考（生成回复，可能包含 tool_calls）
      2. 如果有 tool_calls → 执行工具 → 结果反馈给 LLM
      3. 如果没有 tool_calls → LLM 给出最终回答
    """
    messages = [
        {"role": "system", "content": "你是一个有用的 AI 助手，可以使用工具获取信息。"},
        {"role": "user", "content": query},
    ]
    
    for i in range(max_iterations):
        print(f"\n{'='*50}")
        print(f"🔄 第 {i+1} 轮迭代")
        print(f"{'='*50}")
        
        # 第1步：LLM 思考 → 返回文本或 tool_calls
        response = llm_with_tools.invoke(messages)
        
        # 第2步：检查是否有工具调用
        if not response.tool_calls:
            # LLM 直接回答了，不需要调用工具
            print(f"🤖 AI: {response.content}")
            return response.content
        
        # 第3步：执行工具
        for tool_call in response.tool_calls:
            tool_name = tool_call["name"]
            tool_args = tool_call["args"]
            tool_id = tool_call["id"]
            
            print(f"🔧 调用工具: {tool_name}({tool_args})")
            
            # 根据工具名找到对应的 Python 函数并执行
            for t in tools:
                if t.name == tool_name:
                    result = t.invoke(tool_args)
                    break
            else:
                result = f"错误：未找到工具 {tool_name}"
            
            print(f"📦 工具结果: {result}")
            
            # 将 tool_call 和结果追加到消息列表
            # 这样 LLM 就能看到工具返回的结果
            messages.append(response)  # AI 的 tool_call 消息
            messages.append({
                "role": "tool",
                "tool_call_id": tool_id,
                "content": result,
            })
        
        # 继续循环：LLM 根据工具结果进一步推理...
    
    return "已达到最大迭代次数"


# ---------- 4. 运行测试 ----------
if __name__ == "__main__":
    # 测试1：单一工具
    run_agent("北京的天气怎么样？")
    
    # 测试2：需要多个工具
    # run_agent("了解下上海，顺便告诉我上海的天气？")
    
    # 测试3：需要计算
    # run_agent("计算 (25 * 4 + 100) / 2 的结果")
    
    # 测试4：不需要工具
    # run_agent("你好，你是谁？")


# ============================================================
# 关键理解：
# 1. LLM 不执行工具，它只"决定"要调用哪个工具
# 2. 工具的"执行"由我们的 Python 代码完成
# 3. 工具结果通过 messages 送回给 LLM
# 4. LLM 根据结果决定：继续调工具？还是给出最终回答？
# 5. 这个循环就是 Agent 的基本模式：思考→行动→观察
# ============================================================
```

## 7. 实战：多个 Tools 组合

```python
# tools_combined.py
from langchain_core.tools import tool
from typing import Optional
import json, random, math

# ============================================================
# 定义一个"全能助手"需要的各种工具
# ============================================================

# 工具1：查天气
@tool
def weather(city: str) -> str:
    """查询城市天气，在用户询问天气时使用"""
    data = {"北京": "晴 25°C", "上海": "多云 28°C", "广州": "阵雨 30°C", "深圳": "晴 29°C"}
    return data.get(city, f"{city} 25°C")


# 工具2：计算器
@tool
def calculator(expr: str) -> str:
    """执行数学计算，需要精确计算时使用。例如 '2**10' 或 '(3+5)*2'"""
    allowed = {k: v for k, v in math.__dict__.items() if not k.startswith("_")}
    allowed["abs"] = abs
    return str(eval(expr, {"__builtins__": {}}, allowed))


# 工具3：发送邮件（模拟）
@tool
def send_email(to: str, subject: str, body: str) -> str:
    """发送电子邮件。参数：to=收件人, subject=主题, body=正文"""
    print(f"📧 模拟发送邮件...")
    print(f"   收件人: {to}")
    print(f"   主题: {subject}")
    print(f"   正文: {body[:50]}...")
    return f"邮件已发送到 {to}"


# 工具4：获取当前时间
@tool
def current_time() -> str:
    """获取当前日期和时间"""
    from datetime import datetime, timezone, timedelta
    tz = timezone(timedelta(hours=8))
    return datetime.now(tz).strftime("%Y年%m月%d日 %H:%M:%S")


# 工具5：知识检索（模拟 RAG）
@tool
def search_knowledge(query: str) -> str:
    """搜索内部知识库，当需要查询专业知识时使用"""
    knowledge_base = {
        "python": "Python 是一种解释型、高级、通用编程语言，由 Guido van Rossum 创建于1991年。",
        "ai agent": "AI Agent 是能自主感知环境、做出决策并执行行动的智能系统。",
        "langchain": "LangChain 是一个用于构建 LLM 应用的框架，支持 Chains、Agents、RAG 等。",
    }
    for key, value in knowledge_base.items():
        if key in query.lower():
            return value
    return f"知识库中未找到与 '{query}' 相关的信息。"


# ---------- 把所有工具放在一起 ----------
all_tools = [weather, calculator, send_email, current_time, search_knowledge]


def create_agent(llm, tools: list, system_prompt: str = None):
    """创建带有工具的 LLM Agent"""
    if system_prompt is None:
        system_prompt = """你是一个多功能 AI 助手。
你有以下工具可用：
- weather: 查天气
- calculator: 数学计算
- send_email: 发送邮件（需要用户确认收件人、主题、正文）
- current_time: 获取当前时间
- search_knowledge: 知识库搜索

请根据用户需求选择合适的工具。如果需要多个工具，可以依次调用。"""
    
    llm_with_tools = llm.bind_tools(tools)
    return llm_with_tools, tools, system_prompt


def run_agent_with_print(llm_with_tools, tools: list, system_prompt: str, query: str):
    """运行 Agent 并打印详细信息"""
    from langchain_core.messages import SystemMessage, HumanMessage
    
    messages = [SystemMessage(content=system_prompt), HumanMessage(content=query)]
    
    print(f"\n{'='*60}")
    print(f"🙋 用户: {query}")
    print(f"{'='*60}")
    
    for i in range(10):  # 最多 10 轮
        response = llm_with_tools.invoke(messages)
        
        if not response.tool_calls:
            print(f"🤖 AI: {response.content}")
            return response.content
        
        for tc in response.tool_calls:
            print(f"\n🔧 第{i+1}轮 - 调用: {tc['name']}({tc['args']})")
            
            # 找到并执行工具
            for t in tools:
                if t.name == tc["name"]:
                    result = t.invoke(tc["args"])
                    break
            
            print(f"📦 结果: {result}")
            
            messages.append(response)
            messages.append({
                "role": "tool",
                "tool_call_id": tc["id"],
                "content": result,
            })
    
    return "已达到最大迭代次数"


if __name__ == "__main__":
    from langchain_openai import ChatOpenAI
    from dotenv import load_dotenv
    load_dotenv()
    
    llm = ChatOpenAI(model="deepseek-chat", temperature=0)
    llm_with_tools, tools, system = create_agent(llm, all_tools)
    
    # 测试各种场景
    run_agent_with_print(llm_with_tools, tools, system, "现在几点了？")
    run_agent_with_print(llm_with_tools, tools, system, "深圳天气怎么样？同时计算 2 的 10 次方是多少？")
    run_agent_with_print(llm_with_tools, tools, system, "帮我搜索一下 AI Agent 是什么？")
```

## 8. 高级：Tool 错误处理与重试

```python
from langchain_core.tools import tool
import traceback

# ============================================================
# 工具内部的错误处理
# ============================================================
@tool
def call_api(url: str) -> str:
    """
    调用外部 API 获取数据。
    参数:
        url: API 地址
    """
    import requests
    try:
        response = requests.get(url, timeout=10)
        response.raise_for_status()
        return response.text[:500]
    except requests.Timeout:
        # 超时错误：把错误信息返回给 LLM，让它决定如何处理
        return "错误：API 请求超时（超过10秒），请重试或检查网络"
    except requests.RequestException as e:
        return f"错误：API 请求失败 - {str(e)}"
    except Exception as e:
        return f"错误：未知错误 - {traceback.format_exc()}"


# ============================================================
# 工具调用失败后的 Agent 重试逻辑
# ============================================================
class AgentWithRetry:
    """
    带重试机制的 Agent。
    当工具调用失败时，会把错误信息反馈给 LLM，让 LLM 决定下一步。
    """
    
    def __init__(self, llm, tools: list, max_retries: int = 3):
        self.llm = llm.bind_tools(tools)
        self.tools = {t.name: t for t in tools}
        self.max_retries = max_retries
    
    def run(self, query: str) -> str:
        from langchain_core.messages import SystemMessage, HumanMessage
        
        messages = [
            SystemMessage(content="你是一个助手，可以使用工具。如果工具返回错误，请尝试修复参数后重试。"),
            HumanMessage(content=query),
        ]
        
        for attempt in range(self.max_retries):
            response = self.llm.invoke(messages)
            
            if not response.tool_calls:
                return response.content
            
            for tc in response.tool_calls:
                tool = self.tools.get(tc["name"])
                if not tool:
                    result = f"错误：未知工具 '{tc['name']}'"
                else:
                    try:
                        result = tool.invoke(tc["args"])
                    except Exception as e:
                        result = f"工具执行异常：{e}"
                
                messages.append(response)
                messages.append({
                    "role": "tool",
                    "tool_call_id": tc["id"],
                    "content": result,
                })
        
        # 超出重试次数，让 LLM 给个最终答复
        final = self.llm.invoke(messages)
        return final.content
```

---

## 总结

| 概念 | 说明 |
|------|------|
| **Tool** | LLM 调用外部功能的"接口"——一个函数 + 描述 + 参数 Schema |
| **@tool** | 最简单的工具定义方式，函数名→名称，文档→描述，类型注解→参数 |
| **bind_tools** | 把工具 Schema 注册到 LLM，让 LLM 知道有哪些工具可用 |
| **tool_calls** | LLM 返回的"调用指令"，包含工具名和参数 |
| **执行循环** | LLM 决定 → 代码执行 → 结果送回 LLM → LLM 再决定... |

> **下一章：** [04-langgraph-basics.md](./04-langgraph-basics.md) — LangGraph 基础详解
