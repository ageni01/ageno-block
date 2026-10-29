---
title: "构建智能体实战"

category:
  - AI 应用
tag:
  - AI 应用
---
# 构建智能体实战

> 从零开始搭建一个真正实用的 AI Agent。本章综合前 4 章知识，带你完成 3 个实战项目。

## 目录

1. [项目规划思路](#1-项目规划思路)
2. [项目一：多功能助手 Agent](#2-项目一多功能助手-agent)
3. [项目二：RAG + 工具 Agent](#3-项目二rag--工具-agent)
4. [项目三：多 Agent 协作系统](#4-项目三多-agent-协作系统)
5. [部署与最佳实践](#5-部署与最佳实践)

---

## 1. 项目规划思路

### 搭建 Agent 的标准步骤

```
┌─────────────────────────────────────────────────────┐
│  1. 需求分析                                         │
│     └─ 用户需要什么能力？查天气？搜资料？写代码？      │
├─────────────────────────────────────────────────────┤
│  2. 工具设计                                         │
│     └─ 需要哪些 Tool？每个 Tool 的输入/输出是什么？    │
├─────────────────────────────────────────────────────┤
│  3. 流程设计                                         │
│     └─ LLM 思考 → 执行工具 → 继续思考 → 结束         │
│     是否需要条件分支？是否需要人工确认？               │
├─────────────────────────────────────────────────────┤
│  4. 技术选型                                         │
│     └─ 普通任务：AgentExecutor（简单快速）             │
│     复杂流程：LangGraph（灵活可控）                   │
├─────────────────────────────────────────────────────┤
│  5. 实现与测试                                       │
│     └─ 写代码 → 测边界情况 → 优化 prompt → 上线       │
└─────────────────────────────────────────────────────┘
```

## 2. 项目一：多功能助手 Agent

一个具备天气查询、计算、知识搜索、时间查询的通用助手。

### 2.1 完整代码

```python
# multi_tool_agent.py
"""
一个使用 LangGraph 构建的多功能 AI 助手。
功能：天气查询、数学计算、知识搜索、当前时间
"""
import os
from typing import TypedDict, Annotated, Literal
from datetime import datetime, timezone, timedelta
import json

from dotenv import load_dotenv
from langgraph.graph import StateGraph, END
from langgraph.graph.message import add_messages
from langchain_openai import ChatOpenAI
from langchain_core.tools import tool
from langchain_core.messages import BaseMessage, HumanMessage, ToolMessage

load_dotenv()

# ============================================================
# 第1步：定义工具
# ============================================================

@tool
def get_weather(city: str) -> str:
    """
    查询指定城市的当前天气。
    当用户问"天气"时使用。
    """
    data = {
        "北京": "🌤 晴，25°C，空气质量良",
        "上海": "⛅ 多云，28°C，湿度 65%",
        "广州": "🌦 小雨，30°C，体感闷热",
        "深圳": "☀️ 晴，29°C，适合户外活动",
        "杭州": "☁️ 阴，24°C，适宜出行",
    }
    return data.get(city, f"暂无 {city} 的天气数据")


@tool
def calculator(expression: str) -> str:
    """
    执行数学计算。
    当用户需要精确计算时使用。支持 + - * / ** // % 等运算。
    例如 calculator("2**10") 或 calculator("(15+3)*2")
    """
    import math
    allowed = {k: v for k, v in math.__dict__.items() if not k.startswith("_")}
    allowed.update({"abs": abs, "round": round, "int": int, "float": float})
    try:
        result = eval(expression, {"__builtins__": {}}, allowed)
        return str(result)
    except Exception as e:
        return f"计算错误: {e}"


@tool
def get_current_time() -> str:
    """获取当前的北京时间（日期和时间）。"""
    tz = timezone(timedelta(hours=8))
    now = datetime.now(tz)
    return now.strftime("%Y年%m月%d日 %H:%M:%S，星期%w")


@tool
def search_knowledge(query: str) -> str:
    """
    搜索内置知识库，查找相关知识。
    当需要解释概念或查询信息时使用。
    """
    knowledge = {
        "python": "Python 是一种解释型、高级编程语言，由 Guido van Rossum 于 1991 年创建。以简洁优雅著称。",
        "ai agent": "AI Agent（智能体）是能自主感知环境、做出决策并执行行动的智能系统。核心模式：感知→思考→行动。",
        "langchain": "LangChain 是一个 LLM 应用开发框架，提供模型调用、提示管理、链式组合、Agent 等能力。",
        "langgraph": "LangGraph 是 LangChain 的图结构 Agent 框架，用状态图管理 Agent 执行流程，支持循环、条件分支等。",
        "gpt": "GPT（Generative Pre-trained Transformer）是 OpenAI 开发的大语言模型系列。",
        "机器学习": "机器学习是 AI 的分支，让计算机从数据中学习规律，无需显式编程。包括监督学习、无监督学习、强化学习。",
        "深度学习": "深度学习是机器学习的子集，使用多层神经网络学习数据表征。",
    }
    for key, value in knowledge.items():
        if key in query.lower():
            return value
    return f"知识库中未找到与'{query}'相关的信息。"


# ============================================================
# 第2步：创建工具列表和 LLM
# ============================================================

tools = [get_weather, calculator, get_current_time, search_knowledge]

llm = ChatOpenAI(
    model="deepseek-chat",
    temperature=0,
)
llm_with_tools = llm.bind_tools(tools)


# ============================================================
# 第3步：定义图的状态
# ============================================================

class AgentState(TypedDict):
    """Agent 的状态"""
    messages: Annotated[list, add_messages]


# ============================================================
# 第4步：定义节点
# ============================================================

def call_model(state: AgentState) -> AgentState:
    """LLM 推理节点"""
    messages = state["messages"]
    response = llm_with_tools.invoke(messages)
    return {"messages": [response]}


def call_tools(state: AgentState) -> AgentState:
    """工具执行节点"""
    last_message = state["messages"][-1]
    tool_messages = []

    for tool_call in last_message.tool_calls:
        # 根据工具名分发执行
        for t in tools:
            if t.name == tool_call["name"]:
                result = t.invoke(tool_call["args"])
                break
        else:
            result = f"错误: 未找到工具 {tool_call['name']}"

        tool_messages.append(ToolMessage(
            content=result,
            tool_call_id=tool_call["id"],
        ))

    return {"messages": tool_messages}


# ============================================================
# 第5步：条件判断
# ============================================================

def should_continue(state: AgentState) -> Literal["tools", END]:
    """判断是否需要继续调用工具"""
    last_message = state["messages"][-1]
    return "tools" if last_message.tool_calls else END


# ============================================================
# 第6步：构建图
# ============================================================

workflow = StateGraph(AgentState)
workflow.add_node("agent", call_model)
workflow.add_node("tools", call_tools)
workflow.set_entry_point("agent")

workflow.add_conditional_edges(
    "agent",
    should_continue,
    {"tools": "tools", END: END},
)
workflow.add_edge("tools", "agent")

app = workflow.compile()


# ============================================================
# 第7步：运行入口
# ============================================================

def chat():
    """交互式命令行对话"""
    print("=" * 60)
    print("  多功能 AI 助手已启动！")
    print("  支持：天气查询 / 数学计算 / 知识搜索 / 时间查询")
    print("  输入 'exit' 退出")
    print("=" * 60)

    messages = []

    while True:
        user_input = input("\n🙋 ").strip()
        if user_input.lower() in ("exit", "quit", "q"):
            print("👋 再见！")
            break

        events = app.stream(
            {"messages": [HumanMessage(content=user_input)]},
        )

        for event in events:
            if "messages" in event:
                last_msg = event["messages"][-1]
                if hasattr(last_msg, "content") and last_msg.content:
                    print(f"\n🤖 {last_msg.content}")
                if hasattr(last_msg, "tool_calls") and last_msg.tool_calls:
                    for tc in last_msg.tool_calls:
                        print(f"  🔧 使用工具: {tc['name']}({tc['args']})")


if __name__ == "__main__":
    chat()
```

### 2.2 运行测试

```bash
uv run python multi_tool_agent.py
```

**对话示例：**

```
🙋 北京的天气怎么样？
  🔧 使用工具: get_weather({'city': '北京'})
🤖 北京今天的天气是🌤 晴，25°C，空气质量良。

🙋 计算 25 的平方根，然后加上 100
  🔧 使用工具: calculator({'expression': 'sqrt(25)'})
  🔧 使用工具: calculator({'expression': '5+100'})
🤖 25 的平方根是 5，5+100=105。所以结果是 105。

🙋 现在几点了？
  🔧 使用工具: get_current_time()
🤖 现在是 2026年05月29日 14:30:00，星期5。

🙋 什么是 AI Agent？
  🔧 使用工具: search_knowledge({'query': 'ai agent'})
🤖 AI Agent（智能体）是能自主感知环境、做出决策并执行行动的智能系统。核心模式：感知→思考→行动。
```

## 3. 项目二：RAG + 工具 Agent

结合 RAG（知识库检索）和工具调用能力的增强版 Agent。

```python
# rag_agent.py
"""
带 RAG 知识库的 Agent。
从文档中检索信息，并结合工具能力回答问题。
"""
import os
from typing import TypedDict, Annotated, Literal, List

from dotenv import load_dotenv
from langgraph.graph import StateGraph, END
from langgraph.graph.message import add_messages
from langchain_openai import ChatOpenAI, OpenAIEmbeddings
from langchain_core.tools import tool
from langchain_core.messages import HumanMessage, ToolMessage
from langchain_core.vectorstores import InMemoryVectorStore
from langchain.text_splitter import RecursiveCharacterTextSplitter
from langchain_community.document_loaders import TextLoader

load_dotenv()

# ============================================================
# 第1步：准备 RAG 知识库
# ============================================================

def build_knowledge_base():
    """
    构建内存向量知识库。
    生产环境建议使用 Chroma / PGVector / Pinecone 等持久化存储。
    """
    # 模拟知识库文档
    documents = [
        "LangChain 是一个用于构建 LLM 应用的框架。它提供了模型调用、提示管理、链式组合等能力。"
        "最新版本是 v0.3。支持 OpenAI、Anthropic、Google 等多种模型。",

        "LangGraph 是 LangChain 团队推出的 Agent 框架。它使用有向图来定义 Agent 的执行流程。"
        "核心概念包括 StateGraph、Node、Edge。支持循环、条件分支、人机协同。",

        "AI Agent 是一种能够自主执行任务的智能系统。它通常包含：感知环境、制定计划、执行行动、观察结果四个步骤。"
        "ReAct 模式是 Agent 的经典实现方式，让 LLM 交替进行推理和行动。",

        "Tool Calling（函数调用）是 LLM 的一项关键能力。LLM 可以返回结构化的工具调用指令，"
        "而不是纯文本。这让 LLM 能够调用外部 API、执行代码、查询数据库等。",

        "RAG（检索增强生成）是一种将信息检索与文本生成结合的技术。"
        "流程：用户问题 → 检索相关文档 → 文档+问题 → LLM 生成回答。可以有效减少幻觉。",
    ]

    # 文本分割
    splitter = RecursiveCharacterTextSplitter(
        chunk_size=200,
        chunk_overlap=20,
    )
    chunks = splitter.create_documents(documents)

    # 创建向量存储
    embeddings = OpenAIEmbeddings(model="text-embedding-ada-002")
    vectorstore = InMemoryVectorStore.from_documents(chunks, embeddings)

    return vectorstore.as_retriever(search_kwargs={"k": 2})


# 全局检索器
retriever = build_knowledge_base()


# ============================================================
# 第2步：定义工具（含 RAG 检索）
# ============================================================

@tool
def rag_search(query: str) -> str:
    """
    从内部知识库中检索信息。
    当需要查询框架、概念、技术文档时使用。
    """
    docs = retriever.invoke(query)
    if not docs:
        return "未找到相关信息"
    return "\n\n".join(doc.page_content for doc in docs)


@tool
def web_search(query: str) -> str:
    """
    搜索互联网获取最新信息。
    当需要实时数据或知识库没有的信息时使用。
    """
    # 实际项目中应接入 Google Search / Bing Search / SearXNG 等
    # 这里仅作演示
    return f"[模拟搜索] 关于 '{query}' 的搜索结果：\n1. 相关链接A\n2. 相关链接B"


@tool
def calculator(expression: str) -> str:
    """执行精确的数学计算。"""
    import math
    allowed = {k: v for k, v in math.__dict__.items() if not k.startswith("_")}
    try:
        return str(eval(expression, {"__builtins__": {}}, allowed))
    except Exception as e:
        return f"计算错误: {e}"


tools = [rag_search, web_search, calculator]


# ============================================================
# 第3步：Agent 定义
# ============================================================

llm = ChatOpenAI(model="deepseek-chat", temperature=0).bind_tools(tools)


class AgentState(TypedDict):
    messages: Annotated[list, add_messages]


def call_model(state: AgentState) -> AgentState:
    return {"messages": [llm.invoke(state["messages"])]}


def call_tools(state: AgentState) -> AgentState:
    last_message = state["messages"][-1]
    tool_messages = []
    for tc in last_message.tool_calls:
        for t in tools:
            if t.name == tc["name"]:
                result = t.invoke(tc["args"])
                break
        tool_messages.append(ToolMessage(content=result, tool_call_id=tc["id"]))
    return {"messages": tool_messages}


def should_continue(state: AgentState) -> Literal["tools", END]:
    return "tools" if state["messages"][-1].tool_calls else END


# ---------- 构建图 ----------
workflow = StateGraph(AgentState)
workflow.add_node("agent", call_model)
workflow.add_node("tools", call_tools)
workflow.set_entry_point("agent")
workflow.add_conditional_edges("agent", should_continue, {"tools": "tools", END: END})
workflow.add_edge("tools", "agent")
app = workflow.compile()


# ============================================================
# 第4步：运行
# ============================================================

def ask(query: str):
    """向 Agent 提问"""
    print(f"\n🙋 {query}")
    events = app.stream({"messages": [HumanMessage(content=query)]})
    for event in events:
        if "messages" in event:
            m = event["messages"][-1]
            if hasattr(m, "tool_calls") and m.tool_calls:
                for tc in m.tool_calls:
                    print(f"  🔍 检索: {tc['name']}({tc['args']})")
            if hasattr(m, "content") and m.content:
                print(f"🤖 {m.content}")


if __name__ == "__main__":
    # 测试1：RAG 检索
    ask("什么是 LangGraph？它和普通的 AgentExecutor 有什么区别？")

    # 测试2：需要计算
    ask("如果我有 35 个苹果，每人分 4 个，可以分给几个人？还剩几个？")

    # 测试3：结合 RAG + 计算
    ask("LangChain 最新版本是多少？和 LangGraph 是什么关系？")
```

## 4. 项目三：多 Agent 协作系统

> 真实项目中，一个 Agent 往往不够。分工协作是大型系统的关键。

### 4.1 架构设计

```
                  ┌──────────────────────┐
                  │   Supervisor (主管)    │
                  │   接收用户问题         │
                  │   分派给合适的子 Agent │
                  └──────┬───────────────┘
                         │
           ┌─────────────┼─────────────┐
           │             │             │
      ┌────▼────┐  ┌────▼────┐  ┌────▼────┐
      │ 研究员   │  │ 计算器   │  │ 写作    │
      │ Agent   │  │ Agent   │  │ Agent   │
      └────┬────┘  └────┬────┘  └────┬────┘
           │             │             │
      ┌────▼────┐  ┌────▼────┐  ┌────▼────┐
      │ 搜索工具  │  │ 计算器  │  │ 格式化  │
      │ 知识库   │  │        │  │ 输出    │
      └─────────┘  └─────────┘  └─────────┘
```

### 4.2 代码实现

```python
# multi_agent_system.py
"""
多 Agent 协作系统：主管 Agent 分派任务，子 Agent 专注执行。
"""
import os
from typing import TypedDict, Annotated, Literal, List
from dotenv import load_dotenv
from langgraph.graph import StateGraph, END, MessageGraph
from langgraph.graph.message import add_messages
from langchain_openai import ChatOpenAI
from langchain_core.tools import tool
from langchain_core.messages import HumanMessage, AIMessage, ToolMessage, SystemMessage

load_dotenv()

# ============================================================
# 共享 LLM
# ============================================================
llm = ChatOpenAI(model="deepseek-chat", temperature=0)


# ============================================================
# Agent 1: 研究员 Agent（搜索 + 知识检索）
# ============================================================
@tool
def search_info(query: str) -> str:
    """搜索互联网或知识库获取信息"""
    knowledge = {
        "python": "Python 1991年发布，当前稳定版 3.13",
        "langchain": "LangChain v0.3，支持 OpenAI/Anthropic/Google",
        "golang": "Go 1.22 发布，泛型已稳定",
    }
    return knowledge.get(query.lower(), f"[信息] {query} 的相关资料...")


researcher_tools = [search_info]
researcher_llm = llm.bind_tools(researcher_tools)

def researcher_agent(state: dict) -> dict:
    """研究员 Agent：专门负责搜索和查找信息"""
    system = SystemMessage(content="你是一个研究员。使用 search_info 查找信息，然后用中文总结。")
    messages = [system] + state["messages"]
    response = researcher_llm.invoke(messages)

    if response.tool_calls:
        # 执行工具
        for tc in response.tool_calls:
            for t in researcher_tools:
                if t.name == tc["name"]:
                    result = t.invoke(tc["args"])
                    messages.append(response)
                    messages.append(ToolMessage(content=result, tool_call_id=tc["id"]))
        # 再次调用 LLM 总结
        final = llm.invoke(messages)
        return {"messages": [final]}

    return {"messages": [response]}


# ============================================================
# Agent 2: 计算 Agent（数学运算）
# ============================================================
@tool
def math_calc(expression: str) -> str:
    """执行数学计算"""
    import math
    allowed = {k: v for k, v in math.__dict__.items() if not k.startswith("_")}
    return str(eval(expression, {"__builtins__": {}}, allowed))


calc_tools = [math_calc]
calc_llm = llm.bind_tools(calc_tools)

def calculator_agent(state: dict) -> dict:
    """计算 Agent：专门负责数学计算"""
    system = SystemMessage(content="你是一个数学计算专家。使用 math_calc 计算，然后解释结果。")
    messages = [system] + state["messages"]
    response = calc_llm.invoke(messages)

    if response.tool_calls:
        for tc in response.tool_calls:
            for t in calc_tools:
                if t.name == tc["name"]:
                    result = t.invoke(tc["args"])
                    messages.append(response)
                    messages.append(ToolMessage(content=result, tool_call_id=tc["id"]))
        final = llm.invoke(messages)
        return {"messages": [final]}

    return {"messages": [response]}


# ============================================================
# Agent 3: 写作 Agent（润色和格式化）
# ============================================================
def writer_agent(state: dict) -> dict:
    """写作 Agent：负责将信息整理成易读的格式"""
    system = SystemMessage(content="你是一个写作专家。将输入的信息整理成清晰、有条理的回复。"
                                   "使用 Markdown 格式：标题、列表、重点加粗。")
    messages = [system] + state["messages"]
    response = llm.invoke(messages)
    return {"messages": [response]}


# ============================================================
# Supervisor（主管）：决定派哪个 Agent
# ============================================================

# 主管的"工具"就是三个子 Agent
supervisor_tools = [
    tool(researcher_agent, name="researcher", description="当需要搜索信息、查询知识时使用"),
    tool(calculator_agent, name="calculator", description="当需要数学计算时使用"),
    tool(writer_agent, name="writer", description="当需要整理信息、润色文字时使用"),
]

supervisor_llm = llm.bind_tools(supervisor_tools)


# ============================================================
# 构建主管 Agent 图
# ============================================================

class SupervisorState(TypedDict):
    messages: Annotated[list, add_messages]


def supervisor_node(state: SupervisorState) -> SupervisorState:
    """主管节点：判断应该调用哪个子 Agent"""
    system = SystemMessage(
        content="你是一个主管，负责协调多个专家。\n"
                "用户的问题需要分派给对应的专家：\n"
                "- 查询信息 → 研究员 (researcher)\n"
                "- 数学计算 → 计算器 (calculator)\n"
                "- 整理内容 → 写手 (writer)\n"
                "- 综合问题 → 可以依次调用多个专家\n"
                "根据用户的意图选择最合适的专家。"
    )
    messages = [system] + state["messages"]
    response = supervisor_llm.invoke(messages)
    return {"messages": [response]}


def supervisor_should_continue(state: SupervisorState) -> Literal["tools", END]:
    """判断主管是否需要调用子 Agent"""
    return "tools" if state["messages"][-1].tool_calls else END


def execute_sub_agent(state: SupervisorState) -> SupervisorState:
    """执行子 Agent 节点"""
    last_message = state["messages"][-1]
    tool_messages = []

    for tc in last_message.tool_calls:
        # 调用对应的子 Agent
        for t in supervisor_tools:
            if t.name == tc["name"]:
                # 子 Agent 接收消息并返回结果
                result = t.invoke(tc["args"])
                break
        else:
            result = f"未知 Agent: {tc['name']}"

        tool_messages.append(ToolMessage(content=str(result), tool_call_id=tc["id"]))

    return {"messages": tool_messages}


# 构建主管图
supervisor_graph = StateGraph(SupervisorState)
supervisor_graph.add_node("supervisor", supervisor_node)
supervisor_graph.add_node("sub_agents", execute_sub_agent)
supervisor_graph.set_entry_point("supervisor")
supervisor_graph.add_conditional_edges(
    "supervisor",
    supervisor_should_continue,
    {"sub_agents": "sub_agents", END: END},
)
supervisor_graph.add_edge("sub_agents", "supervisor")

app = supervisor_graph.compile()


# ============================================================
# 运行
# ============================================================

def chat():
    print("=" * 60)
    print("  多 Agent 协作系统 v1.0")
    print("  研究员 + 计算器 + 写手 协同工作")
    print("=" * 60)

    while True:
        query = input("\n🙋 ").strip()
        if query.lower() in ("exit", "quit"):
            break

        events = app.stream({"messages": [HumanMessage(content=query)]})
        for event in events:
            if "messages" in event:
                m = event["messages"][-1]
                if hasattr(m, "tool_calls") and m.tool_calls:
                    for tc in m.tool_calls:
                        print(f"  🔀 分派给: {tc['name']}")
                if hasattr(m, "content") and m.content:
                    print(f"  {m.content[:200]}")


if __name__ == "__main__":
    chat()
```

### 4.3 运行效果

```
================================================================
  多 Agent 协作系统 v1.0
  研究员 + 计算器 + 写手 协同工作
================================================================

🙋 Python 是哪年发布的？请查一下
  🔀 分派给: researcher
  Python 由 Guido van Rossum 于 1991 年发布...

🙋 中国人口约14亿，如果每人捐1元钱，总共多少钱？
  🔀 分派给: calculator
  14亿 × 1元 = 14亿元

🙋 帮我整理一下 Python 和 Go 语言的区别
  🔀 分派给: researcher
  🔀 分派给: writer
  ## Python vs Go 对比
  ...
```

## 5. 部署与最佳实践

### 5.1 项目结构建议

```
my-agent-project/
├── pyproject.toml            # 项目依赖
├── .env                      # 密钥配置
├── src/
│   ├── __init__.py
│   ├── agent/                # Agent 定义
│   │   ├── __init__.py
│   │   ├── graph.py          # LangGraph 图定义
│   │   ├── nodes.py          # 节点函数
│   │   └── state.py          # 状态定义
│   ├── tools/                # 工具定义
│   │   ├── __init__.py
│   │   ├── weather.py
│   │   ├── calculator.py
│   │   └── search.py
│   ├── models/               # 数据模型
│   │   └── schemas.py
│   └── main.py               # 入口
├── tests/                    # 测试
│   ├── test_agent.py
│   └── test_tools.py
└── knowledge/                # 知识库文档
    └── docs.txt
```

### 5.2 最佳实践清单

```python
# ============================================================
# ✅ DO - 推荐做法
# ============================================================

# 1. Tool description 要详细
@tool
def search_products(keyword: str) -> str:
    """Search for products by keyword. Use this when the user wants to find products, shop, or browse items."""
    ...

# 2. Tool 要有完善的错误处理
@tool
def call_api(url: str) -> str:
    try:
        ...
    except Exception as e:
        return f"API Error: {e}"  # 而不是 raise

# 3. 使用 Pydantic 校验参数
from pydantic import BaseModel, Field

class SearchInput(BaseModel):
    query: str = Field(description="用户搜索的关键词，应该提取核心关键词")
    limit: int = Field(default=5, ge=1, le=50, description="返回结果数量")

# 4. prompt 要清晰指定 LLM 何时使用什么工具
SYSTEM_PROMPT = """
你是一个助手。可用工具：
- get_weather: 查天气（当用户提到城市+天气时）
- calculate: 数学计算（当用户需要计算时）
- search: 搜索知识（当用户问"什么是"时）

注意：
- 如果一个工具就能回答，不要调多个工具
- 如果工具出错了，尝试换个参数重试，或直接告诉用户
"""


# ❌ DON'T - 避免做法
# 1. 不要在 tool 里直接 raise Exception
# 2. 不要用模糊的 description
# 3. 不要让 tool 过于通用（一个 tool 做一件事）
# 4. 不要忽略 tool 的调用结果
```

### 5.3 包管理命令速查

```bash
# 创建项目
uv init my-agent && cd my-agent

# 安装依赖
uv add langchain langchain-openai langgraph
uv add python-dotenv httpx

# 开发工具
uv add --dev pytest mypy ruff

# 运行
uv run python src/main.py

# 测试
uv run pytest tests/

# 类型检查
uv run mypy src/

# 代码格式化
uv run ruff format src/
```

---

## 总结

| 项目 | 核心知识点 | 难度 |
|------|-----------|------|
| **多功能助手 Agent** | @tool, bind_tools, LangGraph 基本图 | ⭐ |
| **RAG + 工具 Agent** | 向量检索、知识库、RAG 管道 | ⭐⭐ |
| **多 Agent 协作** | 分层架构、任务分派、子 Agent | ⭐⭐⭐ |

**学习路径建议：**

1. 从项目一开始，理解 Tool + LangGraph 的基本模式
2. 给项目一添加更多工具，熟悉各种工具类型
3. 尝试项目二，掌握 RAG 与 Agent 的融合
4. 最后挑战项目三，学习多 Agent 架构
