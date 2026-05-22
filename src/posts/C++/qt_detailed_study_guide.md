# Qt 详细学习教程

这份文档的目标不是只让你“知道 Qt 有什么”，而是让你能从 0 开始，逐步做出一个真正可用的桌面应用。

你会学到：

1. Qt 项目怎么创建。
2. Qt 工程里 `.pro`、`CMakeLists.txt`、`ui`、`qrc`、`moc` 分别是什么。
3. Qt 在编码层面提供了哪些核心能力。
4. 怎么写窗口、页面、布局、控件和样式。
5. 怎么写常见功能，比如网络、Socket、JSON、文件、打印。
6. 怎么组织一个稍微正式一点的 Qt 项目。
7. 最后怎么在 macOS 和 Windows 上打包。

本文以 Qt 6 为主，Qt 5 大部分写法也通用，但个别模块和 CMake 写法会略有区别。

---

## 1. 先理解 Qt 是什么

Qt 是一个跨平台 C++ 应用开发框架。它不只是“做界面”的库，而是一整套应用开发生态，包含：

- GUI 和窗口系统
- 信号槽机制
- 字符串、容器、文件、时间、线程
- 网络通信
- JSON / XML
- 数据库
- 绘图和打印
- 国际化
- 打包部署工具

你可以把 Qt 理解成：

“用统一的 C++ API，去写 Windows、macOS、Linux 上的桌面程序。”

如果你以前只写过标准 C++，那你会发现 Qt 相当于在标准库之外，又给你配了一整套更偏应用开发的工具箱。

---

## 2. Qt 学习建议路线

建议按这个顺序学：

1. 能创建并运行一个 Qt Widgets 项目
2. 学会控件、布局、信号槽
3. 学会 UI 文件和代码方式写页面
4. 学会资源文件、图片、图标、字体
5. 学会文件读写、JSON、配置保存
6. 学会网络请求和 TCP/UDP Socket
7. 学会模型视图、表格、列表
8. 学会线程和耗时任务处理
9. 学会样式表 QSS
10. 学会打包部署

如果你一开始就上来做复杂业务，通常会卡在“界面能显示，但结构全乱了”。所以建议先把 Qt 的基本工作方式吃透。

---

## 3. 开发环境准备

## 3.1 安装内容

常见安装方式是安装：

- Qt Online Installer
- Qt Creator
- 目标版本 Qt，比如 `Qt 6.5`、`Qt 6.6`、`Qt 6.7`
- 对应编译器套件

Windows 常见组合：

- Qt + MinGW
- Qt + MSVC

macOS 常见组合：

- Qt + Clang

如果你是新手，优先用 Qt Creator 配套的套件，省心很多。

## 3.2 Qt Creator 是什么

Qt Creator 是 Qt 官方 IDE，帮你处理：

- 新建项目
- 编译构建
- 运行调试
- 设计 UI
- 管理 Kits
- 运行部署

不是必须用它，但对入门最友好。

---

## 4. 创建第一个 Qt 项目

Qt 常见项目类型有：

- `Qt Widgets Application`：传统桌面窗口程序，最适合入门
- `Qt Quick Application`：QML 界面，更适合现代动态 UI
- `Console Application`：控制台程序，也能用 Qt 核心模块

如果你的目标是“做一个管理工具、客户端、配置工具、桌面程序”，先学 `Qt Widgets` 最稳。

## 4.1 用 Qt Creator 创建 Widgets 项目

操作路径一般是：

1. `File`
2. `New Project`
3. 选择 `Application`
4. 选择 `Qt Widgets Application`
5. 填写项目名和路径
6. 选择构建系统：
   - 推荐 `CMake`
   - 老项目可能是 `qmake`
7. 选择 Kit
8. 完成创建

创建后你常会看到这些文件：

```text
MyApp/
  CMakeLists.txt
  main.cpp
  mainwindow.h
  mainwindow.cpp
  mainwindow.ui
```

---

## 5. 这些文件分别是干什么的

## 5.1 `main.cpp`

程序入口。最基础的 Qt Widgets 程序大概是这样：

```cpp
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}
```

这里最重要的是：

- `QApplication`：管理整个 GUI 应用
- `w.show()`：显示主窗口
- `app.exec()`：进入事件循环

“事件循环”是 GUI 程序的核心。只有进入事件循环，按钮点击、窗口刷新、输入响应这些事情才会真正发生。

## 5.2 `mainwindow.h`

窗口类声明，一般继承 `QMainWindow`。

```cpp
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};

#endif
```

这里最关键的是 `Q_OBJECT`。

它告诉 Qt：这个类要启用元对象系统，支持：

- 信号槽
- 运行时类型信息
- 动态属性
- `tr()` 国际化

如果类里要写 `signals:`、`slots:`，通常就离不开 `Q_OBJECT`。

## 5.3 `mainwindow.cpp`

窗口逻辑实现文件。

```cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
```

`ui->setupUi(this)` 的作用是：把 `mainwindow.ui` 里设计的界面，真正创建成控件对象并挂到当前窗口上。

## 5.4 `mainwindow.ui`

这是 Qt Designer 生成的 UI 描述文件，本质是 XML。

你可以拖按钮、标签、输入框，然后生成页面。它适合：

- 表单类页面
- 工具类页面
- 后台管理风格页面
- 快速原型

优点是搭界面很快。

缺点是复杂动态界面时，纯拖拽会越来越难维护，所以实战中经常是：

- 静态框架用 `.ui`
- 动态区域用 C++ 代码创建

## 5.5 `CMakeLists.txt`

Qt 6 推荐用 CMake。一个常见最小配置如下：

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyQtApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network PrintSupport)

qt_standard_project_setup()

qt_add_executable(MyQtApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(MyQtApp PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::PrintSupport
)
```

这里几个点你要记住：

- `find_package(Qt6 ...)`：查找 Qt 模块
- `qt_add_executable(...)`：创建 Qt 可执行程序
- `target_link_libraries(...)`：链接需要的模块

如果你要用 JSON，不需要额外模块，因为它属于 `QtCore`。

## 5.6 `.pro` 文件是什么

如果你接触的是老项目，很可能用的是 `qmake`，你会看到：

```pro
QT += core gui widgets network printsupport

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui
```

现在新项目更推荐 CMake，但很多公司和历史项目还在用 `qmake`，所以你最好能看懂。

---

## 6. Qt 的核心思想

你如果把下面几件事真正理解了，Qt 基本就算入门了：

- 对象树
- 信号槽
- 事件循环
- 事件系统
- 资源系统
- 元对象系统

## 6.1 对象树与父子关系

Qt 的很多对象都继承自 `QObject`。

`QObject` 有一个很重要的机制：父子对象关系。

```cpp
QPushButton *button = new QPushButton(this);
```

这里 `this` 是父对象。父对象销毁时，子对象会自动销毁。

这意味着在 Qt 里，你经常不需要手动 `delete` 每个控件，只要父子关系正确，生命周期就能自动管理。

这也是 Qt UI 代码里大量 `new` 却不总是手动释放的原因。

## 6.2 信号槽

这是 Qt 最有代表性的机制。

比如按钮点击：

```cpp
connect(ui->pushButton, &QPushButton::clicked,
        this, &MainWindow::onButtonClicked);
```

意思是：

- 当按钮发出 `clicked` 信号
- 调用当前窗口的 `onButtonClicked` 函数

槽函数可以是普通成员函数：

```cpp
void MainWindow::onButtonClicked()
{
    ui->label->setText("按钮被点击了");
}
```

信号槽的意义不是“少写回调”这么简单，而是：

- 解耦
- 对象之间通信清晰
- 很适合 GUI 驱动的程序

## 6.3 事件循环

GUI 程序不是“从上往下执行完就结束”的脚本，而是长期运行、等待事件。

事件包括：

- 鼠标点击
- 键盘输入
- 定时器触发
- 网络数据到达
- 窗口重绘

`app.exec()` 启动的就是这个循环。

## 6.4 事件系统

Qt 中很多行为本质上都是事件。

你可以重写事件函数：

```cpp
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QMainWindow::mousePressEvent(event);
}
```

常见事件：

- `paintEvent`
- `resizeEvent`
- `mousePressEvent`
- `mouseMoveEvent`
- `keyPressEvent`
- `closeEvent`

适合在需要更细粒度控制时使用。

## 6.5 元对象系统与 moc

Qt 有一个额外步骤：`moc`，即 Meta-Object Compiler。

它会处理带有 `Q_OBJECT` 的类，生成额外代码，使信号槽、反射、动态调用这些特性成立。

所以 Qt 不是“纯标准 C++ 零扩展风格”的库，它有自己的对象系统。

这也是为什么：

- 少了 `Q_OBJECT` 可能信号槽失效
- 改了类结构后有时要重新运行 CMake 或重新构建

---

## 7. Qt 常用模块一览

你可以把 Qt 提供的能力按模块理解：

- `QtCore`：字符串、容器、文件、时间、线程、JSON、对象系统
- `QtGui`：绘图、字体、图片、事件、剪贴板
- `QtWidgets`：按钮、输入框、表格、窗口等传统控件
- `QtNetwork`：TCP、UDP、HTTP
- `QtSql`：数据库
- `QtPrintSupport`：打印
- `QtMultimedia`：音视频
- `QtCharts`：图表
- `QtSvg`：SVG 处理

刚开始最常用的是：

- `QtCore`
- `QtGui`
- `QtWidgets`
- `QtNetwork`
- `QtPrintSupport`

---

## 8. Qt 在编码上提供了哪些“好用的东西”

很多人学 Qt 时，只盯着按钮和窗口，但其实 Qt 的真正价值之一，是它给了你一套应用开发常用基础设施。

## 8.1 字符串：`QString`

Qt 最常用字符串类是 `QString`。

```cpp
QString name = "Alice";
QString text = QString("hello, %1").arg(name);
```

常见操作：

```cpp
QString s = "  abc  ";
s = s.trimmed();        // 去首尾空白
s = s.toUpper();        // 转大写
bool ok = s.isEmpty();  // 是否为空
```

和标准字符串转换：

```cpp
std::string stds = s.toStdString();
QString qs = QString::fromStdString(stds);
```

如果项目里大量使用 Qt API，建议主流程统一用 `QString`。

## 8.2 容器

Qt 常见容器：

- `QList`
- `QVector`
- `QMap`
- `QSet`
- `QHash`

例如：

```cpp
QVector<int> nums{1, 2, 3};
QMap<QString, int> scores;
scores["Tom"] = 95;
```

新项目里，也有人会混用 STL。可以，但要保持团队风格统一。

## 8.3 文件与目录

```cpp
QFile file("config.json");
if (file.open(QIODevice::ReadOnly)) {
    QByteArray data = file.readAll();
    file.close();
}
```

目录处理：

```cpp
QDir dir;
dir.mkpath("output/logs");
```

路径信息：

```cpp
QFileInfo info("/tmp/a.txt");
QString name = info.fileName();
QString suffix = info.suffix();
```

## 8.4 时间与日期

```cpp
QDateTime now = QDateTime::currentDateTime();
QString text = now.toString("yyyy-MM-dd HH:mm:ss");
```

## 8.5 定时器

```cpp
QTimer *timer = new QTimer(this);
connect(timer, &QTimer::timeout, this, &MainWindow::updateClock);
timer->start(1000);
```

这对轮询、倒计时、心跳等很有用。

## 8.6 日志输出

Qt 提供：

- `qDebug()`
- `qInfo()`
- `qWarning()`
- `qCritical()`

```cpp
qDebug() << "port =" << port;
qWarning() << "配置文件打开失败";
```

比 `std::cout` 更适合 Qt 项目调试。

## 8.7 线程

常用方式有：

- `QThread`
- `QObject` + `moveToThread`
- `QtConcurrent`

GUI 线程里不要做长时间阻塞操作，否则界面会卡死。

---

## 9. 页面怎么写：先学 Widgets

Qt Widgets 页面开发通常有两种方式：

1. 拖拽 `.ui`
2. 纯代码创建控件

真实项目经常是两者结合。

## 9.1 常见顶层窗口类

- `QMainWindow`：有菜单栏、工具栏、状态栏，适合主程序
- `QDialog`：对话框
- `QWidget`：通用页面基类

如果你做主程序，一般优先用 `QMainWindow`。

## 9.2 常见控件

- `QLabel`：文字/图片显示
- `QPushButton`：按钮
- `QLineEdit`：单行输入
- `QTextEdit` / `QPlainTextEdit`：多行文本
- `QCheckBox`：复选框
- `QRadioButton`：单选按钮
- `QComboBox`：下拉框
- `QSpinBox` / `QDoubleSpinBox`：数值输入
- `QTableWidget`：简单表格
- `QTableView`：配合模型的表格
- `QListWidget` / `QListView`
- `QTreeWidget` / `QTreeView`
- `QTabWidget`
- `QGroupBox`
- `QProgressBar`

## 9.3 页面布局非常重要

初学 Qt 最常见错误之一，是把控件直接“摆坐标”。

虽然可以 `move(x, y)`，但真正项目里应该尽量依赖布局器：

- `QVBoxLayout`：垂直布局
- `QHBoxLayout`：水平布局
- `QGridLayout`：网格布局
- `QFormLayout`：表单布局

布局的好处：

- 自动适配窗口大小变化
- 更容易维护
- 更适合跨平台
- 更适合不同字体和 DPI

## 9.4 代码方式创建一个页面

```cpp
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Qt Demo");

    auto *nameEdit = new QLineEdit;
    auto *button = new QPushButton("提交");
    auto *resultLabel = new QLabel("请输入内容");

    auto *layout = new QVBoxLayout;
    layout->addWidget(nameEdit);
    layout->addWidget(button);
    layout->addWidget(resultLabel);
    window.setLayout(layout);

    QObject::connect(button, &QPushButton::clicked, [&]() {
        resultLabel->setText("你好，" + nameEdit->text());
    });

    window.resize(360, 160);
    window.show();
    return app.exec();
}
```

这段代码已经体现了 Qt 页面开发的基本模式：

- 创建控件
- 放进布局
- 绑定信号槽
- 显示窗口

---

## 10. 用 `.ui` 写页面时要注意什么

`.ui` 很方便，但不要把所有逻辑都堆进 Designer 思维里。

推荐分工：

- `.ui` 负责静态结构
- `.cpp` 负责逻辑
- 动态区域用代码创建

比如：

- 固定表单、按钮区、标题区放 `.ui`
- 列表项动态生成、复杂卡片区域、数据驱动界面放代码

## 10.1 给控件起好名字

别保留默认名字：

- 不要全是 `pushButton`, `pushButton_2`
- 尽量改成 `connectButton`, `sendButton`, `messageEdit`

这样你写逻辑时可读性会好很多。

## 10.2 在构造函数中做初始化

常见结构：

```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initUi();
    initConnections();
    loadSettings();
}
```

拆分成几个函数比把所有东西塞进构造函数更清楚。

---

## 11. 一个稍微像样的页面初始化结构

建议你尽快养成这种写法：

```cpp
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initUi();
    void initConnections();
    void initNetwork();
    void loadSettings();
    void saveSettings();

private slots:
    void onConnectClicked();
    void onSendClicked();
    void onSocketReadyRead();

private:
    Ui::MainWindow *ui;
};
```

为什么推荐这样分？

- `initUi()`：只处理界面初始状态
- `initConnections()`：只处理信号槽绑定
- `initNetwork()`：只处理网络对象初始化
- `loadSettings()`：只处理配置加载

这样页面越来越大时，不容易失控。

---

## 12. Qt 资源系统：图片、图标、样式文件怎么带进程序

Qt 有资源系统，也就是 `.qrc` 文件。

比如：

```xml
<RCC>
    <qresource prefix="/">
        <file>assets/logo.png</file>
        <file>assets/main.qss</file>
    </qresource>
</RCC>
```

使用时：

```cpp
QPixmap pix(":/assets/logo.png");
QFile file(":/assets/main.qss");
```

优点：

- 资源会被打进程序
- 不依赖外部相对路径
- 部署更方便

对图标、默认配置、QSS、内置图片很有用。

---

## 13. 样式怎么写：QSS 基础

Qt Widgets 的样式可以用 QSS，也就是类似 CSS 的语法。

例如：

```css
QWidget {
    background: #f5f7fa;
    color: #222222;
    font-size: 14px;
}

QPushButton {
    background: #2d7ef7;
    color: white;
    border: none;
    border-radius: 6px;
    padding: 8px 14px;
}

QPushButton:hover {
    background: #1f6de0;
}

QLineEdit {
    background: white;
    border: 1px solid #cfd6df;
    border-radius: 6px;
    padding: 6px 8px;
}
```

加载方式：

```cpp
QFile file(":/assets/main.qss");
if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
}
```

## 13.1 QSS 适合什么

适合：

- 颜色
- 边框
- 圆角
- 间距
- hover / pressed 状态
- 控件基础视觉统一

## 13.2 QSS 不擅长什么

不擅长：

- 太复杂的自定义绘制
- 高度动态的动画效果
- 所有平台都完全一致的像素级外观

遇到复杂视觉时，可能要结合：

- 自定义控件
- `paintEvent`
- 代理绘制

---

## 14. 功能怎么写：推荐项目结构

当项目开始变大，不要把一切都写在 `MainWindow` 里。

推荐目录思路：

```text
src/
  main.cpp
  mainwindow.cpp
  pages/
    homepage.cpp
    settingpage.cpp
  widgets/
    statuscard.cpp
  network/
    tcpclient.cpp
    httpclient.cpp
  service/
    configservice.cpp
    printservice.cpp
include/
  mainwindow.h
  pages/
  widgets/
  network/
  service/
resources/
  assets.qrc
  qss/
  images/
```

常见分层思路：

- 页面层：负责 UI 和交互
- 服务层：负责配置、网络、打印、存储
- 数据层：负责数据模型、协议结构

一个原则：

“界面类尽量别既管按钮，又管协议，又管文件，又管打印。”

---

## 15. Socket 怎么写

Qt 网络编程里，TCP 最常用类是：

- `QTcpSocket`
- `QTcpServer`

UDP 最常用类是：

- `QUdpSocket`

如果你做客户端工具、聊天程序、设备调试工具、上位机，`QTcpSocket` 非常常见。

## 15.1 TCP 客户端示例

```cpp
#include <QTcpSocket>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr)
        : QObject(parent)
        , socket_(new QTcpSocket(this))
    {
        connect(socket_, &QTcpSocket::connected, this, &TcpClient::connected);
        connect(socket_, &QTcpSocket::disconnected, this, &TcpClient::disconnected);
        connect(socket_, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
        connect(socket_, &QTcpSocket::errorOccurred, this, &TcpClient::onErrorOccurred);
    }

    void connectToHost(const QString &host, quint16 port)
    {
        socket_->connectToHost(host, port);
    }

    void sendText(const QString &text)
    {
        socket_->write(text.toUtf8());
    }

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &text);
    void errorOccurred(const QString &text);

private slots:
    void onReadyRead()
    {
        const QByteArray data = socket_->readAll();
        emit messageReceived(QString::fromUtf8(data));
    }

    void onErrorOccurred(QAbstractSocket::SocketError)
    {
        emit errorOccurred(socket_->errorString());
    }

private:
    QTcpSocket *socket_;
};
```

## 15.2 在页面中使用

```cpp
void MainWindow::initNetwork()
{
    client_ = new TcpClient(this);

    connect(client_, &TcpClient::connected, this, [this]() {
        ui->statusLabel->setText("已连接");
    });

    connect(client_, &TcpClient::messageReceived, this, [this](const QString &text) {
        ui->logEdit->appendPlainText(text);
    });

    connect(client_, &TcpClient::errorOccurred, this, [this](const QString &text) {
        ui->statusLabel->setText("错误: " + text);
    });
}
```

## 15.3 为什么不能只靠一次 `readyRead`

TCP 是字节流，不保证一条业务消息正好一次读完。

所以你真正做协议时，通常需要：

1. 自己定义消息边界
2. 维护缓冲区
3. 按协议拆包

常见协议方式：

- 固定长度
- 分隔符
- 长度字段 + 正文

如果你做的是正式项目，建议用“长度字段 + 正文”。

## 15.4 TCP 服务端示例

```cpp
#include <QTcpServer>
#include <QTcpSocket>

class TcpServer : public QObject
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr)
        : QObject(parent)
        , server_(new QTcpServer(this))
    {
        connect(server_, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
    }

    bool start(quint16 port)
    {
        return server_->listen(QHostAddress::Any, port);
    }

private slots:
    void onNewConnection()
    {
        while (server_->hasPendingConnections()) {
            QTcpSocket *client = server_->nextPendingConnection();

            connect(client, &QTcpSocket::readyRead, this, [client]() {
                const QByteArray data = client->readAll();
                client->write("server recv: " + data);
            });

            connect(client, &QTcpSocket::disconnected, client, &QObject::deleteLater);
        }
    }

private:
    QTcpServer *server_;
};
```

---

## 16. JSON 怎么处理

Qt JSON 相关类主要有：

- `QJsonDocument`
- `QJsonObject`
- `QJsonArray`
- `QJsonValue`

## 16.1 生成 JSON

```cpp
QJsonObject obj;
obj["name"] = "Alice";
obj["age"] = 20;
obj["online"] = true;

QJsonArray hobbies;
hobbies.append("coding");
hobbies.append("music");
obj["hobbies"] = hobbies;

QJsonDocument doc(obj);
QByteArray data = doc.toJson(QJsonDocument::Indented);
```

## 16.2 解析 JSON

```cpp
QByteArray data = R"({
    "name": "Tom",
    "age": 18,
    "skills": ["Qt", "C++"]
})";

QJsonParseError error;
QJsonDocument doc = QJsonDocument::fromJson(data, &error);

if (error.error == QJsonParseError::NoError && doc.isObject()) {
    QJsonObject obj = doc.object();
    QString name = obj["name"].toString();
    int age = obj["age"].toInt();
    QJsonArray skills = obj["skills"].toArray();
}
```

## 16.3 保存配置文件

```cpp
void MainWindow::saveSettings()
{
    QJsonObject obj;
    obj["host"] = ui->hostEdit->text();
    obj["port"] = ui->portSpinBox->value();

    QFile file("config.json");
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
}
```

## 16.4 读取配置文件

```cpp
void MainWindow::loadSettings()
{
    QFile file("config.json");
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    const QByteArray data = file.readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }

    const QJsonObject obj = doc.object();
    ui->hostEdit->setText(obj["host"].toString("127.0.0.1"));
    ui->portSpinBox->setValue(obj["port"].toInt(9000));
}
```

如果只是保存简单用户设置，还可以用 `QSettings`，它会更省事。

---

## 17. 打印怎么做

Qt 打印常用模块是 `QtPrintSupport`，主要类有：

- `QPrinter`
- `QPrintDialog`
- `QPrintPreviewDialog`
- `QTextDocument`
- `QPainter`

## 17.1 打印纯文本

```cpp
#include <QPrintDialog>
#include <QPrinter>
#include <QTextDocument>

void MainWindow::printText()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QTextDocument doc;
    doc.setPlainText(ui->logEdit->toPlainText());
    doc.print(&printer);
}
```

## 17.2 打印 HTML 富文本

```cpp
void MainWindow::printHtml()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QTextDocument doc;
    doc.setHtml("<h1>报表</h1><p>这是一段打印内容。</p>");
    doc.print(&printer);
}
```

## 17.3 自定义绘制打印

如果你要打印票据、标签、复杂报表，可以：

- 使用 `QPainter` 在 `QPrinter` 上绘制
- 自己控制线条、文字、图片位置

思路和自定义绘图类似。

---

## 18. 页面上的业务功能一般怎么写

一个按钮对应的业务，推荐分成三段：

1. 取输入
2. 调服务
3. 回填结果

例如发送消息：

```cpp
void MainWindow::onSendClicked()
{
    const QString text = ui->messageEdit->text().trimmed();
    if (text.isEmpty()) {
        ui->statusLabel->setText("消息不能为空");
        return;
    }

    client_->sendText(text);
    ui->logEdit->appendPlainText("me: " + text);
    ui->messageEdit->clear();
}
```

复杂一点时，不要把所有流程都塞到按钮槽函数里，应该抽服务函数：

```cpp
void MainWindow::onExportClicked()
{
    ExportOptions options;
    options.filePath = ui->pathEdit->text();
    options.includeHeader = ui->headerCheckBox->isChecked();

    const bool ok = exportService_->exportReport(options);
    ui->statusLabel->setText(ok ? "导出成功" : "导出失败");
}
```

这样后续测试和维护都更轻松。

---

## 19. 自定义页面与复用组件

当页面复杂后，你不应该所有区域都写在主窗口里。

比如你有一块“连接状态卡片”，就可以单独封装一个控件：

```cpp
class StatusCard : public QWidget
{
    Q_OBJECT

public:
    explicit StatusCard(QWidget *parent = nullptr);
    void setTitle(const QString &title);
    void setStatusText(const QString &text);

private:
    QLabel *titleLabel_;
    QLabel *statusLabel_;
};
```

这样好处很大：

- 复用方便
- 页面主类更干净
- 样式可以集中处理

如果一个界面区域反复出现，或者本身逻辑独立，就应该考虑单独封装为自定义控件。

---

## 20. 表格、列表、树怎么选

很多业务系统离不开这些控件。

## 20.1 简单场景

如果你只是快速展示少量数据：

- `QTableWidget`
- `QListWidget`
- `QTreeWidget`

优点是上手快。

## 20.2 正式场景

如果数据量大、需要排序、过滤、复用模型：

- `QTableView`
- `QListView`
- `QTreeView`
- 搭配 `QAbstractItemModel` 或标准模型

这是 Qt 很重要的一套“模型/视图”体系。入门阶段可以先会用 Widget 版，后面再学 Model/View。

---

## 21. 网络请求怎么做：HTTP

除了 Socket，Qt 还常用于 HTTP 请求。

常见类：

- `QNetworkAccessManager`
- `QNetworkRequest`
- `QNetworkReply`

示例：

```cpp
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

void MainWindow::requestData()
{
    auto *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("https://example.com/api/data"));

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray data = reply->readAll();
        ui->logEdit->appendPlainText(QString::fromUtf8(data));
        reply->deleteLater();
    });
}
```

如果请求很多，通常把 `QNetworkAccessManager` 做成类成员，而不是每次新建。

---

## 22. 文件选择、消息框、对话框

这些也是非常常用的基础能力。

## 22.1 文件选择

```cpp
QString path = QFileDialog::getOpenFileName(
    this,
    "选择文件",
    QString(),
    "JSON Files (*.json);;All Files (*.*)");
```

## 22.2 消息框

```cpp
QMessageBox::information(this, "提示", "保存成功");
QMessageBox::warning(this, "警告", "参数不合法");
QMessageBox::critical(this, "错误", "连接失败");
```

## 22.3 自定义对话框

如果输入项较多，建议单独做一个 `QDialog`，不要什么都挤在主窗口上。

---

## 23. 配置持久化：`QSettings`

如果你只是保存窗口大小、最近地址、复选框状态，`QSettings` 非常省事。

```cpp
QSettings settings("MyCompany", "MyApp");
settings.setValue("host", ui->hostEdit->text());
settings.setValue("port", ui->portSpinBox->value());
```

读取：

```cpp
QSettings settings("MyCompany", "MyApp");
ui->hostEdit->setText(settings.value("host", "127.0.0.1").toString());
ui->portSpinBox->setValue(settings.value("port", 9000).toInt());
```

它会自动存到系统适合的位置：

- Windows 注册表或配置目录
- macOS 偏好设置
- Linux 配置目录

---

## 24. 绘图与自定义控件

如果现成控件不够，你可以重写 `paintEvent()`。

```cpp
void MyWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor("#2d7ef7"));
    painter.drawRoundedRect(rect().adjusted(10, 10, -10, -10), 12, 12);
}
```

常见用途：

- 仪表盘
- 自定义状态块
- 特殊卡片
- 波形图
- 轻量图表

如果你要高度定制视觉，Widgets 体系里这是一条很重要的路。

---

## 25. 多线程：避免界面卡死

耗时操作不要直接写在按钮点击里，比如：

- 大文件读写
- 复杂计算
- 阻塞网络操作
- 批量导出

否则界面会卡住。

常见写法是“工作对象 + 线程”：

```cpp
class Worker : public QObject
{
    Q_OBJECT

public slots:
    void doWork()
    {
        // 耗时任务
        emit finished();
    }

signals:
    void finished();
};
```

然后：

```cpp
QThread *thread = new QThread(this);
Worker *worker = new Worker;
worker->moveToThread(thread);

connect(thread, &QThread::started, worker, &Worker::doWork);
connect(worker, &Worker::finished, thread, &QThread::quit);
connect(worker, &Worker::finished, worker, &QObject::deleteLater);
connect(thread, &QThread::finished, thread, &QObject::deleteLater);

thread->start();
```

牢记一条：

“不要在子线程里直接操作 GUI 控件。”

GUI 更新应该回到主线程做。

---

## 26. 信号槽连接的几种常见方式

## 26.1 新式连接语法

推荐：

```cpp
connect(ui->sendButton, &QPushButton::clicked,
        this, &MainWindow::onSendClicked);
```

优点：

- 编译期检查
- 类型更安全

## 26.2 Lambda 连接

```cpp
connect(ui->clearButton, &QPushButton::clicked, this, [this]() {
    ui->logEdit->clear();
});
```

适合小逻辑，别把很长业务都塞进 lambda。

## 26.3 自动连接

Qt Designer 时代还常见：

```cpp
void MainWindow::on_sendButton_clicked()
{
}
```

它依赖命名规则自动连接。能用，但现代项目很多团队更偏向显式 `connect()`，更清楚。

---

## 27. 命名、拆分、工程习惯

Qt 项目写久了，风格比技巧更重要。

建议：

- 类名：`MainWindow`、`TcpClient`
- 成员变量：`client_`、`statusLabel_`
- UI 对象：`ui->sendButton`
- 初始化逻辑拆函数
- 每个类只管一类职责

不建议：

- 一个 `MainWindow.cpp` 写两三千行
- 槽函数里同时做 UI、网络、文件、数据库
- 所有逻辑都写匿名 lambda

---

## 28. 一个完整小项目的思路示例

假设你要做一个“Socket 调试助手”：

功能包括：

- 输入 IP 和端口
- 点击连接
- 发送文本
- 接收数据显示
- 保存连接配置
- 支持导出日志
- 支持打印日志

你可以这样分：

```text
MainWindow
  - 负责页面、按钮响应、日志展示

TcpClient
  - 负责连接、发送、接收、错误处理

ConfigService
  - 负责配置文件读写

PrintService
  - 负责打印和打印预览
```

页面流程：

1. 启动时加载配置
2. 点击连接时调用 `TcpClient`
3. 接收到消息时更新日志框
4. 点击保存时写配置
5. 点击打印时走打印服务

你会发现：Qt 项目真正要做好的，不只是“按钮能点”，而是“结构别烂”。

---

## 29. 样式设计上的实用建议

如果你做的是工具类桌面应用，建议风格保持：

- 简洁
- 稳定
- 信息层级清楚
- 间距统一

实用建议：

1. 主字号一般 `13px` 到 `15px`
2. 表单行高和按钮高度保持统一
3. 页面边距、控件间距保持固定节奏
4. 操作按钮不要颜色太多
5. 状态颜色要有明确语义
   - 蓝：普通操作
   - 绿：成功
   - 黄：警告
   - 红：错误

## 29.1 控件状态要考虑完整

至少要考虑：

- normal
- hover
- pressed
- disabled
- focus

很多 UI 看着“粗糙”，不是因为不会配色，而是因为只写了默认态。

---

## 30. 高 DPI、字体、跨平台差异

Qt 跨平台不等于“每个平台看起来完全一样”。

你要注意：

- 字体在不同平台默认值不同
- 滚动条、标题栏、菜单栏细节不同
- 文件路径分隔符不同
- 部分系统权限限制不同

建议：

- 少写死宽高
- 多用布局
- 少依赖平台默认字体精确像素
- 多在目标系统上实际测试

---

## 31. 调试建议

Qt 项目里常见调试方式：

- `qDebug()`
- 断点调试
- 查看信号是否触发
- 查看对象是否为空
- 查看是否真的进入事件循环

几个常见问题：

### 31.1 点击按钮没反应

可能原因：

- `connect()` 没连上
- 槽函数没执行
- 控件被禁用了
- 被其他控件遮挡

### 31.2 界面不刷新

可能原因：

- 主线程被耗时任务阻塞
- 数据改了但没更新对应控件

### 31.3 样式不生效

可能原因：

- 选择器没写对
- 对象名没对上
- 样式加载路径错误

### 31.4 TCP 连接失败

可能原因：

- IP 或端口错误
- 服务端未监听
- 被防火墙拦截
- 使用了错误协议

---

## 32. Qt 常见类速查

你可以把这些当常用工具箱：

- 应用入口：`QApplication`
- 主窗口：`QMainWindow`
- 通用控件：`QWidget`
- 按钮：`QPushButton`
- 文本输入：`QLineEdit`
- 多行文本：`QPlainTextEdit`
- 标签：`QLabel`
- 布局：`QVBoxLayout` / `QHBoxLayout` / `QGridLayout`
- 定时器：`QTimer`
- 文件：`QFile`
- 路径：`QDir`
- 配置：`QSettings`
- JSON：`QJsonDocument` / `QJsonObject`
- TCP：`QTcpSocket` / `QTcpServer`
- HTTP：`QNetworkAccessManager`
- 打印：`QPrinter`
- 绘图：`QPainter`
- 线程：`QThread`
- 对话框：`QFileDialog` / `QMessageBox`

---

## 33. 打包部署前你要先知道的事

“能在自己电脑运行”不等于“能发给别人运行”。

Qt 程序依赖：

- 你的可执行文件
- Qt 动态库
- 平台插件
- 图像格式插件
- 可能还有编译器运行时

所以打包部署的核心，就是把程序依赖带齐。

---

## 34. Windows 打包

Windows 上 Qt Widgets 程序通常最终是：

- `MyApp.exe`
- 一批 Qt DLL
- `platforms/qwindows.dll`
- 其他插件目录

Qt 官方常用部署工具是：

- `windeployqt`

## 34.1 基本流程

先编译 Release：

```bash
cmake -S . -B build
cmake --build build --config Release
```

然后执行：

```bash
windeployqt MyApp.exe
```

它会自动拷贝大部分依赖。

如果你用的是 Visual Studio 多配置生成器，程序可能在：

```text
build/Release/MyApp.exe
```

## 34.2 常见注意点

1. 用 Release 版本打包，不要拿 Debug 发用户
2. 检查 `platforms/qwindows.dll` 是否存在
3. 检查是否缺少 VC 运行库
4. 实机测试一台没装 Qt 的电脑

## 34.3 安装包

你可以再配合这些工具做安装包：

- Inno Setup
- NSIS
- WiX

常见思路：

1. 先用 `windeployqt` 整出完整运行目录
2. 再用安装包工具打成安装程序

---

## 35. macOS 打包

macOS 上程序通常是 `.app` 包。

Qt 官方常用部署工具是：

- `macdeployqt`

## 35.1 基本流程

构建 Release 后，通常会得到：

```text
MyApp.app
```

然后执行：

```bash
macdeployqt MyApp.app
```

它会把 Qt 框架和必要插件拷贝进 `.app` 包内部。

## 35.2 `.app` 本质是什么

它其实是一个目录结构，只是 Finder 把它显示成一个应用。

里面常见结构：

```text
MyApp.app/
  Contents/
    MacOS/
    Frameworks/
    PlugIns/
    Resources/
```

## 35.3 需要注意的事情

1. 图标要正确配置
2. 签名和公证是正式分发的重要步骤
3. 在另一台没装 Qt 的 Mac 上测试
4. 如果使用外部动态库，也要一起处理

## 35.4 签名与公证

如果你只是本地学习，可以先不做。

如果你要正式分发给他人，通常需要：

- Apple Developer 证书
- `codesign`
- `notarytool` 公证

否则用户可能遇到系统安全限制。

---

## 36. Linux 部署简单说明

你这次重点问的是 mac 和 win，但顺手提一下：

Linux 下部署通常更分散，因为发行版环境差异更大。

常见方式：

- 携带依赖打包
- AppImage
- Flatpak
- Snap

如果以后你有 Linux 目标，最好单独做一轮部署方案。

---

## 37. 图标、版本号、应用信息

正式应用一般还要补这些：

- 应用图标
- 版本号
- 公司名 / 组织名
- 应用显示名
- 关于对话框

Windows 常见会涉及：

- `.rc` 资源文件

macOS 常见会涉及：

- `Info.plist`

Qt Creator 和 CMake 都能逐步配置这些内容。

---

## 38. 实战时最值得你优先掌握的 10 件事

如果你时间有限，先把这 10 件事啃下来：

1. `QApplication` 与事件循环
2. `QWidget` / `QMainWindow`
3. 布局器
4. 信号槽
5. `.ui` 文件使用
6. `QString` / `QFile` / `QSettings`
7. `QJsonDocument` / `QJsonObject`
8. `QTcpSocket`
9. QSS
10. `windeployqt` / `macdeployqt`

这 10 件事掌握后，你已经可以独立做不少桌面工具了。

---

## 39. 推荐练手项目

按难度递增，你可以做：

1. 记事本
   - 文本编辑
   - 打开/保存文件
   - 打印

2. Socket 调试助手
   - TCP 连接
   - 发送接收
   - 日志显示
   - JSON 配置保存

3. 简单下载器
   - HTTP 请求
   - 进度条
   - 文件保存

4. 配置管理工具
   - 表单页面
   - 多页切换
   - 本地配置持久化

5. 数据报表工具
   - 表格展示
   - JSON / CSV 导入导出
   - 打印预览

这些项目都能覆盖 Qt 最常见的实际能力。

---

## 40. 最后的学习建议

学 Qt 最容易踩的坑不是 API 太多，而是：

- 只会拖控件，不会组织代码
- 只会写功能，不会拆结构
- 只会本机跑，不会部署

所以你学习时，一定要同时训练三件事：

1. 页面怎么搭
2. 功能怎么分层
3. 程序怎么交付

如果你按本文路线走，建议下一步这样练：

1. 先新建一个 Qt Widgets 项目
2. 做一个带输入框、按钮、日志框的小窗口
3. 接入 `QTcpSocket`
4. 用 JSON 保存配置
5. 加一个打印按钮
6. 加一份 QSS
7. 最后跑一遍打包

当你能独立做完这套流程时，Qt 就不再只是“会一点”，而是真能拿来做项目了。

---

## 41. 一个最小综合示例清单

如果你要自己动手做一版练习，最少包含这些文件：

```text
MyQtTool/
  CMakeLists.txt
  main.cpp
  mainwindow.h
  mainwindow.cpp
  mainwindow.ui
  tcpclient.h
  tcpclient.cpp
  resources.qrc
  assets/
    main.qss
    logo.png
  config.json
```

它们分别承担：

- `main.*`：应用入口
- `mainwindow.*`：主界面与交互
- `tcpclient.*`：Socket 封装
- `resources.qrc`：资源管理
- `main.qss`：样式
- `config.json`：配置

这已经是一个很适合练手的正式骨架了。

---

## 42. 总结

Qt 不只是一个“画界面”的工具，它是一套完整的跨平台应用开发框架。

你真正要掌握的是：

- 用 `Widgets` 或 `QML` 做界面
- 用信号槽组织交互
- 用 Qt Core 处理字符串、文件、JSON、时间、配置
- 用 Qt Network 做 HTTP 和 Socket
- 用 QSS 做样式
- 用部署工具完成打包

如果你现在是从 0 开始，这份文档足够作为第一轮完整学习地图。

后续如果要继续深入，建议下一阶段重点补：

- Model/View
- 自定义控件
- 线程与任务调度
- 数据库
- QML / Qt Quick
- 插件化架构

