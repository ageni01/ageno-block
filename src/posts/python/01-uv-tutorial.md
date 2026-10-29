---
title: "uv — 下一代 Python 包管理器"

category:
  - Python
tag:
  - Python
---
# uv — 下一代 Python 包管理器

> uv 是用 Rust 编写的极速 Python 包管理器和项目管理工具，对标 pip + poetry + virtualenv，速度比 pip 快 10-100 倍。

## 目录

1. [安装 uv](#1-安装-uv)
2. [核心命令速览](#2-核心命令速览)
3. [创建与管理项目](#3-创建与管理项目)
4. [虚拟环境](#4-虚拟环境)
5. [依赖管理](#5-依赖管理)
6. [运行脚本与命令](#6-运行脚本与命令)
7. [与 pip 的对比](#7-与-pip-的对比)
8. [LangChain 项目实战](#8-langchain-项目实战)

---

## 1. 安装 uv

```bash
# macOS / Linux
curl -LsSf https://astral.sh/uv/install.sh | sh

# macOS 也可用 Homebrew
brew install uv

# 验证安装
uv --version
```

## 2. 核心命令速览

| 命令 | 功能 | 类比 |
|------|------|------|
| `uv init` | 初始化新项目 | `poetry init` |
| `uv add` | 添加依赖 | `pip install` + `poetry add` |
| `uv remove` | 移除依赖 | `pip uninstall` + `poetry remove` |
| `uv sync` | 同步锁文件依赖 | `poetry install` |
| `uv run` | 在虚拟环境中运行命令 | `poetry run` |
| `uv venv` | 创建虚拟环境 | `python -m venv` |
| `uv pip` | pip 兼容接口 | `pip` |
| `uv lock` | 生成锁文件 | `poetry lock` |
| `uv tool` | 管理全局工具 | `pipx` |
| `uv python` | 管理 Python 版本 | `pyenv` |

## 3. 创建与管理项目

```bash
# 创建新项目
uv init my-agent-project
cd my-agent-project

# 项目结构：
# my-agent-project/
# ├── .python-version    # Python 版本锁定
# ├── pyproject.toml     # 项目元数据与依赖声明
# ├── README.md
# └── src/
#     └── my_agent_project/
#         └── __init__.py

# 查看自动生成的 pyproject.toml
cat pyproject.toml
```

**pyproject.toml 示例：**

```toml
[project]
name = "agent-demo"
version = "0.1.0"
description = "AI Agent 学习项目"
readme = "README.md"
requires-python = ">=3.11"
dependencies = []
```

## 4. 虚拟环境

```bash
# 显式创建虚拟环境（一般在项目根目录）
uv venv

# 会在当前目录创建 .venv 文件夹
# uv 会自动发现并使用它

# 指定 Python 版本创建环境
uv venv --python 3.12

# 激活虚拟环境
source .venv/bin/activate   # macOS/Linux
# 或
.venv\Scripts\activate      # Windows

# uv 的魔法：即使不激活，uv run 也会自动使用 .venv
```

## 5. 依赖管理

```bash
# 添加生产依赖
uv add langchain langchain-openai langgraph

# 添加开发依赖
uv add --dev pytest pytest-asyncio ruff mypy

# 指定版本
uv add "langchain>=0.3,<0.4"

# 移除依赖
uv remove langchain-openai

# 从 requirements.txt 安装
uv pip install -r requirements.txt

# 导出 requirements.txt
uv pip freeze > requirements.txt

# 查看依赖树
uv tree
```

**`uv sync` 的工作流：**

```bash
# 1. 编辑 pyproject.toml 添加依赖
# 2. 同步锁文件并安装
uv sync

# 使用锁定版本（生产部署）
uv sync --frozen
```

## 6. 运行脚本与命令

```bash
# 在项目虚拟环境中运行 Python
uv run python main.py

# 运行模块
uv run python -m my_agent_project.main

# 运行脚本文件（自动使用项目环境）
uv run script.py

# 直接运行工具命令
uv run pytest tests/
uv run ruff check .
uv run mypy src/

# 运行单个 Python 表达式
uv run python -c "import langchain; print(langchain.__version__)"
```

## 7. 与 pip 的对比

| 操作 | pip | uv |
|------|-----|----|
| 安装包 | `pip install xxx` | `uv add xxx` |
| 安装全部 | `pip install -r requirements.txt` | `uv sync` |
| 卸载 | `pip uninstall xxx` | `uv remove xxx` |
| 创建 venv | `python -m venv .venv` | `uv venv` |
| 运行 | `python main.py` | `uv run python main.py` |
| 速度 | 慢（下载 + 解析串行） | 极快（Rust + 并行下载） |
| 锁文件 | ❌ 无 | ✅ `uv.lock` |

## 8. LangChain 项目实战

```bash
# 创建项目
uv init langchain-agent
cd langchain-agent

# 添加依赖
uv add langchain langchain-openai langchain-community langgraph
uv add python-dotenv httpx httpx-sse

# 添加开发工具
uv add --dev pytest jupyter ipykernel

# 创建 .env 文件
echo "OPENAI_API_KEY=sk-your-key-here" > .env
# 或者国内用
echo "OPENAI_API_KEY=sk-your-key-here" > .env
echo "OPENAI_BASE_URL=https://api.deepseek.com" >> .env

# 安装所有依赖
uv sync

# 验证安装
uv run python -c "
import langchain
import langgraph
print(f'langchain: {langchain.__version__}')
print(f'langgraph: {langgraph.__version__}')
"
```

**VS Code 配置：**

```json
// .vscode/settings.json
{
  "python.defaultInterpreterPath": "${workspaceFolder}/.venv/bin/python",
  "python.terminal.activateEnvironment": true
}
```

**常用技巧：**

```bash
# 更新所有依赖
uv sync --upgrade

# 仅更新某个包
uv add langchain@latest

# 检查哪些包可以更新
uv pip list --outdated

# 清理缓存
uv cache clean

# 查看缓存大小
uv cache dir
```

---

> **下一章：** [02-langchain-basics.md](./02-langchain-basics.md) — LangChain 基础详解
