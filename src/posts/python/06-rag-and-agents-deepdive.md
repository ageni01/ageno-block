# RAG、PDF处理与智能体深度详解

> 本文是前 5 章的进阶篇。深入讲解 **RAG 知识库搭建**、**PDF 文档处理**、**LangChain 核心原理**、**LangGraph 进阶用法**，并在最后整合成一个完整的智能体项目。

## 目录

1. [RAG 核心原理](#1-rag-核心原理)
2. [文档加载器 Document Loaders](#2-文档加载器-document-loaders)
3. [PDF 切割与处理](#3-pdf-切割与处理)
4. [文本分块策略 Text Splitters](#4-文本分块策略-text-splitters)
5. [向量存储 Vector Stores](#5-向量存储-vector-stores)
6. [检索器 Retrievers 进阶](#6-检索器-retrievers-进阶)
7. [LangChain 核心原理深挖](#7-langchain-核心原理深挖)
8. [LangGraph 进阶：持久化与多轮对话](#8-langgraph-进阶持久化与多轮对话)
9. [LangGraph 进阶：并行节点与子图](#9-langgraph-进阶并行节点与子图)
10. [完整项目：PDF 智能问答 Agent](#10-完整项目pdf-智能问答-agent)

---

## 1. RAG 核心原理

### 1.1 为什么需要 RAG？

**大模型的四大局限：**

| 局限 | 问题 | RAG 的解决方案 | 实际案例 |
|------|------|--------------|---------|
| **知识截止日期** | 训练数据有时间截止 | 检索实时文档 | GPT-4 不知道2024年的新闻 |
| **私有知识盲区** | 不了解企业内部资料 | 检索私有知识库 | 公司财报、内部规章制度 |
| **幻觉问题** | 编造不存在的事实 | 基于真实文档回答 | 法律/医疗等准确性要求高的领域 |
| **上下文长度限制** | 即使有128K窗口也装不下整个代码仓库 | 只检索相关片段 | 问答整本技术手册 |

**典型对比：**

```python
# ❌ 没有 RAG：大模型凭记忆回答（可能出错/过时）
user: 我们公司 2024 年第三季度的营收是多少？
llm:  我无法访问实时数据，无法回答...

# ✅ 有 RAG：先检索文档，再基于文档回答
user: 我们公司 2024 年第三季度的营收是多少？
      ↓
   检索: Q3_financial_report.pdf
      ↓
llm:  根据《2024 Q3 财务报告》第3页：
      本季度营收为 4.52 亿元，同比增长 28%。
```

**RAG 的核心价值：**
- ✅ 让 LLM 基于**真实文档**回答，而不是凭"想象"
- ✅ 可以持续更新知识库，**无需重新训练模型**
- ✅ 回答可追溯到具体来源（**可解释性**）
- ✅ 隐私：**私有数据不需要上传训练**

### 1.2 RAG 完整流程

```
离线阶段（构建索引）：
┌──────────────────────────────────────────────────────┐
│                                                      │
│  原始文档                                             │
│  (PDF/Word/TXT)                                      │
│      │                                               │
│      ▼                                               │
│  Document Loader  ──→  documents[]                   │
│  （加载器）              [{page_content, metadata}]   │
│      │                                               │
│      ▼                                               │
│  Text Splitter    ──→  chunks[]                      │
│  （文本分割器）          [{page_content, metadata}]   │
│      │                                               │
│      ▼                                               │
│  Embedding Model  ──→  vectors[]                     │
│  （向量模型）            [[0.1, -0.3, 0.8, ...]]      │
│      │                                               │
│      ▼                                               │
│  Vector Store                                        │
│  （向量数据库）  Chroma / FAISS / PGVector            │
└──────────────────────────────────────────────────────┘

在线阶段（查询检索）：
┌──────────────────────────────────────────────────────┐
│                                                      │
│  用户问题 ──→ Embedding ──→ 问题向量                  │
│                                   │                  │
│                                   ▼                  │
│                          向量数据库相似度搜索           │
│                                   │                  │
│                                   ▼                  │
│                          Top-K 相关文档块              │
│                                   │                  │
│                                   ▼                  │
│  Prompt = 系统指令 + 文档块 + 用户问题                 │
│                                   │                  │
│                                   ▼                  │
│                               LLM 生成答案             │
└──────────────────────────────────────────────────────┘
```

### 1.3 向量相似度原理

**核心概念：** 把文本转换成高维空间的点（向量），相似的文本在空间中距离近。

```python
# ============================================================
# Embedding（文本向量化）原理演示
# ============================================================
from langchain_openai import OpenAIEmbeddings
import numpy as np

embeddings = OpenAIEmbeddings(model="text-embedding-ada-002")
# text-embedding-ada-002: 1536 维向量，每个文本 → 1536 个浮点数

# ---------- 示例1：相似语义的文本 → 向量接近 ----------
v1 = embeddings.embed_query("苹果是一种水果")
v2 = embeddings.embed_query("苹果属于果实类食物")
v3 = embeddings.embed_query("Python 是编程语言")

print(f"向量维度: {len(v1)}")  # → 1536
print(f"v1 前5维: {v1[:5]}")   # → [0.0023, -0.0139, 0.0052, ...]

# 计算余弦相似度（范围 -1 到 1，越接近 1 越相似）
def cosine_similarity(a, b):
    """余弦相似度 = 两个向量夹角的余弦值"""
    return np.dot(a, b) / (np.linalg.norm(a) * np.linalg.norm(b))

sim_12 = cosine_similarity(v1, v2)  # "苹果是水果" vs "苹果属于果实"
sim_13 = cosine_similarity(v1, v3)  # "苹果是水果" vs "Python 编程"

print(f"相似文本相似度: {sim_12:.4f}")  # → 0.8923（非常相似）
print(f"不相关文本相似度: {sim_13:.4f}")  # → 0.1254（不相关）

# ---------- 示例2：语义搜索的本质 ----------
# 假设知识库有 3 个文档
docs = [
    "Python 是一种高级编程语言，由 Guido van Rossum 创建于 1991 年。",
    "苹果富含维生素 C，每天吃一个苹果有益健康。",
    "机器学习是人工智能的子领域，通过数据训练模型。",
]

# 所有文档向量化
doc_vectors = [embeddings.embed_query(doc) for doc in docs]

# 用户问题
query = "Python 是什么时候发明的？"
query_vector = embeddings.embed_query(query)

# 计算问题与每个文档的相似度
scores = [cosine_similarity(query_vector, doc_v) for doc_v in doc_vectors]

# 排序，找出最相关的文档
ranked = sorted(zip(docs, scores), key=lambda x: x[1], reverse=True)
print("\n检索结果（按相关性排序）：")
for i, (doc, score) in enumerate(ranked, 1):
    print(f"{i}. [{score:.4f}] {doc[:50]}...")
# 输出：
# 1. [0.8123] Python 是一种高级编程语言，由 Guido van Rossum...  ← 最相关
# 2. [0.3456] 机器学习是人工智能的子领域，通过数据训练模型...
# 3. [0.1012] 苹果富含维生素 C，每天吃一个苹果有益健康...     ← 不相关

# ---------- 关键理解 ----------
# ✅ 向量检索不依赖关键词匹配（关键词搜索会漏掉同义词）
# ✅ 可以理解语义："Python 何时发明"能匹配"1991年创建"
# ✅ 跨语言也能工作（多语言 Embedding 模型）
```

**向量维度的意义：**
```python
# text-embedding-ada-002:       1536 维  → 通用，性价比高
# text-embedding-3-small:       1536 维  → 更新，效果更好
# text-embedding-3-large:       3072 维  → 最强，但贵 2 倍
# m3e-base（中文开源模型）:      768 维   → 本地部署，免费

# 维度越高 → 表达能力越强，但：
#   - 存储空间更大（3072维是1536维的2倍）
#   - 检索速度更慢（计算量更大）
#   - 成本更高（API 按 token 计费）
```

**常见相似度计算方法对比：**
```python
import numpy as np

def cosine_similarity(a, b):
    """余弦相似度：最常用，不受向量长度影响"""
    return np.dot(a, b) / (np.linalg.norm(a) * np.linalg.norm(b))

def euclidean_distance(a, b):
    """欧氏距离：物理空间中的直线距离，距离越小越相似"""
    return np.linalg.norm(a - b)

def dot_product(a, b):
    """点积：简单快速，但受向量长度影响"""
    return np.dot(a, b)

# 推荐：余弦相似度（大多数向量数据库默认）
# Chroma / FAISS / Qdrant 都默认用余弦相似度
```

---

## 2. 文档加载器 Document Loaders

LangChain 提供了几十种加载器，覆盖几乎所有文档格式。

### 2.1 常用加载器一览

```python
# ============================================================
# 安装依赖
# uv add langchain-community pypdf python-docx beautifulsoup4
# ============================================================

# ---------- 1. TXT 文本文件 ----------
from langchain_community.document_loaders import TextLoader

loader = TextLoader("docs/readme.txt", encoding="utf-8")
docs = loader.load()
# docs[0].page_content → 文件内容
# docs[0].metadata    → {"source": "docs/readme.txt"}


# ---------- 2. PDF 文件 ----------
from langchain_community.document_loaders import PyPDFLoader

loader = PyPDFLoader("docs/manual.pdf")
pages = loader.load()
# 每一页是一个 Document
# pages[0].page_content → 第1页内容
# pages[0].metadata     → {"source": "manual.pdf", "page": 0}


# ---------- 3. Word 文档 ----------
from langchain_community.document_loaders import Docx2txtLoader

loader = Docx2txtLoader("docs/report.docx")
docs = loader.load()


# ---------- 4. CSV 文件 ----------
from langchain_community.document_loaders import CSVLoader

loader = CSVLoader(
    "data/products.csv",
    csv_args={"delimiter": ","},
    source_column="id",  # 用哪列作为 metadata source
)
rows = loader.load()
# 每一行是一个 Document


# ---------- 5. 网页 ----------
from langchain_community.document_loaders import WebBaseLoader

loader = WebBaseLoader("https://python.org")
docs = loader.load()


# ---------- 6. 目录批量加载 ----------
from langchain_community.document_loaders import DirectoryLoader

loader = DirectoryLoader(
    "docs/",
    glob="**/*.pdf",        # 匹配所有 PDF
    loader_cls=PyPDFLoader, # 用 PyPDF 加载
    show_progress=True,     # 显示进度条
)
all_docs = loader.load()
print(f"共加载 {len(all_docs)} 个文档块")


# ---------- 7. Markdown 文件 ----------
from langchain_community.document_loaders import UnstructuredMarkdownLoader

loader = UnstructuredMarkdownLoader("README.md")
docs = loader.load()
```

### 2.2 Document 对象结构

```python
from langchain_core.documents import Document

# Document 是 LangChain 的核心数据结构
doc = Document(
    page_content="这是文档的内容文本",  # 核心：文本内容
    metadata={                           # 元数据：来源、页码等信息
        "source": "manual.pdf",
        "page": 5,
        "author": "张三",
        "date": "2024-01-01",
    }
)

print(doc.page_content)  # → "这是文档的内容文本"
print(doc.metadata)      # → {"source": "manual.pdf", "page": 5, ...}
```

---

## 3. PDF 切割与处理

PDF 是最常见的文档格式，但处理起来有不少坑。

### 3.1 基础 PDF 加载

```python
# ============================================================
# 安装：uv add pypdf pymupdf pdfplumber
# ============================================================
from langchain_community.document_loaders import PyPDFLoader

# ---------- 基础加载 ----------
loader = PyPDFLoader("report.pdf")
pages = loader.load()

print(f"共 {len(pages)} 页")
for i, page in enumerate(pages[:3]):
    print(f"\n--- 第 {i+1} 页 ---")
    print(page.page_content[:200])
    print(f"元数据: {page.metadata}")
```

### 3.2 PyMuPDF（更强大）

```python
# PyMuPDF 速度快、支持更多格式，推荐生产环境使用
from langchain_community.document_loaders import PyMuPDFLoader

loader = PyMuPDFLoader("report.pdf")
pages = loader.load()

# PyMuPDF 的 metadata 更丰富
# {
#   "source": "report.pdf",
#   "file_path": "/absolute/path/report.pdf",
#   "page": 0,
#   "total_pages": 42,
#   "format": "PDF 1.7",
#   "title": "年度报告",
#   "author": "财务部",
#   "subject": "",
#   "keywords": "",
#   "creator": "Microsoft Word",
#   "producer": "...",
#   "creationDate": "D:20240101000000",
#   "modDate": "D:20240115000000",
# }
```

### 3.3 pdfplumber（处理表格）

```python
# pdfplumber 擅长提取 PDF 中的表格
import pdfplumber
from langchain_core.documents import Document

def load_pdf_with_tables(pdf_path: str) -> list[Document]:
    """
    加载 PDF，并特别处理表格内容。
    表格会转换为 Markdown 格式，保留结构信息。
    """
    documents = []

    with pdfplumber.open(pdf_path) as pdf:
        for page_num, page in enumerate(pdf.pages):
            content_parts = []

            # 提取文本
            text = page.extract_text() or ""
            if text.strip():
                content_parts.append(text)

            # 提取表格 → 转为 Markdown
            tables = page.extract_tables()
            for table_idx, table in enumerate(tables):
                if not table:
                    continue

                # 转换为 Markdown 表格
                md_table = []
                for row_idx, row in enumerate(table):
                    # 清理单元格内容（去除 None 和换行）
                    cells = [str(cell or "").replace("\n", " ") for cell in row]
                    md_table.append("| " + " | ".join(cells) + " |")

                    # 在表头后添加分隔线
                    if row_idx == 0:
                        separator = "| " + " | ".join(["---"] * len(cells)) + " |"
                        md_table.append(separator)

                content_parts.append(f"\n[表格 {table_idx+1}]\n" + "\n".join(md_table))

            if content_parts:
                documents.append(Document(
                    page_content="\n\n".join(content_parts),
                    metadata={
                        "source": pdf_path,
                        "page": page_num,
                        "total_pages": len(pdf.pages),
                    }
                ))

    return documents


# 使用示例
docs = load_pdf_with_tables("financial_report.pdf")
print(f"共加载 {len(docs)} 页，含表格内容")
```

### 3.4 批量处理多个 PDF

```python
import os
from pathlib import Path
from typing import List
from langchain_core.documents import Document
from langchain_community.document_loaders import PyMuPDFLoader

def load_pdf_directory(
    dir_path: str,
    recursive: bool = True,
) -> List[Document]:
    """
    批量加载目录下的所有 PDF 文件。

    参数:
        dir_path:  PDF 目录路径
        recursive: 是否递归子目录
    """
    pdf_dir = Path(dir_path)
    pattern = "**/*.pdf" if recursive else "*.pdf"
    pdf_files = list(pdf_dir.glob(pattern))

    print(f"发现 {len(pdf_files)} 个 PDF 文件")

    all_docs = []
    for i, pdf_path in enumerate(pdf_files, 1):
        try:
            print(f"[{i}/{len(pdf_files)}] 加载: {pdf_path.name}")
            loader = PyMuPDFLoader(str(pdf_path))
            docs = loader.load()
            all_docs.extend(docs)
        except Exception as e:
            print(f"  ⚠️ 加载失败: {e}")
            continue

    print(f"共加载 {len(all_docs)} 页")
    return all_docs


# 使用
docs = load_pdf_directory("./knowledge_base/")
```

### 3.5 PDF 预处理与清洗

PDF 提取的文本往往有噪声，需要清洗。

```python
import re
from langchain_core.documents import Document

def clean_pdf_text(text: str) -> str:
    """
    清洗 PDF 提取的文本。
    处理常见问题：乱码、多余空白、页眉页脚、换行问题。
    """
    # 1. 去除非打印字符（保留中文、英文、数字、标点）
    text = re.sub(r"[^\u4e00-\u9fff\u0020-\u007e\uff00-\uffef\n]", "", text)

    # 2. 合并因 PDF 换行导致的断词（中文不需要，英文需要）
    # 英文：行末有连字符的断词
    text = re.sub(r"(\w)-\n(\w)", r"\1\2", text)

    # 3. 合并中文段落中的多余换行（同一段落的连续行）
    # 策略：如果上一行不以句号/问号/叹号结尾，则合并
    lines = text.split("\n")
    merged_lines = []
    for i, line in enumerate(lines):
        line = line.strip()
        if not line:
            merged_lines.append("")
            continue
        # 如果不是段落结尾（没有结束标点）且下一行不为空，则合并
        if (i < len(lines) - 1
                and not re.search(r"[。！？.!?]$", line)
                and lines[i + 1].strip()):
            merged_lines.append(line)
        else:
            merged_lines.append(line + "\n")

    text = "".join(merged_lines)

    # 4. 压缩多余空行（超过2个空行 → 1个空行）
    text = re.sub(r"\n{3,}", "\n\n", text)

    # 5. 去除每行首尾空格
    text = "\n".join(line.strip() for line in text.split("\n"))

    return text.strip()


def clean_documents(docs: list[Document]) -> list[Document]:
    """批量清洗文档列表"""
    cleaned = []
    for doc in docs:
        cleaned_content = clean_pdf_text(doc.page_content)
        if len(cleaned_content.strip()) < 10:
            # 跳过内容太少的页（可能是扫描图片页）
            continue
        cleaned.append(Document(
            page_content=cleaned_content,
            metadata=doc.metadata,
        ))
    return cleaned
```

---

## 4. 文本分块策略 Text Splitters

分块是 RAG 中最关键、最容易被忽视的环节。块的大小和策略直接影响检索质量。

### 4.1 为什么要分块？

```
原始文档：100,000 字的技术手册
                  │
          直接送入 LLM？
                  │
         ╳ 超出上下文窗口
         ╳ 成本极高
         ╳ 回答质量下降（注意力分散）
                  │
          分成若干块：500字/块
                  │
         ✅ 按需检索 Top-3 相关块
         ✅ 只送入 1500 字给 LLM
         ✅ 成本低、精度高
```

### 4.2 RecursiveCharacterTextSplitter（推荐）

```python
from langchain.text_splitter import RecursiveCharacterTextSplitter

# ============================================================
# RecursiveCharacterTextSplitter 是最常用的分割器
# 核心思想：按优先级尝试不同的分隔符，直到块足够小
# 默认分隔符顺序：["\n\n", "\n", " ", ""]
# ============================================================
splitter = RecursiveCharacterTextSplitter(
    chunk_size=500,        # 每块最大字符数（不是 token！）
    chunk_overlap=50,      # 相邻块重叠字符数（防止语义断裂）
    length_function=len,   # 用 len() 计算长度（也可用 tiktoken）
    # 中文文档推荐分隔符
    separators=[
        "\n\n",   # 段落分隔（最优先）
        "\n",     # 换行
        "。",     # 中文句号
        "！",     # 中文感叹号
        "？",     # 中文问号
        "；",     # 中文分号
        "，",     # 中文逗号
        " ",      # 空格
        "",       # 字符级（最后手段）
    ],
)

# 分割文档列表
chunks = splitter.split_documents(documents)

# 或分割纯文本
text_chunks = splitter.split_text(long_text)

# 查看分割结果
print(f"原始文档: {len(documents)} 个")
print(f"分割后: {len(chunks)} 个块")
print(f"平均块大小: {sum(len(c.page_content) for c in chunks) / len(chunks):.0f} 字符")

# 块的 metadata 会自动继承
print(chunks[0].metadata)
# → {"source": "manual.pdf", "page": 0}
```

### 4.3 基于 Token 的分割（精确控制）

```python
# ============================================================
# LLM 的上下文窗口按 token 计算，不是字符
# 中文1字 ≈ 1-2 token；英文1词 ≈ 1-1.5 token
# 精确控制 token 数用 tiktoken
# ============================================================
# uv add tiktoken
from langchain.text_splitter import RecursiveCharacterTextSplitter
import tiktoken

def get_token_splitter(
    model_name: str = "gpt-3.5-turbo",
    chunk_size: int = 400,      # 单位：token
    chunk_overlap: int = 40,
):
    """创建基于 token 计数的分割器"""
    encoding = tiktoken.encoding_for_model(model_name)

    def token_length(text: str) -> int:
        return len(encoding.encode(text))

    return RecursiveCharacterTextSplitter(
        chunk_size=chunk_size,
        chunk_overlap=chunk_overlap,
        length_function=token_length,  # 用 token 计算长度
        separators=["\n\n", "\n", "。", "！", "？", " ", ""],
    )

splitter = get_token_splitter(chunk_size=400)
chunks = splitter.split_documents(docs)
```

### 4.4 SemanticChunker（语义分块，进阶）

```python
# ============================================================
# 语义分块：基于语义相似度来决定分割点
# 两句话意思相差很大时 → 分割
# 比固定大小分块效果更好，但速度更慢（需要计算 Embedding）
# uv add langchain-experimental
# ============================================================
from langchain_experimental.text_splitter import SemanticChunker
from langchain_openai import OpenAIEmbeddings

embeddings = OpenAIEmbeddings()

semantic_splitter = SemanticChunker(
    embeddings,
    breakpoint_threshold_type="percentile",  # 分割阈值类型
    breakpoint_threshold_amount=90,          # 第90百分位差异处分割
)

chunks = semantic_splitter.split_documents(documents)
# 效果：同一话题的内容会被分在一块，不同话题自动切分
```

### 4.5 分块策略对比

| 策略 | 方法 | 优点 | 缺点 | 适用场景 |
|------|------|------|------|----------|
| 固定大小 | `CharacterTextSplitter` | 简单快速 | 可能截断语义 | 快速原型 |
| 递归字符 | `RecursiveCharacterTextSplitter` | 尊重文档结构 | 块大小不均匀 | **大多数场景** |
| Token 计数 | `RecursiveCharacterTextSplitter` + tiktoken | 精确控制 token | 需要额外依赖 | 严格 token 预算 |
| 语义分块 | `SemanticChunker` | 语义连贯 | 慢、依赖 Embedding | 高质量 RAG |
| Markdown | `MarkdownHeaderTextSplitter` | 保留标题结构 | 仅适用 Markdown | 技术文档 |

### 4.6 MarkdownHeaderTextSplitter

```python
# 针对 Markdown 文档，按标题级别分割，保留层级信息
from langchain.text_splitter import MarkdownHeaderTextSplitter

headers_to_split_on = [
    ("#", "H1"),
    ("##", "H2"),
    ("###", "H3"),
]

splitter = MarkdownHeaderTextSplitter(headers_to_split_on=headers_to_split_on)
chunks = splitter.split_text(markdown_text)

# 每个块的 metadata 会包含所属标题
# chunks[0].metadata → {"H1": "安装指南", "H2": "环境要求"}
```

---

## 5. 向量存储 Vector Stores

### 5.1 主流向量数据库对比

| 数据库 | 特点 | 适用场景 |
|--------|------|----------|
| **Chroma** | 轻量、本地、易用 | 开发/原型 |
| **FAISS** | Meta 出品，速度极快 | 纯内存、大规模 |
| **PGVector** | PostgreSQL 扩展 | 生产、已有 PG |
| **Milvus** | 分布式、云原生 | 大规模生产 |
| **Qdrant** | 高性能、Rust 实现 | 生产推荐 |
| **Pinecone** | 全托管 SaaS | 快速上线 |

### 5.2 Chroma（开发首选）

```python
# uv add chromadb langchain-chroma
from langchain_chroma import Chroma
from langchain_openai import OpenAIEmbeddings
from langchain_core.documents import Document

embeddings = OpenAIEmbeddings(model="text-embedding-ada-002")

# ============================================================
# 创建向量存储
# ============================================================

# 方式1：从文档列表直接创建（一次性）
vectorstore = Chroma.from_documents(
    documents=chunks,           # 文档块列表
    embedding=embeddings,       # 向量模型
    persist_directory="./chroma_db",  # 持久化目录
    collection_name="my_docs",  # 集合名称
)

# 方式2：加载已有的向量库
vectorstore = Chroma(
    persist_directory="./chroma_db",
    embedding_function=embeddings,
    collection_name="my_docs",
)

# ============================================================
# 增删改查
# ============================================================

# 新增文档
new_docs = [Document(page_content="新增内容", metadata={"source": "new.txt"})]
vectorstore.add_documents(new_docs)

# 相似度搜索（返回 Document 列表）
results = vectorstore.similarity_search("什么是 RAG？", k=3)
for doc in results:
    print(f"相似度文档: {doc.page_content[:100]}")
    print(f"来源: {doc.metadata}")

# 带分数的相似度搜索（分数越低越相似）
results_with_score = vectorstore.similarity_search_with_score("RAG 原理", k=3)
for doc, score in results_with_score:
    print(f"相似度分数: {score:.4f}")
    print(f"内容: {doc.page_content[:100]}")

# 删除文档（需要知道 ID）
vectorstore.delete(ids=["doc_id_1", "doc_id_2"])
```

### 5.3 FAISS（高性能内存存储）

```python
# uv add faiss-cpu
from langchain_community.vectorstores import FAISS
from langchain_openai import OpenAIEmbeddings

embeddings = OpenAIEmbeddings()

# 从文档创建
vectorstore = FAISS.from_documents(chunks, embeddings)

# 保存到磁盘
vectorstore.save_local("./faiss_index")

# 从磁盘加载
vectorstore = FAISS.load_local(
    "./faiss_index",
    embeddings,
    allow_dangerous_deserialization=True,  # 需要显式允许
)

# 合并两个 FAISS 索引
vectorstore2 = FAISS.from_documents(more_chunks, embeddings)
vectorstore.merge_from(vectorstore2)
```

### 5.4 向量存储的元数据过滤

```python
# 很多场景下需要先过滤再检索（例如：只搜索某个用户的文档）
from langchain_chroma import Chroma

vectorstore = Chroma(
    persist_directory="./chroma_db",
    embedding_function=embeddings,
)

# 带元数据过滤的搜索
results = vectorstore.similarity_search(
    query="项目预算是多少？",
    k=3,
    filter={"source": "financial_report.pdf"},  # 只搜索这个文件
)

# 多条件过滤
results = vectorstore.similarity_search(
    query="安全漏洞",
    k=5,
    filter={
        "$and": [
            {"category": {"$eq": "security"}},
            {"year": {"$gte": 2023}},
        ]
    }
)
```

---

## 6. 检索器 Retrievers 进阶

检索器是 RAG 的核心，好的检索器能大幅提升答案质量。

### 6.1 基础检索器

```python
# 最基础的相似度检索
retriever = vectorstore.as_retriever(
    search_type="similarity",     # 相似度搜索
    search_kwargs={"k": 4},       # 返回 4 个文档
)

# MMR（最大边际相关）：在相似度和多样性之间取平衡
# 避免检索到内容重复的文档
retriever = vectorstore.as_retriever(
    search_type="mmr",
    search_kwargs={
        "k": 4,                   # 最终返回 4 个
        "fetch_k": 20,            # 先检索 20 个候选
        "lambda_mult": 0.5,       # 0=最大多样性，1=最大相似度
    },
)

# 带分数阈值（只返回高于阈值的文档）
retriever = vectorstore.as_retriever(
    search_type="similarity_score_threshold",
    search_kwargs={"score_threshold": 0.7, "k": 5},
)

# 使用检索器
docs = retriever.invoke("什么是向量数据库？")
```

### 6.2 MultiQueryRetriever（多问法检索）

```python
# ============================================================
# 核心思想：
# 用 LLM 将一个问题改写成多个角度的问题，
# 分别检索，再合并去重结果。
# 解决：用户表述方式与文档表述不一致的问题。
# ============================================================
from langchain.retrievers import MultiQueryRetriever
from langchain_openai import ChatOpenAI

llm = ChatOpenAI(model="deepseek-chat", temperature=0)

retriever = MultiQueryRetriever.from_llm(
    retriever=vectorstore.as_retriever(search_kwargs={"k": 3}),
    llm=llm,
)

# 原始问题："LangGraph 怎么用？"
# LLM 会生成多个变体：
#   1. "如何使用 LangGraph 框架？"
#   2. "LangGraph 的基本用法是什么？"
#   3. "LangGraph 入门教程"
# 三个问题分别检索，结果去重合并 → 更全面

docs = retriever.invoke("LangGraph 怎么用？")
```

### 6.3 ContextualCompressionRetriever（上下文压缩）

```python
# ============================================================
# 问题：检索到的文档块往往包含大量无关内容
# 解决：检索后用 LLM 压缩，只保留和问题相关的部分
# ============================================================
from langchain.retrievers import ContextualCompressionRetriever
from langchain.retrievers.document_compressors import LLMChainExtractor

# 基础检索器
base_retriever = vectorstore.as_retriever(search_kwargs={"k": 4})

# 压缩器：用 LLM 提取相关段落
compressor = LLMChainExtractor.from_llm(llm)

# 组合
compression_retriever = ContextualCompressionRetriever(
    base_compressor=compressor,
    base_retriever=base_retriever,
)

# 使用：检索后自动压缩，只返回相关内容
compressed_docs = compression_retriever.invoke("什么是 LCEL？")
```

### 6.4 EnsembleRetriever（混合检索）

```python
# ============================================================
# BM25（关键词检索）+ 向量检索 混合使用
# BM25 对精确词匹配效果好，向量检索对语义相似效果好
# 两者互补，提高召回率
# uv add rank_bm25
# ============================================================
from langchain.retrievers import EnsembleRetriever
from langchain_community.retrievers import BM25Retriever

# BM25 关键词检索（需要原始文档列表）
bm25_retriever = BM25Retriever.from_documents(chunks)
bm25_retriever.k = 3

# 向量检索
vector_retriever = vectorstore.as_retriever(search_kwargs={"k": 3})

# 混合：BM25 占 40% 权重，向量检索占 60% 权重
ensemble_retriever = EnsembleRetriever(
    retrievers=[bm25_retriever, vector_retriever],
    weights=[0.4, 0.6],
)

docs = ensemble_retriever.invoke("RAG 检索增强生成的原理")
```

### 6.5 自查询检索器（Self-Querying）

```python
# ============================================================
# 让 LLM 把自然语言问题转换为结构化查询（向量搜索 + 元数据过滤）
# 用户说"找2023年之后的关于安全的文档" →
# LLM 生成: query="安全", filter={year: {$gte: 2023}}
# ============================================================
from langchain.retrievers.self_query.base import SelfQueryRetriever
from langchain.chains.query_constructor.base import AttributeInfo
from langchain_chroma import Chroma

# 描述 metadata 字段，让 LLM 知道可以过滤什么
metadata_field_info = [
    AttributeInfo(
        name="source",
        description="文档的文件名",
        type="string",
    ),
    AttributeInfo(
        name="page",
        description="文档的页码",
        type="integer",
    ),
    AttributeInfo(
        name="year",
        description="文档发布年份",
        type="integer",
    ),
    AttributeInfo(
        name="category",
        description="文档类别，如 'security', 'finance', 'hr'",
        type="string",
    ),
]

self_query_retriever = SelfQueryRetriever.from_llm(
    llm=llm,
    vectorstore=vectorstore,
    document_contents="公司内部文档",  # 文档内容描述
    metadata_field_info=metadata_field_info,
)

# 自然语言查询，自动生成过滤条件
docs = self_query_retriever.invoke("找一下 2023 年之后关于财务的文档")
```

---

## 7. LangChain 核心原理深挖

### 7.1 Runnable 协议：一切皆 Runnable

LangChain v0.1 之后，所有组件都实现了 **Runnable 接口**，这是整个框架的基石。

```python
# ============================================================
# Runnable 协议定义了 4 个核心方法：
# ============================================================
from langchain_core.runnables import Runnable

class MyComponent(Runnable):
    def invoke(self, input, config=None):
        """同步调用，返回单个结果"""
        ...

    def stream(self, input, config=None):
        """流式调用，返回生成器"""
        ...

    async def ainvoke(self, input, config=None):
        """异步调用"""
        ...

    async def astream(self, input, config=None):
        """异步流式调用"""
        ...

# ============================================================
# 所有 LangChain 组件都是 Runnable：
# ============================================================
from langchain_openai import ChatOpenAI
from langchain_core.prompts import ChatPromptTemplate
from langchain_core.output_parsers import StrOutputParser

llm = ChatOpenAI(model="deepseek-chat")       # Runnable
prompt = ChatPromptTemplate.from_template("...") # Runnable
parser = StrOutputParser()                     # Runnable

# 用 | 串联（这是 __or__ 运算符重载）
chain = prompt | llm | parser                  # 也是 Runnable！

# 4种调用方式完全对称
result = chain.invoke({"topic": "Python"})
for chunk in chain.stream({"topic": "Python"}):
    print(chunk, end="")
result = await chain.ainvoke({"topic": "Python"})
```

### 7.2 LCEL 深度解析

```python
# ============================================================
# LCEL（LangChain Expression Language）
# | 运算符本质上是创建了 RunnableSequence
# ============================================================
from langchain_core.runnables import RunnableSequence, RunnableParallel, RunnableLambda

# 以下两种写法完全等价
chain1 = prompt | llm | parser
chain2 = RunnableSequence(first=prompt, middle=[llm], last=parser)

# ---------- RunnableLambda：把任意函数变成 Runnable ----------
def add_context(query: str) -> dict:
    return {"question": query, "language": "中文"}

chain = (
    RunnableLambda(add_context)
    | prompt
    | llm
    | parser
)

result = chain.invoke("什么是协程？")

# ---------- RunnablePassthrough：透传或添加字段 ----------
from langchain_core.runnables import RunnablePassthrough

chain = (
    # 接收 {"question": "..."} → 添加 docs 字段
    RunnablePassthrough.assign(
        docs=lambda x: retriever.invoke(x["question"]),
        timestamp=lambda x: "2024-01-01",
    )
    | prompt
    | llm
    | parser
)

# ---------- RunnableParallel：并行执行，合并结果 ----------
parallel = RunnableParallel({
    "english": ChatPromptTemplate.from_template("Translate to English: {text}") | llm | parser,
    "summary": ChatPromptTemplate.from_template("Summarize: {text}") | llm | parser,
    "original": RunnablePassthrough(),  # 直接透传原始输入
})

result = parallel.invoke({"text": "人工智能正在改变世界"})
# → {
#   "english": "Artificial intelligence is changing the world",
#   "summary": "这段话描述了AI的影响力",
#   "original": {"text": "人工智能正在改变世界"}
# }
```

### 7.3 完整 RAG Chain 的最优写法

```python
from langchain_core.prompts import ChatPromptTemplate
from langchain_core.output_parsers import StrOutputParser
from langchain_core.runnables import RunnablePassthrough
from langchain_openai import ChatOpenAI

llm = ChatOpenAI(model="deepseek-chat", temperature=0)

RAG_PROMPT = ChatPromptTemplate.from_template("""
你是一个知识问答助手。请严格根据以下参考资料回答问题。
如果参考资料中没有相关信息，请说"根据现有资料，我无法回答这个问题"，不要编造内容。

参考资料：
{context}

问题：{question}

回答：
""")

def format_docs(docs):
    """将检索到的文档列表格式化为字符串"""
    formatted = []
    for i, doc in enumerate(docs, 1):
        source = doc.metadata.get("source", "未知来源")
        page = doc.metadata.get("page", "")
        page_info = f"（第{page+1}页）" if page != "" else ""
        formatted.append(f"[资料{i}] 来源：{source}{page_info}\n{doc.page_content}")
    return "\n\n".join(formatted)

# 构建 RAG 链
rag_chain = (
    RunnablePassthrough.assign(
        context=lambda x: format_docs(retriever.invoke(x["question"]))
    )
    | RAG_PROMPT
    | llm
    | StrOutputParser()
)

# 调用
answer = rag_chain.invoke({"question": "公司的年假政策是什么？"})
print(answer)

# 带来源的 RAG 链
rag_chain_with_source = (
    RunnablePassthrough.assign(
        docs=lambda x: retriever.invoke(x["question"])
    )
    | RunnablePassthrough.assign(
        context=lambda x: format_docs(x["docs"])
    )
    | {
        "answer": RAG_PROMPT | llm | StrOutputParser(),
        "sources": lambda x: [d.metadata for d in x["docs"]],
    }
)

result = rag_chain_with_source.invoke({"question": "报销流程是什么？"})
print(result["answer"])
print("来源：", result["sources"])
```

### 7.4 Callbacks：观察和监控

```python
# ============================================================
# Callbacks 让你能观察 LangChain 内部的每一步
# 常用于：日志记录、性能监控、流式输出、成本统计
# ============================================================
from langchain_core.callbacks import BaseCallbackHandler
from langchain_core.outputs import LLMResult

class MyCallbackHandler(BaseCallbackHandler):
    """自定义回调：记录每次 LLM 调用的信息"""

    def on_llm_start(self, serialized, prompts, **kwargs):
        """LLM 开始生成时触发"""
        print(f"🚀 LLM 开始，输入长度：{sum(len(p) for p in prompts)} 字符")

    def on_llm_end(self, response: LLMResult, **kwargs):
        """LLM 完成时触发"""
        total_tokens = response.llm_output.get("token_usage", {}).get("total_tokens", 0)
        print(f"✅ LLM 完成，消耗 {total_tokens} tokens")

    def on_llm_error(self, error, **kwargs):
        """LLM 出错时触发"""
        print(f"❌ LLM 错误：{error}")

    def on_chain_start(self, serialized, inputs, **kwargs):
        print(f"⛓  Chain 开始：{serialized.get('name', '未知')}")

    def on_tool_start(self, serialized, input_str, **kwargs):
        print(f"🔧 Tool 开始：{serialized.get('name', '未知')}，输入：{input_str[:50]}")

    def on_tool_end(self, output, **kwargs):
        print(f"📦 Tool 结果：{output[:100]}")


# 使用方式1：全局配置
handler = MyCallbackHandler()
chain = prompt | llm | parser
result = chain.invoke({"topic": "AI"}, config={"callbacks": [handler]})

# 使用方式2：在 LLM 初始化时配置
llm = ChatOpenAI(
    model="deepseek-chat",
    callbacks=[MyCallbackHandler()],
)
```

### 7.5 Output Parsers 全解

```python
from pydantic import BaseModel, Field
from typing import List
from langchain_core.output_parsers import (
    StrOutputParser,
    JsonOutputParser,
    PydanticOutputParser,
)

# ============================================================
# 1. PydanticOutputParser：最强大的解析器
#    自动生成格式说明 + 自动解析 + 类型验证
# ============================================================
class ArticleAnalysis(BaseModel):
    title: str = Field(description="文章标题")
    summary: str = Field(description="100字以内的摘要")
    key_points: List[str] = Field(description="3-5个关键点")
    sentiment: str = Field(description="情感倾向：positive/negative/neutral")
    category: str = Field(description="文章分类")

parser = PydanticOutputParser(pydantic_object=ArticleAnalysis)

prompt = ChatPromptTemplate.from_messages([
    ("system", "你是一个文章分析专家。请分析以下文章。\n{format_instructions}"),
    ("human", "{article}"),
])

chain = prompt | llm | parser

result = chain.invoke({
    "format_instructions": parser.get_format_instructions(),
    "article": "文章内容...",
})

# result 是一个 ArticleAnalysis 对象，有类型保证
print(result.title)        # str
print(result.key_points)   # List[str]
print(result.sentiment)    # str


# ============================================================
# 2. JsonOutputParser：流式 JSON 解析（支持流式输出）
# ============================================================
from langchain_core.output_parsers import JsonOutputParser

json_chain = prompt | llm | JsonOutputParser()
# 流式输出时，会逐步返回部分 JSON（当字段完整时）
for partial in json_chain.stream({"topic": "Python"}):
    print(partial)  # 逐步显示解析的 JSON


# ============================================================
# 3. with_structured_output：最新推荐方式
# ============================================================
# 直接让 LLM 返回结构化数据（利用 Function Calling）
structured_llm = llm.with_structured_output(ArticleAnalysis)
result = structured_llm.invoke("分析这篇文章：...")
# result 直接是 ArticleAnalysis 对象，不需要额外 Parser
```

---

## 8. LangGraph 进阶：持久化与多轮对话

### 8.1 Checkpointer 持久化原理

```
没有 Checkpointer：
  每次 invoke() 都是全新的对话，没有记忆

有 Checkpointer：
  每执行一步，都保存一次状态快照
  下次调用相同 thread_id，从上次断点继续

用途：
  ✅ 多轮对话（记住上下文）
  ✅ 长任务断点续传
  ✅ 人机协同（中断 + 恢复）
  ✅ 对话历史查询和回放
```

### 8.2 MemorySaver（内存持久化）

```python
from langgraph.graph import StateGraph, END
from langgraph.graph.message import add_messages
from langgraph.checkpoint.memory import MemorySaver
from langchain_openai import ChatOpenAI
from langchain_core.messages import HumanMessage, SystemMessage
from typing import TypedDict, Annotated

llm = ChatOpenAI(model="deepseek-chat", temperature=0.3)

class ChatState(TypedDict):
    messages: Annotated[list, add_messages]

def call_model(state: ChatState) -> ChatState:
    system = SystemMessage(content="你是一个友好的 AI 助手，请用中文回答。")
    response = llm.invoke([system] + state["messages"])
    return {"messages": [response]}

# 构建图
workflow = StateGraph(ChatState)
workflow.add_node("chat", call_model)
workflow.set_entry_point("chat")
workflow.add_edge("chat", END)

# ============================================================
# 关键：编译时传入 checkpointer
# MemorySaver 存在内存中（重启后丢失）
# ============================================================
checkpointer = MemorySaver()
app = workflow.compile(checkpointer=checkpointer)

# ============================================================
# 关键：使用 thread_id 区分不同对话
# 同一 thread_id = 同一对话（有记忆）
# 不同 thread_id = 不同对话（互相隔离）
# ============================================================
config_user1 = {"configurable": {"thread_id": "user_alice"}}
config_user2 = {"configurable": {"thread_id": "user_bob"}}

# 用户 Alice 的第1轮
result = app.invoke(
    {"messages": [HumanMessage(content="我叫 Alice，我是一名设计师。")]},
    config=config_user1,
)
print(result["messages"][-1].content)

# 用户 Alice 的第2轮（有记忆）
result = app.invoke(
    {"messages": [HumanMessage(content="你还记得我是做什么的吗？")]},
    config=config_user1,
)
print(result["messages"][-1].content)
# → "你是 Alice，你是一名设计师。"  ✅ 有记忆！

# 用户 Bob（不同 thread_id，没有 Alice 的记忆）
result = app.invoke(
    {"messages": [HumanMessage(content="你还记得我是做什么的吗？")]},
    config=config_user2,
)
print(result["messages"][-1].content)
# → "您好！我不太了解您的职业..." ✅ 互相隔离！
```

### 8.3 SQLite 持久化（重启后保留）

```python
# ============================================================
# 生产环境用 SQLite 或 PostgreSQL 持久化
# 重启应用后对话历史仍然存在
# uv add langgraph-checkpoint-sqlite
# ============================================================
from langgraph.checkpoint.sqlite import SqliteSaver

# SQLite 持久化（数据保存到 .db 文件）
with SqliteSaver.from_conn_string("./conversations.db") as checkpointer:
    app = workflow.compile(checkpointer=checkpointer)

    config = {"configurable": {"thread_id": "user_001"}}

    # 第一次对话
    app.invoke(
        {"messages": [HumanMessage(content="我叫张三，今年 30 岁")]},
        config=config,
    )

    # 重启程序后...再次创建 app 并使用相同的 thread_id
    # 仍然可以记住之前的对话！

    result = app.invoke(
        {"messages": [HumanMessage(content="我多大了？")]},
        config=config,
    )
    print(result["messages"][-1].content)
    # → "你今年 30 岁。"
```

### 8.4 查看和管理状态历史

```python
# ============================================================
# LangGraph 提供完整的状态历史查询 API
# ============================================================

config = {"configurable": {"thread_id": "user_alice"}}

# 获取当前状态
current_state = app.get_state(config)
print("当前消息数:", len(current_state.values["messages"]))
print("下一步:", current_state.next)

# 获取完整历史（每一步的状态快照）
history = list(app.get_state_history(config))
print(f"共 {len(history)} 个历史状态")

# 回放某个历史状态
for snapshot in history:
    print(f"时间戳: {snapshot.created_at}")
    print(f"消息数: {len(snapshot.values.get('messages', []))}")
    print("---")

# 回滚到某个历史点（时间旅行！）
old_state = history[-2]  # 倒数第二个状态
# 用旧状态的 config（包含 checkpoint_id）重新执行
result = app.invoke(
    {"messages": [HumanMessage(content="从这里继续")]},
    config=old_state.config,
)
```

### 8.5 Human-in-the-Loop（人机协同）进阶

```python
# ============================================================
# interrupt_before/interrupt_after：
# 在特定节点前后暂停，等待人类介入
# ============================================================
from langgraph.checkpoint.memory import MemorySaver
from langgraph.graph import StateGraph, END
from langchain_core.messages import HumanMessage, ToolMessage
from typing import TypedDict, Annotated
from langgraph.graph.message import add_messages

checkpointer = MemorySaver()

# 编译时设置中断点
app = workflow.compile(
    checkpointer=checkpointer,
    interrupt_before=["tools"],  # 在执行工具前中断
)

config = {"configurable": {"thread_id": "approval_flow_1"}}

# 第一步：运行到中断点停止
result = app.invoke(
    {"messages": [HumanMessage(content="帮我删除所有临时文件")]},
    config=config,
)

# 此时 app 已在 tools 节点前暂停
current_state = app.get_state(config)
print("当前状态（等待确认）:", current_state.next)
# → ('tools',)  表示下一步是 tools，但还没执行

# 查看 LLM 想要执行什么操作
last_msg = current_state.values["messages"][-1]
if last_msg.tool_calls:
    for tc in last_msg.tool_calls:
        print(f"⚠️ 将要执行: {tc['name']}({tc['args']})")

# 人类审核后决定：继续 or 拒绝
user_decision = input("是否允许？(y/n): ")

if user_decision.lower() == "y":
    # 继续执行（传 None 表示从断点继续）
    final_result = app.invoke(None, config=config)
    print("执行完成：", final_result["messages"][-1].content)
else:
    # 拒绝：手动添加拒绝消息，让 LLM 告知用户
    app.update_state(
        config,
        {
            "messages": [
                ToolMessage(
                    content="用户拒绝了此操作。",
                    tool_call_id=last_msg.tool_calls[0]["id"],
                )
            ]
        },
    )
    # 继续，让 LLM 处理拒绝情况
    final_result = app.invoke(None, config=config)
    print("AI回复：", final_result["messages"][-1].content)
```

---

## 9. LangGraph 进阶：并行节点与子图

### 9.1 并行执行节点

```python
# ============================================================
# LangGraph 支持并行执行多个独立的节点
# 大幅提升效率（如：同时调用多个 API）
# ============================================================
from langgraph.graph import StateGraph, END
from typing import TypedDict, Annotated
import operator
import asyncio

class ParallelState(TypedDict):
    query: str
    weather_result: str
    news_result: str
    calc_result: str
    final_answer: str


def fetch_weather(state: ParallelState) -> ParallelState:
    """并行任务1：获取天气"""
    import time; time.sleep(1)  # 模拟网络请求
    return {"weather_result": f"天气：晴，25°C"}


def fetch_news(state: ParallelState) -> ParallelState:
    """并行任务2：获取新闻"""
    import time; time.sleep(1)
    return {"news_result": f"今日新闻：AI 技术持续突破"}


def do_calc(state: ParallelState) -> ParallelState:
    """并行任务3：数学计算"""
    return {"calc_result": f"计算结果：100"}


def merge_results(state: ParallelState) -> ParallelState:
    """合并所有并行结果"""
    summary = (
        f"天气信息：{state['weather_result']}\n"
        f"新闻信息：{state['news_result']}\n"
        f"计算结果：{state['calc_result']}"
    )
    return {"final_answer": summary}


# 构建并行图
workflow = StateGraph(ParallelState)
workflow.add_node("fetch_weather", fetch_weather)
workflow.add_node("fetch_news", fetch_news)
workflow.add_node("do_calc", do_calc)
workflow.add_node("merge", merge_results)

# 设置并行入口
workflow.set_entry_point("fetch_weather")

# 从入口同时启动三个并行节点
# 注意：需要用 send_message 或者直接从 START 连三条边
from langgraph.graph import START

workflow = StateGraph(ParallelState)
workflow.add_node("fetch_weather", fetch_weather)
workflow.add_node("fetch_news", fetch_news)
workflow.add_node("do_calc", do_calc)
workflow.add_node("merge", merge_results)

# 三个节点都从 START 并行开始
workflow.add_edge(START, "fetch_weather")
workflow.add_edge(START, "fetch_news")
workflow.add_edge(START, "do_calc")

# 三个节点都完成后汇聚到 merge
workflow.add_edge("fetch_weather", "merge")
workflow.add_edge("fetch_news", "merge")
workflow.add_edge("do_calc", "merge")
workflow.add_edge("merge", END)

app = workflow.compile()

import time
start = time.time()
result = app.invoke({"query": "帮我查一下天气、新闻，并计算100"})
print(f"耗时：{time.time() - start:.1f}秒")  # ≈1秒（并行）而非3秒（串行）
print(result["final_answer"])
```

### 9.2 子图（Subgraph）

```python
# ============================================================
# 复杂系统中，可以把一个完整的图作为另一个图的节点
# 实现模块化、可复用的 Agent 组件
# ============================================================

# ---------- 子图1：研究员 ----------
class ResearchState(TypedDict):
    query: str
    research_result: str

researcher_graph = StateGraph(ResearchState)

def do_research(state: ResearchState) -> ResearchState:
    # 模拟搜索
    return {"research_result": f"关于 '{state['query']}' 的研究结果..."}

researcher_graph.add_node("research", do_research)
researcher_graph.set_entry_point("research")
researcher_graph.add_edge("research", END)
researcher_app = researcher_graph.compile()


# ---------- 主图：把子图作为节点 ----------
class MainState(TypedDict):
    user_query: str
    research_result: str
    final_answer: str

main_graph = StateGraph(MainState)

def prepare(state: MainState) -> MainState:
    """准备阶段"""
    return state

def call_researcher(state: MainState) -> MainState:
    """调用研究员子图"""
    result = researcher_app.invoke({"query": state["user_query"]})
    return {"research_result": result["research_result"]}

def generate_answer(state: MainState) -> MainState:
    """基于研究结果生成答案"""
    answer = f"根据研究：{state['research_result']}\n综合回答：..."
    return {"final_answer": answer}

main_graph.add_node("prepare", prepare)
main_graph.add_node("researcher", call_researcher)  # 子图作为节点
main_graph.add_node("answer", generate_answer)

main_graph.set_entry_point("prepare")
main_graph.add_edge("prepare", "researcher")
main_graph.add_edge("researcher", "answer")
main_graph.add_edge("answer", END)

main_app = main_graph.compile()
result = main_app.invoke({"user_query": "Python 的最新版本是什么？"})
print(result["final_answer"])
```

### 9.3 动态路由：Send API

```python
# ============================================================
# Send API：动态生成节点调用
# 场景：根据输入，动态决定并行调用多少个节点
# 例如：用户输入了5个城市，并行查5个城市的天气
# ============================================================
from langgraph.types import Send
from typing import TypedDict, Annotated, List
import operator

class WeatherBatchState(TypedDict):
    cities: List[str]                   # 待查询的城市列表
    results: Annotated[List[str], operator.add]  # 累积结果

class SingleCityState(TypedDict):
    city: str
    result: str

def query_weather(state: SingleCityState) -> SingleCityState:
    """查单个城市的天气"""
    return {"result": f"{state['city']}: 晴 25°C"}

def collect_results(state: WeatherBatchState) -> WeatherBatchState:
    """汇总所有天气结果"""
    return state  # 结果已经通过 Annotated[List, operator.add] 自动合并

def dispatch_queries(state: WeatherBatchState):
    """动态生成并行任务：每个城市一个 Send"""
    return [
        Send("query_weather", {"city": city})
        for city in state["cities"]
    ]

workflow = StateGraph(WeatherBatchState)
workflow.add_node("query_weather", query_weather)
workflow.add_node("collect", collect_results)

workflow.set_entry_point("dispatch")
workflow.add_node("dispatch", lambda s: s)  # 占位节点

workflow.add_conditional_edges(
    "dispatch",
    dispatch_queries,  # 返回 Send 列表 → 并行执行
    ["query_weather"],
)
workflow.add_edge("query_weather", "collect")
workflow.add_edge("collect", END)

app = workflow.compile()
result = app.invoke({"cities": ["北京", "上海", "广州", "深圳"], "results": []})
print(result["results"])
# → ["北京: 晴 25°C", "上海: 晴 25°C", "广州: 晴 25°C", "深圳: 晴 25°C"]
```

---

## 10. 完整项目：PDF 智能问答 Agent

把前面所有知识整合成一个完整的、生产可用的 PDF 问答 Agent。

**功能：**
- 上传多个 PDF，自动建立向量知识库
- 支持自然语言问答，回答基于文档内容
- 回答时注明来源（文件名 + 页码）
- 支持多轮对话（有记忆）
- 不相关问题可以使用工具补充回答

### 10.1 项目结构

```
pdf-qa-agent/
├── pyproject.toml
├── .env
├── src/
│   ├── __init__.py
│   ├── loader.py       # PDF 加载与处理
│   ├── vectorstore.py  # 向量存储管理
│   ├── agent.py        # LangGraph Agent 定义
│   └── main.py         # 入口
├── pdfs/               # 放置 PDF 文件
└── data/
    └── chroma_db/      # 向量数据库（自动创建）
```

### 10.2 依赖配置

```toml
# pyproject.toml
[project]
name = "pdf-qa-agent"
version = "0.1.0"
requires-python = ">=3.11"
dependencies = [
    "langchain>=0.3",
    "langchain-openai>=0.2",
    "langchain-community>=0.3",
    "langchain-chroma>=0.1",
    "langgraph>=0.2",
    "pymupdf>=1.24",
    "python-dotenv>=1.0",
    "tiktoken>=0.7",
    "chromadb>=0.5",
    "rich>=13.0",        # 美化终端输出
]
```

### 10.3 loader.py — PDF 加载与处理

```python
# src/loader.py
"""
PDF 文档加载、清洗、分块的完整流程。
"""
import re
from pathlib import Path
from typing import List

from langchain_community.document_loaders import PyMuPDFLoader
from langchain_core.documents import Document
from langchain.text_splitter import RecursiveCharacterTextSplitter


def load_pdfs(pdf_dir: str) -> List[Document]:
    """
    加载目录下所有 PDF 文件。

    返回: Document 列表（每页一个）
    """
    pdf_path = Path(pdf_dir)
    pdf_files = list(pdf_path.glob("**/*.pdf"))

    if not pdf_files:
        print(f"⚠️  在 {pdf_dir} 中未找到 PDF 文件")
        return []

    print(f"📂 发现 {len(pdf_files)} 个 PDF 文件")
    all_docs = []

    for pdf_file in pdf_files:
        try:
            loader = PyMuPDFLoader(str(pdf_file))
            docs = loader.load()
            all_docs.extend(docs)
            print(f"  ✅ {pdf_file.name} ({len(docs)} 页)")
        except Exception as e:
            print(f"  ❌ {pdf_file.name}: {e}")

    print(f"📄 共加载 {len(all_docs)} 页")
    return all_docs


def clean_text(text: str) -> str:
    """清洗 PDF 提取的文本"""
    # 去除非打印字符（保留中英文、数字、常用标点）
    text = re.sub(r"[^\u4e00-\u9fff\u0020-\u007e\uff00-\uffef\n\t]", " ", text)
    # 压缩多余空白
    text = re.sub(r"[ \t]+", " ", text)
    text = re.sub(r"\n{3,}", "\n\n", text)
    return text.strip()


def clean_documents(docs: List[Document]) -> List[Document]:
    """批量清洗文档，过滤内容太少的页"""
    cleaned = []
    for doc in docs:
        content = clean_text(doc.page_content)
        if len(content) < 20:  # 跳过内容极少的页（可能是图片页）
            continue
        cleaned.append(Document(page_content=content, metadata=doc.metadata))

    print(f"🧹 清洗后保留 {len(cleaned)} 页（过滤 {len(docs) - len(cleaned)} 页）")
    return cleaned


def split_documents(docs: List[Document], chunk_size: int = 500) -> List[Document]:
    """
    将文档分块。

    参数:
        docs:       文档列表
        chunk_size: 每块字符数（默认500）
    """
    splitter = RecursiveCharacterTextSplitter(
        chunk_size=chunk_size,
        chunk_overlap=50,
        length_function=len,
        separators=["\n\n", "\n", "。", "！", "？", "；", "，", " ", ""],
    )

    chunks = splitter.split_documents(docs)
    print(f"✂️  分块完成：{len(docs)} 页 → {len(chunks)} 个块")
    print(f"   平均块大小：{sum(len(c.page_content) for c in chunks) / len(chunks):.0f} 字符")
    return chunks


def process_pdfs(pdf_dir: str, chunk_size: int = 500) -> List[Document]:
    """完整的 PDF 处理流程：加载 → 清洗 → 分块"""
    docs = load_pdfs(pdf_dir)
    if not docs:
        return []
    docs = clean_documents(docs)
    chunks = split_documents(docs, chunk_size)
    return chunks
```

### 10.4 vectorstore.py — 向量库管理

```python
# src/vectorstore.py
"""
向量存储的创建、更新和检索。
"""
import os
from pathlib import Path
from typing import List, Optional

from langchain_chroma import Chroma
from langchain_core.documents import Document
from langchain_core.vectorstores import VectorStoreRetriever
from langchain_openai import OpenAIEmbeddings


def get_embeddings() -> OpenAIEmbeddings:
    """初始化 Embedding 模型"""
    return OpenAIEmbeddings(
        model="text-embedding-ada-002",
        # 中文建议用 text-embedding-3-small（更新、更便宜）
        # model="text-embedding-3-small",
    )


def build_vectorstore(
    chunks: List[Document],
    persist_dir: str = "./data/chroma_db",
    collection_name: str = "pdf_docs",
) -> Chroma:
    """
    从文档块构建向量存储。
    如果已存在，则追加（不重复添加）。
    """
    embeddings = get_embeddings()
    persist_path = Path(persist_dir)

    if persist_path.exists() and any(persist_path.iterdir()):
        # 加载已有向量库
        print(f"📦 加载已有向量库：{persist_dir}")
        vectorstore = Chroma(
            persist_directory=persist_dir,
            embedding_function=embeddings,
            collection_name=collection_name,
        )
        # 追加新文档
        if chunks:
            print(f"➕ 追加 {len(chunks)} 个新文档块")
            vectorstore.add_documents(chunks)
    else:
        # 创建新向量库
        print(f"🏗️  创建新向量库：{persist_dir}")
        vectorstore = Chroma.from_documents(
            documents=chunks,
            embedding=embeddings,
            persist_directory=persist_dir,
            collection_name=collection_name,
        )
        print(f"✅ 向量库创建完成，存入 {len(chunks)} 个块")

    return vectorstore


def get_retriever(
    vectorstore: Chroma,
    k: int = 4,
    search_type: str = "mmr",
) -> VectorStoreRetriever:
    """
    创建检索器。

    参数:
        k:           返回文档数
        search_type: "similarity" 或 "mmr"（推荐 mmr，多样性更好）
    """
    return vectorstore.as_retriever(
        search_type=search_type,
        search_kwargs={
            "k": k,
            "fetch_k": k * 4 if search_type == "mmr" else k,
        },
    )
```

### 10.5 agent.py — LangGraph Agent

```python
# src/agent.py
"""
基于 LangGraph 的 PDF 问答 Agent。
支持 RAG 检索 + 工具调用 + 多轮对话。
"""
from typing import TypedDict, Annotated, Literal
from datetime import datetime, timezone, timedelta

from langgraph.graph import StateGraph, END
from langgraph.graph.message import add_messages
from langgraph.checkpoint.memory import MemorySaver
from langchain_openai import ChatOpenAI
from langchain_core.tools import tool
from langchain_core.messages import (
    HumanMessage, SystemMessage, ToolMessage, AIMessage
)
from langchain_core.vectorstores import VectorStoreRetriever


# ============================================================
# 状态定义
# ============================================================
class AgentState(TypedDict):
    messages: Annotated[list, add_messages]


# ============================================================
# 工具定义
# ============================================================

def create_rag_tool(retriever: VectorStoreRetriever):
    """
    工厂函数：根据 retriever 创建 RAG 检索工具。
    """
    @tool
    def search_documents(query: str) -> str:
        """
        在 PDF 知识库中搜索相关内容。
        当用户询问文档中的信息时使用。
        会返回相关段落及其来源（文件名和页码）。
        """
        docs = retriever.invoke(query)
        if not docs:
            return "在知识库中未找到相关内容。"

        results = []
        for i, doc in enumerate(docs, 1):
            source = doc.metadata.get("source", "未知文件")
            # 只保留文件名，去掉路径
            source = source.split("/")[-1].split("\\")[-1]
            page = doc.metadata.get("page", 0) + 1  # 转为1-based页码
            results.append(
                f"[来源 {i}] 文件：{source} 第{page}页\n{doc.page_content}"
            )

        return "\n\n---\n\n".join(results)

    return search_documents


@tool
def get_current_time() -> str:
    """获取当前的北京时间，当用户询问时间时使用。"""
    tz = timezone(timedelta(hours=8))
    return datetime.now(tz).strftime("%Y年%m月%d日 %H:%M:%S")


@tool
def calculate(expression: str) -> str:
    """
    执行数学计算。
    当用户需要精确计算时使用。
    例如：calculate("1024 * 1024") 或 calculate("sqrt(144)")
    """
    import math
    allowed = {k: v for k, v in math.__dict__.items() if not k.startswith("_")}
    allowed.update({"abs": abs, "round": round})
    try:
        result = eval(expression, {"__builtins__": {}}, allowed)
        return str(result)
    except Exception as e:
        return f"计算错误: {e}"


# ============================================================
# 系统提示词
# ============================================================
SYSTEM_PROMPT = """你是一个专业的 PDF 文档问答助手。

你可以使用以下工具：
- search_documents: 在 PDF 知识库中搜索相关内容（**回答文档相关问题时必须先调用**）
- get_current_time: 获取当前时间
- calculate: 数学计算

回答规范：
1. 回答文档内容相关的问题，必须先调用 search_documents 检索，不要凭记忆回答
2. 回答时请注明信息来源（文件名和页码）
3. 如果检索结果与问题无关，请如实告知"文档中未找到相关信息"
4. 使用中文回答，语言简洁清晰
5. 对于计算类问题，使用 calculate 工具确保准确"""


# ============================================================
# 构建 Agent 图
# ============================================================

def build_agent(retriever: VectorStoreRetriever) -> "CompiledGraph":
    """
    构建 PDF 问答 Agent。

    参数:
        retriever: 向量检索器
    """
    # 初始化工具
    rag_tool = create_rag_tool(retriever)
    tools = [rag_tool, get_current_time, calculate]

    # 初始化 LLM
    llm = ChatOpenAI(model="deepseek-chat", temperature=0)
    llm_with_tools = llm.bind_tools(tools)
    tool_map = {t.name: t for t in tools}

    # ---------- 节点 ----------
    def call_model(state: AgentState) -> AgentState:
        """LLM 推理节点"""
        system = SystemMessage(content=SYSTEM_PROMPT)
        response = llm_with_tools.invoke([system] + state["messages"])
        return {"messages": [response]}

    def call_tools(state: AgentState) -> AgentState:
        """工具执行节点"""
        last_message = state["messages"][-1]
        tool_messages = []

        for tc in last_message.tool_calls:
            t = tool_map.get(tc["name"])
            if t:
                try:
                    result = t.invoke(tc["args"])
                except Exception as e:
                    result = f"工具执行出错: {e}"
            else:
                result = f"未知工具: {tc['name']}"

            tool_messages.append(ToolMessage(
                content=result,
                tool_call_id=tc["id"],
            ))

        return {"messages": tool_messages}

    def should_continue(state: AgentState) -> Literal["tools", "__end__"]:
        last_message = state["messages"][-1]
        return "tools" if last_message.tool_calls else END

    # ---------- 构建图 ----------
    workflow = StateGraph(AgentState)
    workflow.add_node("agent", call_model)
    workflow.add_node("tools", call_tools)
    workflow.set_entry_point("agent")
    workflow.add_conditional_edges("agent", should_continue, {"tools": "tools", END: END})
    workflow.add_edge("tools", "agent")

    # 使用 MemorySaver 支持多轮对话
    checkpointer = MemorySaver()
    return workflow.compile(checkpointer=checkpointer)
```

### 10.6 main.py — 交互入口

```python
# src/main.py
"""
PDF 问答 Agent 入口。
使用方式：
  1. 把 PDF 文件放入 ./pdfs/ 目录
  2. uv run python src/main.py
"""
import os
import sys
from pathlib import Path

from dotenv import load_dotenv
from langchain_core.messages import HumanMessage
from rich.console import Console
from rich.panel import Panel
from rich.text import Text

# 添加 src 到 Python 路径
sys.path.insert(0, str(Path(__file__).parent))

from loader import process_pdfs
from vectorstore import build_vectorstore, get_retriever
from agent import build_agent

load_dotenv()
console = Console()


def main():
    # ---------- 欢迎界面 ----------
    console.print(Panel(
        "[bold cyan]📚 PDF 智能问答 Agent[/bold cyan]\n"
        "基于 LangChain + LangGraph + RAG 构建\n"
        "支持多文档检索 + 多轮对话 + 工具调用",
        title="欢迎",
        border_style="cyan",
    ))

    # ---------- 加载 PDF ----------
    pdf_dir = "./pdfs"
    Path(pdf_dir).mkdir(exist_ok=True)

    # 检查是否有 PDF
    pdf_files = list(Path(pdf_dir).glob("**/*.pdf"))
    if not pdf_files:
        console.print(f"[yellow]⚠️  请把 PDF 文件放入 {pdf_dir}/ 目录后重新运行[/yellow]")
        console.print("示例：cp your_file.pdf pdfs/")
        return

    # 处理 PDF
    console.print(f"\n[bold]正在处理 PDF 文件...[/bold]")
    chunks = process_pdfs(pdf_dir, chunk_size=500)

    if not chunks:
        console.print("[red]PDF 处理失败，请检查文件[/red]")
        return

    # 构建向量库
    console.print(f"\n[bold]正在构建向量知识库...[/bold]")
    vectorstore = build_vectorstore(chunks)
    retriever = get_retriever(vectorstore, k=4)

    # 构建 Agent
    console.print(f"\n[bold]初始化 Agent...[/bold]")
    app = build_agent(retriever)

    console.print("\n[green]✅ 准备就绪！[/green]")
    console.print("已加载文档：")
    for f in pdf_files:
        console.print(f"  📄 {f.name}")
    console.print("\n输入 'exit' 退出，输入 'new' 开始新对话\n")

    # ---------- 对话循环 ----------
    thread_id = "default_session"
    config = {"configurable": {"thread_id": thread_id}}
    turn = 0

    while True:
        try:
            user_input = console.input("[bold blue]你：[/bold blue]").strip()
        except (KeyboardInterrupt, EOFError):
            console.print("\n[yellow]👋 再见！[/yellow]")
            break

        if not user_input:
            continue
        if user_input.lower() in ("exit", "quit", "q"):
            console.print("[yellow]👋 再见！[/yellow]")
            break
        if user_input.lower() == "new":
            # 开始新对话（新的 thread_id）
            import time
            thread_id = f"session_{int(time.time())}"
            config = {"configurable": {"thread_id": thread_id}}
            turn = 0
            console.print("[green]🔄 已开始新对话[/green]\n")
            continue

        turn += 1

        # 流式调用 Agent
        console.print()
        answer_parts = []
        tool_calls_info = []

        events = app.stream(
            {"messages": [HumanMessage(content=user_input)]},
            config=config,
        )

        for event in events:
            # 提取每步事件中的最新消息
            for node_name, node_output in event.items():
                if "messages" not in node_output:
                    continue
                for msg in node_output["messages"]:
                    if hasattr(msg, "tool_calls") and msg.tool_calls:
                        for tc in msg.tool_calls:
                            tool_calls_info.append(
                                f"  🔍 {tc['name']}({list(tc['args'].values())[0] if tc['args'] else ''})"
                            )
                    if hasattr(msg, "content") and msg.content and not getattr(msg, "tool_calls", None):
                        answer_parts.append(msg.content)

        # 显示工具调用信息
        if tool_calls_info:
            console.print("[dim]" + "\n".join(tool_calls_info) + "[/dim]")

        # 显示最终回答
        if answer_parts:
            final_answer = answer_parts[-1]  # 取最后一个回答（最终结果）
            console.print(Panel(
                final_answer,
                title="[bold green]AI[/bold green]",
                border_style="green",
            ))
        console.print()


if __name__ == "__main__":
    main()
```

### 10.7 运行方式

```bash
# 1. 创建项目并安装依赖
uv init pdf-qa-agent
cd pdf-qa-agent
uv add langchain langchain-openai langchain-community langchain-chroma
uv add langgraph pymupdf python-dotenv tiktoken chromadb rich

# 2. 配置 .env
echo "OPENAI_API_KEY=sk-your-key" > .env
echo "OPENAI_BASE_URL=https://api.deepseek.com" >> .env

# 3. 放入 PDF 文件
mkdir pdfs
cp ~/Documents/your_manual.pdf pdfs/

# 4. 运行
uv run python src/main.py
```

**运行效果示例：**

```
╔══════════════════════════════════════╗
║      📚 PDF 智能问答 Agent           ║
╚══════════════════════════════════════╝

正在处理 PDF 文件...
  ✅ 产品手册.pdf (42 页)
  ✅ 用户协议.pdf (15 页)
分块完成：57 页 → 234 个块

✅ 准备就绪！
已加载文档：
  📄 产品手册.pdf
  📄 用户协议.pdf

你：产品的质保期是多少？
  🔍 search_documents(质保期)

╔═ AI ══════════════════════════════════╗
║ 根据《产品手册.pdf》第8页的说明：      ║
║                                       ║
║ 本产品质保期为**2年**，自购买日期起    ║
║ 计算。质保期内，因产品本身质量问题     ║
║ 导致的损坏，可免费维修或更换。         ║
╚═══════════════════════════════════════╝

你：如果超出质保期需要多少费用？
  🔍 search_documents(超出质保期 维修费用)
...
```

---

## 总结

### 技术栈全览

```
用户问题
    │
    ▼
LangGraph Agent
    │
    ├── RAG 检索工具
    │       ├── PyMuPDF（PDF 加载）
    │       ├── RecursiveCharacterTextSplitter（文本分块）
    │       ├── OpenAIEmbeddings（向量化）
    │       └── Chroma（向量存储 + 检索）
    │
    ├── 计算工具
    ├── 时间工具
    │
    ├── LangChain LCEL（链式组合）
    ├── MemorySaver（多轮对话记忆）
    │
    └── DeepSeek / OpenAI（LLM）
```

### 关键知识点速查

| 模块 | 推荐选择 | 备注 |
|------|----------|------|
| PDF 加载 | `PyMuPDFLoader` | 速度快，元数据丰富 |
| 文本分块 | `RecursiveCharacterTextSplitter` | 适合绝大多数场景 |
| 分块大小 | 400–600 字符 | 中文文档的经验值 |
| Embedding | `text-embedding-ada-002` | 或 `text-embedding-3-small`（更新） |
| 向量库（开发） | `Chroma` | 本地持久化，易用 |
| 向量库（生产） | `Qdrant` / `PGVector` | 高性能，生产就绪 |
| 检索策略 | MMR 混合 BM25 | 兼顾相似度和多样性 |
| Agent 框架 | `LangGraph` | 灵活可控，推荐新项目 |
| 对话记忆 | `MemorySaver` → `SqliteSaver` | 开发用内存，生产用持久化 |

### 进一步学习

- **LangSmith**：LangChain 官方的调试和监控平台，强烈建议集成
- **LangGraph Platform**：部署 LangGraph Agent 为 API 服务
- **GraphRAG**：微软推出的知识图谱 + RAG，处理复杂关系效果更好
- **Agentic RAG**：让 Agent 自主决定何时检索、检索什么

---

> **前置章节：** [05-building-agents.md](./05-building-agents.md)
> **系列完整索引：**
> - [01 - uv 包管理器](./01-uv-tutorial.md)
> - [02 - LangChain 基础](./02-langchain-basics.md)
> - [03 - LangChain Tools](./03-langchain-tools.md)
> - [04 - LangGraph 基础](./04-langgraph-basics.md)
> - [05 - 构建智能体实战](./05-building-agents.md)
> - **[06 - RAG + PDF + 深度进阶（本章）](./06-rag-and-agents-deepdive.md)**
