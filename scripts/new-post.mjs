import { mkdir, writeFile } from 'node:fs/promises';
import { resolve } from 'node:path';
const [topic, slug, title] = process.argv.slice(2);
const topics = { cpp: 'C++', java: 'Java', frontend: '前端', python: 'Python', unreal: 'Unreal Engine', ai: 'AI 应用', cli: '命令行' };
if (!topics[topic] || !/^[a-z0-9]+(?:-[a-z0-9]+)*$/.test(slug ?? '') || !title?.trim()) {
  console.error('用法：npm run post:new -- <cpp|java|frontend|python|unreal|ai|cli> <英文短横线文件名> "文章标题"');
  process.exit(1);
}
const folder = resolve('src/posts', topic);
await mkdir(folder, { recursive: true });
const file = resolve(folder, `${slug}.md`);
const date = new Intl.DateTimeFormat('en-CA', { timeZone: 'Asia/Shanghai', year: 'numeric', month: '2-digit', day: '2-digit' }).format(new Date());
await writeFile(file, `---\ntitle: ${JSON.stringify(title)}\ndate: ${date}\ncategory:\n  - ${topics[topic]}\ntag:\n  - ${topics[topic]}\n---\n\n# ${title}\n\n一句话说明本文解决的问题。\n\n<!-- more -->\n\n## 适用环境\n\n说明版本与前置知识。\n\n## 实现步骤\n\n## 验证结果\n\n## 参考资料\n`, { flag: 'wx' });
console.log(`已创建：${file}\n写完后，在 src/posts/README.md 对应主题下添加链接。`);
