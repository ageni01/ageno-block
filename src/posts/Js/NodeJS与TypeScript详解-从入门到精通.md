# Node.js与TypeScript详解：从入门到精通

> 全面深入理解Node.js运行时和TypeScript类型系统，掌握现代JavaScript开发

---

## 目录

### Part 1: Node.js
1. [Node.js基础](#1-nodejs基础)
2. [模块系统](#2-模块系统)
3. [核心模块](#3-核心模块)
4. [异步编程](#4-异步编程)
5. [Express框架](#5-express框架)
6. [数据库操作](#6-数据库操作)
7. [认证与安全](#7-认证与安全)

### Part 2: TypeScript
8. [TypeScript基础](#8-typescript基础)
9. [类型系统](#9-类型系统)
10. [高级类型](#10-高级类型)
11. [泛型](#11-泛型)
12. [装饰器](#12-装饰器)
13. [TypeScript配置](#13-typescript配置)
14. [实战项目](#14-实战项目)

---

## Part 1: Node.js

## 1. Node.js基础

### 1.1 什么是Node.js

```
Node.js是什么：
- JavaScript运行时环境（基于Chrome V8引擎）
- 事件驱动、非阻塞I/O模型
- 适合构建高并发、I/O密集型应用

特点：
✓ 单线程事件循环
✓ 异步非阻塞
✓ 跨平台
✓ NPM生态系统
✓ 高性能

应用场景：
- Web服务器（API、微服务）
- 实时应用（聊天、游戏）
- 命令行工具
- 构建工具（Webpack、Vite）
- 桌面应用（Electron）
```

### 1.2 安装与环境配置

```bash
# 使用nvm安装（推荐）
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.0/install.sh | bash

# 安装Node.js
nvm install 18
nvm use 18
nvm alias default 18

# 验证安装
node --version
npm --version

# 全局工具
npm install -g typescript
npm install -g ts-node
npm install -g nodemon
npm install -g pnpm
```

### 1.3 第一个Node.js程序

```javascript
// hello.js
console.log('Hello, Node.js!');

// 运行
// node hello.js
```

```javascript
// server.js - 简单的HTTP服务器
const http = require('http');

const server = http.createServer((req, res) => {
  res.writeHead(200, { 'Content-Type': 'text/plain' });
  res.end('Hello, World!\n');
});

const PORT = 3000;
server.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}/`);
});
```

### 1.4 REPL环境

```bash
# 启动REPL
node

# 常用命令
.help      # 帮助
.break     # 退出多行表达式
.clear     # 重置上下文
.exit      # 退出REPL
.save      # 保存当前会话
.load      # 加载文件
```

---

## 2. 模块系统

### 2.1 CommonJS模块（Node.js默认）

```javascript
// math.js - 导出模块
function add(a, b) {
  return a + b;
}

function subtract(a, b) {
  return a - b;
}

// 导出方式1：exports对象
exports.add = add;
exports.subtract = subtract;

// 导出方式2：module.exports
module.exports = {
  add,
  subtract,
  multiply: (a, b) => a * b,
};

// 导出方式3：单个值
module.exports = class Calculator {
  add(a, b) {
    return a + b;
  }
};
```

```javascript
// app.js - 导入模块
// 导入方式1：解构
const { add, subtract } = require('./math');
console.log(add(5, 3));

// 导入方式2：整体导入
const math = require('./math');
console.log(math.add(5, 3));

// 导入核心模块
const fs = require('fs');
const path = require('path');

// 导入第三方模块
const express = require('express');
const axios = require('axios');
```

### 2.2 ES6模块（ESM）

```javascript
// math.mjs 或在package.json中设置"type": "module"

// 命名导出
export function add(a, b) {
  return a + b;
}

export function subtract(a, b) {
  return a - b;
}

export const PI = 3.14159;

// 默认导出
export default class Calculator {
  add(a, b) {
    return a + b;
  }
}
```

```javascript
// app.mjs - 导入ES6模块

// 导入命名导出
import { add, subtract, PI } from './math.mjs';

// 导入默认导出
import Calculator from './math.mjs';

// 导入所有
import * as math from './math.mjs';

// 动态导入
const module = await import('./math.mjs');
```

**package.json配置**：
```json
{
  "name": "my-project",
  "version": "1.0.0",
  "type": "module",  // 使用ES6模块
  "main": "index.js",
  "scripts": {
    "start": "node index.js",
    "dev": "nodemon index.js"
  },
  "dependencies": {
    "express": "^4.18.0"
  }
}
```

---

## 3. 核心模块

### 3.1 文件系统（fs）

```javascript
const fs = require('fs');
const fsPromises = require('fs').promises;

// ===== 同步读取 =====
try {
  const data = fs.readFileSync('file.txt', 'utf8');
  console.log(data);
} catch (err) {
  console.error(err);
}

// ===== 异步回调 =====
fs.readFile('file.txt', 'utf8', (err, data) => {
  if (err) {
    console.error(err);
    return;
  }
  console.log(data);
});

// ===== Promise方式 =====
fsPromises.readFile('file.txt', 'utf8')
  .then(data => console.log(data))
  .catch(err => console.error(err));

// ===== async/await =====
async function readFile() {
  try {
    const data = await fsPromises.readFile('file.txt', 'utf8');
    console.log(data);
  } catch (err) {
    console.error(err);
  }
}

// ===== 写入文件 =====
// 覆盖写入
await fsPromises.writeFile('output.txt', 'Hello, World!');

// 追加写入
await fsPromises.appendFile('output.txt', '\nNew line');

// ===== 文件操作 =====
// 检查文件是否存在
const exists = fs.existsSync('file.txt');

// 获取文件信息
const stats = await fsPromises.stat('file.txt');
console.log({
  isFile: stats.isFile(),
  isDirectory: stats.isDirectory(),
  size: stats.size,
  created: stats.birthtime,
  modified: stats.mtime,
});

// 复制文件
await fsPromises.copyFile('source.txt', 'dest.txt');

// 删除文件
await fsPromises.unlink('file.txt');

// 重命名
await fsPromises.rename('old.txt', 'new.txt');

// ===== 目录操作 =====
// 创建目录
await fsPromises.mkdir('newdir', { recursive: true });

// 读取目录
const files = await fsPromises.readdir('.');
console.log(files);

// 删除目录
await fsPromises.rmdir('newdir', { recursive: true });

// ===== 流式读写（处理大文件）=====
const readStream = fs.createReadStream('large.txt', 'utf8');
const writeStream = fs.createWriteStream('output.txt');

readStream.on('data', (chunk) => {
  console.log('Received chunk:', chunk.length);
  writeStream.write(chunk);
});

readStream.on('end', () => {
  console.log('Finished reading');
  writeStream.end();
});

// 使用pipe
fs.createReadStream('input.txt')
  .pipe(fs.createWriteStream('output.txt'));
```

### 3.2 路径（path）

```javascript
const path = require('path');

// 路径拼接
const fullPath = path.join(__dirname, 'files', 'data.txt');
// /Users/user/project/files/data.txt

// 解析路径
const resolved = path.resolve('files', 'data.txt');
// 转换为绝对路径

// 路径信息
const filePath = '/home/user/file.txt';
console.log({
  dirname: path.dirname(filePath),    // /home/user
  basename: path.basename(filePath),  // file.txt
  extname: path.extname(filePath),    // .txt
  parse: path.parse(filePath),
  // {
  //   root: '/',
  //   dir: '/home/user',
  //   base: 'file.txt',
  //   ext: '.txt',
  //   name: 'file'
  // }
});

// 规范化路径
const normalized = path.normalize('/home//user/../file.txt');
// /home/file.txt

// 相对路径
const relative = path.relative('/home/user', '/home/user/files/data.txt');
// files/data.txt

// 判断绝对路径
const isAbsolute = path.isAbsolute('/home/user');  // true
```

### 3.3 HTTP模块

```javascript
const http = require('http');

// ===== 创建服务器 =====
const server = http.createServer((req, res) => {
  // 请求信息
  console.log({
    method: req.method,
    url: req.url,
    headers: req.headers,
  });
  
  // 路由
  if (req.url === '/') {
    res.writeHead(200, { 'Content-Type': 'text/html' });
    res.end('<h1>Home Page</h1>');
  } else if (req.url === '/api/data') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ message: 'Hello API' }));
  } else {
    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end('404 Not Found');
  }
});

server.listen(3000, () => {
  console.log('Server running on port 3000');
});

// ===== 处理POST请求 =====
const server2 = http.createServer((req, res) => {
  if (req.method === 'POST') {
    let body = '';
    
    req.on('data', chunk => {
      body += chunk.toString();
    });
    
    req.on('end', () => {
      console.log('Received:', body);
      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end(JSON.stringify({ received: body }));
    });
  }
});

// ===== 发送HTTP请求 =====
const options = {
  hostname: 'api.example.com',
  port: 80,
  path: '/data',
  method: 'GET',
};

const req = http.request(options, (res) => {
  let data = '';
  
  res.on('data', (chunk) => {
    data += chunk;
  });
  
  res.on('end', () => {
    console.log(data);
  });
});

req.on('error', (error) => {
  console.error(error);
});

req.end();
```

### 3.4 事件（events）

```javascript
const EventEmitter = require('events');

// 创建事件发射器
class MyEmitter extends EventEmitter {}
const myEmitter = new MyEmitter();

// 监听事件
myEmitter.on('event', (arg1, arg2) => {
  console.log('Event occurred:', arg1, arg2);
});

// 一次性监听
myEmitter.once('once-event', () => {
  console.log('This will only fire once');
});

// 触发事件
myEmitter.emit('event', 'arg1', 'arg2');
myEmitter.emit('once-event');
myEmitter.emit('once-event');  // 不会触发

// 移除监听器
const listener = () => console.log('Event');
myEmitter.on('event', listener);
myEmitter.removeListener('event', listener);

// 实际应用：自定义事件
class Logger extends EventEmitter {
  log(message) {
    console.log(message);
    this.emit('logged', { message, timestamp: new Date() });
  }
}

const logger = new Logger();
logger.on('logged', (data) => {
  console.log('Log event:', data);
});

logger.log('Hello, World!');
```

### 3.5 进程（process）

```javascript
// 环境变量
console.log(process.env.NODE_ENV);
console.log(process.env.PORT);

// 命令行参数
// node app.js arg1 arg2
console.log(process.argv);
// ['node', '/path/to/app.js', 'arg1', 'arg2']

// 当前工作目录
console.log(process.cwd());

// 进程信息
console.log({
  pid: process.pid,
  platform: process.platform,
  version: process.version,
  uptime: process.uptime(),
  memoryUsage: process.memoryUsage(),
});

// 退出进程
process.exit(0);  // 成功退出
process.exit(1);  // 错误退出

// 监听退出
process.on('exit', (code) => {
  console.log(`Process exiting with code: ${code}`);
});

// 监听未捕获的异常
process.on('uncaughtException', (error) => {
  console.error('Uncaught Exception:', error);
  process.exit(1);
});

// 监听未处理的Promise拒绝
process.on('unhandledRejection', (reason, promise) => {
  console.error('Unhandled Rejection:', reason);
});
```

### 3.6 Buffer

```javascript
// 创建Buffer
const buf1 = Buffer.from('Hello');
const buf2 = Buffer.from([72, 101, 108, 108, 111]);
const buf3 = Buffer.alloc(10);  // 分配10字节，初始化为0
const buf4 = Buffer.allocUnsafe(10);  // 更快但不初始化

// 读写Buffer
console.log(buf1.toString());  // 'Hello'
console.log(buf1.toString('hex'));  // '48656c6c6f'
console.log(buf1[0]);  // 72 (H的ASCII码)

buf3.write('Hello');
console.log(buf3.toString());

// Buffer操作
const buf5 = Buffer.concat([buf1, buf2]);
const sliced = buf1.slice(0, 3);
const copied = Buffer.from(buf1);

// 比较Buffer
console.log(buf1.equals(buf2));  // true
console.log(Buffer.compare(buf1, buf2));  // 0

// JSON转换
const json = JSON.stringify(buf1);
const parsed = Buffer.from(JSON.parse(json));
```

---

## 4. 异步编程

### 4.1 回调函数

```javascript
const fs = require('fs');

// 回调地狱（Callback Hell）
fs.readFile('file1.txt', 'utf8', (err, data1) => {
  if (err) {
    console.error(err);
    return;
  }
  
  fs.readFile('file2.txt', 'utf8', (err, data2) => {
    if (err) {
      console.error(err);
      return;
    }
    
    fs.readFile('file3.txt', 'utf8', (err, data3) => {
      if (err) {
        console.error(err);
        return;
      }
      
      console.log(data1, data2, data3);
    });
  });
});
```

### 4.2 Promise

```javascript
const fs = require('fs').promises;

// Promise链
fs.readFile('file1.txt', 'utf8')
  .then(data1 => {
    console.log(data1);
    return fs.readFile('file2.txt', 'utf8');
  })
  .then(data2 => {
    console.log(data2);
    return fs.readFile('file3.txt', 'utf8');
  })
  .then(data3 => {
    console.log(data3);
  })
  .catch(err => {
    console.error(err);
  })
  .finally(() => {
    console.log('Done');
  });

// 创建Promise
function delay(ms) {
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      resolve(`Waited ${ms}ms`);
    }, ms);
  });
}

// Promise.all - 并行执行
Promise.all([
  fs.readFile('file1.txt', 'utf8'),
  fs.readFile('file2.txt', 'utf8'),
  fs.readFile('file3.txt', 'utf8'),
])
  .then(([data1, data2, data3]) => {
    console.log(data1, data2, data3);
  })
  .catch(err => {
    console.error(err);
  });

// Promise.race - 返回最快完成的
Promise.race([
  delay(1000),
  delay(2000),
  delay(3000),
])
  .then(result => {
    console.log(result);  // 'Waited 1000ms'
  });

// Promise.allSettled - 等待所有完成（无论成功失败）
Promise.allSettled([
  Promise.resolve('Success'),
  Promise.reject('Error'),
  delay(1000),
])
  .then(results => {
    console.log(results);
    // [
    //   { status: 'fulfilled', value: 'Success' },
    //   { status: 'rejected', reason: 'Error' },
    //   { status: 'fulfilled', value: 'Waited 1000ms' }
    // ]
  });
```

### 4.3 async/await

```javascript
const fs = require('fs').promises;

// async函数
async function readFiles() {
  try {
    const data1 = await fs.readFile('file1.txt', 'utf8');
    console.log(data1);
    
    const data2 = await fs.readFile('file2.txt', 'utf8');
    console.log(data2);
    
    const data3 = await fs.readFile('file3.txt', 'utf8');
    console.log(data3);
    
    return 'All files read';
  } catch (err) {
    console.error(err);
    throw err;
  }
}

// 并行执行
async function readFilesParallel() {
  try {
    const [data1, data2, data3] = await Promise.all([
      fs.readFile('file1.txt', 'utf8'),
      fs.readFile('file2.txt', 'utf8'),
      fs.readFile('file3.txt', 'utf8'),
    ]);
    
    console.log(data1, data2, data3);
  } catch (err) {
    console.error(err);
  }
}

// 顶层await（ES2022，需要ESM）
// const data = await fs.readFile('file.txt', 'utf8');

// 错误处理
async function handleErrors() {
  try {
    const data = await fs.readFile('nonexistent.txt', 'utf8');
  } catch (err) {
    if (err.code === 'ENOENT') {
      console.log('File not found');
    } else {
      throw err;
    }
  }
}

// 循环中的async/await
async function processFiles(files) {
  // 串行处理
  for (const file of files) {
    const data = await fs.readFile(file, 'utf8');
    console.log(data);
  }
  
  // 并行处理
  const promises = files.map(file => fs.readFile(file, 'utf8'));
  const results = await Promise.all(promises);
  console.log(results);
}
```

### 4.4 事件循环

```javascript
/**
 * Node.js事件循环阶段：
 * 
 * 1. timers：执行setTimeout和setInterval回调
 * 2. pending callbacks：执行延迟到下一个循环的I/O回调
 * 3. idle, prepare：内部使用
 * 4. poll：检索新的I/O事件，执行I/O回调
 * 5. check：执行setImmediate回调
 * 6. close callbacks：执行关闭事件回调
 */

console.log('1: Start');

setTimeout(() => {
  console.log('2: setTimeout');
}, 0);

setImmediate(() => {
  console.log('3: setImmediate');
});

process.nextTick(() => {
  console.log('4: process.nextTick');
});

Promise.resolve().then(() => {
  console.log('5: Promise');
});

console.log('6: End');

// 输出顺序：
// 1: Start
// 6: End
// 4: process.nextTick  (微任务，最高优先级)
// 5: Promise           (微任务)
// 2: setTimeout        (宏任务)
// 3: setImmediate      (宏任务)
```



---

## 5. Express框架

### 5.1 Express基础

```javascript
const express = require('express');
const app = express();

// 中间件
app.use(express.json());  // 解析JSON
app.use(express.urlencoded({ extended: true }));  // 解析URL编码

// 静态文件
app.use(express.static('public'));

// 基本路由
app.get('/', (req, res) => {
  res.send('Hello, Express!');
});

app.post('/api/users', (req, res) => {
  const user = req.body;
  res.status(201).json(user);
});

// 路由参数
app.get('/users/:id', (req, res) => {
  const { id } = req.params;
  res.json({ id });
});

// 查询参数
app.get('/search', (req, res) => {
  const { q, page = 1 } = req.query;
  res.json({ query: q, page });
});

// 启动服务器
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
```

### 5.2 中间件

```javascript
const express = require('express');
const app = express();

// 应用级中间件
app.use((req, res, next) => {
  console.log(`${req.method} ${req.url}`);
  next();
});

// 日志中间件
const logger = (req, res, next) => {
  const start = Date.now();
  res.on('finish', () => {
    const duration = Date.now() - start;
    console.log(`${req.method} ${req.url} - ${res.statusCode} (${duration}ms)`);
  });
  next();
};

app.use(logger);

// 认证中间件
const auth = (req, res, next) => {
  const token = req.headers.authorization;
  
  if (!token) {
    return res.status(401).json({ error: 'No token provided' });
  }
  
  // 验证token
  try {
    req.user = verifyToken(token);
    next();
  } catch (err) {
    res.status(401).json({ error: 'Invalid token' });
  }
};

// 路由级中间件
app.get('/api/protected', auth, (req, res) => {
  res.json({ message: 'Protected data', user: req.user });
});

// 错误处理中间件
app.use((err, req, res, next) => {
  console.error(err.stack);
  res.status(500).json({
    error: 'Internal Server Error',
    message: err.message,
  });
});

// 404处理
app.use((req, res) => {
  res.status(404).json({ error: 'Not Found' });
});
```

### 5.3 路由模块化

```javascript
// routes/users.js
const express = require('express');
const router = express.Router();

// 用户列表
router.get('/', async (req, res) => {
  try {
    const users = await User.find();
    res.json(users);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 获取单个用户
router.get('/:id', async (req, res) => {
  try {
    const user = await User.findById(req.params.id);
    if (!user) {
      return res.status(404).json({ error: 'User not found' });
    }
    res.json(user);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 创建用户
router.post('/', async (req, res) => {
  try {
    const user = new User(req.body);
    await user.save();
    res.status(201).json(user);
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

// 更新用户
router.put('/:id', async (req, res) => {
  try {
    const user = await User.findByIdAndUpdate(
      req.params.id,
      req.body,
      { new: true, runValidators: true }
    );
    if (!user) {
      return res.status(404).json({ error: 'User not found' });
    }
    res.json(user);
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

// 删除用户
router.delete('/:id', async (req, res) => {
  try {
    const user = await User.findByIdAndDelete(req.params.id);
    if (!user) {
      return res.status(404).json({ error: 'User not found' });
    }
    res.json({ message: 'User deleted' });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

module.exports = router;
```

```javascript
// app.js - 使用路由
const express = require('express');
const usersRouter = require('./routes/users');
const productsRouter = require('./routes/products');

const app = express();

app.use(express.json());

// 挂载路由
app.use('/api/users', usersRouter);
app.use('/api/products', productsRouter);

app.listen(3000);
```

### 5.4 请求验证

```javascript
const express = require('express');
const { body, param, query, validationResult } = require('express-validator');

const app = express();
app.use(express.json());

// 验证中间件
const validate = (req, res, next) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    return res.status(400).json({ errors: errors.array() });
  }
  next();
};

// 创建用户验证
app.post('/api/users',
  [
    body('email').isEmail().normalizeEmail(),
    body('password').isLength({ min: 6 }),
    body('age').optional().isInt({ min: 0, max: 120 }),
    body('name').trim().notEmpty(),
  ],
  validate,
  async (req, res) => {
    // 创建用户
    res.status(201).json({ message: 'User created' });
  }
);

// 路径参数验证
app.get('/api/users/:id',
  [
    param('id').isMongoId(),
  ],
  validate,
  async (req, res) => {
    // 获取用户
    res.json({ id: req.params.id });
  }
);

// 查询参数验证
app.get('/api/users',
  [
    query('page').optional().isInt({ min: 1 }),
    query('limit').optional().isInt({ min: 1, max: 100 }),
  ],
  validate,
  async (req, res) => {
    // 获取用户列表
    res.json({ users: [] });
  }
);
```

## 7. 认证与安全

### 7.1 密码哈希

```javascript
const bcrypt = require('bcrypt');

// 哈希密码
async function hashPassword(password) {
  const saltRounds = 10;
  const hash = await bcrypt.hash(password, saltRounds);
  return hash;
}

// 验证密码
async function comparePassword(password, hash) {
  const isMatch = await bcrypt.compare(password, hash);
  return isMatch;
}

// 使用示例
async function registerUser(username, password) {
  const hashedPassword = await hashPassword(password);
  // 存储到数据库
  await User.create({ username, password: hashedPassword });
}

async function loginUser(username, password) {
  const user = await User.findOne({ username });
  if (!user) {
    throw new Error('User not found');
  }
  
  const isValid = await comparePassword(password, user.password);
  if (!isValid) {
    throw new Error('Invalid password');
  }
  
  return user;
}
```

### 7.2 JWT认证

```javascript
const jwt = require('jsonwebtoken');

// 生成JWT Token
function generateToken(payload, expiresIn = '24h') {
  const secret = process.env.JWT_SECRET || 'your-secret-key';
  return jwt.sign(payload, secret, { expiresIn });
}

// 验证JWT Token
function verifyToken(token) {
  const secret = process.env.JWT_SECRET || 'your-secret-key';
  try {
    return jwt.verify(token, secret);
  } catch (error) {
    throw new Error('Invalid token');
  }
}

// 刷新Token
function refreshToken(token) {
  const decoded = verifyToken(token);
  // 移除过期时间
  delete decoded.exp;
  delete decoded.iat;
  return generateToken(decoded);
}

// Express中间件
const authMiddleware = (req, res, next) => {
  const authHeader = req.headers.authorization;
  
  if (!authHeader || !authHeader.startsWith('Bearer ')) {
    return res.status(401).json({ error: 'No token provided' });
  }
  
  const token = authHeader.substring(7);
  
  try {
    const decoded = verifyToken(token);
    req.user = decoded;
    next();
  } catch (error) {
    res.status(401).json({ error: 'Invalid token' });
  }
};

// 路由使用
app.post('/api/login', async (req, res) => {
  try {
    const { username, password } = req.body;
    const user = await loginUser(username, password);
    
    const token = generateToken({
      id: user._id,
      username: user.username,
      role: user.role,
    });
    
    res.json({ token, user: { id: user._id, username: user.username } });
  } catch (error) {
    res.status(401).json({ error: error.message });
  }
});

app.get('/api/protected', authMiddleware, (req, res) => {
  res.json({ message: 'Protected data', user: req.user });
});
```

### 7.3 OAuth 2.0

```javascript
const passport = require('passport');
const GoogleStrategy = require('passport-google-oauth20').Strategy;
const GitHubStrategy = require('passport-github2').Strategy;

// Google OAuth配置
passport.use(new GoogleStrategy({
  clientID: process.env.GOOGLE_CLIENT_ID,
  clientSecret: process.env.GOOGLE_CLIENT_SECRET,
  callbackURL: '/auth/google/callback',
}, async (accessToken, refreshToken, profile, done) => {
  try {
    // 查找或创建用户
    let user = await User.findOne({ googleId: profile.id });
    
    if (!user) {
      user = await User.create({
        googleId: profile.id,
        email: profile.emails[0].value,
        name: profile.displayName,
        avatar: profile.photos[0].value,
      });
    }
    
    done(null, user);
  } catch (error) {
    done(error, null);
  }
}));

// GitHub OAuth配置
passport.use(new GitHubStrategy({
  clientID: process.env.GITHUB_CLIENT_ID,
  clientSecret: process.env.GITHUB_CLIENT_SECRET,
  callbackURL: '/auth/github/callback',
}, async (accessToken, refreshToken, profile, done) => {
  try {
    let user = await User.findOne({ githubId: profile.id });
    
    if (!user) {
      user = await User.create({
        githubId: profile.id,
        username: profile.username,
        name: profile.displayName,
        avatar: profile.photos[0].value,
      });
    }
    
    done(null, user);
  } catch (error) {
    done(error, null);
  }
}));

// 序列化用户
passport.serializeUser((user, done) => {
  done(null, user.id);
});

passport.deserializeUser(async (id, done) => {
  try {
    const user = await User.findById(id);
    done(null, user);
  } catch (error) {
    done(error, null);
  }
});

// Express配置
app.use(passport.initialize());
app.use(passport.session());

// 路由
app.get('/auth/google',
  passport.authenticate('google', { scope: ['profile', 'email'] })
);

app.get('/auth/google/callback',
  passport.authenticate('google', { failureRedirect: '/login' }),
  (req, res) => {
    res.redirect('/dashboard');
  }
);

app.get('/auth/github',
  passport.authenticate('github', { scope: ['user:email'] })
);

app.get('/auth/github/callback',
  passport.authenticate('github', { failureRedirect: '/login' }),
  (req, res) => {
    res.redirect('/dashboard');
  }
);
```

### 7.4 CORS配置

```javascript
const cors = require('cors');

// 基本CORS配置
app.use(cors());

// 详细CORS配置
const corsOptions = {
  origin: function (origin, callback) {
    const allowedOrigins = [
      'http://localhost:3000',
      'https://example.com',
    ];
    
    if (!origin || allowedOrigins.includes(origin)) {
      callback(null, true);
    } else {
      callback(new Error('Not allowed by CORS'));
    }
  },
  credentials: true,
  methods: ['GET', 'POST', 'PUT', 'DELETE', 'PATCH'],
  allowedHeaders: ['Content-Type', 'Authorization'],
  exposedHeaders: ['X-Total-Count'],
  maxAge: 86400, // 24小时
};

app.use(cors(corsOptions));

// 预检请求处理
app.options('*', cors(corsOptions));
```

### 7.5 安全最佳实践

```javascript
const helmet = require('helmet');
const rateLimit = require('express-rate-limit');
const mongoSanitize = require('express-mongo-sanitize');
const xss = require('xss-clean');
const hpp = require('hpp');

// ===== Helmet - 设置安全HTTP头 =====
app.use(helmet());

// ===== 速率限制 =====
const limiter = rateLimit({
  windowMs: 15 * 60 * 1000, // 15分钟
  max: 100, // 限制100个请求
  message: 'Too many requests from this IP',
  standardHeaders: true,
  legacyHeaders: false,
});

app.use('/api/', limiter);

// 登录速率限制
const loginLimiter = rateLimit({
  windowMs: 15 * 60 * 1000,
  max: 5,
  message: 'Too many login attempts',
});

app.post('/api/login', loginLimiter, loginHandler);

// ===== NoSQL注入防护 =====
app.use(mongoSanitize());

// ===== XSS防护 =====
app.use(xss());

// ===== HTTP参数污染防护 =====
app.use(hpp());

// ===== HTTPS重定向 =====
app.use((req, res, next) => {
  if (req.secure || process.env.NODE_ENV === 'development') {
    next();
  } else {
    res.redirect(`https://${req.headers.host}${req.url}`);
  }
});

// ===== 输入验证 =====
const { body, validationResult } = require('express-validator');

app.post('/api/users',
  [
    body('email').isEmail().normalizeEmail(),
    body('password').isLength({ min: 8 }).matches(/^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)/),
    body('username').trim().isLength({ min: 3, max: 30 }).matches(/^[a-zA-Z0-9_]+$/),
  ],
  (req, res) => {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({ errors: errors.array() });
    }
    // 处理请求
  }
);

// ===== CSRF保护 =====
const csrf = require('csurf');
const csrfProtection = csrf({ cookie: true });

app.get('/form', csrfProtection, (req, res) => {
  res.render('form', { csrfToken: req.csrfToken() });
});

app.post('/process', csrfProtection, (req, res) => {
  res.send('Form processed');
});

// ===== 会话安全 =====
const session = require('express-session');
const MongoStore = require('connect-mongo');

app.use(session({
  secret: process.env.SESSION_SECRET,
  resave: false,
  saveUninitialized: false,
  cookie: {
    secure: process.env.NODE_ENV === 'production', // HTTPS only
    httpOnly: true, // 防止XSS
    maxAge: 24 * 60 * 60 * 1000, // 24小时
    sameSite: 'strict', // CSRF保护
  },
  store: MongoStore.create({
    mongoUrl: process.env.MONGODB_URI,
    touchAfter: 24 * 3600, // 24小时内只更新一次
  }),
}));

// ===== 环境变量管理 =====
// .env文件
/*
NODE_ENV=production
PORT=3000
MONGODB_URI=mongodb://localhost:27017/myapp
JWT_SECRET=your-super-secret-jwt-key
SESSION_SECRET=your-super-secret-session-key
GOOGLE_CLIENT_ID=your-google-client-id
GOOGLE_CLIENT_SECRET=your-google-client-secret
*/

// 加载环境变量
require('dotenv').config();

// 验证必需的环境变量
const requiredEnvVars = ['MONGODB_URI', 'JWT_SECRET', 'SESSION_SECRET'];
for (const envVar of requiredEnvVars) {
  if (!process.env[envVar]) {
    console.error(`Missing required environment variable: ${envVar}`);
    process.exit(1);
  }
}

// ===== 错误处理 =====
// 404处理
app.use((req, res, next) => {
  res.status(404).json({ error: 'Not Found' });
});

// 全局错误处理
app.use((err, req, res, next) => {
  console.error(err.stack);
  
  // 不要在生产环境中暴露错误详情
  const error = process.env.NODE_ENV === 'production'
    ? { message: 'Internal Server Error' }
    : { message: err.message, stack: err.stack };
  
  res.status(err.status || 500).json({ error });
});

// ===== 日志记录 =====
const winston = require('winston');

const logger = winston.createLogger({
  level: 'info',
  format: winston.format.json(),
  transports: [
    new winston.transports.File({ filename: 'error.log', level: 'error' }),
    new winston.transports.File({ filename: 'combined.log' }),
  ],
});

if (process.env.NODE_ENV !== 'production') {
  logger.add(new winston.transports.Console({
    format: winston.format.simple(),
  }));
}

// 记录所有请求
app.use((req, res, next) => {
  logger.info(`${req.method} ${req.url}`, {
    ip: req.ip,
    userAgent: req.get('user-agent'),
  });
  next();
});
```

### 7.6 双因素认证（2FA）

```javascript
const speakeasy = require('speakeasy');
const QRCode = require('qrcode');

// 生成2FA密钥
async function generate2FASecret(user) {
  const secret = speakeasy.generateSecret({
    name: `MyApp (${user.email})`,
    length: 32,
  });
  
  // 生成QR码
  const qrCodeUrl = await QRCode.toDataURL(secret.otpauth_url);
  
  return {
    secret: secret.base32,
    qrCode: qrCodeUrl,
  };
}

// 验证2FA令牌
function verify2FAToken(secret, token) {
  return speakeasy.totp.verify({
    secret,
    encoding: 'base32',
    token,
    window: 2, // 允许前后2个时间窗口
  });
}

// 启用2FA
app.post('/api/2fa/enable', authMiddleware, async (req, res) => {
  try {
    const user = await User.findById(req.user.id);
    
    if (user.twoFactorEnabled) {
      return res.status(400).json({ error: '2FA already enabled' });
    }
    
    const { secret, qrCode } = await generate2FASecret(user);
    
    // 临时存储secret（等待验证）
    user.twoFactorTempSecret = secret;
    await user.save();
    
    res.json({ qrCode, secret });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// 确认启用2FA
app.post('/api/2fa/verify', authMiddleware, async (req, res) => {
  try {
    const { token } = req.body;
    const user = await User.findById(req.user.id);
    
    if (!user.twoFactorTempSecret) {
      return res.status(400).json({ error: '2FA setup not initiated' });
    }
    
    const isValid = verify2FAToken(user.twoFactorTempSecret, token);
    
    if (!isValid) {
      return res.status(400).json({ error: 'Invalid token' });
    }
    
    // 启用2FA
    user.twoFactorSecret = user.twoFactorTempSecret;
    user.twoFactorEnabled = true;
    user.twoFactorTempSecret = undefined;
    await user.save();
    
    res.json({ message: '2FA enabled successfully' });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// 登录时验证2FA
app.post('/api/login', async (req, res) => {
  try {
    const { username, password, token } = req.body;
    
    const user = await User.findOne({ username });
    if (!user) {
      return res.status(401).json({ error: 'Invalid credentials' });
    }
    
    const isValidPassword = await comparePassword(password, user.password);
    if (!isValidPassword) {
      return res.status(401).json({ error: 'Invalid credentials' });
    }
    
    // 如果启用了2FA
    if (user.twoFactorEnabled) {
      if (!token) {
        return res.status(200).json({ requires2FA: true });
      }
      
      const isValid2FA = verify2FAToken(user.twoFactorSecret, token);
      if (!isValid2FA) {
        return res.status(401).json({ error: 'Invalid 2FA token' });
      }
    }
    
    const jwtToken = generateToken({ id: user._id, username: user.username });
    res.json({ token: jwtToken });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});
```

---

## 6. 数据库操作

### 6.1 MongoDB与Mongoose

```javascript
const mongoose = require('mongoose');

// 连接数据库
mongoose.connect('mongodb://localhost:27017/myapp', {
  useNewUrlParser: true,
  useUnifiedTopology: true,
})
  .then(() => console.log('MongoDB connected'))
  .catch(err => console.error('MongoDB connection error:', err));

// 定义Schema
const userSchema = new mongoose.Schema({
  username: {
    type: String,
    required: true,
    unique: true,
    trim: true,
    minlength: 3,
  },
  email: {
    type: String,
    required: true,
    unique: true,
    lowercase: true,
    validate: {
      validator: (v) => /^[\w-\.]+@([\w-]+\.)+[\w-]{2,4}$/.test(v),
      message: 'Invalid email format',
    },
  },
  password: {
    type: String,
    required: true,
    minlength: 6,
  },
  age: {
    type: Number,
    min: 0,
    max: 120,
  },
  role: {
    type: String,
    enum: ['user', 'admin'],
    default: 'user',
  },
  createdAt: {
    type: Date,
    default: Date.now,
  },
  profile: {
    bio: String,
    avatar: String,
  },
  posts: [{
    type: mongoose.Schema.Types.ObjectId,
    ref: 'Post',
  }],
});

// 实例方法
userSchema.methods.comparePassword = function(password) {
  return bcrypt.compare(password, this.password);
};

// 静态方法
userSchema.statics.findByEmail = function(email) {
  return this.findOne({ email });
};

// 中间件（钩子）
userSchema.pre('save', async function(next) {
  if (!this.isModified('password')) return next();
  
  this.password = await bcrypt.hash(this.password, 10);
  next();
});

// 虚拟属性
userSchema.virtual('fullName').get(function() {
  return `${this.firstName} ${this.lastName}`;
});

// 创建Model
const User = mongoose.model('User', userSchema);

// CRUD操作
async function examples() {
  // 创建
  const user = new User({
    username: 'john',
    email: 'john@example.com',
    password: 'password123',
  });
  await user.save();
  
  // 或使用create
  const user2 = await User.create({
    username: 'jane',
    email: 'jane@example.com',
    password: 'password123',
  });
  
  // 查询
  const users = await User.find();
  const user = await User.findById('507f1f77bcf86cd799439011');
  const user3 = await User.findOne({ email: 'john@example.com' });
  
  // 条件查询
  const adults = await User.find({ age: { $gte: 18 } });
  const admins = await User.find({ role: 'admin' }).limit(10).sort({ createdAt: -1 });
  
  // 更新
  await User.updateOne({ _id: user._id }, { age: 25 });
  await User.updateMany({ role: 'user' }, { $set: { verified: false } });
  
  const updated = await User.findByIdAndUpdate(
    user._id,
    { age: 26 },
    { new: true, runValidators: true }
  );
  
  // 删除
  await User.deleteOne({ _id: user._id });
  await User.deleteMany({ verified: false });
  const deleted = await User.findByIdAndDelete(user._id);
  
  // 聚合
  const stats = await User.aggregate([
    { $match: { role: 'user' } },
    { $group: { _id: null, avgAge: { $avg: '$age' } } },
  ]);
  
  // 关联查询
  const userWithPosts = await User.findById(user._id).populate('posts');
}
```

### 6.2 PostgreSQL与Sequelize

```javascript
const { Sequelize, DataTypes } = require('sequelize');

// 连接数据库
const sequelize = new Sequelize('database', 'username', 'password', {
  host: 'localhost',
  dialect: 'postgres',
  logging: false,
});

// 定义模型
const User = sequelize.define('User', {
  id: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true,
  },
  username: {
    type: DataTypes.STRING,
    allowNull: false,
    unique: true,
    validate: {
      len: [3, 50],
    },
  },
  email: {
    type: DataTypes.STRING,
    allowNull: false,
    unique: true,
    validate: {
      isEmail: true,
    },
  },
  password: {
    type: DataTypes.STRING,
    allowNull: false,
  },
  role: {
    type: DataTypes.ENUM('user', 'admin'),
    defaultValue: 'user',
  },
}, {
  timestamps: true,
  hooks: {
    beforeCreate: async (user) => {
      user.password = await bcrypt.hash(user.password, 10);
    },
  },
});

// 关联
const Post = sequelize.define('Post', {
  title: DataTypes.STRING,
  content: DataTypes.TEXT,
});

User.hasMany(Post);
Post.belongsTo(User);

// 同步数据库
await sequelize.sync({ force: false });

// CRUD操作
async function examples() {
  // 创建
  const user = await User.create({
    username: 'john',
    email: 'john@example.com',
    password: 'password123',
  });
  
  // 查询
  const users = await User.findAll();
  const user2 = await User.findByPk(1);
  const user3 = await User.findOne({ where: { email: 'john@example.com' } });
  
  // 条件查询
  const admins = await User.findAll({
    where: { role: 'admin' },
    limit: 10,
    order: [['createdAt', 'DESC']],
  });
  
  // 更新
  await user.update({ role: 'admin' });
  await User.update(
    { verified: true },
    { where: { role: 'user' } }
  );
  
  // 删除
  await user.destroy();
  await User.destroy({ where: { verified: false } });
  
  // 关联查询
  const userWithPosts = await User.findByPk(1, {
    include: Post,
  });
}
```

---

## Part 2: TypeScript

## 8. TypeScript基础

### 8.1 TypeScript简介

```typescript
/**
 * TypeScript特点：
 * 1. 静态类型检查
 * 2. 类型推断
 * 3. 接口和类型别名
 * 4. 泛型
 * 5. 装饰器
 * 6. 编译到JavaScript
 */

// 安装TypeScript
// npm install -g typescript

// 编译TypeScript
// tsc hello.ts

// 监听模式
// tsc --watch
```

### 8.2 基本类型

```typescript
// ===== 基础类型 =====
let isDone: boolean = false;
let decimal: number = 6;
let color: string = "blue";

// 数组
let list1: number[] = [1, 2, 3];
let list2: Array<number> = [1, 2, 3];

// 元组
let tuple: [string, number] = ["hello", 10];

// 枚举
enum Color {
  Red,
  Green,
  Blue,
}
let c: Color = Color.Green;

// 枚举with值
enum Direction {
  Up = "UP",
  Down = "DOWN",
  Left = "LEFT",
  Right = "RIGHT",
}

// any - 任意类型
let notSure: any = 4;
notSure = "maybe a string";
notSure = false;

// unknown - 类型安全的any
let value: unknown = 4;
// value.toFixed();  // 错误
if (typeof value === 'number') {
  value.toFixed();  // 正确
}

// void - 无返回值
function warnUser(): void {
  console.log("Warning");
}

// null和undefined
let u: undefined = undefined;
let n: null = null;

// never - 永不返回
function error(message: string): never {
  throw new Error(message);
}

// object
let obj: object = { name: "John" };

// 类型断言
let someValue: any = "this is a string";
let strLength: number = (someValue as string).length;
// 或
let strLength2: number = (<string>someValue).length;
```

### 8.3 接口

```typescript
// 基本接口
interface User {
  id: number;
  name: string;
  email: string;
}

const user: User = {
  id: 1,
  name: "John",
  email: "john@example.com",
};

// 可选属性
interface UserProfile {
  username: string;
  age?: number;  // 可选
  bio?: string;
}

// 只读属性
interface Point {
  readonly x: number;
  readonly y: number;
}

const point: Point = { x: 10, y: 20 };
// point.x = 5;  // 错误

// 函数类型
interface SearchFunc {
  (source: string, subString: string): boolean;
}

const mySearch: SearchFunc = (src, sub) => {
  return src.indexOf(sub) > -1;
};

// 索引签名
interface StringArray {
  [index: number]: string;
}

const myArray: StringArray = ["Bob", "Fred"];

interface Dictionary {
  [key: string]: any;
}

// 类接口
interface ClockInterface {
  currentTime: Date;
  setTime(d: Date): void;
}

class Clock implements ClockInterface {
  currentTime: Date = new Date();
  
  setTime(d: Date) {
    this.currentTime = d;
  }
}

// 接口继承
interface Shape {
  color: string;
}

interface Square extends Shape {
  sideLength: number;
}

const square: Square = {
  color: "blue",
  sideLength: 10,
};

// 混合类型
interface Counter {
  (start: number): string;
  interval: number;
  reset(): void;
}

function getCounter(): Counter {
  const counter = (function (start: number) {
    return start.toString();
  }) as Counter;
  
  counter.interval = 123;
  counter.reset = function () {};
  
  return counter;
}
```

### 8.4 类

```typescript
// 基本类
class Greeter {
  greeting: string;
  
  constructor(message: string) {
    this.greeting = message;
  }
  
  greet() {
    return `Hello, ${this.greeting}`;
  }
}

const greeter = new Greeter("world");

// 继承
class Animal {
  name: string;
  
  constructor(name: string) {
    this.name = name;
  }
  
  move(distance: number = 0) {
    console.log(`${this.name} moved ${distance}m.`);
  }
}

class Dog extends Animal {
  bark() {
    console.log("Woof! Woof!");
  }
}

const dog = new Dog("Buddy");
dog.bark();
dog.move(10);

// 访问修饰符
class Person {
  public name: string;      // 公共（默认）
  private age: number;      // 私有
  protected email: string;  // 受保护
  
  constructor(name: string, age: number, email: string) {
    this.name = name;
    this.age = age;
    this.email = email;
  }
  
  getAge(): number {
    return this.age;
  }
}

// 参数属性简写
class Person2 {
  constructor(
    public name: string,
    private age: number,
    protected email: string
  ) {}
}

// readonly修饰符
class Octopus {
  readonly name: string;
  readonly numberOfLegs: number = 8;
  
  constructor(name: string) {
    this.name = name;
  }
}

// 存取器
class Employee {
  private _fullName: string = "";
  
  get fullName(): string {
    return this._fullName;
  }
  
  set fullName(newName: string) {
    this._fullName = newName;
  }
}

// 静态成员
class Grid {
  static origin = { x: 0, y: 0 };
  
  static calculateDistance(point: { x: number; y: number }) {
    const xDist = point.x - Grid.origin.x;
    const yDist = point.y - Grid.origin.y;
    return Math.sqrt(xDist * xDist + yDist * yDist);
  }
}

// 抽象类
abstract class Department {
  constructor(public name: string) {}
  
  printName(): void {
    console.log(`Department name: ${this.name}`);
  }
  
  abstract printMeeting(): void;  // 必须在派生类中实现
}

class AccountingDepartment extends Department {
  constructor() {
    super("Accounting");
  }
  
  printMeeting(): void {
    console.log("Meeting at 10am");
  }
}
```



---

## 9. 类型系统

### 9.1 类型别名

```typescript
// 基本类型别名
type ID = string | number;
type Point = { x: number; y: number };

const id: ID = "123";
const point: Point = { x: 10, y: 20 };

// 联合类型
type Status = "pending" | "success" | "error";
type Result = string | number | boolean;

function setStatus(status: Status) {
  console.log(status);
}

// 交叉类型
type Person = { name: string };
type Employee = { employeeId: number };
type Staff = Person & Employee;

const staff: Staff = {
  name: "John",
  employeeId: 123,
};

// 函数类型别名
type Callback = (data: string) => void;
type MathOperation = (a: number, b: number) => number;

const add: MathOperation = (a, b) => a + b;
```

### 9.2 类型断言

```typescript
// as语法
const someValue: any = "this is a string";
const strLength: number = (someValue as string).length;

// 尖括号语法（不推荐，JSX中不可用）
const strLength2: number = (<string>someValue).length;

// 非空断言
function processValue(value: string | null) {
  // 确定value不为null
  console.log(value!.length);
}

// const断言
const colors = ["red", "green", "blue"] as const;
// colors: readonly ["red", "green", "blue"]

const config = {
  host: "localhost",
  port: 3000,
} as const;
// 所有属性变为readonly

// 类型断言的限制
interface Cat {
  meow(): void;
}

interface Dog {
  bark(): void;
}

const cat = { meow: () => {} } as Cat;
// const dog = cat as Dog;  // 错误：类型不兼容
const dog = cat as unknown as Dog;  // 可以，但不安全
```

### 9.3 类型守卫

```typescript
// typeof守卫
function printValue(value: string | number) {
  if (typeof value === "string") {
    console.log(value.toUpperCase());
  } else {
    console.log(value.toFixed(2));
  }
}

// instanceof守卫
class Bird {
  fly() { console.log("Flying"); }
}

class Fish {
  swim() { console.log("Swimming"); }
}

function move(animal: Bird | Fish) {
  if (animal instanceof Bird) {
    animal.fly();
  } else {
    animal.swim();
  }
}

// in守卫
type Circle = { radius: number };
type Rectangle = { width: number; height: number };

function getArea(shape: Circle | Rectangle) {
  if ("radius" in shape) {
    return Math.PI * shape.radius ** 2;
  } else {
    return shape.width * shape.height;
  }
}

// 自定义类型守卫
interface Cat {
  meow(): void;
}

interface Dog {
  bark(): void;
}

function isCat(pet: Cat | Dog): pet is Cat {
  return (pet as Cat).meow !== undefined;
}

function makeSound(pet: Cat | Dog) {
  if (isCat(pet)) {
    pet.meow();
  } else {
    pet.bark();
  }
}
```

### 9.4 字面量类型

```typescript
// 字符串字面量
type Direction = "north" | "south" | "east" | "west";
let direction: Direction = "north";

// 数字字面量
type DiceRoll = 1 | 2 | 3 | 4 | 5 | 6;
let roll: DiceRoll = 3;

// 布尔字面量
type True = true;
type False = false;

// 对象字面量
type Response = {
  status: 200;
  data: string;
} | {
  status: 404;
  error: string;
};

function handleResponse(response: Response) {
  if (response.status === 200) {
    console.log(response.data);
  } else {
    console.log(response.error);
  }
}

// 模板字面量类型（TypeScript 4.1+）
type Color = "red" | "blue" | "green";
type Quantity = "one" | "two";
type SeussFish = `${Quantity | Color} fish`;
// "one fish" | "two fish" | "red fish" | "blue fish" | "green fish"
```

---

## 10. 高级类型

### 10.1 映射类型

```typescript
// Partial - 所有属性可选
interface User {
  id: number;
  name: string;
  email: string;
}

type PartialUser = Partial<User>;
// { id?: number; name?: string; email?: string; }

// Required - 所有属性必需
type RequiredUser = Required<PartialUser>;

// Readonly - 所有属性只读
type ReadonlyUser = Readonly<User>;

// Pick - 选择部分属性
type UserPreview = Pick<User, "id" | "name">;
// { id: number; name: string; }

// Omit - 排除部分属性
type UserWithoutEmail = Omit<User, "email">;
// { id: number; name: string; }

// Record - 创建对象类型
type Role = "admin" | "user" | "guest";
type Permissions = Record<Role, string[]>;
// {
//   admin: string[];
//   user: string[];
//   guest: string[];
// }

// Exclude - 从联合类型中排除
type T1 = Exclude<"a" | "b" | "c", "a">;  // "b" | "c"

// Extract - 从联合类型中提取
type T2 = Extract<"a" | "b" | "c", "a" | "d">;  // "a"

// NonNullable - 排除null和undefined
type T3 = NonNullable<string | number | null | undefined>;  // string | number

// ReturnType - 获取函数返回类型
function getData() {
  return { id: 1, name: "John" };
}

type Data = ReturnType<typeof getData>;
// { id: number; name: string; }

// Parameters - 获取函数参数类型
function createUser(name: string, age: number) {
  return { name, age };
}

type CreateUserParams = Parameters<typeof createUser>;
// [name: string, age: number]
```

### 10.2 条件类型

```typescript
// 基本条件类型
type IsString<T> = T extends string ? true : false;

type A = IsString<string>;  // true
type B = IsString<number>;  // false

// 分布式条件类型
type ToArray<T> = T extends any ? T[] : never;
type StrOrNumArray = ToArray<string | number>;
// string[] | number[]

// infer关键字
type UnpackPromise<T> = T extends Promise<infer R> ? R : T;

type T1 = UnpackPromise<Promise<string>>;  // string
type T2 = UnpackPromise<number>;  // number

// 获取数组元素类型
type ElementType<T> = T extends (infer E)[] ? E : T;

type T3 = ElementType<string[]>;  // string
type T4 = ElementType<number>;  // number

// 获取函数返回类型
type ReturnTypeCustom<T> = T extends (...args: any[]) => infer R ? R : never;

function foo(): string {
  return "hello";
}

type FooReturn = ReturnTypeCustom<typeof foo>;  // string

// 实用条件类型
type NonEmptyArray<T> = [T, ...T[]];

const arr1: NonEmptyArray<number> = [1];  // 正确
const arr2: NonEmptyArray<number> = [1, 2, 3];  // 正确
// const arr3: NonEmptyArray<number> = [];  // 错误
```

### 10.3 索引类型

```typescript
// 索引访问类型
interface Person {
  name: string;
  age: number;
  address: {
    street: string;
    city: string;
  };
}

type NameType = Person["name"];  // string
type AgeType = Person["age"];  // number
type AddressType = Person["address"];  // { street: string; city: string; }
type CityType = Person["address"]["city"];  // string

// keyof操作符
type PersonKeys = keyof Person;  // "name" | "age" | "address"

function getProperty<T, K extends keyof T>(obj: T, key: K): T[K] {
  return obj[key];
}

const person: Person = {
  name: "John",
  age: 30,
  address: { street: "123 Main St", city: "NYC" },
};

const name = getProperty(person, "name");  // string
const age = getProperty(person, "age");  // number

// 映射类型中的索引
type Getters<T> = {
  [K in keyof T as `get${Capitalize<string & K>}`]: () => T[K];
};

type PersonGetters = Getters<Person>;
// {
//   getName: () => string;
//   getAge: () => number;
//   getAddress: () => { street: string; city: string; };
// }
```

### 10.4 工具类型

```typescript
// Awaited - 解包Promise类型
type A = Awaited<Promise<string>>;  // string
type B = Awaited<Promise<Promise<number>>>;  // number

// ConstructorParameters - 获取构造函数参数
class MyClass {
  constructor(public name: string, public age: number) {}
}

type MyClassParams = ConstructorParameters<typeof MyClass>;
// [name: string, age: number]

// InstanceType - 获取构造函数实例类型
type MyClassInstance = InstanceType<typeof MyClass>;
// MyClass

// ThisParameterType - 获取函数this参数类型
function toHex(this: Number) {
  return this.toString(16);
}

type T = ThisParameterType<typeof toHex>;  // Number

// OmitThisParameter - 移除this参数
type Fn = OmitThisParameter<typeof toHex>;
// () => string

// 自定义工具类型
type DeepPartial<T> = {
  [P in keyof T]?: T[P] extends object ? DeepPartial<T[P]> : T[P];
};

interface Config {
  server: {
    host: string;
    port: number;
  };
  database: {
    host: string;
    port: number;
  };
}

type PartialConfig = DeepPartial<Config>;
// 所有嵌套属性都是可选的

// DeepReadonly
type DeepReadonly<T> = {
  readonly [P in keyof T]: T[P] extends object ? DeepReadonly<T[P]> : T[P];
};
```

---

## 11. 泛型

### 11.1 泛型函数

```typescript
// 基本泛型函数
function identity<T>(arg: T): T {
  return arg;
}

const num = identity<number>(42);
const str = identity<string>("hello");
const auto = identity(42);  // 类型推断

// 泛型数组
function getFirstElement<T>(arr: T[]): T | undefined {
  return arr[0];
}

const first = getFirstElement([1, 2, 3]);  // number | undefined

// 多个类型参数
function pair<T, U>(first: T, second: U): [T, U] {
  return [first, second];
}

const p = pair("hello", 42);  // [string, number]

// 泛型约束
interface Lengthwise {
  length: number;
}

function logLength<T extends Lengthwise>(arg: T): T {
  console.log(arg.length);
  return arg;
}

logLength("hello");  // 正确
logLength([1, 2, 3]);  // 正确
logLength({ length: 10, value: 3 });  // 正确
// logLength(3);  // 错误：number没有length属性

// 使用类型参数约束
function getProperty<T, K extends keyof T>(obj: T, key: K): T[K] {
  return obj[key];
}

const obj = { a: 1, b: 2, c: 3 };
getProperty(obj, "a");  // 正确
// getProperty(obj, "d");  // 错误：不存在属性d
```

### 11.2 泛型接口

```typescript
// 基本泛型接口
interface Box<T> {
  value: T;
}

const numberBox: Box<number> = { value: 42 };
const stringBox: Box<string> = { value: "hello" };

// 泛型函数接口
interface GenericIdentityFn<T> {
  (arg: T): T;
}

const myIdentity: GenericIdentityFn<number> = (arg) => arg;

// 多个泛型参数
interface Pair<T, U> {
  first: T;
  second: U;
}

const pair: Pair<string, number> = {
  first: "hello",
  second: 42,
};

// 泛型接口继承
interface Container<T> {
  value: T;
}

interface NamedContainer<T> extends Container<T> {
  name: string;
}

const namedBox: NamedContainer<number> = {
  name: "My Box",
  value: 42,
};
```

### 11.3 泛型类

```typescript
// 基本泛型类
class GenericNumber<T> {
  zeroValue: T;
  add: (x: T, y: T) => T;
  
  constructor(zeroValue: T, add: (x: T, y: T) => T) {
    this.zeroValue = zeroValue;
    this.add = add;
  }
}

const myGenericNumber = new GenericNumber<number>(
  0,
  (x, y) => x + y
);

const myGenericString = new GenericNumber<string>(
  "",
  (x, y) => x + y
);

// 泛型类方法
class DataStorage<T> {
  private data: T[] = [];
  
  addItem(item: T): void {
    this.data.push(item);
  }
  
  removeItem(item: T): void {
    const index = this.data.indexOf(item);
    if (index > -1) {
      this.data.splice(index, 1);
    }
  }
  
  getItems(): T[] {
    return [...this.data];
  }
}

const textStorage = new DataStorage<string>();
textStorage.addItem("hello");
textStorage.addItem("world");

// 泛型约束
class Collection<T extends { id: number }> {
  private items: T[] = [];
  
  add(item: T): void {
    this.items.push(item);
  }
  
  findById(id: number): T | undefined {
    return this.items.find(item => item.id === id);
  }
}

const users = new Collection<{ id: number; name: string }>();
users.add({ id: 1, name: "John" });
```

### 11.4 泛型约束与默认值

```typescript
// 泛型约束
interface HasLength {
  length: number;
}

function longest<T extends HasLength>(a: T, b: T): T {
  return a.length > b.length ? a : b;
}

longest("hello", "world");  // 正确
longest([1, 2], [1, 2, 3]);  // 正确
// longest(1, 2);  // 错误

// 泛型默认值
interface Container<T = string> {
  value: T;
}

const stringContainer: Container = { value: "hello" };
const numberContainer: Container<number> = { value: 42 };

// 类型参数约束
function create<T extends new (...args: any[]) => any>(
  constructor: T,
  ...args: ConstructorParameters<T>
): InstanceType<T> {
  return new constructor(...args);
}

class Point {
  constructor(public x: number, public y: number) {}
}

const point = create(Point, 10, 20);  // Point

// 使用类型参数
function merge<T extends object, U extends object>(
  obj1: T,
  obj2: U
): T & U {
  return { ...obj1, ...obj2 };
}

const merged = merge({ name: "John" }, { age: 30 });
// { name: string; age: number; }
```

---

## 12. 装饰器

### 12.1 装饰器基础

```typescript
// tsconfig.json需要开启
// "experimentalDecorators": true

// 类装饰器
function sealed(constructor: Function) {
  Object.seal(constructor);
  Object.seal(constructor.prototype);
}

@sealed
class Greeter {
  greeting: string;
  
  constructor(message: string) {
    this.greeting = message;
  }
}

// 装饰器工厂
function log(prefix: string) {
  return function (constructor: Function) {
    console.log(`${prefix}: ${constructor.name}`);
  };
}

@log("LOG")
class MyClass {}

// 类装饰器修改类
function addTimestamp<T extends { new(...args: any[]): {} }>(
  constructor: T
) {
  return class extends constructor {
    timestamp = new Date();
  };
}

@addTimestamp
class User {
  constructor(public name: string) {}
}

const user = new User("John");
console.log((user as any).timestamp);
```

### 12.2 方法装饰器

```typescript
// 方法装饰器
function log(
  target: any,
  propertyKey: string,
  descriptor: PropertyDescriptor
) {
  const originalMethod = descriptor.value;
  
  descriptor.value = function (...args: any[]) {
    console.log(`Calling ${propertyKey} with args:`, args);
    const result = originalMethod.apply(this, args);
    console.log(`Result:`, result);
    return result;
  };
  
  return descriptor;
}

class Calculator {
  @log
  add(a: number, b: number): number {
    return a + b;
  }
}

// 方法装饰器工厂
function measure(target: any, propertyKey: string, descriptor: PropertyDescriptor) {
  const originalMethod = descriptor.value;
  
  descriptor.value = async function (...args: any[]) {
    const start = performance.now();
    const result = await originalMethod.apply(this, args);
    const end = performance.now();
    console.log(`${propertyKey} took ${end - start}ms`);
    return result;
  };
  
  return descriptor;
}

class Service {
  @measure
  async fetchData(): Promise<string> {
    await new Promise(resolve => setTimeout(resolve, 1000));
    return "data";
  }
}
```

### 12.3 属性装饰器

```typescript
// 属性装饰器
function readonly(target: any, propertyKey: string) {
  Object.defineProperty(target, propertyKey, {
    writable: false,
  });
}

class Person {
  @readonly
  name: string = "John";
}

const person = new Person();
// person.name = "Jane";  // 运行时错误

// 属性验证装饰器
function min(limit: number) {
  return function (target: any, propertyKey: string) {
    let value: number;
    
    const getter = function () {
      return value;
    };
    
    const setter = function (newVal: number) {
      if (newVal < limit) {
        throw new Error(`${propertyKey} must be >= ${limit}`);
      }
      value = newVal;
    };
    
    Object.defineProperty(target, propertyKey, {
      get: getter,
      set: setter,
    });
  };
}

class Product {
  @min(0)
  price: number;
  
  constructor(price: number) {
    this.price = price;
  }
}
```

### 12.4 参数装饰器

```typescript
// 参数装饰器
function required(
  target: any,
  propertyKey: string,
  parameterIndex: number
) {
  const existingRequiredParameters: number[] = 
    Reflect.getOwnMetadata("required", target, propertyKey) || [];
  
  existingRequiredParameters.push(parameterIndex);
  
  Reflect.defineMetadata(
    "required",
    existingRequiredParameters,
    target,
    propertyKey
  );
}

function validate(
  target: any,
  propertyKey: string,
  descriptor: PropertyDescriptor
) {
  const method = descriptor.value;
  
  descriptor.value = function (...args: any[]) {
    const requiredParameters: number[] = 
      Reflect.getOwnMetadata("required", target, propertyKey) || [];
    
    for (const parameterIndex of requiredParameters) {
      if (args[parameterIndex] === undefined) {
        throw new Error(`Missing required argument at index ${parameterIndex}`);
      }
    }
    
    return method.apply(this, args);
  };
}

class UserService {
  @validate
  createUser(@required name: string, age?: number) {
    console.log(`Creating user: ${name}, ${age}`);
  }
}
```

### 12.5 装饰器组合

```typescript
// 多个装饰器
function first() {
  console.log("first(): factory evaluated");
  return function (target: any, propertyKey: string, descriptor: PropertyDescriptor) {
    console.log("first(): called");
  };
}

function second() {
  console.log("second(): factory evaluated");
  return function (target: any, propertyKey: string, descriptor: PropertyDescriptor) {
    console.log("second(): called");
  };
}

class Example {
  @first()
  @second()
  method() {}
}

// 输出：
// first(): factory evaluated
// second(): factory evaluated
// second(): called
// first(): called
```



---

## 13. TypeScript配置

### 13.1 tsconfig.json详解

```json
{
  "compilerOptions": {
    /* 基本选项 */
    "target": "ES2020",                    // 目标JavaScript版本
    "module": "commonjs",                  // 模块系统
    "lib": ["ES2020", "DOM"],             // 包含的库
    "outDir": "./dist",                    // 输出目录
    "rootDir": "./src",                    // 源代码根目录
    
    /* 严格检查 */
    "strict": true,                        // 启用所有严格类型检查
    "noImplicitAny": true,                // 禁止隐式any
    "strictNullChecks": true,             // 严格空值检查
    "strictFunctionTypes": true,          // 严格函数类型检查
    "strictBindCallApply": true,          // 严格bind/call/apply检查
    "strictPropertyInitialization": true, // 严格属性初始化
    "noImplicitThis": true,               // 禁止隐式this
    "alwaysStrict": true,                 // 始终以严格模式解析
    
    /* 额外检查 */
    "noUnusedLocals": true,               // 检查未使用的局部变量
    "noUnusedParameters": true,           // 检查未使用的参数
    "noImplicitReturns": true,            // 检查函数是否有返回值
    "noFallthroughCasesInSwitch": true,   // 检查switch的fallthrough
    
    /* 模块解析 */
    "moduleResolution": "node",            // 模块解析策略
    "baseUrl": "./",                       // 基础目录
    "paths": {                             // 路径映射
      "@/*": ["src/*"],
      "@components/*": ["src/components/*"]
    },
    "esModuleInterop": true,              // ES模块互操作性
    "allowSyntheticDefaultImports": true, // 允许默认导入
    "resolveJsonModule": true,            // 解析JSON模块
    
    /* Source Map */
    "sourceMap": true,                     // 生成source map
    "inlineSourceMap": false,             // 内联source map
    "declarationMap": true,               // 生成声明文件的source map
    
    /* 声明文件 */
    "declaration": true,                   // 生成.d.ts文件
    "declarationDir": "./types",          // 声明文件输出目录
    
    /* 实验性功能 */
    "experimentalDecorators": true,        // 装饰器
    "emitDecoratorMetadata": true,        // 装饰器元数据
    
    /* 高级选项 */
    "skipLibCheck": true,                  // 跳过库文件类型检查
    "forceConsistentCasingInFileNames": true, // 强制文件名大小写一致
    "removeComments": true,               // 移除注释
    "noEmitOnError": true,                // 有错误时不输出
    "incremental": true,                  // 增量编译
    
    /* JSX */
    "jsx": "react",                        // JSX模式
    // "jsx": "react-jsx",                 // React 17+新JSX转换
  },
  
  /* 包含和排除 */
  "include": [
    "src/**/*"
  ],
  "exclude": [
    "node_modules",
    "dist",
    "**/*.spec.ts"
  ],
  
  /* 引用项目 */
  "references": [
    { "path": "./tsconfig.node.json" }
  ]
}
```

### 13.2 声明文件

```typescript
// types/custom.d.ts

// 声明全局变量
declare const API_URL: string;

// 声明全局函数
declare function myFunction(arg: string): void;

// 声明模块
declare module "my-module" {
  export function doSomething(): void;
  export default class MyClass {}
}

// 扩展现有模块
declare module "express" {
  interface Request {
    user?: {
      id: string;
      name: string;
    };
  }
}

// 声明全局命名空间
declare namespace MyNamespace {
  interface Config {
    apiUrl: string;
    timeout: number;
  }
  
  function initialize(config: Config): void;
}

// 声明全局类型
declare global {
  interface Window {
    myCustomProperty: string;
  }
}

// 类型定义文件
// types/user.d.ts
export interface User {
  id: number;
  name: string;
  email: string;
}

export type UserRole = "admin" | "user" | "guest";

// 三斜线指令
/// <reference types="node" />
/// <reference path="./types.d.ts" />
```

### 13.3 命名空间

```typescript
// 命名空间（不推荐，使用模块代替）
namespace Validation {
  export interface StringValidator {
    isValid(s: string): boolean;
  }
  
  const lettersRegexp = /^[A-Za-z]+$/;
  
  export class LettersOnlyValidator implements StringValidator {
    isValid(s: string): boolean {
      return lettersRegexp.test(s);
    }
  }
}

// 使用
const validator = new Validation.LettersOnlyValidator();
```

---

## 14. 实战项目

### 14.1 Express + TypeScript项目

**项目结构**：
```
project/
├── src/
│   ├── controllers/
│   │   └── user.controller.ts
│   ├── models/
│   │   └── user.model.ts
│   ├── routes/
│   │   └── user.routes.ts
│   ├── middleware/
│   │   └── auth.middleware.ts
│   ├── types/
│   │   └── index.ts
│   ├── utils/
│   │   └── logger.ts
│   └── app.ts
├── package.json
└── tsconfig.json
```

**类型定义**：
```typescript
// src/types/index.ts
export interface User {
  id: string;
  username: string;
  email: string;
  password: string;
  role: UserRole;
  createdAt: Date;
  updatedAt: Date;
}

export type UserRole = "admin" | "user" | "guest";

export interface AuthRequest extends Request {
  user?: {
    id: string;
    role: UserRole;
  };
}

export interface ApiResponse<T> {
  success: boolean;
  data?: T;
  error?: string;
  message?: string;
}

// 配置类型
export interface Config {
  port: number;
  database: {
    url: string;
    options: {
      useNewUrlParser: boolean;
      useUnifiedTopology: boolean;
    };
  };
  jwt: {
    secret: string;
    expiresIn: string;
  };
}
```

**模型**：
```typescript
// src/models/user.model.ts
import mongoose, { Document, Schema } from "mongoose";
import { User as IUser } from "../types";

export interface UserDocument extends Omit<IUser, "id">, Document {}

const userSchema = new Schema<UserDocument>(
  {
    username: {
      type: String,
      required: true,
      unique: true,
      trim: true,
    },
    email: {
      type: String,
      required: true,
      unique: true,
      lowercase: true,
    },
    password: {
      type: String,
      required: true,
    },
    role: {
      type: String,
      enum: ["admin", "user", "guest"],
      default: "user",
    },
  },
  {
    timestamps: true,
  }
);

export const User = mongoose.model<UserDocument>("User", userSchema);
```

**控制器**：
```typescript
// src/controllers/user.controller.ts
import { Request, Response, NextFunction } from "express";
import { User } from "../models/user.model";
import { ApiResponse } from "../types";
import { hashPassword, comparePassword } from "../utils/password";
import { generateToken } from "../utils/jwt";

export class UserController {
  async register(
    req: Request,
    res: Response<ApiResponse<{ token: string }>>,
    next: NextFunction
  ): Promise<void> {
    try {
      const { username, email, password } = req.body;
      
      // 检查用户是否存在
      const existingUser = await User.findOne({ $or: [{ username }, { email }] });
      if (existingUser) {
        res.status(400).json({
          success: false,
          error: "User already exists",
        });
        return;
      }
      
      // 创建用户
      const hashedPassword = await hashPassword(password);
      const user = new User({
        username,
        email,
        password: hashedPassword,
      });
      await user.save();
      
      // 生成token
      const token = generateToken({ id: user._id, role: user.role });
      
      res.status(201).json({
        success: true,
        data: { token },
        message: "User registered successfully",
      });
    } catch (error) {
      next(error);
    }
  }
  
  async login(
    req: Request,
    res: Response<ApiResponse<{ token: string }>>,
    next: NextFunction
  ): Promise<void> {
    try {
      const { email, password } = req.body;
      
      // 查找用户
      const user = await User.findOne({ email });
      if (!user) {
        res.status(401).json({
          success: false,
          error: "Invalid credentials",
        });
        return;
      }
      
      // 验证密码
      const isValid = await comparePassword(password, user.password);
      if (!isValid) {
        res.status(401).json({
          success: false,
          error: "Invalid credentials",
        });
        return;
      }
      
      // 生成token
      const token = generateToken({ id: user._id, role: user.role });
      
      res.json({
        success: true,
        data: { token },
        message: "Login successful",
      });
    } catch (error) {
      next(error);
    }
  }
  
  async getProfile(
    req: Request,
    res: Response<ApiResponse<Omit<IUser, "password">>>,
    next: NextFunction
  ): Promise<void> {
    try {
      const userId = (req as AuthRequest).user?.id;
      
      const user = await User.findById(userId).select("-password");
      if (!user) {
        res.status(404).json({
          success: false,
          error: "User not found",
        });
        return;
      }
      
      res.json({
        success: true,
        data: user.toObject(),
      });
    } catch (error) {
      next(error);
    }
  }
}
```

**中间件**：
```typescript
// src/middleware/auth.middleware.ts
import { Request, Response, NextFunction } from "express";
import jwt from "jsonwebtoken";
import { AuthRequest } from "../types";

export const authenticate = (
  req: Request,
  res: Response,
  next: NextFunction
): void => {
  try {
    const token = req.headers.authorization?.split(" ")[1];
    
    if (!token) {
      res.status(401).json({
        success: false,
        error: "No token provided",
      });
      return;
    }
    
    const decoded = jwt.verify(token, process.env.JWT_SECRET!) as {
      id: string;
      role: string;
    };
    
    (req as AuthRequest).user = decoded;
    next();
  } catch (error) {
    res.status(401).json({
      success: false,
      error: "Invalid token",
    });
  }
};

export const authorize = (...roles: string[]) => {
  return (req: Request, res: Response, next: NextFunction): void => {
    const user = (req as AuthRequest).user;
    
    if (!user || !roles.includes(user.role)) {
      res.status(403).json({
        success: false,
        error: "Forbidden",
      });
      return;
    }
    
    next();
  };
};
```

**路由**：
```typescript
// src/routes/user.routes.ts
import { Router } from "express";
import { UserController } from "../controllers/user.controller";
import { authenticate, authorize } from "../middleware/auth.middleware";
import { validate } from "../middleware/validation.middleware";
import { registerSchema, loginSchema } from "../validation/user.validation";

const router = Router();
const userController = new UserController();

router.post("/register", validate(registerSchema), userController.register);
router.post("/login", validate(loginSchema), userController.login);
router.get("/profile", authenticate, userController.getProfile);
router.get("/admin", authenticate, authorize("admin"), (req, res) => {
  res.json({ message: "Admin only" });
});

export default router;
```

**应用入口**：
```typescript
// src/app.ts
import express, { Application, Request, Response, NextFunction } from "express";
import cors from "cors";
import helmet from "helmet";
import morgan from "morgan";
import mongoose from "mongoose";
import userRoutes from "./routes/user.routes";
import { errorHandler } from "./middleware/error.middleware";

class App {
  public app: Application;
  
  constructor() {
    this.app = express();
    this.connectDatabase();
    this.initializeMiddleware();
    this.initializeRoutes();
    this.initializeErrorHandling();
  }
  
  private async connectDatabase(): Promise<void> {
    try {
      await mongoose.connect(process.env.MONGODB_URI!);
      console.log("MongoDB connected");
    } catch (error) {
      console.error("MongoDB connection error:", error);
      process.exit(1);
    }
  }
  
  private initializeMiddleware(): void {
    this.app.use(cors());
    this.app.use(helmet());
    this.app.use(morgan("dev"));
    this.app.use(express.json());
    this.app.use(express.urlencoded({ extended: true }));
  }
  
  private initializeRoutes(): void {
    this.app.get("/", (req: Request, res: Response) => {
      res.json({ message: "API is running" });
    });
    
    this.app.use("/api/users", userRoutes);
  }
  
  private initializeErrorHandling(): void {
    this.app.use(errorHandler);
  }
  
  public listen(port: number): void {
    this.app.listen(port, () => {
      console.log(`Server running on port ${port}`);
    });
  }
}

export default App;
```

**启动文件**：
```typescript
// src/server.ts
import dotenv from "dotenv";
import App from "./app";

dotenv.config();

const port = parseInt(process.env.PORT || "3000", 10);
const app = new App();

app.listen(port);
```

**package.json**：
```json
{
  "name": "express-typescript-api",
  "version": "1.0.0",
  "scripts": {
    "dev": "ts-node-dev --respawn --transpile-only src/server.ts",
    "build": "tsc",
    "start": "node dist/server.js",
    "lint": "eslint . --ext .ts",
    "test": "jest"
  },
  "dependencies": {
    "express": "^4.18.2",
    "mongoose": "^7.0.0",
    "bcrypt": "^5.1.0",
    "jsonwebtoken": "^9.0.0",
    "cors": "^2.8.5",
    "helmet": "^7.0.0",
    "morgan": "^1.10.0",
    "dotenv": "^16.0.3",
    "joi": "^17.9.0"
  },
  "devDependencies": {
    "@types/express": "^4.17.17",
    "@types/node": "^20.0.0",
    "@types/bcrypt": "^5.0.0",
    "@types/jsonwebtoken": "^9.0.2",
    "@types/cors": "^2.8.13",
    "@types/morgan": "^1.9.4",
    "typescript": "^5.0.0",
    "ts-node-dev": "^2.0.0",
    "@typescript-eslint/eslint-plugin": "^5.59.0",
    "@typescript-eslint/parser": "^5.59.0",
    "eslint": "^8.40.0"
  }
}
```

### 14.2 TypeScript最佳实践

```typescript
// ===== 1. 使用严格模式 =====
// tsconfig.json: "strict": true

// ===== 2. 避免any =====
// 不好
function process(data: any) {
  return data;
}

// 好
function process<T>(data: T): T {
  return data;
}

// ===== 3. 使用联合类型而非枚举 =====
// 不推荐
enum Status {
  Active,
  Inactive,
}

// 推荐
type Status = "active" | "inactive";

// ===== 4. 使用类型守卫 =====
function isString(value: unknown): value is string {
  return typeof value === "string";
}

// ===== 5. 使用readonly =====
interface Config {
  readonly apiUrl: string;
  readonly timeout: number;
}

// ===== 6. 使用可选链和空值合并 =====
const value = data?.user?.name ?? "默认名称";

// ===== 7. 类型导入 =====
import type { User } from "./types";

// ===== 8. 避免类型断言 =====
// 不好
const user = response as User;

// 好
function isUser(obj: any): obj is User {
  return obj && typeof obj.id === "string";
}

if (isUser(response)) {
  const user = response;
}

// ===== 9. 使用映射类型 =====
type ReadonlyUser = Readonly<User>;
type PartialUser = Partial<User>;
type RequiredUser = Required<User>;

// ===== 10. 文档注释 =====
/**
 * 创建用户
 * @param name - 用户名
 * @param email - 邮箱
 * @returns 创建的用户对象
 */
function createUser(name: string, email: string): User {
  // ...
}
```

---

## 15. 总结

### 15.1 Node.js核心要点

```
✓ 事件驱动、非阻塞I/O
✓ 模块系统（CommonJS、ESM）
✓ 核心模块（fs、path、http、events）
✓ 异步编程（回调、Promise、async/await）
✓ Express框架与中间件
✓ 数据库集成（MongoDB、PostgreSQL）
✓ RESTful API设计
```

### 15.2 TypeScript核心要点

```
✓ 静态类型系统
✓ 接口与类型别名
✓ 泛型编程
✓ 高级类型（映射类型、条件类型）
✓ 装饰器
✓ 类型守卫与类型推断
✓ 配置与声明文件
```

### 15.3 学习路径建议

```
初级：
1. Node.js基础语法
2. 核心模块使用
3. TypeScript基本类型
4. 接口与类

中级：
1. Express框架
2. 数据库操作
3. 泛型与高级类型
4. 异步编程模式

高级：
1. 微服务架构
2. 性能优化
3. 装饰器与元编程
4. 类型体操
```

---

**文档完成！**
**版本**：v1.0
**最后更新**：2024年

> 本文档涵盖了Node.js和TypeScript从基础到高级的所有核心知识点，适合系统学习和日常参考。

