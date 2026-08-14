# LangGraph 基础详解

> **LangGraph 是 LangChain 团队推出的 Agent 框架**，用**图 (Graph)** 来定义 Agent 的执行流程。相比传统的 `AgentExecutor`，LangGraph 更灵活、可控、可观察。

## 目录

1. [为什么需要 LangGraph？](#1-为什么需要-langgraph)
2. [核心概念](#2-核心概念)
3. [最简单的 StateGraph](#3-最简单的-stategraph)
4. [节点 (Nodes)](#4-节点-nodes)
5. [边 (Edges)](#5-边-edges)
6. [带 Tool Calling 的 Agent](#6-带-tool-calling-的-agent)
7. [条件分支](#7-条件分支)
8. [人机协同 (Human-in-the-Loop)](#8-人机协同-human-in-the-loop)

---

## 1. 为什么需要 LangGraph？

### 传统 Agent 的问题 (AgentExecutor)

```python
# 传统的 AgentExecutor 是一个黑盒循环
agent = AgentExecutor(agent=agent, tools=tools)
result = agent.invoke({"input": "北京的天气？"})
# ↑ 你不知道它内部调用了多少次 LLM，调用了哪些工具
```

| 问题 | AgentExecutor | LangGraph |
|------|---------------|-----------|
| **流程可视** | 黑盒，内部循环不可见 | ✅ 图结构，每步可监控 |
| **灵活控制** | 固定循环，难以干预 | ✅ 可在任意节点插入逻辑 |
| **状态管理** | 简单消息列表 | ✅ 自定义状态，可持久化 |
| **人机交互** | 不支持中途打断 | ✅ 支持中断和人类确认 |
| **并行执行** | ❌ | ✅ 支持并行边 |
| **条件逻辑** | 隐式 | ✅ 显式条件边 |

### LangGraph 的核心思想

```
把 Agent 的执行流程建模为"有向图"：

   ┌──────────┐
   │  开始节点  │
   └────┬─────┘
        │
   ┌────▼─────┐
   │  LLM 思考  │ ← 节点（Node）：执行具体逻辑
   └────┬─────┘
        │
   ┌────▼─────┐
   │ 需要工具？ │ ← 条件边（Conditional Edge）
   └────┬─────┘
        │
    ┌───┴───┐
    │       │
    ▼       ▼
  结束    执行工具 ← 节点
   │       │
   │  ┌────▼─────┐
   │  │ 返回结果   │
   │  └────┬─────┘
   └───────┘
           │
           ▼
        LLM 再思考...
```

## 2. 核心概念

```python
from langgraph.graph import StateGraph, State

# ============================================================
# LangGraph 的 4 个核心元素
# ============================================================

# 1. State（状态）
#    整个图的"共享内存"，所有节点都可以读写
#    通常是一个 TypedDict 或 Pydantic BaseModel
class AgentState(State):
    messages: list       # 对话历史
    next_step: str       # 下一步指令
    user_info: dict = {} # 可选字段

# 2. Node（节点）
#    图中的一个"处理单元"，接收 State，修改 State
def my_node(state: AgentState) -> AgentState:
    state["messages"].append({"role": "ai", "content": "处理完成"})
    return state

# 3. Edge（边）
#    节点之间的连接线，决定执行顺序

# 4. Conditional Edge（条件边）
#    根据当前状态决定走哪条路径
```

## 3. 最简单的 StateGraph

```python
from langgraph.graph import StateGraph, END
from typing import TypedDict, Annotated
import operator

# ============================================================
# 第1步：定义状态类型
# ============================================================
class SimpleState(TypedDict):
    """
    图的状态定义。
    所有节点共享这个状态字典。
    """
    messages: Annotated[list, operator.add]  # Annotated + operator.add 表示追加模式
    count: int                                # 普通字段，直接覆盖

# ============================================================
# 第2步：定义节点函数
# ============================================================
def node_a(state: SimpleState) -> SimpleState:
    """节点A：添加一条消息"""
    print(f"--- 执行 Node A ---")
    state["messages"].append("A 处理了")
    state["count"] = state.get("count", 0) + 1
    return state

def node_b(state: SimpleState) -> SimpleState:
    """节点B：添加一条消息"""
    print(f"--- 执行 Node B ---")
    state["messages"].append("B 处理了")
    state["count"] = state.get("count", 0) + 1
    return state

# ============================================================
# 第3步：构建图
# ============================================================
# 创建图，传入状态类型
workflow = StateGraph(SimpleState)

# 添加节点
workflow.add_node("node_a", node_a)  # 节点名 → 函数
workflow.add_node("node_b", node_b)

# 设置入口边（从哪里开始执行）
workflow.set_entry_point("node_a")

# 添加普通边：A → B
workflow.add_edge("node_a", "node_b")

# 添加结束边：B → END（END 是 LangGraph 的内置终止节点）
workflow.add_edge("node_b", END)

# ============================================================
# 第4步：编译图 → 获得可调用的应用
# ============================================================
app = workflow.compile()

# ============================================================
# 第5步：执行图
# ============================================================
result = app.invoke({
    "messages": [],  # 初始消息列表
    "count": 0,      # 初始计数
})

print(f"最终状态: {result}")
# messages: ["A 处理了", "B 处理了"]
# count: 2
```

### 可视化

想看图的结构？

```python
# 可以用 Mermaid 格式输出
print(app.get_graph().draw_mermaid())
# 或保存为图片
with open("graph.png", "wb") as f:
    f.write(app.get_graph().draw_png())
```

## 4. 节点 (Nodes)

### 4.1 节点的本质

**节点就是一个函数**，签名固定：`(State) -> State`

```python
def my_node(state: MyState) -> MyState:
    # 读：从 state 中读取数据
    messages = state["messages"]
    
    # 处理：执行业务逻辑
    result = some_function(messages)
    
    # 写：更新 state（返回即可）
    state["processed"] = result
    return state
```

### 4.2 多种节点类型

```python
# ---------- 1. 普通节点 ----------
def simple_node(state: dict) -> dict:
    state["step"] = "done"
    return state

# ---------- 2. LLM 节点 ----------
from langchain_openai import ChatOpenAI
from langchain_core.messages import HumanMessage, AIMessage

llm = ChatOpenAI(model="deepseek-chat", temperature=0)

def llm_node(state: dict) -> dict:
    """调用 LLM 的节点"""
    messages = state["messages"]
    response = llm.invoke(messages)
    state["messages"].append(response)
    return state

# ---------- 3. 工具执行节点 ----------
@tool
def get_weather(city: str) -> str:
    """查天气"""
    return f"{city}: 晴 25°C"

def tool_executor_node(state: dict) -> dict:
    """执行 LLM 请求的工具调用"""
    last_message = state["messages"][-1]
    
    if not last_message.tool_calls:
        return state
    
    for tool_call in last_message.tool_calls:
        tool_name = tool_call["name"]
        tool_args = tool_call["args"]
        
        if tool_name == "get_weather":
            result = get_weather.invoke(tool_args)
        
        state["messages"].append({
            "role": "tool",
            "tool_call_id": tool_call["id"],
            "content": result,
        })
    
    return state
```

## 5. 边 (Edges)

```python
from langgraph.graph import StateGraph, END

# ============================================================
# 三种边
# ============================================================

# 1️⃣ 普通边：无条件，A 执行完一定走 B
workflow.add_edge("node_a", "node_b")

# 2️⃣ 入口边：图从哪里开始
workflow.set_entry_point("start_node")

# 3️⃣ 条件边：根据条件选择不同路径
def should_continue(state: dict) -> str:
    """条件判断函数：返回下一个节点的名称，或 END"""
    if state.get("need_more"):
        return "process_more"  # 走这个节点
    else:
        return END             # 结束

workflow.add_conditional_edges(
    "decision_node",        # 出发点
    should_continue,        # 判断函数
    {
        "process_more": "next_node",  # 映射：函数返回值 → 目标节点
        END: END,
    }
)
```

## 6. 带 Tool Calling 的 Agent

这是 LangGraph 最经典的用法——实现一个完整的 ReAct Agent：

```python
import os
from dotenv import load_dotenv
from typing import TypedDict, Annotated, Literal
from langgraph.graph import StateGraph, END
from langgraph.graph.message import add_messages
from langchain_openai import ChatOpenAI
from langchain_core.tools import tool
from langchain_core.messages import BaseMessage, HumanMessage
import operator

load_dotenv()

# ============================================================
# 完整的 LangGraph Agent
# ============================================================

# ---------- 1. 定义工具 ----------
@tool
def get_weather(city: str) -> str:
    """查询城市天气"""
    data = {"北京": "晴 25°C", "上海": "多云 28°C", "广州": "小雨 30°C"}
    return data.get(city, f"{city}: 未知天气")

@tool
def calculator(expression: str) -> str:
    """数学计算，如 '2**10'"""
    import math
    allowed = {k: v for k, v in math.__dict__.items() if not k.startswith("_")}
    return str(eval(expression, {"__builtins__": {}}, allowed))

tools = [get_weather, calculator]

# ---------- 2. 定义状态 ----------
class AgentState(TypedDict):
    """
    状态定义：
    - messages: 对话消息列表，使用 add_messages 实现追加行为
    """
    messages: Annotated[list, add_messages]

# ---------- 3. 定义 LLM 节点 ----------
llm = ChatOpenAI(model="deepseek-chat", temperature=0)
llm_with_tools = llm.bind_tools(tools)

def call_model(state: AgentState) -> AgentState:
    """
    LLM 节点：
    1. 获取当前消息列表
    2. 调用 LLM（LLM 可能返回文本，也可能返回 tool_calls）
    3. 将 LLM 回复追加到消息列表
    """
    messages = state["messages"]
    response = llm_with_tools.invoke(messages)
    
    # 更新状态：追加 LLM 的回复
    return {"messages": [response]}

# ---------- 4. 定义工具执行节点 ----------
def call_tools(state: AgentState) -> AgentState:
    """
    工具执行节点：
    1. 获取 LLM 最后一条消息中的 tool_calls
    2. 遍历执行每个工具
    3. 将工具结果追加到消息列表
    """
    last_message = state["messages"][-1]
    tool_messages = []
    
    for tool_call in last_message.tool_calls:
        # 找到对应的工具
        for t in tools:
            if t.name == tool_call["name"]:
                result = t.invoke(tool_call["args"])
                break
        else:
            result = f"未知工具: {tool_call['name']}"
        
        # 创建工具结果消息（LangGraph 要求的格式）
        from langchain_core.messages import ToolMessage
        tool_messages.append(ToolMessage(
            content=result,
            tool_call_id=tool_call["id"],
        ))
    
    # 返回工具结果消息，它们会被追加到 messages 中
    return {"messages": tool_messages}

# ---------- 5. 条件边：判断是否要调用工具 ----------
def should_continue(state: AgentState) -> Literal["tools", END]:
    """
    条件判断：
    - 如果 LLM 返回了 tool_calls → 去执行工具
    - 否则 → 结束
    """
    last_message = state["messages"][-1]
    if last_message.tool_calls:
        return "tools"
    return END

# ---------- 6. 构建图 ----------
workflow = StateGraph(AgentState)

# 添加节点
workflow.add_node("agent", call_model)     # LLM 思考节点
workflow.add_node("tools", call_tools)     # 工具执行节点

# 设置入口
workflow.set_entry_point("agent")

# 条件边：agent 节点之后，根据 should_continue 决定去向
workflow.add_conditional_edges(
    "agent",
    should_continue,
    {
        "tools": "tools",  # 需要工具 → 去 tools 节点
        END: END,           # 不需要 → 结束
    }
)

# 普通边：tools 执行完 → 回到 agent 继续思考
workflow.add_edge("tools", "agent")

# 编译
app = workflow.compile()

# ---------- 7. 运行 ----------
def run_agent(query: str):
    """运行代理并输出过程"""
    events = app.stream(
        {"messages": [HumanMessage(content=query)]},
        # stream_mode="values" 可以打印每步状态
    )
    
    for event in events:
        # event 是每步执行后的状态快照
        if "messages" in event:
            last_msg = event["messages"][-1]
            if hasattr(last_msg, "content") and last_msg.content:
                print(f"🤖 {last_msg.content}")
            if hasattr(last_msg, "tool_calls") and last_msg.tool_calls:
                for tc in last_msg.tool_calls:
                    print(f"🔧 调用: {tc['name']}({tc['args']})")

# 测试
if __name__ == "__main__":
    print("=" * 50)
    print("测试：需要工具的查询")
    print("=" * 50)
    run_agent("北京的天气怎么样？")
    
    print("\n" + "=" * 50)
    print("测试：需要工具的复合查询")
    print("=" * 50)
    run_agent("计算 15 * 28 + 100 等于多少？")
    
    print("\n" + "=" * 50)
    print("测试：不需要工具的查询")
    print("=" * 50)
    run_agent("你好！请简单介绍一下自己。")
```

### 图的结构

```
      ┌──────────┐
      │  入口     │
      └────┬─────┘
           │
      ┌────▼─────┐
      │  agent    │  ← LLM 思考
      └────┬─────┘
           │
      ┌────▼─────┐
      │ 条件判断   │  ← should_continue()
      └────┬─────┘
           │
     ┌─────┴──────┐
     │            │
     ▼            ▼
  tools         END
     │
     └──────► agent (循环)
```

## 7. 条件分支

### 7.1 复杂条件路由

```python
from typing import TypedDict, Literal

class RouterState(TypedDict):
    query: str
    category: str
    result: str

# ============================================================
# 分类路由器：根据用户问题类型，路由到不同的处理节点
# ============================================================

def classify_query(state: RouterState) -> Literal["weather", "math", "general"]:
    """根据查询内容分类"""
    q = state["query"]
    if any(kw in q for kw in ["天气", "温度", "下雨", "刮风"]):
        return "weather"
    elif any(kw in q for kw in ["计算", "等于", "加", "减", "乘", "除"]):
        return "math"
    else:
        return "general"

# 在图中使用
workflow.add_conditional_edges(
    "classifier",          # 分类节点
    classify_query,        # 分类函数
    {
        "weather": "weather_node",  # weather 类 → 天气节点
        "math": "math_node",        # math 类 → 数学节点
        "general": "general_node",  # general 类 → 通用节点
    }
)
```

### 7.2 自循环（直到条件满足）

```python
# 实现一个"反复处理直到达到目标"的循环
def check_quality(state: dict) -> Literal["improve", END]:
    """质量检查：如果不够好，继续改进"""
    if state.get("quality_score", 0) >= 90:
        return END        # ≥ 90 分 → 结束
    else:
        return "improve"  # < 90 分 → 继续改进

workflow.add_conditional_edges(
    "quality_check",
    check_quality,
    {
        "improve": "rewrite_node",  # 回到改写节点
        END: END,
    }
)
# rewrite_node → quality_check → (improve → rewrite_node | END)
```

## 8. 人机协同 (Human-in-the-Loop)

```python
# ============================================================
# 在关键节点让人类确认
# LangGraph 可以在执行到特定节点时中断，等待人类输入
# ============================================================

def human_approval_node(state: dict) -> dict:
    """
    人类确认节点：
    在发送邮件/执行危险操作前，让用户确认
    """
    last_message = state["messages"][-1]
    
    if last_message.tool_calls:
        print(f"\n⚠️  需要您的确认：")
        for tc in last_message.tool_calls:
            print(f"  → 将执行: {tc['name']}(参数: {tc['args']})")
        
        # 等待用户输入
        user_input = input("确认执行？(y/n): ").strip().lower()
        
        if user_input != "y":
            # 用户拒绝 → 修改 state，让 LLM 知道被拒绝了
            state["messages"].append({
                "role": "tool",
                "tool_call_id": last_message.tool_calls[0]["id"],
                "content": "用户取消了此操作。请告知用户操作已取消。",
            })
    
    return state


# 在构建图时，可以在 agent 和 tools 之间插入 human_approval
# workflow.add_node("human_approval", human_approval_node)
# workflow.add_edge("agent", "human_approval")
# workflow.add_conditional_edges("human_approval", should_continue, ...)

# 或者使用 LangGraph 内置的 interrupt 机制
from langgraph.checkpoint import MemorySaver

# 使用 checkpointer 可以保存/恢复状态
checkpointer = MemorySaver()

# 编译时设置中断点
app = workflow.compile(
    checkpointer=checkpointer,
    interrupt_before=["tools"],  # 在执行 tools 之前中断
)

# 运行后，可以手动恢复
# config = {"configurable": {"thread_id": "1"}}
# result = app.invoke(input_data, config)
# # 此时中断在 tools 前
# # 检查、修改后继续
# result = app.invoke(None, config)  # 继续执行
```

## 总结

| 概念 | 说明 |
|------|------|
| **StateGraph** | LangGraph 的核心类，定义图的结构 |
| **State** | 图的共享状态，所有节点可读写 |
| **Node** | 图中的处理单元（函数：State → State） |
| **Edge** | 节点间的连接（普通边 / 条件边） |
| **Conditional Edge** | 根据状态动态选择路径 |
| **add_messages** | 消息追加 reducer，自动合并消息列表 |
| **END** | 内置终止节点 |
| **Compile** | 编译图获得可调用的应用 |

> **下一章：** [05-building-agents.md](./05-building-agents.md) — 构建智能体实战
