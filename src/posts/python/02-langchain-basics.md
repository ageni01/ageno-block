# LangChain 基础详解

> LangChain 是一个构建 LLM 应用的框架，核心概念：**模型 (Models)**、**提示 (Prompts)**、**链 (Chains)**、**记忆 (Memory)**、**工具 (Tools)**、**智能体 (Agents)**。

## 目录

1. [核心架构](#1-核心架构)
2. [Chat Models](#2-chat-models)
3. [Prompt Templates](#3-prompt-templates)
4. [Output Parsers](#4-output-parsers)
5. [Chains](#5-chains)
6. [Memory](#6-memory)
7. [RAG 基础](#7-rag-基础)
8. [完整示例：问答机器人](#8-完整示例问答机器人)

---

## 1. 核心架构

```
┌─────────────────────────────────────────────┐
│                  Application                  │
│  ┌─────────┐  ┌─────────┐  ┌─────────────┐  │
│  │ Chains  │  │ Agents  │  │  RAG 管道    │  │
│  └────┬────┘  └────┬────┘  └──────┬──────┘  │
│       │            │              │          │
│  ┌────▼────────────▼──────────────▼──────┐   │
│  │           Core Components              │   │
│  │  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐  │   │
│  │  │Models│ │Prompt│ │Memory│ │Tools │  │   │
│  │  └──────┘ └──────┘ └──────┘ └──────┘  │   │
│  └────────────────────────────────────────┘   │
└─────────────────────────────────────────────┘
```

## 2. Chat Models

LangChain 支持多种模型提供商，最重要的是 **Chat Models**（对话模型）。

### 2.1 支持的模型

| 提供商 | 类名 | 包名 |
|--------|------|------|
| OpenAI | `ChatOpenAI` | `langchain-openai` |
| DeepSeek | `ChatOpenAI`（兼容接口） | `langchain-openai` |
| Anthropic | `ChatAnthropic` | `langchain-anthropic` |
| 阿里通义 | `Tongyi` | `langchain-community` |
| 智谱 | `ChatZhipuAI` | `langchain-community` |
| 百度文心 | `QianfanChatEndpoint` | `langchain-community` |

### 2.2 基本使用

```python
# 01_chat_model_demo.py
import os
from dotenv import load_dotenv

# langchain_openai 不仅支持 OpenAI，也支持任何兼容 OpenAI API 的服务
from langchain_openai import ChatOpenAI

load_dotenv()

# ============================================================
# 初始化聊天模型
# ============================================================
# 关键参数：
#   model:     模型名称，如 "gpt-4o", "deepseek-chat"
#   temperature: 0-2，越高越随机（创意任务用 0.8+，精确任务用 0-0.3）
#   max_tokens:  最大输出 token 数
# ============================================================
llm = ChatOpenAI(
    model="deepseek-chat",               # 使用的模型名称
    temperature=0,                        # 0 = 最确定性的输出
    max_tokens=2048,                      # 单次最大输出
    api_key=os.getenv("OPENAI_API_KEY"),  # API 密钥
    base_url=os.getenv("OPENAI_BASE_URL"),# API 地址（国内模型转发）
)

# ============================================================
# 最基本用法：直接调用
# ============================================================
response = llm.invoke("用一句话解释什么是 AI Agent")
print(response.content)
# 输出类似：AI Agent 是一个能够自主感知环境、做出决策并执行行动的智能程序。

# ============================================================
# 消息格式：LangChain 使用消息对象，而不是纯字符串
# ============================================================
from langchain_core.messages import HumanMessage, SystemMessage, AIMessage

messages = [
    # SystemMessage: 系统指令，设定 AI 的角色和行为
    SystemMessage(content="你是一个资深 Python 工程师，擅长解释技术概念。"),
    # HumanMessage: 用户消息
    HumanMessage(content="什么是装饰器？"),
    # AIMessage: AI 的回复（可用于多轮对话）
    # AIMessage(content="之前说的..."),
]

response = llm.invoke(messages)
print(response.content)
```

### 2.3 流式输出（打字机效果）

```python
# ============================================================
# 流式输出：大模型逐 token 返回结果
# ============================================================
# 适用场景：需要实时显示生成过程，提升用户体验
# ============================================================
for chunk in llm.stream("讲一个关于 Python 程序员的笑话"):
    print(chunk.content, end="", flush=True)
# 效果：文字一个个出现，像打字机一样
```

## 3. Prompt Templates

### 3.1 基本模板

```python
from langchain_core.prompts import ChatPromptTemplate

# ============================================================
# ChatPromptTemplate：结构化提示模板
# 用 {variable} 占位符插入变量
# ============================================================
prompt = ChatPromptTemplate.from_messages([
    # 每条消息可以是 (角色, 内容) 元组
    ("system", "你是一个 {role}，请用 {language} 回答问题。"),
    ("human", "{question}"),
])

# 填充变量 → 生成消息列表
formatted = prompt.invoke({
    "role": "Python 技术专家",
    "language": "简体中文",
    "question": "什么是异步编程？",
})

print(formatted.to_messages())
# 输出：
# [
#   SystemMessage(content="你是一个 Python 技术专家，请用 简体中文 回答问题。"),
#   HumanMessage(content="什么是异步编程？"),
# ]
```

### 3.2 Few-Shot 模板（少样本学习）

```python
from langchain_core.prompts import FewShotChatMessagePromptTemplate

# ============================================================
# Few-Shot：给大模型提供示例，让它模仿回答模式
# ============================================================
examples = [
    {"input": "2+2=?", "output": "4"},
    {"input": "3*3=?", "output": "9"},
]

example_prompt = ChatPromptTemplate.from_messages([
    ("human", "{input}"),
    ("ai", "{output}"),
])

few_shot_prompt = FewShotChatMessagePromptTemplate(
    examples=examples,
    example_prompt=example_prompt,
)

final_prompt = ChatPromptTemplate.from_messages([
    ("system", "你是一个计算器，只返回数字结果。"),
    few_shot_prompt,
    ("human", "{input}"),
])

messages = final_prompt.invoke({"input": "100/4=?"})
# → AI 会模仿示例，直接回复 "25"
```

## 4. Output Parsers

```python
from langchain_core.output_parsers import StrOutputParser, CommaSeparatedListOutputParser
from langchain_core.output_parsers import JsonOutputParser, PydanticOutputParser
from pydantic import BaseModel, Field

# ============================================================
# 1. StrOutputParser：最简单的，只提取字符串内容
# ============================================================
parser = StrOutputParser()
result = parser.invoke(AIMessage(content="Hello"))  # → "Hello"

# ============================================================
# 2. JsonOutputParser：解析 JSON 输出
# ============================================================
json_parser = JsonOutputParser()
result = json_parser.parse('{"name": "Agent", "version": 1}')
# → {"name": "Agent", "version": 1}  （Python 字典）

# ============================================================
# 3. PydanticOutputParser：解析为结构化 Pydantic 对象（推荐）
# ============================================================
class Person(BaseModel):
    name: str = Field(description="姓名")
    age: int = Field(description="年龄")

pydantic_parser = PydanticOutputParser(pydantic_object=Person)
# 它会自动生成格式说明，加到 prompt 末尾
format_instructions = pydantic_parser.get_format_instructions()
print(format_instructions)
# → "You must format your output as a JSON value that adheres to..."
```

## 5. Chains

**Chain（链）** 是 LangChain 的核心抽象，把多个组件串联成管道。

### 5.1 LCEL（LangChain Expression Language）

```python
from langchain_core.output_parsers import StrOutputParser
from langchain_core.prompts import ChatPromptTemplate
from langchain_openai import ChatOpenAI

# ============================================================
# LCEL：用 | 运算符串联组件，类似 Unix 管道
#   prompt → llm → parser
# ============================================================
llm = ChatOpenAI(model="deepseek-chat", temperature=0)
prompt = ChatPromptTemplate.from_template("用 {language} 解释 {concept}")

# 一行链式定义
chain = prompt | llm | StrOutputParser()

# 调用链
result = chain.invoke({
    "language": "简体中文",
    "concept": "递归",
})
print(result)
```

### 5.2 RunnablePassthrough / RunnableParallel

```python
from langchain_core.runnables import RunnablePassthrough, RunnableParallel
from langchain_core.output_parsers import StrOutputParser

# ============================================================
# RunnablePassthrough：透传数据，或添加新字段
# ============================================================
chain = (
    # 输入 {"concept": "闭包", "language": "中文"}
    RunnablePassthrough.assign(
        # 添加 summary 字段，内容是 "请用一句话解释 {concept}"
        summary=lambda x: f"请用一句话解释 {x['concept']}"
    )
    | prompt
    | llm
    | StrOutputParser()
)

# ============================================================
# RunnableParallel：并行运行多个链，合并结果
# ============================================================
chain1 = ChatPromptTemplate.from_template("翻译成英文：{text}") | llm | StrOutputParser()
chain2 = ChatPromptTemplate.from_template("总结这段文字：{text}") | llm | StrOutputParser()

parallel_chain = RunnableParallel(
    translation=chain1,
    summary=chain2,
)

result = parallel_chain.invoke({"text": "人工智能正在改变世界。"})
# → {
#   "translation": "Artificial intelligence is changing the world.",
#   "summary": "这段文字讨论了人工智能对全球的影响..."
# }
```

### 5.3 RunnableBranch（条件分支）

```python
from langchain_core.runnables import RunnableBranch

# 根据输入选择不同的处理路径
branch = RunnableBranch(
    (lambda x: len(x["text"]) > 100, long_text_chain),   # 条件1 → 长文本链
    (lambda x: len(x["text"]) > 20,  medium_text_chain), # 条件2 → 中文本链
    short_text_chain,                                     # 默认 → 短文本链
)
```

## 6. Memory

```python
from langchain.memory import ConversationSummaryMemory
from langchain_community.chat_message_histories import ChatMessageHistory

# ============================================================
# ConversationSummaryMemory：对话记忆
# 每次对话后自动总结历史，作为后续对话的上下文
# ============================================================
memory = ConversationSummaryMemory(
    llm=ChatOpenAI(model="deepseek-chat"),  # 用于生成总结的模型
    memory_key="history",                    # 在 prompt 中使用的变量名
    return_messages=True,                    # 返回消息对象而不是字符串
)

# 保存对话
memory.save_context(
    {"input": "我的名字是张三"},
    {"output": "你好张三！"},
)
memory.save_context(
    {"input": "我住在北京"},
    {"output": "北京是个好地方！"},
)

# 加载记忆
history = memory.load_memory_variables({})
print(history["history"])
# → 张三介绍了自己，他住在北京。
```

> **注意：** LangChain 的 Memory 模块在 v0.3 之后逐步被 LangGraph 的持久化取代。推荐新项目直接使用 LangGraph 管理状态。

## 7. RAG 基础

```python
# ============================================================
# RAG（检索增强生成）核心流程：
#   文档 → 分割 → 向量化 → 存储 → 检索 → 生成
# ============================================================
from langchain_community.document_loaders import TextLoader
from langchain.text_splitter import RecursiveCharacterTextSplitter
from langchain_community.vectorstores import Chroma
from langchain_openai import OpenAIEmbeddings
from langchain_core.runnables import RunnablePassthrough

# 1️⃣ 加载文档
loader = TextLoader("docs/knowledge.txt")
documents = loader.load()

# 2️⃣ 文本分割（将长文档切成块）
text_splitter = RecursiveCharacterTextSplitter(
    chunk_size=500,       # 每块最大字符数
    chunk_overlap=50,     # 块之间重叠，防止上下文断裂
    separators=["\n\n", "\n", "。", "！", "？", "，"],  # 分割优先级
)
chunks = text_splitter.split_documents(documents)

# 3️⃣ 向量化并存储（Embedding + VectorStore）
embeddings = OpenAIEmbeddings(model="text-embedding-ada-002")
vectorstore = Chroma.from_documents(
    documents=chunks,
    embedding=embeddings,
    persist_directory="./chroma_db",  # 持久化到磁盘
)

# 4️⃣ 创建检索器
retriever = vectorstore.as_retriever(
    search_type="similarity",    # 相似度搜索（vs "mmr" 多样性搜索）
    search_kwargs={"k": 3},      # 返回最相似的 3 个文档块
)

# 5️⃣ 组装 RAG 链
rag_prompt = ChatPromptTemplate.from_template("""
根据以下上下文回答问题。如果上下文中没有相关信息，请说"我不知道"。

上下文：
{context}

问题：{question}
""")

def format_docs(docs):
    """将检索到的文档格式化为单个字符串"""
    return "\n\n".join(doc.page_content for doc in docs)

rag_chain = (
    # 并行执行：一边检索文档，一边透传问题
    RunnablePassthrough.assign(context=(lambda x: format_docs(retriever.invoke(x["question"]))))
    | rag_prompt
    | llm
    | StrOutputParser()
)

answer = rag_chain.invoke({"question": "什么是机器学习？"})
print(answer)
```

## 8. 完整示例：问答机器人

```python
# qa_bot.py
import os
from dotenv import load_dotenv
from langchain_openai import ChatOpenAI
from langchain_core.prompts import ChatPromptTemplate, MessagesPlaceholder
from langchain_core.output_parsers import StrOutputParser
from langchain_core.messages import HumanMessage, AIMessage

load_dotenv()

# ---------- 模型 ----------
llm = ChatOpenAI(
    model="deepseek-chat",
    temperature=0.3,
)

# ---------- 带历史记录的 Prompt ----------
prompt = ChatPromptTemplate.from_messages([
    ("system", "你是一个乐于助人的助手。请用中文回答问题。"),
    # MessagesPlaceholder: 运行时插入历史消息
    MessagesPlaceholder(variable_name="history"),
    ("human", "{question}"),
])

# ---------- 链 ----------
chain = prompt | llm | StrOutputParser()

# ---------- 多轮对话 ----------
history = []
questions = [
    "Python 的 GIL 是什么？",
    "它有什么影响？",
    "如何绕过它？",
]

for q in questions:
    print(f"\n🙋 {q}")

    # 调用链，传入历史
    answer = chain.invoke({
        "question": q,
        "history": history,
    })
    print(f"🤖 {answer}")

    # 保存本轮对话到历史
    history.append(HumanMessage(content=q))
    history.append(AIMessage(content=answer))

# ---------- 运行 ----------
# $ uv run python qa_bot.py
```

---

> **下一章：** [03-langchain-tools.md](./03-langchain-tools.md) — 如何给大模型编写 Tools
