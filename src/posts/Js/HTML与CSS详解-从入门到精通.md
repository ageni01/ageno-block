---
title: "HTML与CSS详解：从入门到精通"

category:
  - 前端
tag:
  - 前端
---
# HTML与CSS详解：从入门到精通

> 全面掌握Web前端基础：HTML5语义化标签、CSS3样式设计与响应式布局

---

## 目录

### Part 1: HTML
1. [HTML基础](#1-html基础)
2. [HTML5语义化标签](#2-html5语义化标签)
3. [表单与输入](#3-表单与输入)
4. [多媒体元素](#4-多媒体元素)
5. [HTML最佳实践](#5-html最佳实践)

### Part 2: CSS
6. [CSS基础](#6-css基础)
7. [选择器](#7-选择器)
8. [盒模型](#8-盒模型)
9. [布局技术](#9-布局技术)
10. [Flexbox布局](#10-flexbox布局)
11. [Grid布局](#11-grid布局)
12. [响应式设计](#12-响应式设计)
13. [CSS动画](#13-css动画)
14. [CSS预处理器](#14-css预处理器)
15. [现代CSS特性](#15-现代css特性)

---

## Part 1: HTML

## 1. HTML基础

### 1.1 HTML文档结构

```html
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta name="description" content="页面描述">
  <meta name="keywords" content="关键词1, 关键词2">
  <meta name="author" content="作者名">
  <title>页面标题</title>
  
  <!-- CSS -->
  <link rel="stylesheet" href="styles.css">
  
  <!-- 网站图标 -->
  <link rel="icon" href="favicon.ico">
  
  <!-- Open Graph (社交媒体分享) -->
  <meta property="og:title" content="页面标题">
  <meta property="og:description" content="页面描述">
  <meta property="og:image" content="image.jpg">
</head>
<body>
  <!-- 页面内容 -->
  <h1>Hello, World!</h1>
  
  <!-- JavaScript -->
  <script src="script.js"></script>
</body>
</html>
```

### 1.2 文本标签

```html
<!-- 标题 -->
<h1>一级标题</h1>
<h2>二级标题</h2>
<h3>三级标题</h3>
<h4>四级标题</h4>
<h5>五级标题</h5>
<h6>六级标题</h6>

<!-- 段落 -->
<p>这是一个段落。</p>

<!-- 换行 -->
<p>第一行<br>第二行</p>

<!-- 水平线 -->
<hr>

<!-- 文本格式化 -->
<strong>粗体（强调）</strong>
<b>粗体</b>
<em>斜体（强调）</em>
<i>斜体</i>
<mark>高亮</mark>
<small>小字</small>
<del>删除线</del>
<ins>下划线</ins>
<sub>下标</sub>
<sup>上标</sup>

<!-- 引用 -->
<blockquote cite="https://example.com">
  这是一个块引用。
</blockquote>

<q>这是一个行内引用。</q>

<!-- 代码 -->
<code>console.log('Hello');</code>

<pre>
  <code>
    function hello() {
      console.log('Hello');
    }
  </code>
</pre>

<!-- 缩写 -->
<abbr title="HyperText Markup Language">HTML</abbr>

<!-- 地址 -->
<address>
  联系方式：<a href="mailto:email@example.com">email@example.com</a>
</address>
```

### 1.3 列表

```html
<!-- 无序列表 -->
<ul>
  <li>项目1</li>
  <li>项目2</li>
  <li>项目3</li>
</ul>

<!-- 有序列表 -->
<ol>
  <li>第一步</li>
  <li>第二步</li>
  <li>第三步</li>
</ol>

<!-- 自定义起始数字 -->
<ol start="5">
  <li>第五项</li>
  <li>第六项</li>
</ol>

<!-- 描述列表 -->
<dl>
  <dt>HTML</dt>
  <dd>超文本标记语言</dd>
  
  <dt>CSS</dt>
  <dd>层叠样式表</dd>
  
  <dt>JavaScript</dt>
  <dd>编程语言</dd>
</dl>

<!-- 嵌套列表 -->
<ul>
  <li>
    水果
    <ul>
      <li>苹果</li>
      <li>香蕉</li>
    </ul>
  </li>
  <li>
    蔬菜
    <ul>
      <li>胡萝卜</li>
      <li>西兰花</li>
    </ul>
  </li>
</ul>
```

### 1.4 链接与图片

```html
<!-- 链接 -->
<a href="https://example.com">外部链接</a>
<a href="/about">内部链接</a>
<a href="#section">锚点链接</a>
<a href="mailto:email@example.com">邮件链接</a>
<a href="tel:+8612345678">电话链接</a>

<!-- 新窗口打开 -->
<a href="https://example.com" target="_blank" rel="noopener noreferrer">
  新窗口打开
</a>

<!-- 下载链接 -->
<a href="file.pdf" download="文件名.pdf">下载PDF</a>

<!-- 图片 -->
<img src="image.jpg" alt="图片描述" width="300" height="200">

<!-- 响应式图片 -->
<img 
  src="small.jpg" 
  srcset="small.jpg 480w, medium.jpg 800w, large.jpg 1200w"
  sizes="(max-width: 600px) 480px, (max-width: 900px) 800px, 1200px"
  alt="响应式图片"
>

<!-- picture元素 -->
<picture>
  <source media="(min-width: 800px)" srcset="large.jpg">
  <source media="(min-width: 400px)" srcset="medium.jpg">
  <img src="small.jpg" alt="图片">
</picture>

<!-- 图片映射 -->
<img src="map.jpg" alt="地图" usemap="#map">
<map name="map">
  <area shape="rect" coords="0,0,100,100" href="link1.html" alt="区域1">
  <area shape="circle" coords="200,200,50" href="link2.html" alt="区域2">
</map>

<!-- 图片与说明 -->
<figure>
  <img src="image.jpg" alt="图片">
  <figcaption>图片说明文字</figcaption>
</figure>
```

### 1.5 表格

```html
<!-- 基本表格 -->
<table>
  <thead>
    <tr>
      <th>姓名</th>
      <th>年龄</th>
      <th>城市</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>张三</td>
      <td>25</td>
      <td>北京</td>
    </tr>
    <tr>
      <td>李四</td>
      <td>30</td>
      <td>上海</td>
    </tr>
  </tbody>
  <tfoot>
    <tr>
      <td colspan="3">总计：2人</td>
    </tr>
  </tfoot>
</table>

<!-- 合并单元格 -->
<table>
  <tr>
    <th>姓名</th>
    <th colspan="2">联系方式</th>
  </tr>
  <tr>
    <td rowspan="2">张三</td>
    <td>电话</td>
    <td>123456</td>
  </tr>
  <tr>
    <td>邮箱</td>
    <td>zhang@example.com</td>
  </tr>
</table>

<!-- 表格样式 -->
<table border="1" cellpadding="10" cellspacing="0">
  <!-- 内容 -->
</table>
```

---

## 2. HTML5语义化标签

### 2.1 页面结构

```html
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <title>语义化页面</title>
</head>
<body>
  <!-- 页眉 -->
  <header>
    <nav>
      <ul>
        <li><a href="/">首页</a></li>
        <li><a href="/about">关于</a></li>
        <li><a href="/contact">联系</a></li>
      </ul>
    </nav>
  </header>
  
  <!-- 主要内容 -->
  <main>
    <!-- 文章 -->
    <article>
      <header>
        <h1>文章标题</h1>
        <p>
          <time datetime="2024-01-15">2024年1月15日</time>
          作者：<span>张三</span>
        </p>
      </header>
      
      <section>
        <h2>章节标题</h2>
        <p>章节内容...</p>
      </section>
      
      <section>
        <h2>另一个章节</h2>
        <p>章节内容...</p>
      </section>
      
      <footer>
        <p>标签：HTML, CSS, JavaScript</p>
      </footer>
    </article>
    
    <!-- 侧边栏 -->
    <aside>
      <h2>相关文章</h2>
      <ul>
        <li><a href="#">文章1</a></li>
        <li><a href="#">文章2</a></li>
      </ul>
    </aside>
  </main>
  
  <!-- 页脚 -->
  <footer>
    <p>&copy; 2024 公司名称. 保留所有权利。</p>
  </footer>
</body>
</html>
```

### 2.2 语义化标签详解

```html
<!-- header: 页眉或章节头部 -->
<header>
  <h1>网站标题</h1>
  <nav>导航</nav>
</header>

<!-- nav: 导航链接 -->
<nav>
  <ul>
    <li><a href="/">首页</a></li>
    <li><a href="/about">关于</a></li>
  </ul>
</nav>

<!-- main: 主要内容（一个页面只能有一个） -->
<main>
  <h1>主要内容</h1>
</main>

<!-- article: 独立的内容（文章、博客等） -->
<article>
  <h2>文章标题</h2>
  <p>文章内容...</p>
</article>

<!-- section: 文档中的节 -->
<section>
  <h2>章节标题</h2>
  <p>章节内容...</p>
</section>

<!-- aside: 侧边栏内容 -->
<aside>
  <h3>相关链接</h3>
  <ul>
    <li><a href="#">链接1</a></li>
  </ul>
</aside>

<!-- footer: 页脚 -->
<footer>
  <p>版权信息</p>
</footer>

<!-- details/summary: 可折叠内容 -->
<details>
  <summary>点击展开</summary>
  <p>隐藏的内容</p>
</details>

<!-- mark: 高亮文本 -->
<p>这是<mark>重要的</mark>文本。</p>

<!-- time: 时间 -->
<time datetime="2024-01-15T10:00:00">2024年1月15日</time>

<!-- progress: 进度条 -->
<progress value="70" max="100">70%</progress>

<!-- meter: 度量 -->
<meter value="0.6" min="0" max="1">60%</meter>
```

---

## 3. 表单与输入

### 3.1 基本表单

```html
<form action="/submit" method="POST" enctype="multipart/form-data">
  <!-- 文本输入 -->
  <label for="username">用户名：</label>
  <input type="text" id="username" name="username" required>
  
  <!-- 密码 -->
  <label for="password">密码：</label>
  <input type="password" id="password" name="password" required>
  
  <!-- 邮箱 -->
  <label for="email">邮箱：</label>
  <input type="email" id="email" name="email" required>
  
  <!-- 数字 -->
  <label for="age">年龄：</label>
  <input type="number" id="age" name="age" min="0" max="120">
  
  <!-- 日期 -->
  <label for="birthday">生日：</label>
  <input type="date" id="birthday" name="birthday">
  
  <!-- 单选按钮 -->
  <fieldset>
    <legend>性别：</legend>
    <label>
      <input type="radio" name="gender" value="male"> 男
    </label>
    <label>
      <input type="radio" name="gender" value="female"> 女
    </label>
  </fieldset>
  
  <!-- 复选框 -->
  <fieldset>
    <legend>兴趣：</legend>
    <label>
      <input type="checkbox" name="hobby" value="reading"> 阅读
    </label>
    <label>
      <input type="checkbox" name="hobby" value="sports"> 运动
    </label>
  </fieldset>
  
  <!-- 下拉选择 -->
  <label for="city">城市：</label>
  <select id="city" name="city">
    <option value="">请选择</option>
    <option value="beijing">北京</option>
    <option value="shanghai">上海</option>
    <option value="guangzhou">广州</option>
  </select>
  
  <!-- 多选下拉 -->
  <label for="skills">技能：</label>
  <select id="skills" name="skills" multiple size="4">
    <option value="html">HTML</option>
    <option value="css">CSS</option>
    <option value="js">JavaScript</option>
    <option value="react">React</option>
  </select>
  
  <!-- 文本域 -->
  <label for="message">留言：</label>
  <textarea id="message" name="message" rows="5" cols="30"></textarea>
  
  <!-- 文件上传 -->
  <label for="avatar">头像：</label>
  <input type="file" id="avatar" name="avatar" accept="image/*">
  
  <!-- 提交按钮 -->
  <button type="submit">提交</button>
  <button type="reset">重置</button>
  <button type="button">普通按钮</button>
</form>
```

### 3.2 HTML5表单特性

```html
<form>
  <!-- 占位符 -->
  <input type="text" placeholder="请输入用户名">
  
  <!-- 自动完成 -->
  <input type="text" name="username" autocomplete="username">
  
  <!-- 自动聚焦 -->
  <input type="text" autofocus>
  
  <!-- 必填 -->
  <input type="text" required>
  
  <!-- 模式匹配 -->
  <input type="text" pattern="[0-9]{6}" title="请输入6位数字">
  
  <!-- 最小/最大长度 -->
  <input type="text" minlength="3" maxlength="20">
  
  <!-- 数字范围 -->
  <input type="number" min="0" max="100" step="5">
  
  <!-- 范围滑块 -->
  <input type="range" min="0" max="100" value="50">
  
  <!-- 颜色选择器 -->
  <input type="color" value="#ff0000">
  
  <!-- URL -->
  <input type="url" placeholder="https://example.com">
  
  <!-- 电话 -->
  <input type="tel" pattern="[0-9]{11}">
  
  <!-- 搜索 -->
  <input type="search" placeholder="搜索...">
  
  <!-- 时间相关 -->
  <input type="datetime-local">
  <input type="month">
  <input type="week">
  <input type="time">
  
  <!-- 数据列表 -->
  <input list="browsers" name="browser">
  <datalist id="browsers">
    <option value="Chrome">
    <option value="Firefox">
    <option value="Safari">
    <option value="Edge">
  </datalist>
  
  <!-- 表单验证 -->
  <input type="email" required>
  <button type="submit">提交</button>
</form>

<!-- 自定义验证 -->
<script>
  const input = document.querySelector('input[type="email"]');
  
  input.addEventListener('invalid', (e) => {
    if (input.validity.valueMissing) {
      input.setCustomValidity('请输入邮箱');
    } else if (input.validity.typeMismatch) {
      input.setCustomValidity('请输入有效的邮箱地址');
    }
  });
  
  input.addEventListener('input', () => {
    input.setCustomValidity('');
  });
</script>
```

---

## 4. 多媒体元素

### 4.1 音频

```html
<!-- 基本音频 -->
<audio controls>
  <source src="audio.mp3" type="audio/mpeg">
  <source src="audio.ogg" type="audio/ogg">
  您的浏览器不支持音频标签。
</audio>

<!-- 自动播放（需要muted） -->
<audio controls autoplay muted loop>
  <source src="audio.mp3" type="audio/mpeg">
</audio>

<!-- 预加载 -->
<audio controls preload="auto">
  <source src="audio.mp3" type="audio/mpeg">
</audio>
```

### 4.2 视频

```html
<!-- 基本视频 -->
<video controls width="640" height="360">
  <source src="video.mp4" type="video/mp4">
  <source src="video.webm" type="video/webm">
  您的浏览器不支持视频标签。
</video>

<!-- 完整配置 -->
<video 
  controls 
  autoplay 
  muted 
  loop 
  poster="thumbnail.jpg"
  preload="metadata"
  width="640"
  height="360"
>
  <source src="video.mp4" type="video/mp4">
  <track kind="subtitles" src="subs_en.vtt" srclang="en" label="English">
  <track kind="subtitles" src="subs_zh.vtt" srclang="zh" label="中文" default>
</video>

<!-- YouTube嵌入 -->
<iframe 
  width="560" 
  height="315" 
  src="https://www.youtube.com/embed/VIDEO_ID" 
  frameborder="0" 
  allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" 
  allowfullscreen
></iframe>
```

### 4.3 Canvas

```html
<canvas id="myCanvas" width="400" height="300"></canvas>

<script>
  const canvas = document.getElementById('myCanvas');
  const ctx = canvas.getContext('2d');
  
  // 绘制矩形
  ctx.fillStyle = 'blue';
  ctx.fillRect(10, 10, 150, 100);
  
  // 绘制圆形
  ctx.beginPath();
  ctx.arc(200, 75, 50, 0, 2 * Math.PI);
  ctx.fillStyle = 'red';
  ctx.fill();
  
  // 绘制文字
  ctx.font = '30px Arial';
  ctx.fillStyle = 'black';
  ctx.fillText('Hello Canvas', 50, 200);
</script>
```

### 4.4 SVG

```html
<!-- 内联SVG -->
<svg width="200" height="200">
  <!-- 矩形 -->
  <rect x="10" y="10" width="100" height="100" fill="blue" />
  
  <!-- 圆形 -->
  <circle cx="150" cy="60" r="40" fill="red" />
  
  <!-- 线条 -->
  <line x1="0" y1="150" x2="200" y2="150" stroke="black" stroke-width="2" />
  
  <!-- 多边形 -->
  <polygon points="100,150 120,190 80,190" fill="green" />
  
  <!-- 文字 -->
  <text x="50" y="120" font-size="20" fill="white">SVG</text>
</svg>

<!-- 外部SVG -->
<img src="image.svg" alt="SVG图片">

<!-- SVG作为背景 -->
<div style="background-image: url('image.svg');"></div>
```

---

## 5. HTML最佳实践

### 5.1 语义化HTML

```html
<!-- 不推荐 -->
<div class="header">
  <div class="nav">...</div>
</div>
<div class="content">
  <div class="article">...</div>
</div>

<!-- 推荐 -->
<header>
  <nav>...</nav>
</header>
<main>
  <article>...</article>
</main>
```

### 5.2 无障碍访问（Accessibility）

```html
<!-- 图片alt属性 -->
<img src="dog.jpg" alt="一只金毛犬在草地上玩耍">

<!-- ARIA属性 -->
<button aria-label="关闭对话框" aria-pressed="false">
  <span aria-hidden="true">&times;</span>
</button>

<!-- 语言属性 -->
<html lang="zh-CN">

<!-- 跳过导航链接 -->
<a href="#main-content" class="skip-link">跳到主内容</a>
<main id="main-content">...</main>

<!-- 表单标签 -->
<label for="email">邮箱：</label>
<input type="email" id="email" name="email">

<!-- 键盘导航 -->
<div tabindex="0" role="button">可聚焦的元素</div>
```

### 5.3 SEO优化

```html
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <!-- 标题 (50-60字符) -->
  <title>页面标题 - 网站名称</title>
  
  <!-- 描述 (150-160字符) -->
  <meta name="description" content="页面描述，包含关键词">
  
  <!-- 关键词 -->
  <meta name="keywords" content="关键词1, 关键词2, 关键词3">
  
  <!-- Canonical URL -->
  <link rel="canonical" href="https://example.com/page">
  
  <!-- Open Graph -->
  <meta property="og:title" content="页面标题">
  <meta property="og:description" content="页面描述">
  <meta property="og:image" content="https://example.com/image.jpg">
  <meta property="og:url" content="https://example.com/page">
  <meta property="og:type" content="website">
  
  <!-- Twitter Card -->
  <meta name="twitter:card" content="summary_large_image">
  <meta name="twitter:title" content="页面标题">
  <meta name="twitter:description" content="页面描述">
  <meta name="twitter:image" content="https://example.com/image.jpg">
  
  <!-- 结构化数据 (Schema.org) -->
  <script type="application/ld+json">
  {
    "@context": "https://schema.org",
    "@type": "Article",
    "headline": "文章标题",
    "author": {
      "@type": "Person",
      "name": "作者名"
    },
    "datePublished": "2024-01-15"
  }
  </script>
</head>
<body>
  <!-- 使用语义化标签 -->
  <header>
    <h1>主标题</h1>
  </header>
  <main>
    <article>
      <h2>副标题</h2>
      <p>内容...</p>
    </article>
  </main>
</body>
</html>
```



---

## Part 2: CSS

## 6. CSS基础

### 6.1 CSS引入方式

```html
<!-- 1. 外部样式表（推荐） -->
<link rel="stylesheet" href="styles.css">

<!-- 2. 内部样式表 -->
<style>
  body {
    background-color: #f0f0f0;
  }
</style>

<!-- 3. 内联样式 -->
<p style="color: red; font-size: 16px;">内联样式</p>

<!-- 4. @import（不推荐，性能较差） -->
<style>
  @import url('styles.css');
</style>
```

### 6.2 CSS基本语法

```css
/* 选择器 {
  属性: 值;
  属性: 值;
} */

/* 注释 */

body {
  font-family: Arial, sans-serif;
  font-size: 16px;
  line-height: 1.6;
  color: #333;
  background-color: #fff;
  margin: 0;
  padding: 0;
}

/* 多个选择器 */
h1, h2, h3 {
  font-weight: bold;
  color: #2c3e50;
}

/* 属性简写 */
.box {
  /* margin: top right bottom left */
  margin: 10px 20px 10px 20px;
  /* margin: vertical horizontal */
  margin: 10px 20px;
  /* margin: all */
  margin: 10px;
  
  /* padding同理 */
  padding: 10px;
  
  /* border简写 */
  border: 1px solid #ccc;
  
  /* background简写 */
  background: #fff url('bg.jpg') no-repeat center center / cover;
  
  /* font简写 */
  font: italic bold 16px/1.5 Arial, sans-serif;
}
```

### 6.3 颜色表示

```css
.colors {
  /* 颜色名称 */
  color: red;
  
  /* 十六进制 */
  color: #ff0000;
  color: #f00;  /* 简写 */
  
  /* RGB */
  color: rgb(255, 0, 0);
  
  /* RGBA（带透明度） */
  color: rgba(255, 0, 0, 0.5);
  
  /* HSL */
  color: hsl(0, 100%, 50%);
  
  /* HSLA */
  color: hsla(0, 100%, 50%, 0.5);
  
  /* 透明 */
  color: transparent;
  
  /* 当前颜色 */
  border-color: currentColor;
}
```

---

## 7. 选择器

### 7.1 基本选择器

```css
/* 元素选择器 */
p {
  color: blue;
}

/* 类选择器 */
.className {
  font-size: 18px;
}

/* ID选择器 */
#id {
  background-color: yellow;
}

/* 通用选择器 */
* {
  box-sizing: border-box;
}

/* 属性选择器 */
[type="text"] {
  border: 1px solid #ccc;
}

[href^="https"] {  /* 以https开头 */
  color: green;
}

[href$=".pdf"] {  /* 以.pdf结尾 */
  color: red;
}

[href*="example"] {  /* 包含example */
  font-weight: bold;
}

[class~="btn"] {  /* 包含完整单词btn */
  padding: 10px;
}

[lang|="en"] {  /* 等于en或以en-开头 */
  color: blue;
}
```

### 7.2 组合选择器

```css
/* 后代选择器 */
div p {
  color: red;
}

/* 子选择器 */
div > p {
  color: blue;
}

/* 相邻兄弟选择器 */
h1 + p {
  margin-top: 0;
}

/* 通用兄弟选择器 */
h1 ~ p {
  color: gray;
}

/* 多类选择器 */
.class1.class2 {
  font-weight: bold;
}

/* 分组选择器 */
h1, h2, h3 {
  font-family: Arial;
}
```

### 7.3 伪类选择器

```css
/* 链接伪类 */
a:link {  /* 未访问 */
  color: blue;
}

a:visited {  /* 已访问 */
  color: purple;
}

a:hover {  /* 鼠标悬停 */
  color: red;
}

a:active {  /* 激活状态 */
  color: orange;
}

/* 焦点 */
input:focus {
  outline: 2px solid blue;
}

/* 结构伪类 */
li:first-child {  /* 第一个子元素 */
  font-weight: bold;
}

li:last-child {  /* 最后一个子元素 */
  color: red;
}

li:nth-child(2) {  /* 第2个子元素 */
  color: green;
}

li:nth-child(odd) {  /* 奇数 */
  background-color: #f0f0f0;
}

li:nth-child(even) {  /* 偶数 */
  background-color: #fff;
}

li:nth-child(3n) {  /* 3的倍数 */
  color: blue;
}

li:nth-last-child(2) {  /* 倒数第2个 */
  color: orange;
}

p:only-child {  /* 唯一子元素 */
  font-style: italic;
}

p:first-of-type {  /* 同类型第一个 */
  margin-top: 0;
}

/* 否定伪类 */
div:not(.excluded) {
  display: block;
}

/* 空元素 */
p:empty {
  display: none;
}

/* 表单伪类 */
input:checked {
  background-color: yellow;
}

input:disabled {
  opacity: 0.5;
}

input:enabled {
  background-color: white;
}

input:required {
  border-color: red;
}

input:optional {
  border-color: gray;
}

input:valid {
  border-color: green;
}

input:invalid {
  border-color: red;
}

input:in-range {
  border-color: green;
}

input:out-of-range {
  border-color: red;
}
```

### 7.4 伪元素选择器

```css
/* before和after */
.icon::before {
  content: "→ ";
  color: blue;
}

.box::after {
  content: "";
  display: table;
  clear: both;
}

/* 首字母 */
p::first-letter {
  font-size: 2em;
  font-weight: bold;
  float: left;
}

/* 首行 */
p::first-line {
  color: blue;
  text-transform: uppercase;
}

/* 选中文本 */
::selection {
  background-color: yellow;
  color: black;
}

/* 占位符 */
input::placeholder {
  color: #999;
  font-style: italic;
}
```

---

## 8. 盒模型

### 8.1 标准盒模型vs怪异盒模型

```css
/* 标准盒模型（默认） */
.standard-box {
  box-sizing: content-box;
  width: 200px;
  padding: 20px;
  border: 10px solid black;
  margin: 30px;
  
  /* 总宽度 = 200 + 20*2 + 10*2 = 260px */
}

/* 怪异盒模型（推荐） */
.border-box {
  box-sizing: border-box;
  width: 200px;
  padding: 20px;
  border: 10px solid black;
  
  /* 总宽度 = 200px */
  /* 内容区 = 200 - 20*2 - 10*2 = 140px */
}

/* 全局设置 */
* {
  box-sizing: border-box;
}
```

### 8.2 盒模型属性

```css
.box {
  /* 宽度 */
  width: 300px;
  min-width: 200px;
  max-width: 500px;
  
  /* 高度 */
  height: 200px;
  min-height: 100px;
  max-height: 400px;
  
  /* 内边距 */
  padding: 20px;
  padding-top: 10px;
  padding-right: 20px;
  padding-bottom: 10px;
  padding-left: 20px;
  
  /* 边框 */
  border: 1px solid #ccc;
  border-width: 1px;
  border-style: solid;  /* solid, dashed, dotted, double */
  border-color: #ccc;
  
  /* 单独设置边框 */
  border-top: 2px solid red;
  border-radius: 10px;
  border-radius: 10px 20px 30px 40px;  /* 左上 右上 右下 左下 */
  
  /* 外边距 */
  margin: 20px;
  margin-top: 10px;
  margin-right: auto;  /* 水平居中 */
  margin-bottom: 10px;
  margin-left: auto;
  
  /* 轮廓（不占空间） */
  outline: 2px solid blue;
  outline-offset: 5px;
}
```

### 8.3 显示类型

```css
/* 块级元素 */
.block {
  display: block;
  /* 独占一行，可设置宽高 */
}

/* 行内元素 */
.inline {
  display: inline;
  /* 不换行，不能设置宽高 */
}

/* 行内块元素 */
.inline-block {
  display: inline-block;
  /* 不换行，可设置宽高 */
}

/* 隐藏元素 */
.hidden {
  display: none;  /* 不占空间 */
  visibility: hidden;  /* 占空间但不可见 */
  opacity: 0;  /* 占空间，不可见，可交互 */
}
```

### 8.4 溢出处理

```css
.overflow {
  width: 300px;
  height: 200px;
  
  /* 溢出处理 */
  overflow: visible;  /* 默认，内容溢出可见 */
  overflow: hidden;   /* 隐藏溢出内容 */
  overflow: scroll;   /* 总是显示滚动条 */
  overflow: auto;     /* 需要时显示滚动条 */
  
  /* 分别设置 */
  overflow-x: hidden;
  overflow-y: scroll;
  
  /* 文本溢出省略号 */
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  
  /* 多行文本溢出 */
  display: -webkit-box;
  -webkit-line-clamp: 3;
  -webkit-box-orient: vertical;
  overflow: hidden;
}
```

---

## 9. 布局技术

### 9.1 定位（Position）

```css
/* 静态定位（默认） */
.static {
  position: static;
}

/* 相对定位 */
.relative {
  position: relative;
  top: 10px;
  left: 20px;
  /* 相对于原来位置偏移，原来位置仍占据空间 */
}

/* 绝对定位 */
.absolute {
  position: absolute;
  top: 0;
  right: 0;
  /* 相对于最近的非static祖先元素，脱离文档流 */
}

/* 固定定位 */
.fixed {
  position: fixed;
  bottom: 20px;
  right: 20px;
  /* 相对于视口固定，脱离文档流 */
}

/* 粘性定位 */
.sticky {
  position: sticky;
  top: 0;
  /* 滚动到阈值时固定 */
}

/* 层叠顺序 */
.z-index {
  position: relative;
  z-index: 10;
}

/* 居中定位 */
.center {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
}
```

### 9.2 浮动（Float）

```css
/* 浮动 */
.float-left {
  float: left;
  margin-right: 20px;
}

.float-right {
  float: right;
  margin-left: 20px;
}

/* 清除浮动 */
.clearfix::after {
  content: "";
  display: table;
  clear: both;
}

/* 或 */
.clear {
  clear: both;
}

/* 或使用overflow */
.container {
  overflow: auto;
}
```

### 9.3 多列布局

```css
.multi-column {
  column-count: 3;  /* 列数 */
  column-gap: 20px;  /* 列间距 */
  column-rule: 1px solid #ccc;  /* 列分隔线 */
  column-width: 200px;  /* 列宽 */
  
  /* 简写 */
  columns: 3 200px;
}

.column-span {
  column-span: all;  /* 跨越所有列 */
}
```

---

## 10. Flexbox布局

### 10.1 Flex容器属性

```css
.container {
  display: flex;  /* 或 inline-flex */
  
  /* 主轴方向 */
  flex-direction: row;  /* 默认，水平 */
  flex-direction: row-reverse;  /* 水平反向 */
  flex-direction: column;  /* 垂直 */
  flex-direction: column-reverse;  /* 垂直反向 */
  
  /* 换行 */
  flex-wrap: nowrap;  /* 默认，不换行 */
  flex-wrap: wrap;  /* 换行 */
  flex-wrap: wrap-reverse;  /* 反向换行 */
  
  /* 简写 */
  flex-flow: row wrap;
  
  /* 主轴对齐 */
  justify-content: flex-start;  /* 默认 */
  justify-content: flex-end;
  justify-content: center;
  justify-content: space-between;
  justify-content: space-around;
  justify-content: space-evenly;
  
  /* 交叉轴对齐 */
  align-items: stretch;  /* 默认，拉伸 */
  align-items: flex-start;
  align-items: flex-end;
  align-items: center;
  align-items: baseline;
  
  /* 多行对齐 */
  align-content: stretch;
  align-content: flex-start;
  align-content: flex-end;
  align-content: center;
  align-content: space-between;
  align-content: space-around;
  
  /* 间距（CSS Gap） */
  gap: 20px;
  row-gap: 20px;
  column-gap: 10px;
}
```

### 10.2 Flex项目属性

```css
.item {
  /* 放大比例 */
  flex-grow: 0;  /* 默认，不放大 */
  flex-grow: 1;  /* 放大 */
  
  /* 缩小比例 */
  flex-shrink: 1;  /* 默认，可缩小 */
  flex-shrink: 0;  /* 不缩小 */
  
  /* 基础大小 */
  flex-basis: auto;  /* 默认 */
  flex-basis: 200px;
  flex-basis: 30%;
  
  /* 简写 */
  flex: 0 1 auto;  /* 默认 */
  flex: 1;  /* flex: 1 1 0 */
  flex: auto;  /* flex: 1 1 auto */
  flex: none;  /* flex: 0 0 auto */
  
  /* 单独对齐 */
  align-self: auto;  /* 默认 */
  align-self: flex-start;
  align-self: flex-end;
  align-self: center;
  align-self: baseline;
  align-self: stretch;
  
  /* 顺序 */
  order: 0;  /* 默认 */
  order: 1;  /* 越小越靠前 */
}
```

### 10.3 Flexbox实战案例

```html
<style>
  /* 水平垂直居中 */
  .center {
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh;
  }
  
  /* 导航栏 */
  .navbar {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 1rem;
  }
  
  .navbar .logo {
    flex-shrink: 0;
  }
  
  .navbar .nav-links {
    display: flex;
    gap: 1rem;
    list-style: none;
  }
  
  /* 卡片布局 */
  .card-container {
    display: flex;
    flex-wrap: wrap;
    gap: 20px;
  }
  
  .card {
    flex: 1 1 300px;  /* 最小宽度300px */
    max-width: 400px;
  }
  
  /* 等高列 */
  .columns {
    display: flex;
  }
  
  .column {
    flex: 1;
    padding: 20px;
  }
  
  /* 圣杯布局 */
  .holy-grail {
    display: flex;
    flex-direction: column;
    min-height: 100vh;
  }
  
  .header, .footer {
    flex-shrink: 0;
  }
  
  .content {
    display: flex;
    flex: 1;
  }
  
  .main {
    flex: 1;
  }
  
  .sidebar {
    flex: 0 0 200px;
  }
</style>
```



---

## 11. Grid布局

### 11.1 Grid容器属性

```css
.container {
  display: grid;  /* 或 inline-grid */
  
  /* 定义列 */
  grid-template-columns: 200px 200px 200px;
  grid-template-columns: 1fr 2fr 1fr;  /* fr：fraction */
  grid-template-columns: repeat(3, 1fr);  /* 重复 */
  grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));  /* 自动填充 */
  grid-template-columns: 100px auto 100px;
  
  /* 定义行 */
  grid-template-rows: 100px 200px;
  grid-template-rows: repeat(2, 100px);
  
  /* 间距 */
  gap: 20px;  /* 行列间距 */
  row-gap: 20px;
  column-gap: 10px;
  
  /* 对齐内容 */
  justify-content: start;  /* 水平对齐：start, end, center, stretch, space-around, space-between, space-evenly */
  align-content: start;  /* 垂直对齐 */
  place-content: center;  /* justify-content和align-content简写 */
  
  /* 对齐项目 */
  justify-items: stretch;  /* 项目水平对齐 */
  align-items: stretch;  /* 项目垂直对齐 */
  place-items: center;  /* 简写 */
  
  /* 隐式网格 */
  grid-auto-rows: 100px;
  grid-auto-columns: 100px;
  grid-auto-flow: row;  /* row, column, dense */
}
```

### 11.2 Grid项目属性

```css
.item {
  /* 列位置 */
  grid-column-start: 1;
  grid-column-end: 3;
  grid-column: 1 / 3;  /* 简写 */
  grid-column: 1 / span 2;  /* 跨越2列 */
  
  /* 行位置 */
  grid-row-start: 1;
  grid-row-end: 3;
  grid-row: 1 / 3;
  grid-row: 1 / span 2;
  
  /* 区域 */
  grid-area: 1 / 1 / 3 / 3;  /* row-start / col-start / row-end / col-end */
  
  /* 对齐自身 */
  justify-self: start;  /* start, end, center, stretch */
  align-self: start;
  place-self: center;  /* 简写 */
}
```

### 11.3 Grid命名区域

```css
.container {
  display: grid;
  grid-template-areas:
    "header header header"
    "sidebar main main"
    "footer footer footer";
  grid-template-columns: 200px 1fr 1fr;
  grid-template-rows: 100px 1fr 100px;
  gap: 20px;
}

.header {
  grid-area: header;
}

.sidebar {
  grid-area: sidebar;
}

.main {
  grid-area: main;
}

.footer {
  grid-area: footer;
}
```

### 11.4 Grid实战案例

```html
<style>
  /* 响应式网格 */
  .card-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
    gap: 20px;
  }
  
  /* 圣杯布局 */
  .layout {
    display: grid;
    grid-template-areas:
      "header header header"
      "nav main aside"
      "footer footer footer";
    grid-template-columns: 200px 1fr 200px;
    grid-template-rows: auto 1fr auto;
    min-height: 100vh;
  }
  
  /* 复杂布局 */
  .dashboard {
    display: grid;
    grid-template-columns: repeat(12, 1fr);
    grid-template-rows: repeat(6, 100px);
    gap: 10px;
  }
  
  .widget-1 {
    grid-column: 1 / 7;
    grid-row: 1 / 3;
  }
  
  .widget-2 {
    grid-column: 7 / 13;
    grid-row: 1 / 4;
  }
  
  .widget-3 {
    grid-column: 1 / 5;
    grid-row: 3 / 7;
  }
  
  /* 居中对齐 */
  .center {
    display: grid;
    place-items: center;
    min-height: 100vh;
  }
</style>
```

---

## 12. 响应式设计

### 12.1 媒体查询

```css
/* 移动优先 */
.container {
  width: 100%;
  padding: 10px;
}

/* 平板 */
@media (min-width: 768px) {
  .container {
    width: 750px;
    margin: 0 auto;
    padding: 20px;
  }
}

/* 桌面 */
@media (min-width: 1024px) {
  .container {
    width: 960px;
  }
}

/* 大屏 */
@media (min-width: 1280px) {
  .container {
    width: 1200px;
  }
}

/* 媒体特性 */
@media (max-width: 600px) {
  /* 小屏幕 */
}

@media (orientation: landscape) {
  /* 横屏 */
}

@media (orientation: portrait) {
  /* 竖屏 */
}

@media (prefers-color-scheme: dark) {
  /* 深色模式 */
  body {
    background-color: #1a1a1a;
    color: #fff;
  }
}

@media (prefers-reduced-motion: reduce) {
  /* 减少动画 */
  * {
    animation: none !important;
    transition: none !important;
  }
}

@media print {
  /* 打印样式 */
  .no-print {
    display: none;
  }
}
```

### 12.2 响应式单位

```css
.responsive {
  /* 视口单位 */
  width: 100vw;  /* 视口宽度 */
  height: 100vh;  /* 视口高度 */
  font-size: 5vw;  /* 视口宽度的5% */
  
  /* 相对单位 */
  font-size: 1rem;  /* 相对于根元素字体大小 */
  padding: 2em;  /* 相对于当前元素字体大小 */
  width: 50%;  /* 相对于父元素 */
  
  /* 计算 */
  width: calc(100% - 40px);
  font-size: clamp(1rem, 2vw, 2rem);  /* 最小值, 首选值, 最大值 */
  
  /* 现代响应式字体 */
  font-size: clamp(1rem, 0.5rem + 2vw, 3rem);
}

/* 容器查询单位 */
@container (min-width: 400px) {
  .card {
    font-size: 1rem;
  }
}
```

### 12.3 响应式图片

```html
<!-- 响应式图片 -->
<img
  src="small.jpg"
  srcset="small.jpg 400w, medium.jpg 800w, large.jpg 1200w"
  sizes="(max-width: 600px) 400px, (max-width: 900px) 800px, 1200px"
  alt="响应式图片"
>

<!-- picture元素 -->
<picture>
  <source media="(min-width: 1024px)" srcset="large.jpg">
  <source media="(min-width: 768px)" srcset="medium.jpg">
  <img src="small.jpg" alt="图片">
</picture>

<style>
  /* CSS响应式图片 */
  img {
    max-width: 100%;
    height: auto;
  }
  
  /* 背景图片 */
  .hero {
    background-image: url('small.jpg');
  }
  
  @media (min-width: 768px) {
    .hero {
      background-image: url('large.jpg');
    }
  }
</style>
```

### 12.4 响应式布局模式

```css
/* 堆叠布局 */
.stack {
  display: flex;
  flex-direction: column;
}

@media (min-width: 768px) {
  .stack {
    flex-direction: row;
  }
}

/* 侧边栏布局 */
.sidebar-layout {
  display: grid;
  gap: 20px;
}

@media (min-width: 768px) {
  .sidebar-layout {
    grid-template-columns: 250px 1fr;
  }
}

/* Pancake布局 */
.pancake {
  display: flex;
  flex-wrap: wrap;
}

.pancake > * {
  flex: 1 1 250px;
}

/* Holy Grail布局 */
.holy-grail {
  display: grid;
  grid-template-rows: auto 1fr auto;
  min-height: 100vh;
}

@media (min-width: 768px) {
  .holy-grail-content {
    display: grid;
    grid-template-columns: 200px 1fr 200px;
  }
}
```

---

## 13. CSS动画

### 13.1 过渡（Transition）

```css
.button {
  background-color: blue;
  color: white;
  padding: 10px 20px;
  border: none;
  cursor: pointer;
  
  /* 过渡 */
  transition: background-color 0.3s ease;
  
  /* 多个属性 */
  transition: background-color 0.3s ease, transform 0.2s;
  
  /* 所有属性 */
  transition: all 0.3s ease;
  
  /* 延迟 */
  transition: background-color 0.3s ease 0.1s;
}

.button:hover {
  background-color: darkblue;
  transform: scale(1.05);
}

/* 过渡属性详解 */
.box {
  transition-property: background-color, transform;
  transition-duration: 0.3s, 0.5s;
  transition-timing-function: ease, ease-in-out;
  transition-delay: 0s, 0.1s;
}

/* 时间函数 */
.timing-functions {
  transition-timing-function: ease;  /* 慢-快-慢 */
  transition-timing-function: linear;  /* 匀速 */
  transition-timing-function: ease-in;  /* 慢-快 */
  transition-timing-function: ease-out;  /* 快-慢 */
  transition-timing-function: ease-in-out;  /* 慢-快-慢 */
  transition-timing-function: cubic-bezier(0.42, 0, 0.58, 1);  /* 贝塞尔曲线 */
}
```

### 13.2 动画（Animation）

```css
/* 定义关键帧 */
@keyframes fadeIn {
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
}

/* 百分比形式 */
@keyframes slide {
  0% {
    transform: translateX(0);
  }
  50% {
    transform: translateX(100px);
  }
  100% {
    transform: translateX(0);
  }
}

/* 使用动画 */
.animated {
  animation: fadeIn 1s ease-in-out;
  
  /* 详细属性 */
  animation-name: fadeIn;
  animation-duration: 1s;
  animation-timing-function: ease-in-out;
  animation-delay: 0.5s;
  animation-iteration-count: infinite;  /* 无限循环 */
  animation-direction: alternate;  /* normal, reverse, alternate, alternate-reverse */
  animation-fill-mode: forwards;  /* none, forwards, backwards, both */
  animation-play-state: running;  /* running, paused */
}

/* 多个动画 */
.multi-animation {
  animation: fadeIn 1s, slide 2s;
}
```

### 13.3 常用动画效果

```css
/* 淡入淡出 */
@keyframes fadeIn {
  from { opacity: 0; }
  to { opacity: 1; }
}

@keyframes fadeOut {
  from { opacity: 1; }
  to { opacity: 0; }
}

/* 滑入滑出 */
@keyframes slideIn {
  from { transform: translateX(-100%); }
  to { transform: translateX(0); }
}

/* 缩放 */
@keyframes zoomIn {
  from {
    opacity: 0;
    transform: scale(0.5);
  }
  to {
    opacity: 1;
    transform: scale(1);
  }
}

/* 旋转 */
@keyframes rotate {
  from { transform: rotate(0deg); }
  to { transform: rotate(360deg); }
}

/* 弹跳 */
@keyframes bounce {
  0%, 20%, 50%, 80%, 100% {
    transform: translateY(0);
  }
  40% {
    transform: translateY(-30px);
  }
  60% {
    transform: translateY(-15px);
  }
}

/* 脉冲 */
@keyframes pulse {
  0%, 100% {
    transform: scale(1);
  }
  50% {
    transform: scale(1.1);
  }
}

/* 闪烁 */
@keyframes blink {
  0%, 50%, 100% {
    opacity: 1;
  }
  25%, 75% {
    opacity: 0;
  }
}

/* 加载动画 */
@keyframes spin {
  to { transform: rotate(360deg); }
}

.loader {
  border: 4px solid #f3f3f3;
  border-top: 4px solid #3498db;
  border-radius: 50%;
  width: 40px;
  height: 40px;
  animation: spin 1s linear infinite;
}
```

### 13.4 Transform变换

```css
.transform {
  /* 2D变换 */
  transform: translate(50px, 100px);
  transform: translateX(50px);
  transform: translateY(100px);
  transform: scale(1.5);
  transform: scaleX(1.5);
  transform: scaleY(0.5);
  transform: rotate(45deg);
  transform: skew(10deg, 20deg);
  transform: skewX(10deg);
  transform: skewY(20deg);
  
  /* 组合变换 */
  transform: translate(50px, 100px) rotate(45deg) scale(1.2);
  
  /* 变换原点 */
  transform-origin: center;  /* 默认 */
  transform-origin: top left;
  transform-origin: 50% 50%;
  transform-origin: 0 0;
  
  /* 3D变换 */
  transform: translateZ(100px);
  transform: translate3d(50px, 100px, 50px);
  transform: rotateX(45deg);
  transform: rotateY(45deg);
  transform: rotateZ(45deg);
  transform: rotate3d(1, 1, 1, 45deg);
  transform: perspective(500px);
  
  /* 3D变换样式 */
  transform-style: preserve-3d;
  perspective: 1000px;
  perspective-origin: center;
  backface-visibility: hidden;
}

/* 3D卡片翻转 */
.card {
  position: relative;
  width: 300px;
  height: 400px;
  transform-style: preserve-3d;
  transition: transform 0.6s;
}

.card:hover {
  transform: rotateY(180deg);
}

.card-front, .card-back {
  position: absolute;
  width: 100%;
  height: 100%;
  backface-visibility: hidden;
}

.card-back {
  transform: rotateY(180deg);
}
```

---

## 14. CSS预处理器

### 14.1 Sass/SCSS

```scss
// 变量
$primary-color: #3498db;
$font-stack: Arial, sans-serif;
$base-padding: 10px;

body {
  font-family: $font-stack;
  color: $primary-color;
  padding: $base-padding;
}

// 嵌套
.nav {
  ul {
    margin: 0;
    padding: 0;
    list-style: none;
  }
  
  li {
    display: inline-block;
  }
  
  a {
    text-decoration: none;
    color: $primary-color;
    
    &:hover {  // & 表示父选择器
      color: darken($primary-color, 10%);
    }
  }
}

// Mixin（混入）
@mixin border-radius($radius) {
  -webkit-border-radius: $radius;
  -moz-border-radius: $radius;
  border-radius: $radius;
}

.box {
  @include border-radius(10px);
}

// 带默认值的mixin
@mixin box-shadow($x: 0, $y: 0, $blur: 5px, $color: rgba(0,0,0,0.3)) {
  box-shadow: $x $y $blur $color;
}

// 继承
.message {
  border: 1px solid #ccc;
  padding: 10px;
  color: #333;
}

.success {
  @extend .message;
  border-color: green;
}

.error {
  @extend .message;
  border-color: red;
}

// 运算
.container {
  width: 100% - 40px;
  padding: $base-padding * 2;
  margin: $base-padding / 2;
}

// 函数
@function calculate-rem($size) {
  @return $size / 16px * 1rem;
}

.text {
  font-size: calculate-rem(18px);
}

// 条件语句
@mixin theme($theme) {
  @if $theme == dark {
    background-color: #000;
    color: #fff;
  } @else {
    background-color: #fff;
    color: #000;
  }
}

// 循环
@for $i from 1 through 3 {
  .col-#{$i} {
    width: 100% / $i;
  }
}

@each $color in red, green, blue {
  .text-#{$color} {
    color: $color;
  }
}

// 导入
@import 'variables';
@import 'mixins';
@import 'base';
```

### 14.2 Less

```less
// 变量
@primary-color: #3498db;
@font-stack: Arial, sans-serif;

body {
  font-family: @font-stack;
  color: @primary-color;
}

// 嵌套
.nav {
  ul {
    list-style: none;
  }
  
  li {
    display: inline-block;
  }
  
  a {
    color: @primary-color;
    
    &:hover {
      color: darken(@primary-color, 10%);
    }
  }
}

// Mixin
.border-radius(@radius: 5px) {
  -webkit-border-radius: @radius;
  -moz-border-radius: @radius;
  border-radius: @radius;
}

.box {
  .border-radius(10px);
}

// 运算
@base-padding: 10px;

.container {
  padding: @base-padding * 2;
  width: 100% - 40px;
}

// 函数
.lighten-text {
  color: lighten(@primary-color, 20%);
}

// 导入
@import "variables.less";
```

---

## 15. 现代CSS特性

### 15.1 CSS变量

```css
:root {
  /* 定义全局变量 */
  --primary-color: #3498db;
  --secondary-color: #2ecc71;
  --font-size: 16px;
  --spacing: 20px;
}

/* 使用变量 */
.button {
  background-color: var(--primary-color);
  font-size: var(--font-size);
  padding: var(--spacing);
}

/* 局部变量 */
.dark-theme {
  --primary-color: #1a1a1a;
  --text-color: #fff;
}

/* 带回退值 */
.box {
  color: var(--text-color, #000);
}

/* JavaScript操作变量 */
```

```html
<script>
  // 获取变量
  const root = document.documentElement;
  const primaryColor = getComputedStyle(root).getPropertyValue('--primary-color');
  
  // 设置变量
  root.style.setProperty('--primary-color', '#ff0000');
</script>
```

### 15.2 Container Queries

```css
/* 容器查询 */
.container {
  container-type: inline-size;
  container-name: card;
}

@container card (min-width: 400px) {
  .card-content {
    display: grid;
    grid-template-columns: 1fr 1fr;
  }
}
```

### 15.3 Logical Properties

```css
/* 传统属性 */
.traditional {
  margin-left: 10px;
  padding-right: 20px;
  border-top: 1px solid black;
}

/* 逻辑属性（支持RTL） */
.logical {
  margin-inline-start: 10px;  /* LTR: left, RTL: right */
  padding-inline-end: 20px;   /* LTR: right, RTL: left */
  border-block-start: 1px solid black;  /* 顶部 */
  
  /* 其他逻辑属性 */
  inset-inline-start: 0;  /* left/right */
  inset-block-start: 0;   /* top/bottom */
  block-size: 100px;      /* height */
  inline-size: 100%;      /* width */
}
```

### 15.4 Aspect Ratio

```css
.video-container {
  aspect-ratio: 16 / 9;
  width: 100%;
}

.square {
  aspect-ratio: 1;
  width: 200px;
}
```

### 15.5 现代选择器

```css
/* :is() 选择器 */
:is(h1, h2, h3) {
  color: blue;
}

/* :where() 选择器（0特异性） */
:where(h1, h2, h3) {
  color: blue;
}

/* :has() 父选择器 */
div:has(> p) {
  background-color: yellow;
}

/* :not() 增强 */
button:not(.primary, .secondary) {
  background-color: gray;
}
```

### 15.6 其他现代特性

```css
/* Scroll Snap */
.scroll-container {
  scroll-snap-type: x mandatory;
  overflow-x: scroll;
}

.scroll-item {
  scroll-snap-align: start;
}

/* Backdrop Filter */
.glass {
  backdrop-filter: blur(10px);
  background-color: rgba(255, 255, 255, 0.5);
}

/* Mix Blend Mode */
.blend {
  mix-blend-mode: multiply;
}

/* Clip Path */
.clip {
  clip-path: circle(50%);
  clip-path: polygon(50% 0%, 100% 50%, 50% 100%, 0% 50%);
}

/* Object Fit */
img {
  object-fit: cover;  /* cover, contain, fill, none, scale-down */
  object-position: center;
}

/* Writing Mode */
.vertical {
  writing-mode: vertical-rl;
}

/* Filter */
.filtered {
  filter: blur(5px);
  filter: brightness(1.5);
  filter: contrast(200%);
  filter: grayscale(100%);
  filter: hue-rotate(90deg);
  filter: invert(100%);
  filter: saturate(200%);
  filter: sepia(100%);
  filter: drop-shadow(2px 2px 4px rgba(0,0,0,0.5));
  
  /* 组合 */
  filter: blur(2px) brightness(1.2) contrast(110%);
}
```

---

## 总结与最佳实践

### CSS最佳实践

```css
/**
 * 1. 组织结构
 *    - 使用一致的命名规范（BEM、OOCSS等）
 *    - 按功能组织样式文件
 *    - 使用注释分隔不同部分
 *
 * 2. 性能优化
 *    - 减少选择器复杂度
 *    - 避免过度嵌套
 *    - 使用CSS变量减少重复
 *    - 压缩CSS文件
 *
 * 3. 可维护性
 *    - 使用语义化类名
 *    - 避免使用!important
 *    - 保持样式简洁
 *    - 使用预处理器组织代码
 *
 * 4. 响应式设计
 *    - 移动优先
 *    - 使用相对单位
 *    - 测试多种设备
 *
 * 5. 可访问性
 *    - 保证足够的对比度
 *    - 支持键盘导航
 *    - 使用语义化HTML
 *    - 测试屏幕阅读器
 */

/* BEM命名规范 */
.block {}
.block__element {}
.block--modifier {}
.block__element--modifier {}

/* 示例 */
.card {}
.card__title {}
.card__content {}
.card--featured {}
.card__title--large {}
```

### 常用CSS工具

```
1. 重置样式：
   - Normalize.css
   - Reset.css

2. CSS框架：
   - Bootstrap
   - Tailwind CSS
   - Bulma

3. 预处理器：
   - Sass/SCSS
   - Less
   - Stylus

4. 后处理器：
   - PostCSS
   - Autoprefixer

5. 工具库：
   - Animate.css
   - Hover.css
```

本文档全面介绍了HTML和CSS从基础到高级的所有核心知识点，掌握这些内容将使您能够创建现代、响应式、高性能的Web页面。

**学习建议**：
1. 从基础开始，循序渐进
2. 多实践，多写代码
3. 参考优秀网站学习布局
4. 关注Web标准和最新特性
5. 注重可访问性和性能优化

祝您学习愉快！🎨
