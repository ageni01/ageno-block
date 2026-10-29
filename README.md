# ageno 的博客

基于 VuePress 2 + Theme Hope，Markdown 存在 `src/posts/`。推送到 `main` 后，GitHub Actions 构建并发布到 `blog-pages`。

## 日常使用

首次安装 Node.js 22，然后在仓库目录运行：

```bash
npm ci
npm run dev
```

打开终端显示的本地地址，修改 Markdown 后浏览器会更新。停止预览按 Ctrl+C。

新建文章：

```bash
npm run post:new -- java thread-pool "Java 线程池实践"
```

写完后在 `src/posts/README.md` 对应主题添加链接，然后检查并发布：

```bash
npm run build
git status
git add src
git commit -m "docs: 添加线程池实践"
git push origin main
```

提交前检查暂存区只包含本次要发布的文件。首次发布本次配置、脚本等变更时，也需明确暂存这些文件。推送后在 GitHub 仓库的 Actions 页面确认部署成功；推送成功不代表部署成功。

## 文件在哪里

| 路径 | 用途 |
| --- | --- |
| `src/posts/` | 正式文章与主题导航 |
| `src/README.md` | 博客首页 |
| `src/intro.md` | 关于页 |
| `src/.vuepress/public/` | 图片等静态资源 |
| `src/.vuepress/` | 站点配置、导航、样式 |
| `src/demo/` | 主题语法参考，已从主导航和文章流移除 |
| `scripts/` | 写作辅助命令 |
| `docs/` | 维护和写作规范，不作为博客文章发布 |

详细规则见 [写作规范](docs/WRITING.md) 和 [本次整理说明](docs/ORGANIZATION.md)。
