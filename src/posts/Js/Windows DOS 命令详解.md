---
title: "Windows DOS 命令详解文档"

category:
  - 命令行
tag:
  - 命令行
---
# Windows DOS 命令详解文档
> 适用环境：Windows 命令提示符 (cmd.exe)
> 也适用于：Windows PowerShell（多数命令兼容）

---

# 目录

1. [DOS 基础入门](#1-dos-基础入门)
2. [文件与目录操作](#2-文件与目录操作)
3. [磁盘管理](#3-磁盘管理)
4. [系统信息与管理](#4-系统信息与管理)
5. [网络命令](#5-网络命令)
6. [进程与服务管理](#6-进程与服务管理)
7. [文本处理](#7-文本处理)
8. [批处理脚本编写](#8-批处理脚本编写)
9. [任务调度](#9-任务调度)
10. [系统资源监控](#10-系统资源监控)
11. [安全与权限](#11-安全与权限)
12. [综合实战案例](#12-综合实战案例)
13. [附录](#13-附录)

---

# 1. DOS 基础入门

## 1.1 打开命令提示符

```batch
:: ============================================================
:: 打开 CMD 的多种方式
:: ============================================================

:: 方式一：Win + R → 输入 cmd → 回车
:: 方式二：开始菜单 → 搜索 "cmd" 或 "命令提示符"
:: 方式三：在文件夹地址栏输入 cmd 回车（直接打开该路径）
:: 方式四：Shift + 右键 → "在此处打开命令行窗口"

:: 以管理员身份运行：
:: 方式五：Win + R → 输入 cmd → Ctrl + Shift + 回车
:: 方式六：开始菜单 → 右键 cmd → "以管理员身份运行"

:: 常用启动参数：
cmd /c "command"        :: 执行命令后退出
cmd /k "command"        :: 执行命令后保留窗口
cmd /t:1F               :: 指定背景/前景色
cmd /q                  :: 安静模式
cmd /d                  :: 禁用 AutoRun 命令
```

## 1.2 常用特殊键与快捷键

```batch
:: ============================================================
:: CMD 快捷键
:: ============================================================

Tab                      :: 自动补全路径/文件名
Tab (连续按)             :: 循环显示匹配项
↑ / ↓                    :: 浏览命令历史
← / →                    :: 移动光标
Ctrl + ← / →             :: 按单词移动光标
Home                     :: 跳到行首
End                      :: 跳到行尾
Insert                   :: 切换插入/改写模式
Delete                   :: 删除光标后字符
Backspace                :: 删除光标前字符
Esc                      :: 清除当前行
F1                       :: 逐字复制上一条命令
F2 + 字符                :: 复制到指定字符
F3                       :: 复制上一条命令（完整）
F4 + 字符                :: 删除到指定字符
F5                       :: 向前翻历史（类似 ↑）
F6                       :: 输入 EOF 标识 (Ctrl+Z)
F7                       :: 显示历史命令列表（可选择）
F8                       :: 循环匹配历史
F9                       :: 输入历史命令编号执行
Alt + Enter              :: 全屏/窗口切换（部分终端支持）
Ctrl + C                 :: 终止当前命令
Ctrl + S                 :: 暂停输出（继续按任意键）
Ctrl + Break             :: 终止批处理执行
Ctrl + PgUp/PgDn         :: 滚动缓冲区
```

## 1.3 基础语法

```batch
:: ============================================================
:: CMD 命令基本语法规则
:: ============================================================

:: 命令格式： command [参数] [选项]

:: 参数与选项说明
dir                         :: 列出当前目录
dir C:\Windows              :: 指定路径
dir /w                      :: 宽格式显示
dir /p                      :: 分页显示（按任意键继续）
dir /a                      :: 显示所有文件（包括隐藏和系统文件）
dir /s                      :: 递归显示子目录
dir /b                      :: 只显示文件名（简洁模式）
dir /o:n                    :: 按名称排序

:: 命令连接
command1 & command2         :: 顺序执行（不管成败）
command1 && command2        :: 前一个成功才执行后一个
command1 || command2        :: 前一个失败才执行后一个

:: 输出重定向
command > file.txt          :: 覆盖写入文件
command >> file.txt         :: 追加写入文件
command 2> error.txt        :: 错误输出重定向
command > file.txt 2>&1     :: 合并标准输出和错误
command > nul               :: 抑制输出（不显示结果）
command 2> nul              :: 抑制错误输出

:: 管道
command1 | command2         :: 将前一个命令的输出传给后一个命令

:: 注释
REM 这是注释
:: 这也是注释（推荐使用）

:: 变量
set name=张三               :: 定义变量
echo %name%                 :: 使用变量（用 % 包裹）
set /p name=请输入:         :: 从用户输入获取变量

:: 回显控制
@echo off                   :: 关闭命令回显（@ 表示本行也不显示）
echo on                     :: 开启命令回显
echo.                       :: 输出空行
echo 文本内容               :: 输出文本
```

## 1.4 帮助系统

```batch
:: ============================================================
:: CMD 内置帮助
:: ============================================================

help                        :: 显示所有命令列表
help dir                    :: 查看 dir 命令的详细帮助
dir /?                      :: 查看 dir 命令的详细帮助（等价）
command /?                  :: 任何命令都可以使用 /? 查看帮助

:: 使用 more 分页查看帮助
help | more                 :: 分页显示帮助
help > commands.txt         :: 将帮助导出到文件
```

---

# 2. 文件与目录操作

## 2.1 目录导航

```batch
:: ============================================================
:: cd / chdir - 切换目录
:: ============================================================

cd                          :: 显示当前目录
cd C:\Windows               :: 切换到指定目录
cd /d D:\Projects           :: 切换到其他盘符（/d 是必需的）
cd ..                       :: 返回上级目录
cd ../..                    :: 返回上两级目录
cd %USERPROFILE%            :: 切换到用户目录
cd /d %~dp0                 :: 在批处理中切换到脚本所在目录

:: ============================================================
:: dir - 列出目录内容
:: ============================================================

dir                         :: 列出当前目录
dir C:\Windows\System32     :: 列出指定目录
dir *.exe                   :: 列出所有 exe 文件
dir /a                      :: 显示所有文件（包括隐藏和系统文件）
dir /a:d                    :: 只显示目录
dir /a:-d                   :: 只显示文件（不显示目录）
dir /s                      :: 递归显示所有子目录
dir /b                      :: 简洁格式（只显示名称）
dir /w                      :: 宽格式显示
dir /on                     :: 按名称排序
dir /os                     :: 按大小排序
dir /od                     :: 按日期排序
dir /oe                     :: 按扩展名排序
dir /p                      :: 分页显示
dir /-c                     :: 不显示千位分隔符
dir /t:w                    :: 按最后写入时间排序（w=写入,a=访问,c=创建）

:: 组合使用
dir /a /s /b "*.jpg"        :: 列出所有 JPG 文件（含子目录，只输路径）
dir /a:d /s /b              :: 列出所有目录的完整路径

:: ============================================================
:: tree - 树形显示目录结构
:: ============================================================

tree                        :: 树形显示当前目录
tree /f                     :: 包含文件
tree /a                     :: 使用 ASCII 字符（不用图形字符）
tree D:\Projects > struct.txt :: 导出目录结构
```

## 2.2 文件创建与查看

```batch
:: ============================================================
:: type - 显示文件内容
:: ============================================================

type file.txt               :: 显示文本文件内容
type file.txt | more        :: 分页显示
type file.txt > output.txt  :: 复制文件内容到新文件

:: ============================================================
:: more - 分页显示
:: ============================================================

type large.txt | more       :: 分页查看文件
more large.txt              :: 同上
more /s /c /p large.txt     :: /s 压缩空行 /c 清屏 /p 展开换页符

:: ============================================================
:: 创建空文件（多种方式）
:: ============================================================

type nul > empty.txt        :: 创建空文件
echo. > file.txt            :: 创建带空行的文件
copy nul empty.txt          :: 复制空设备到文件
fsutil file createnew test.txt 1024 :: 创建指定大小（1024字节）的二进制文件

:: ============================================================
:: find - 搜索文本
:: ============================================================

find "keyword" file.txt             :: 在文件中搜索关键词
find /i "keyword" file.txt          :: 忽略大小写
find /c "keyword" file.txt          :: 统计匹配的行数
find /n "keyword" file.txt          :: 显示行号
find /v "keyword" file.txt          :: 显示不包含的行
findstr "pattern" file.txt          :: 更强大的搜索（支持正则）

:: 查找字符串在多个文件中
find /i "error" *.log               :: 在所有日志文件中搜索 error
findstr /m "ERROR" *.log            :: 只显示包含匹配内容的文件名
findstr /si "TODO" *.bat            :: 递归搜索不区分大小写

:: findstr 正则示例
findstr "^[0-9]" file.txt               :: 以数字开头的行
findstr "^ERROR.*timeout" file.txt       :: ERROR 开头且包含 timeout
findstr /r "[0-9][0-9]\.[0-9]" file.txt :: 正则模式（/r）
```

## 2.3 文件操作

```batch
:: ============================================================
:: copy - 复制文件
:: ============================================================

copy source.txt dest.txt                    :: 复制文件并重命名
copy source.txt C:\Backup\                  :: 复制到其他目录
copy *.txt C:\Backup\                       :: 复制所有 txt 文件
copy /y source.txt dest.txt                 :: 覆盖时不提示（确认）
copy /-y source.txt dest.txt                :: 覆盖时提示
copy /v source.txt dest.txt                 :: 复制后验证
copy /a source.txt dest.txt                 :: 以 ASCII 模式复制
copy /b image1.jpg + image2.jpg merged.jpg  :: 合并二进制文件（/b）
copy con newfile.txt                        :: 从键盘输入创建文件（Ctrl+Z 结束）
copy NUL empty.txt                          :: 创建空文件

:: ============================================================
:: xcopy - 增强复制（支持目录）
:: ============================================================

xcopy source dest /e                      :: 复制目录（包括空目录）
xcopy source dest /s                      :: 复制目录（不包括空目录）
xcopy source dest /h                      :: 复制隐藏和系统文件
xcopy source dest /k                      :: 保留属性
xcopy source dest /y                      :: 覆盖时不提示
xcopy source dest /d                      :: 只复制更新的文件
xcopy source dest /exclude:list.txt       :: 排除列表中指定的文件
xcopy source /i dest                      :: /i 如果目标不存在则创建目录

:: 常用组合
xcopy D:\Data E:\Backup /e /h /k /y      :: 完整目录备份

:: ============================================================
:: robocopy - 可靠复制（Windows Vista+，功能最强大）
:: ============================================================

robocopy source dest /e                   :: 复制目录（含空目录）
robocopy source dest /mir                 :: 镜像同步（= /e + /purge）
robocopy source dest /copyall             :: 复制所有文件信息
robocopy source dest /sec                 :: 复制安全信息
robocopy source dest /log:log.txt         :: 记录日志
robocopy source dest /r:3 /w:5            :: 失败重试3次，等待5秒
robocopy source dest /mon:1               :: 监控变更（有变更就同步）
robocopy source dest /mot:10              :: 每10分钟检查一次

:: 常用组合
robocopy "C:\Users\%USERNAME%" "D:\Backup" /e /copyall /r:2 /w:3 /log:backup.log

:: ============================================================
:: move - 移动/重命名
:: ============================================================

move file.txt C:\Backup\                  :: 移动文件
move old.txt new.txt                      :: 重命名文件
move /y source dest                       :: 覆盖时不提示
move *.* C:\Backup\                       :: 移动所有文件

:: ============================================================
:: ren / rename - 重命名
:: ============================================================

ren old.txt new.txt                       :: 重命名文件
ren *.txt *.bak                           :: 批量改扩展名
ren "my file.txt" "my document.txt"       :: 含空格的路径用引号

:: 批量重命名示例
:: 将 file1.txt, file2.txt 重命名为 doc1.txt, doc2.txt
for %i in (*.txt) do ren "%i" "doc%i"

:: ============================================================
:: del / erase - 删除文件
:: ============================================================

del file.txt                              :: 删除文件
del *.tmp                                 :: 删除所有 tmp 文件
del /f file.txt                           :: 强制删除只读文件
del /s *.tmp                              :: 递归删除子目录中的 tmp 文件
del /q *.tmp                              :: 安静模式（不提示确认）
del /a:r *.tmp                            :: 删除只读文件（/a 属性筛选）

:: ============================================================
:: rd / rmdir - 删除目录
:: ============================================================

rd emptydir                               :: 删除空目录
rd /s dir                                 :: 删除目录及其文件（会提示确认）
rd /s /q dir                              :: 安静删除目录及所有文件（不提示）

:: 实用技巧：删除所有 bin 和 obj 目录
for /d /r . %%d in (bin obj) do @if exist "%%d" rd /s /q "%%d"
```

## 2.4 文件属性操作

```batch
:: ============================================================
:: attrib - 修改文件属性
:: ============================================================

attrib file.txt                           :: 查看文件属性
attrib +r file.txt                        :: 设为只读 (Read-only)
attrib -r file.txt                        :: 取消只读
attrib +h file.txt                        :: 设为隐藏 (Hidden)
attrib -h file.txt                        :: 取消隐藏
attrib +s file.txt                        :: 设为系统文件 (System)
attrib -s file.txt                        :: 取消系统文件
attrib +a file.txt                        :: 设为存档 (Archive)
attrib +r +h +s file.txt                  :: 组合设置

attrib *.*                                :: 查看当前目录所有文件属性
attrib /s -h *.txt                        :: 递归取消隐藏所有 txt 文件

:: ============================================================
:: icacls - 查看/修改文件权限（Vista+）
:: ============================================================

icacls file.txt                           :: 查看文件权限
icacls file.txt /grant %USERNAME%:F       :: 给予完全控制权 (Full)
icacls file.txt /grant %USERNAME%:R       :: 给予只读权限 (Read)
icacls file.txt /grant %USERNAME%:W       :: 给予写入权限 (Write)
icacls file.txt /grant %USERNAME%:M       :: 给予修改权限 (Modify)
icacls file.txt /deny %USERNAME%:F        :: 拒绝完全控制
icacls file.txt /remove %USERNAME%        :: 移除用户的权限
icacls file.txt /reset                    :: 重置为默认权限
icacls * /t /grant %USERNAME%:F           :: 递归设置目录所有文件权限

:: ============================================================
:: takeown - 获取文件所有权
:: ============================================================

takeown /f file.txt                       :: 获得文件所有权
takeown /f * /r                           :: 递归获取当前目录所有权
takeown /f "%WINDIR%\system32" /r         :: 获取系统目录所有权（需管理员）
```

## 2.5 文件查找

```batch
:: ============================================================
:: where - 查找文件位置
:: ============================================================

where notepad                            :: 查找 notepad.exe 的路径
where /r C:\Windows *.exe                :: 递归查找特定文件
where /q notepad                         :: 只设置退出码（不显示输出）

:: ============================================================
:: dir /s - 搜索文件
:: ============================================================

dir /s /b "*.config"                     :: 查找所有配置文件
dir /s /b /a:d "Temp"                    :: 查找所有 Temp 目录

:: ============================================================
:: forfiles - 按条件处理旧文件
:: ============================================================

:: 查看30天前的文件
forfiles /p "C:\Logs" /s /m *.log /d -30 /c "cmd /c echo @path"

:: 删除7天前的备份文件
forfiles /p "D:\Backup" /s /m *.bak /d -7 /c "cmd /c del @path"

:: 参数说明：
:: /p path     起始路径
:: /s          递归子目录
:: /m mask     文件通配符（默认 *.*）
:: /d date     日期筛选（-N 表示 N 天前）
:: /c cmd      要执行的命令
```

---

# 3. 磁盘管理

## 3.1 磁盘信息查看

```batch
:: ============================================================
:: wmic logicaldisk - 磁盘信息
:: ============================================================

wmic logicaldisk get name,description,size,freespace
:: 显示所有磁盘的盘符、类型、总大小、剩余空间

:: 更可读的格式
wmic logicaldisk get name,size,freespace /format:list
wmic logicaldisk where "drivetype=3" get name,size,freespace
:: drivetype=3 表示本地磁盘（3=本地, 2=可移动, 4=网络）

:: ============================================================
:: fsutil - 文件系统工具
:: ============================================================

fsutil fsinfo drives                    :: 列出所有驱动器
fsutil fsinfo volumeinfo C:             :: 查看 C 盘卷信息
fsutil fsinfo ntfsinfo C:               :: 查看 NTFS 详细信息
fsutil fsinfo statistics C:             :: 查看文件系统统计信息

:: ============================================================
:: vol - 显示卷标和序列号
:: ============================================================

vol C:                                  :: 查看 C 盘卷标和序列号
```

## 3.2 磁盘空间管理

```batch
:: ============================================================
:: chkdsk - 磁盘检查与修复
:: ============================================================

chkdsk C:                               :: 检查磁盘（只读模式）
chkdsk C: /f                            :: 检查并修复错误（需锁定磁盘）
chkdsk C: /r                            :: 修复 + 恢复坏扇区（最彻底）
chkdsk C: /scan                         :: 在线扫描（Win 8+）
chkdsk C: /perf                         :: 更快扫描（使用更多资源）
chkdsk C: /spotfix                      :: 快速修复（Win 8+）

:: 注意：/f 和 /r 需要锁定磁盘，C 盘会在下次重启时检查

:: ============================================================
:: defrag - 磁盘碎片整理
:: ============================================================

defrag C:                               :: 分析并整理 C 盘
defrag C: /a                            :: 只分析，不整理
defrag C: /o                           :: 优化（有 SSD 时自动 TRIM）
defrag C: /u /v                         :: 显示详细进度

:: ============================================================
:: cleanmgr - 磁盘清理
:: ============================================================

cleanmgr                                :: 启动磁盘清理（选择驱动器）
cleanmgr /sageset:1                     :: 设置清理选项
cleanmgr /sagerun:1                     :: 执行预设置的清理
cleanmgr /d C:                          :: 直接清理 C 盘

:: 使用 Dism 清理（Win 8+）
Dism /online /Cleanup-Image /StartComponentCleanup
Dism /online /Cleanup-Image /StartComponentCleanup /ResetBase
```

## 3.3 分区管理

```batch
:: ============================================================
:: diskpart - 磁盘分区管理（需管理员）
:: ============================================================

:: 交互模式
diskpart                                :: 进入 diskpart 交互界面
  list disk                             :: 列出所有磁盘
  select disk 0                         :: 选择磁盘 0
  list partition                        :: 列出分区
  select partition 1                    :: 选择分区 1
  detail disk                           :: 查看磁盘详细信息

  :: 创建分区示例
  clean                                 :: 清除所有分区
  create partition primary size=50000   :: 创建 50GB 主分区
  format fs=ntfs quick                  :: 快速格式化为 NTFS
  assign letter=D                       :: 分配盘符 D

  exit                                  :: 退出 diskpart

:: 直接执行脚本
diskpart /s script.txt                  :: 执行 diskpart 脚本
:: 脚本示例 (create_partition.txt):
:: select disk 0
:: create partition primary size=100000
:: format fs=ntfs quick label="Data"
:: assign letter=D

:: ============================================================
:: format - 格式化磁盘
:: ============================================================

format D: /fs:ntfs                      :: 格式化为 NTFS
format D: /fs:fat32                     :: 格式化为 FAT32
format D: /fs:exfat                     :: 格式化为 exFAT
format D: /q                            :: 快速格式化
format D: /v:MyDisk                     :: 设置卷标
format D: /p:3                          :: 覆写3次后格式化（安全擦除）
```

## 3.4 磁盘空间分析

```batch
:: ============================================================
:: diruse / windirstat 替代
:: 使用 dir 和 PowerShell 分析空间
:: ============================================================

:: 查看各目录大小（PowerShell）
powershell "Get-ChildItem -Path C:\ -Directory | ForEach-Object { $size = (Get-ChildItem $_.FullName -Recurse -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum; [PSCustomObject]@{Name=$_.Name;SizeMB=[math]::Round($size/1MB,2)} } | Sort-Object SizeMB -Descending"

:: 使用 wmic
wmic logicaldisk where "drivetype=3" get name,size,freespace

:: 获取最大的10个文件
dir /s /b /a:-d | sort /r /o:size.txt
```

---

# 4. 系统信息与管理

## 4.1 系统基本信息

```batch
:: ============================================================
:: systeminfo - 系统详细信息
:: ============================================================

systeminfo                              :: 显示完整的系统信息
:: 输出包括：
::   - 主机名、OS 名称/版本
::   - 制造商、型号
::   - 物理内存、虚拟内存
::   - 网络适配器、域信息
::   - 补丁（KB编号）
::   - 启动时间、时区

systeminfo | findstr /i "OS Name"       :: 只查看 OS 名称
systeminfo | find /i "启动时间"         :: 查看系统运行时间
systeminfo | find /i "物理内存"         :: 查看物理内存
systeminfo | more                       :: 分页查看

:: ============================================================
:: ver - 版本号
:: ============================================================

ver                                     :: 显示 Windows 版本

:: ============================================================
:: winver - 图形界面版本信息
:: ============================================================

winver                                  :: 打开"关于 Windows"对话框

:: ============================================================
:: hostname - 主机名
:: ============================================================

hostname                                :: 显示计算机名

:: ============================================================
:: whoami - 当前用户
:: ============================================================

whoami                                  :: 显示当前用户名（域\用户名）
whoami /user                            :: 显示 SID
whoami /groups                          :: 显示用户所属组
whoami /priv                            :: 显示用户权限

:: ============================================================
:: date / time - 日期时间
:: ============================================================

date                                    :: 显示/设置日期
date /t                                 :: 只显示日期（不提示修改）
time                                    :: 显示/设置时间
time /t                                 :: 只显示时间（不提示修改）
```

## 4.2 硬件信息

```batch
:: ============================================================
:: MSInfo32 - 系统信息（图形界面）
:: ============================================================

msinfo32                                :: 打开系统信息工具
msinfo32 /report sysinfo.txt            :: 导出报告

:: ============================================================
:: wmic - Windows 管理规范（极其强大）
:: ============================================================

:: CPU 信息
wmic cpu get name,numberofcores,maxclockspeed,status

:: 内存信息
wmic memorychip get banklabel,capacity,speed,manufacturer

:: 磁盘信息
wmic diskdrive get model,size,interfacetype,status

:: 显卡信息
wmic path win32_videocontroller get name,adapterram,driverversion

:: BIOS 信息
wmic bios get name,serialnumber,manufacturer,smbiosbiosversion

:: 主板信息
wmic baseboard get product,manufacturer,version,serialnumber

:: 系统启动项
wmic startup get name,caption,command

:: 系统服务
wmic service get name,displayname,state,startmode

:: 所有已安装的软件
wmic product get name,version,vendor

:: 系统更新（KB）
wmic qfe get hotfixid,description,installedon

:: ============================================================
:: dxdiag - DirectX 诊断工具
:: ============================================================

dxdiag                                  :: 打开 DirectX 诊断工具
dxdiag /t dxdiag.txt                    :: 导出文本报告
dxdiag /dontskip                        :: 显示所有页
```

---

# 5. 网络命令

## 5.1 网络配置

```batch
:: ============================================================
:: ipconfig - IP 配置
:: ============================================================

ipconfig                                :: 显示基本 IP 配置
ipconfig /all                           :: 显示完整配置（包括 MAC、DHCP、DNS）
ipconfig /displaydns                    :: 显示 DNS 缓存
ipconfig /flushdns                      :: 刷新 DNS 缓存
ipconfig /release                       :: 释放 DHCP 租约
ipconfig /renew                         :: 重新获取 DHCP 租约
ipconfig /registerdns                   :: 重新注册 DNS
ipconfig /showclassid *                  :: 显示 DHCP 类别 ID

:: ============================================================
:: netsh - 网络配置工具（功能极强）
:: ============================================================

:: 查看网络配置
netsh interface ip show config          :: 显示接口 IP 配置
netsh interface ip show addresses       :: 显示 IP 地址
netsh interface ip show dns             :: 显示 DNS 配置
netsh wlan show profiles                :: 显示已保存的 WiFi
netsh wlan show profile name="SSID" key=clear :: 查看 WiFi 密码

:: 设置静态 IP（需管理员）
netsh interface ip set address "以太网" static 192.168.1.100 255.255.255.0 192.168.1.1
netsh interface ip set dns "以太网" static 8.8.8.8
netsh interface ip add dns "以太网" 8.8.4.4 index=2

:: 设置 DHCP
netsh interface ip set address "以太网" dhcp
netsh interface ip set dns "以太网" dhcp

:: 防火墙配置
netsh advfirewall show allprofiles      :: 查看防火墙状态
netsh advfirewall set allprofiles state on  :: 开启防火墙
netsh advfirewall set allprofiles state off :: 关闭防火墙
netsh advfirewall firewall add rule name="Open 80" dir=in action=allow protocol=tcp localport=80
netsh advfirewall firewall delete rule name="Open 80"

:: ============================================================
:: ping - 网络连通性测试
:: ============================================================

ping google.com                         :: 持续 ping（Windows 默认4次）
ping -n 10 google.com                   :: ping 10次
ping -t google.com                      :: 持续 ping（按 Ctrl+C 停止）
ping -l 1024 google.com                 :: 设置发送包大小（字节）
ping -w 3000 google.com                 :: 设置超时（毫秒）
ping -4 google.com                      :: 强制 IPv4
ping -6 google.com                      :: 强制 IPv6
ping -a 192.168.1.1                     :: 解析主机名
```

## 5.2 网络诊断

```batch
:: ============================================================
:: tracert - 路由追踪
:: ============================================================

tracert google.com                      :: 追踪到目标的路由路径
tracert -d google.com                   :: 不解析域名（更快）
tracert -h 30 google.com                :: 最大跳数（默认30）
tracert -w 1000 google.com              :: 超时时间（毫秒）

:: ============================================================
:: pathping - 路由追踪 + 丢包分析（结合 ping 和 tracert）
:: ============================================================

pathping google.com                     :: 分析路由和丢包率（较慢但精准）
pathping -n google.com                  :: 不解析域名

:: ============================================================
:: nslookup - DNS 查询
:: ============================================================

nslookup google.com                     :: 查询域名解析
nslookup -type=mx gmail.com            :: 查询 MX 记录
nslookup -type=ns baidu.com            :: 查询 NS 记录
nslookup 8.8.8.8                       :: 反向查询（IP 到域名）
nslookup google.com 8.8.8.8            :: 使用指定 DNS 服务器

:: ============================================================
:: netstat - 网络连接状态
:: ============================================================

netstat -an                             :: 显示所有连接和监听端口
netstat -ano                            :: 显示连接 + PID
netstat -b                              :: 显示连接 + 进程名（需管理员）
netstat -r                              :: 显示路由表
netstat -s                              :: 显示网络统计信息
netstat -e                              :: 显示以太网统计
netstat -p tcp                          :: 只显示 TCP 连接
netstat -p udp                          :: 只显示 UDP 连接
netstat -ano | find ":80"               :: 查找特定端口的使用情况

:: 找出使用特定端口的进程
netstat -ano | find ":3306"
:: 记下 PID，然后在任务管理器或使用 tasklist 查找
tasklist /fi "PID eq 1234"
```

## 5.3 网络工具

```batch
:: ============================================================
:: telnet - Telnet 客户端（测试端口连通性）
:: ============================================================

:: 安装 telnet（默认未安装）
dism /online /Enable-Feature /FeatureName:TelnetClient

:: 测试端口
telnet google.com 80                    :: 测试 80 端口是否通
telnet smtp.gmail.com 587               :: 测试邮件端口
:: 如果端口开放会进入空白屏，如果关闭会显示连接失败

:: ============================================================
:: curl - HTTP 请求（Win 10 1803+ 内置）
:: ============================================================

curl https://api.example.com            :: GET 请求
curl -o file.zip https://example.com/file :: 下载文件
curl -I https://example.com             :: 只获取响应头
curl -L https://short.url               :: 跟随重定向
curl -H "Authorization: Bearer token" https://api.example.com  :: 自定义头
curl -d "key=value" -X POST https://api.example.com  :: POST 请求

:: ============================================================
:: bitsadmin - 后台智能传输（Windows 内置下载工具）
:: ============================================================

bitsadmin /transfer job1 /download /priority high https://example.com/file.zip C:\file.zip

:: ============================================================
:: ftp - FTP 客户端
:: ============================================================

ftp ftp.example.com                     :: 连接 FTP 服务器
:: 交互命令：
::   ls - 列出文件
::   get file - 下载文件
::   put file - 上传文件
::   bye - 退出
```

## 5.4 远程管理

```batch
:: ============================================================
:: mstsc - 远程桌面连接
:: ============================================================

mstsc                                   :: 打开远程桌面连接
mstsc /v:192.168.1.100                  :: 直接连接到远程计算机
mstsc /v:192.168.1.100 /f               :: 全屏模式
mstsc /admin                            :: 管理员模式
mstsc /edit connection.rdp              :: 编辑 RDP 配置文件

:: ============================================================
:: psexec - 远程执行命令（需下载 Sysinternals 工具集）
:: ============================================================

:: 下载地址：https://docs.microsoft.com/en-us/sysinternals/downloads/psexec
psexec \\computername cmd               :: 远程打开命令行
psexec \\computername -u admin -p pass ipconfig :: 远程执行命令
psexec \\computername -c script.bat     :: 复制并执行脚本
```

---

# 6. 进程与服务管理

## 6.1 进程管理

```batch
:: ============================================================
:: tasklist - 列出进程
:: ============================================================

tasklist                                :: 列出所有进程
tasklist /v                             :: 详细显示
tasklist /svc                           :: 显示进程对应的服务
tasklist /m                             :: 显示加载的 DLL
tasklist /fi "STATUS eq running"        :: 筛选运行中的进程
tasklist /fi "MEMUSAGE gt 50000"        :: 内存使用大于 50MB 的进程
tasklist /fi "USERNAME ne NT AUTHORITY\SYSTEM" :: 筛选条件组合
tasklist /fi "PID eq 1234"              :: 按 PID 查找进程
tasklist /fi "IMAGENAME eq nginx*"      :: 按镜像名称匹配
tasklist /s 192.168.1.100 /u admin      :: 查看远程计算机进程
tasklist /fo csv > processes.csv        :: 导出为 CSV

:: ============================================================
:: taskkill - 终止进程
:: ============================================================

taskkill /pid 1234                      :: 按 PID 杀死进程
taskkill /f /pid 1234                   :: 强制杀死进程
taskkill /im notepad.exe                :: 按镜像名称杀死
taskkill /f /im notepad.exe             :: 强制终止所有记事本
taskkill /fi "STATUS eq not responding" :: 杀死所有无响应的进程
taskkill /f /fi "USERNAME eq %USERNAME%" /im *  :: 结束当前用户的所有进程
taskkill /s server /u admin /im *       :: 杀掉远程计算机上的进程

:: 实用：查找并结束占用端口的进程
netstat -ano | find ":8080"
taskkill /pid <找到的PID> /f

:: ============================================================
:: wmic process - 更详细的进程管理
:: ============================================================

:: 查看进程
wmic process get name,processid,workingsetsize,commandline

:: 终止进程
wmic process where name="notepad.exe" delete

:: 启动进程
wmic process call create "notepad.exe"
```

## 6.2 服务管理

```batch
:: ============================================================
:: net service - 服务管理（基本）
:: ============================================================

net start                               :: 列出正在运行的服务
net start "服务名"                      :: 启动服务
net stop "服务名"                       :: 停止服务
net pause "服务名"                      :: 暂停服务
net continue "服务名"                   :: 恢复服务

:: 示例
net start w3svc                         :: 启动 Web 服务（IIS）
net stop "Windows Update"               :: 停止 Windows 更新服务

:: ============================================================
:: sc - 服务控制（功能更强大）
:: ============================================================

sc query                                :: 列出所有服务
sc query state= all                     :: 列出所有服务（包括未运行）
sc query | find "SERVICE_NAME"          :: 列出服务名称
sc query wuauserv                       :: 查看特定服务的状态
sc queryex wuauserv                     :: 查看服务详细信息

:: 服务控制
sc start wuauserv                       :: 启动服务
sc stop wuauserv                        :: 停止服务
sc pause wuauserv                       :: 暂停服务
sc continue wuauserv                    :: 恢复服务
sc config wuauserv start= disabled      :: 禁用服务（= 后面必须有空格）
sc config wuauserv start= auto          :: 自动启动
sc config wuauserv start= demand        :: 手动启动
sc config wuauserv start= delayed-auto  :: 延迟自动启动（Vista+）

:: 创建/删除服务
sc create MyService binpath= "C:\app\myservice.exe"
sc delete MyService

:: 查看服务依赖
sc enumdep wuauserv                     :: 查看服务依赖关系

:: 远程管理
sc \\192.168.1.100 query wuauserv       :: 查看远程计算机的服务
```

## 6.3 自启动管理

```batch
:: ============================================================
:: 管理开机自启动
:: ============================================================

:: 查看启动项
wmic startup get name,caption,command   :: 查看所有启动项

:: 使用注册表管理
reg query HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
reg query HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Run

:: 添加启动项
reg add HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Run /v MyApp /t REG_SZ /d "C:\app.exe"

:: 删除启动项
reg delete HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Run /v MyApp /f

:: 使用 msconfig
msconfig                                :: 打开系统配置（图形界面管理启动项）
```

---

# 7. 文本处理

## 7.1 文本过滤与查找

```batch
:: ============================================================
:: find - 文本查找
:: ============================================================

find "keyword" file.txt                 :: 查找包含关键词的行
find /i "keyword" file.txt              :: 忽略大小写
find /c "keyword" file.txt              :: 统计匹配行数
find /n "keyword" file.txt              :: 显示行号
find /v "keyword" file.txt              :: 显示不包含的行
find /i "error" *.log                   :: 在多个文件中查找

:: ============================================================
:: findstr - 高级文本搜索（支持正则）
:: ============================================================

findstr "error" file.txt                :: 基础搜索
findstr /i "error" file.txt             :: 忽略大小写
findstr /n "error" file.txt             :: 显示行号
findstr /m "error" *.log               :: 只显示文件名
findstr /s "error" *.log               :: 递归搜索子目录
findstr /v "error" file.txt             :: 反向匹配
findstr /r "^[0-9]" file.txt           :: 正则：以数字开头的行
findstr /x "error" file.txt             :: 完全匹配整行
findstr /g:keywords.txt file.txt        :: 从文件读取搜索关键词
findstr /l "exact.string" file.txt      :: 文字搜索（不解释正则）

:: 正则示例
findstr /r "^[A-Z].*[0-9]$" file.txt   :: 大写开头，数字结尾
findstr /r "error.*failed" file.txt     :: error 后面跟着 failed
findstr /c:"error 404" file.txt         :: 精确短语（按字面搜索空格）

:: 在 dir 输出中过滤
dir /s /b *.log | findstr /i "error"
dir | findstr /i "目录"
```

## 7.2 文本排序与去重

```batch
:: ============================================================
:: sort - 排序
:: ============================================================

sort file.txt                           :: 按字母序排序
sort /r file.txt                        :: 逆序排序
sort /+10 file.txt                      :: 从第10个字符开始排序
sort /unique file.txt                   :: 排序并去重
sort < input.txt > output.txt           :: 输入重定向

:: 按列排序示例（需要其他工具配合）
:: 先用 for 提取列，再用 sort
```

## 7.3 文本替换与处理

```batch
:: ============================================================
:: more - 分页查看（带额外功能）
:: ============================================================

more file.txt                           :: 分页查看（空格翻页，q退出）
type file.txt | more                    :: 管道分页
more /s file.txt                        :: 压缩多个空行为一个
more /t4 file.txt                       :: Tab 展开为4个空格
more +10 file.txt                       :: 从第10行开始显示

:: ============================================================
:: 使用 for 循环处理文本
:: ============================================================

:: 逐行读取文件
for /f "tokens=*" %%a in (file.txt) do (
    echo 行: %%a
)

:: 按逗号分隔读取 CSV
for /f "tokens=1-3 delims=," %%a in (data.csv) do (
    echo 列1: %%a, 列2: %%b, 列3: %%c
)

:: 跳过前5行，读取第6行以后
for /f "skip=5 tokens=*" %%a in (file.txt) do (
    echo %%a
)

:: 只处理包含特定字符串的行
for /f "tokens=*" %%a in ('findstr "error" file.txt') do (
    echo 错误行: %%a
)

:: ============================================================
:: 使用变量替换处理字符串
:: ============================================================

set str=Hello World
echo %str%                              :: Hello World
echo %str:~0,5%                         :: Hello （取前5个字符）
echo %str:~-5%                          :: World （取后5个字符）
echo %str:H=J%                          :: Jello World（替换第一个H）
echo %str:o=@%                          :: Hell@ W@rld（替换所有o）
echo %str: World=!%                     :: Hello!（删除/替换子串）
echo %str:~6,3%                         :: Wor（从第6位取3个）
```

---

# 8. 批处理脚本编写

## 8.1 批处理文件基础

```batch
:: ============================================================
:: 批处理文件结构
:: 文件扩展名: .bat 或 .cmd
:: 编码建议: ANSI（中文环境为 GBK），或用 UTF-8 with BOM
:: ============================================================

@echo off
REM ========================================================
REM 文件名: example.bat
REM 描述: 批处理脚本示例
REM 用法: example.bat [参数1] [参数2]
REM 作者: Your Name
REM 日期: 2026-06-08
REM ========================================================

:: 关闭回显（@ 让本行也不显示）
:: echo off 后的命令都不会回显

setlocal enabledelayedexpansion
REM ========================================================
REM setlocal：
REM   - 创建局部变量作用域（脚本结束时自动恢复）
REM   - enabledelayedexpansion：启用延迟变量扩展
REM     （在 for 循环中需要用 !var! 而不是 %var%）
REM endlocal：结束局部作用域（可选，脚本退出时自动调用）
REM ========================================================

:: 输出文本
echo 欢迎使用批处理脚本！

:: 暂停（按任意键继续）
pause

:: 退出脚本
exit /b 0
REM exit /b 只退出批处理（不退出 CMD 窗口）
REM exit 会完全退出 CMD 窗口
```

## 8.2 变量

```batch
:: ============================================================
:: 变量定义与使用
:: ============================================================

@echo off
setlocal enabledelayedexpansion

:: --- 定义变量 ---
set name=张三
set age=25
set greeting=你好，%name%！

:: --- 使用变量 ---
echo %greeting%                     :: 输出：你好，张三！
echo 年龄：%age%

:: --- 数字计算 ---
set /a num1=10
set /a num2=20
set /a sum=num1+num2
echo %num1% + %num2% = %sum%

:: 算术运算符
set /a result=5+3                   :: 加法
set /a result=10-4                  :: 减法
set /a result=3*4                   :: 乘法
set /a result=20/6                  :: 除法（整数，结果为3）
set /a result=20%%6                 :: 取模（结果为2）
set /a result=5**3                  :: 幂运算（结果为125）
set /a result=10>>1                 :: 右移
set /a result=1<<3                  :: 左移
set /a result=7^&3                  :: 按位与（^ 是转义字符）
set /a result=7^|3                  :: 按位或
set /a result=~7                    :: 按位取反

:: --- 用户输入 ---
set /p input=请输入内容：
echo 你输入的是：%input%

:: --- 环境变量 ---
echo %USERNAME%                     :: 当前用户名
echo %USERPROFILE%                  :: 用户目录
echo %WINDIR%                       :: Windows 目录
echo %SYSTEMROOT%                   :: 系统根目录
echo %TEMP%                         :: 临时文件夹
echo %PATH%                         :: 可执行文件路径
echo %DATE%                         :: 当前日期
echo %TIME%                         :: 当前时间
echo %RANDOM%                       :: 随机数 (0-32767)

:: --- 延迟变量扩展（在 for 循环中很重要） ---
REM 错误示例（不使用延迟扩展）：
set var=before
for %%i in (1) do (
    set var=after
    echo %var%    REM 输出 "before"（因为 %var% 在解析时已展开）
)

REM 正确示例（使用延迟扩展）：
set var=before
for %%i in (1) do (
    set var=after
    echo !var!    REM 输出 "after"（!var! 在运行时展开）
)
```

## 8.3 参数传递

```batch
:: ============================================================
:: 批处理参数
:: ============================================================

:: 脚本内访问参数
%0      :: 脚本文件名（含路径）
%1      :: 第一个参数
%2      :: 第二个参数...
%9      :: 第九个参数
%*      :: 所有参数

:: 参数修饰符（用于 %0-%9）
%~1     :: 去掉引号
%~f1    :: 完整路径
%~d1    :: 盘符
%~p1    :: 路径（不含盘符）
%~n1    :: 文件名（不含扩展名）
%~x1    :: 扩展名
%~s1    :: 短路径（8.3 格式）
%~a1    :: 文件属性
%~t1    :: 文件修改时间
%~z1    :: 文件大小
%~dp1   :: 盘符+路径
%~nx1   :: 文件名+扩展名
%~dp0   :: 脚本所在目录（括号内数字为0表示脚本自身）

:: 示例：批处理模板
@echo off
setlocal

echo 脚本名: %0
echo 完整路径: %~f0
echo 所在目录: %~dp0
echo 参数个数: %~0 个参数
echo 第一个参数: %1
echo 第二个参数: %2
echo 所有参数: %*

:: 参数检查
if "%1"=="" (
    echo 用法：%0 [参数1] [参数2]
    exit /b 1
)

:: shift 命令 - 移动参数（可用于遍历多于9个的参数）
:loop
if "%1"=="" goto end
echo 处理参数：%1
shift
goto loop
:end
```

## 8.4 流程控制

```batch
:: ============================================================
:: if 条件判断
:: ============================================================

@echo off
setlocal

:: --- 字符串比较 ---
if "%1"=="help" (
    echo 显示帮助信息
) else if "%1"=="" (
    echo 参数不能为空
) else (
    echo 参数: %1
)

:: --- 数字比较 ---
set num=10
if %num% EQU 10  echo 等于 10    :: EQU (equal)
if %num% NEQ 5   echo 不等于 5    :: NEQ (not equal)
if %num% LSS 100 echo 小于 100   :: LSS (less than)
if %num% LEQ 10  echo 小于等于 10 :: LEQ (less or equal)
if %num% GTR 5   echo 大于 5     :: GTR (greater than)
if %num% GEQ 10  echo 大于等于 10 :: GEQ (greater or equal)

:: --- 文件/目录判断 ---
if exist "C:\Windows" (
    echo Windows 目录存在
)

if exist "config.txt" (
    echo 配置文件存在
) else (
    echo 配置文件不存在，创建默认配置...
    echo default=value > config.txt
)

if not exist "C:\Temp" mkdir C:\Temp

:: --- 变量是否定义 ---
if defined MyVar (
    echo MyVar 已定义
) else (
    echo MyVar 未定义
)
set MyVar=hello

:: --- 错误码判断 ---
command_that_might_fail
if errorlevel 1 (
    echo 命令执行失败（错误码 >= 1）
)
if %errorlevel% NEQ 0 (
    echo 命令执行失败
)

:: --- 组合条件 ---
if exist "file1.txt" if exist "file2.txt" (
    echo 两个文件都存在
)

:: --- 区分大小写 ---
if /i "%1"=="HELP" (
    echo 不区分大小写匹配
)
```

## 8.5 循环

```batch
:: ============================================================
:: for 循环
:: ============================================================

:: --- 遍历文件 ---
:: 格式：for %%变量 in (集合) do (命令)
REM 在批处理中使用 %%i，在命令行中直接用 %i

:: 遍历当前目录所有 txt 文件
for %%f in (*.txt) do (
    echo 发现文件: %%f
)

:: 遍历多个模式
for %%f in (*.txt *.log *.ini) do (
    echo 配置文件: %%f
)

:: --- 数字循环 ---
:: /l 表示数字范围 (起始,步长,结束)
for /l %%i in (1,1,10) do (
    echo 数字: %%i
)

:: 倒序
for /l %%i in (10,-1,1) do (
    echo 倒数: %%i
)

:: --- 目录循环 ---
:: /d 表示目录
for /d %%d in (*) do (
    echo 目录: %%d
)

:: --- 递归循环 ---
:: /r 递归遍历
for /r C:\Windows %%f in (*.log) do (
    echo 日志文件: %%f
)

:: --- 命令结果循环 ---
:: /f 处理命令输出
for /f "tokens=*" %%a in ('dir /b *.txt') do (
    echo 文本文件: %%a
)

:: 解析带分隔符的文本
:: tokens=1,3 取第1和第3列
:: delims=,   使用逗号分隔
for /f "tokens=1,3 delims=," %%a in (data.csv) do (
    echo 第1列: %%a, 第3列: %%b
)

:: 解析命令输出
for /f "tokens=1,5" %%a in ('netstat -an ^| find "ESTABLISHED"') do (
    echo 连接: %%a - %%b
)
REM 注意：管道符 | 在 for 中需要转义为 ^|

:: --- 遍历环境变量 ---
for %%v in (PATH TEMP USERNAME WINDIR) do (
    echo %%v = !%%v!
)
```

## 8.6 函数与子程序

```batch
:: ============================================================
:: 标签与调用 (gosub)
:: ============================================================

@echo off
setlocal

echo 主程序开始

:: 调用子程序
call :greet 张三 25
call :greet 李四 30
call :show_date

echo 主程序结束

:: 退出批处理（不执行后面的标签）
exit /b 0

:: ============================================================
:: 子程序（标签）
:: ============================================================

:greet
REM 参数：%1=姓名, %2=年龄
echo 你好！我是%1，今年%2岁。
goto :eof
REM goto :eof 返回调用处（相当于 return）

:show_date
echo 当前日期: %DATE%
echo 当前时间: %TIME%
goto :eof

:: ============================================================
:: 真正的函数（通过 for 模拟）
:: ============================================================

:: 定义函数（返回值）
:add
setlocal
set /a result=%1 + %2
echo %result%
endlocal
goto :eof

:: 使用
for /f %%r in ('call :add 10 20') do set sum=%%r
echo 10 + 20 = %sum%
```

## 8.7 输入输出与重定向

```batch
:: ============================================================
:: I/O 重定向高级用法
:: ============================================================

:: 标准输出到文件
echo 内容 > file.txt                    :: 覆盖
echo 内容 >> file.txt                   :: 追加

:: 标准错误到文件
command 2> error.txt                    :: 错误单独输出
command 2>> error.txt                   :: 追加错误

:: 合并输出和错误
command > all.txt 2>&1                  :: 合并到一个文件
command > output.txt 2> error.txt       :: 分别输出

:: 抑制输出
command > nul                           :: 不显示输出
command 2> nul                          :: 不显示错误
command > nul 2>&1                      :: 什么都不显示

:: 管道
command1 | command2                     :: 将 command1 的输出传给 command2
dir | findstr "txt"                     :: 过滤 dir 输出
type file.txt | more                    :: 分页查看

:: 多命令
command1 & command2                     :: 无条件顺序执行
command1 && command2                    :: 前成功后执行
command1 || command2                    :: 前失败后执行
(command1 && command2) || command3      :: 组合

:: 输入重定向
command < input.txt                     :: 从文件读取输入
sort < input.txt > sorted.txt           :: 排序文件
```

## 8.7 常用批处理技巧

```batch
:: ============================================================
:: 获取当前脚本所在目录
:: ============================================================
set SCRIPT_DIR=%~dp0

:: ============================================================
:: 获取日期/时间（用于文件名）
:: ============================================================
:: 注意：%DATE% 和 %TIME% 的格式取决于系统区域设置

:: 中英文系统通用方法
set YEAR=%DATE:~0,4%
set MONTH=%DATE:~5,2%
set DAY=%DATE:~8,2%
set HH=%TIME:~0,2%
set MI=%TIME:~3,2%
set SS=%TIME:~6,2%

:: 去除小时前的空格（如果小时是个位数）
if "%HH:~0,1%"==" " set HH=0%HH:~1,1%

set TIMESTAMP=%YEAR%%MONTH%%DAY%_%HH%%MI%%SS%
echo 时间戳: %TIMESTAMP%

:: ============================================================
:: 管理员权限检查
:: ============================================================
@echo off
net session >nul 2>&1
if %errorlevel% NEQ 0 (
    echo 请以管理员身份运行此脚本！
    pause
    exit /b 1
)
echo 已拥有管理员权限

:: 另一种方式（使用 whoami）
whoami /groups | findstr /i "S-1-16-12288" >nul
if %errorlevel% NEQ 0 (
    echo 请以管理员身份运行！
    pause
    exit /b 1
)

:: ============================================================
:: 提升到管理员权限（UAC 提权）
:: ============================================================
@echo off
fltmc >nul 2>&1 || (
    echo 正在请求管理员权限...
    mshta "javascript: var shell = new ActiveXObject('Shell.Application'); shell.ShellExecute('%~s0', '', '', 'runas', 1); close();"
    exit /b
)
echo 现在以管理员身份运行

:: ============================================================
:: 临时文件
:: ============================================================
set TMPFILE=%TEMP%\myscript_%RANDOM%.tmp
:: 使用临时文件...
if exist "%TMPFILE%" del "%TMPFILE%"

:: ============================================================
:: 检查操作系统版本
:: ============================================================
ver | findstr /i "10.0" >nul
if %errorlevel% EQU 0 (
    echo Windows 10 / Windows Server 2016+
) else (
    ver | findstr /i "6.3" >nul
    if %errorlevel% EQU 0 (
        echo Windows 8.1 / Windows Server 2012 R2
    )
)

:: ============================================================
:: 处理拖放文件
:: ============================================================
:: 用户将文件拖到脚本上时，%1 是文件路径
echo 你拖放的文件: %~nx1
echo 完整路径: %~f1

:: ============================================================
:: 彩色输出
:: ============================================================
:: 使用 color 命令改变控制台颜色
color 0A                              :: 黑色背景，绿色文字
echo 绿色文字

:: 颜色代码：
:: 0=黑 1=蓝 2=绿 3=青 4=红 5=紫 6=黄 7=白
:: 8=灰 9=亮蓝 A=亮绿 B=亮青 C=亮红 D=亮紫 E=亮黄 F=亮白

:: 恢复默认颜色
color 07

:: ============================================================
:: 进度条模拟
:: ============================================================
@echo off
setlocal

set total=50
for /l %%i in (1,1,%total%) do (
    set /p "=[%%i/%total%]" <nul
    ping -n 1 127.0.0.1 >nul      :: 模拟工作（等待约1秒）
)
echo 完成！
```

---

# 9. 任务调度

## 9.1 schtasks - 计划任务

```batch
:: ============================================================
:: schtasks - Windows 任务计划程序（功能强大，替代 at 命令）
:: ============================================================

:: --- 创建计划任务 ---

:: 每天在指定时间运行
schtasks /create /tn "MyTask" /tr "C:\script.bat" /sc daily /st 02:00

:: 每周一至周五运行
schtasks /create /tn "MyTask" /tr "script.bat" /sc weekly /d MON,TUE,WED,THU,FRI /st 09:00

:: 系统启动时运行
schtasks /create /tn "MyTask" /tr "script.bat" /sc onstart

:: 用户登录时运行
schtasks /create /tn "MyTask" /tr "script.bat" /sc onlogon

:: 空闲时运行
schtasks /create /tn "MyTask" /tr "script.bat" /sc onidle /i 10

:: 每分钟运行
schtasks /create /tn "MyTask" /tr "script.bat" /sc minute /mo 1

:: 每小时运行
schtasks /create /tn "MyTask" /tr "script.bat" /sc hourly /mo 1

:: 每天每3小时运行
schtasks /create /tn "MyTask" /tr "script.bat" /sc hourly /mo 3

:: 每周五晚上10点运行
schtasks /create /tn "MyTask" /tr "script.bat" /sc weekly /d FRI /st 22:00

:: 每月1号和15号运行
schtasks /create /tn "MyTask" /tr "script.bat" /sc monthly /d 1 /m JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC
schtasks /create /tn "MyTask" /tr "script.bat" /sc monthly /d 15

:: 一次性任务（指定日期时间）
schtasks /create /tn "MyTask" /tr "script.bat" /sc once /st 14:00 /sd 2026/06/10

:: 系统事件触发（如特定日志）
schtasks /create /tn "MyTask" /tr "script.bat" /sc onevent /ec system /mo "*[System[EventID=1000]]"

:: --- 常用选项 ---
:: /ru SYSTEM       使用 SYSTEM 账户运行
:: /rp ""           无密码
:: /rl highest      以最高权限运行
:: /delay 00:05:00  延迟5分钟
:: /f              强制创建（覆盖已存在）
:: /it             允许交互式运行

:: 示例：以最高权限运行备份脚本
schtasks /create /tn "DailyBackup" /tr "C:\backup.bat" ^
    /sc daily /st 02:00 /ru SYSTEM /rl highest /f

:: --- 查询计划任务 ---
schtasks                                :: 列出所有计划任务
schtasks /query /v                      :: 详细列表
schtasks /query /fo list                :: 列表格式（详细）
schtasks /query /fo csv > tasks.csv     :: 导出 CSV
schtasks /query /tn "MyTask"            :: 查询特定任务

:: --- 运行/停止任务 ---
schtasks /run /tn "MyTask"              :: 立即运行任务
schtasks /end /tn "MyTask"              :: 停止正在运行的任务

:: --- 修改任务 ---
schtasks /change /tn "MyTask" /st 03:00     :: 修改运行时间
schtasks /change /tn "MyTask" /ru SYSTEM    :: 修改运行用户
schtasks /change /tn "MyTask" /disable      :: 禁用任务
schtasks /change /tn "MyTask" /enable       :: 启用任务

:: --- 删除任务 ---
schtasks /delete /tn "MyTask"               :: 删除任务（有确认）
schtasks /delete /tn "MyTask" /f            :: 强制删除（无确认）
schtasks /delete /tn "*" /f                 :: 删除所有任务（危险！）

:: --- 导出/导入任务 ---
schtasks /query /tn "MyTask" /xml > task.xml   :: 导出为 XML
schtasks /create /tn "MyTask" /xml task.xml    :: 从 XML 导入
```

## 9.2 at - 传统计划任务（旧）

```batch
:: ============================================================
:: at - 一次性计划任务（旧版，仅保留兼容）
:: Windows 8+ 建议用 schtasks 代替
:: ============================================================

:: 启动 Schedule 服务
net start schedule

:: 在指定时间执行
at 02:00 "C:\backup.bat"
at 14:30 /interactive "C:\script.bat"    :: 交互式（与桌面交互）

:: 指定日期
at 02:00 /next:Friday "C:\script.bat"
at 02:00 /next:1 "C:\script.bat"         :: 明天

:: 查看任务
at                                    :: 列出所有 at 任务

:: 删除任务
at 5 /delete                          :: 删除 ID 为 5 的任务
at /delete /yes                       :: 删除所有任务
```

## 9.3 使用批处理实现循环任务

```batch
:: ============================================================
:: 批处理自带循环执行（不依赖任务计划程序）
:: ============================================================

:: --- 无限循环（类似守护进程） ---
@echo off
:loop
    echo [%DATE% %TIME%] 服务运行中...
    :: 这里放你的任务
    timeout /t 60 /nobreak >nul       :: 等待60秒
goto loop

:: --- 带条件的循环 ---
@echo off
setlocal

set /a count=0
:RepeatTask
set /a count+=1
echo 第 !count! 次执行...

:: 执行任务
call :do_work

:: 等待30秒后再次执行（共执行10次）
if !count! LSS 10 (
    timeout /t 30 /nobreak >nul
    goto RepeatTask
)

echo 任务完成！
exit /b 0

:do_work
echo 正在处理数据...
exit /b 0
```

---

# 10. 系统资源监控

## 10.1 tasklist / taskmgr - 进程监控

```batch
:: ============================================================
:: 进程监控
:: ============================================================

:: 查看所有进程及其内存使用
tasklist /v /fo list | more

:: 按内存使用排序（使用 wmic）
wmic process get name,processid,workingsetsize /format:list
wmic process get name,processid,workingsetsize | sort /r /+30

:: 查看特定进程的内存
tasklist /fi "IMAGENAME eq chrome.exe" /fo list

:: 打开任务管理器
taskmgr

:: 资源监视器
resmon
```

## 10.2 systeminfo - 系统资源

```batch
:: ============================================================
:: 系统资源信息
:: ============================================================

:: 查看系统总内存
systeminfo | findstr /i "物理内存"

:: 查看系统运行时间
systeminfo | findstr /i "系统启动时间"
net statistics workstation | findstr /i "启动"

:: 查看已安装的更新
systeminfo | findstr /i "KB[0-9]"
wmic qfe get hotfixid,installedon,caption
```

## 10.3 performance Monitor 命令

```batch
:: ============================================================
:: perfmon / typeperf - 性能监视器
:: ============================================================

:: 打开性能监视器（图形界面）
perfmon

:: 使用 typeperf 命令行收集性能数据
:: 查看 CPU 使用率
typeperf "\Processor(_Total)\%% Processor Time"

:: 查看内存使用
typeperf "\Memory\Available MBytes"

:: 查看磁盘使用
typeperf "\PhysicalDisk(_Total)\%% Disk Time"

:: 查看网络带宽
typeperf "\Network Interface(*)\Bytes Total/sec"

:: 查看多个计数器
typeperf "\Processor(_Total)\%% Processor Time" "\Memory\Available MBytes" "\PhysicalDisk(_Total)\%% Disk Time"

:: 每秒采样一次，共10次
typeperf "\Processor(_Total)\%% Processor Time" -si 1 -sc 10

:: 输出到 CSV 文件
typeperf "\Processor(_Total)\%% Processor Time" -si 5 -sc 12 -o cpu_usage.csv

:: 列出所有可用的性能计数器
typeperf -q
```

## 10.4 wmic - 强大的系统监控

```batch
:: ============================================================
:: wmic 系统监控
:: ============================================================

:: CPU 使用率
wmic cpu get loadpercentage

:: 内存信息
wmic os get totalvisiblememorysize,freephysicalmemory,freespaceinpagingfiles

:: 磁盘使用
wmic logicaldisk where "drivetype=3" get name,size,freespace

:: 进程详细信息
wmic process get name,processid,workingsetsize,kerneltime,usertime,commandline
wmic process where "name='chrome.exe'" get processid,workingsetsize

:: 网络接口统计
wmic nic where "netenabled=true" get name,speed,macaddress

:: 持续监控（每2秒）
:monitor
cls
wmic cpu get loadpercentage
wmic os get totalvisiblememorysize,freephysicalmemory
timeout /t 2 /nobreak >nul
goto monitor
```

## 10.5 网络监控

```batch
:: ============================================================
:: 网络连接监控
:: ============================================================

:: 当前网络连接数
netstat -an | find "ESTABLISHED" /c

:: 监听端口
netstat -an | find "LISTEN"

:: 查看各连接状态的统计
netstat -an | findstr /i "established|listening|close_wait|time_wait" | sort

:: 实时网络连接监控
:netmon
cls
echo 当前时间: %DATE% %TIME%
echo 活跃连接数:
netstat -an | find /c "ESTABLISHED"
echo.
echo 监听端口:
netstat -an | find "LISTEN"
timeout /t 5 /nobreak >nul
goto netmon

:: ============================================================
:: 带宽测试
:: ============================================================

:: 使用内置 PowerShell 命令测试网速
powershell "(New-Object Net.WebClient).DownloadString('http://speedtest.net')"

:: 简单的下载测速
echo 测试下载速度...
powershell "$url='http://speedtest.tele2.net/100MB.zip';$sw=[Diagnostics.Stopwatch]::StartNew();$wc=New-Object Net.WebClient;$data=$wc.DownloadData($url);$sw.Stop();'速度: ' + [math]::Round($data.Length/1MB/($sw.Elapsed.TotalSeconds),2) + ' MB/s'"
```

---

# 11. 安全与权限

## 11.1 用户与组管理

```batch
:: ============================================================
:: net user / net localgroup - 用户和组管理
:: ============================================================

:: 用户管理
net user                                :: 列出所有用户
net user username                       :: 查看用户详细信息
net user username /add                  :: 创建用户
net user username *                     :: 修改密码（交互式）
net user username 新密码                  :: 直接修改密码
net user username /delete               :: 删除用户
net user username /active:yes           :: 启用用户
net user username /active:no            :: 禁用用户
net user username /expires:2026-12-31   :: 设置账户过期时间
net user username /expires:never        :: 永不过期

:: 组管理
net localgroup                          :: 列出所有本地组
net localgroup Administrators           :: 查看组成员
net localgroup Administrators username /add     :: 添加到管理员组
net localgroup Administrators username /delete  :: 从组中移除
net localgroup Users username /add              :: 添加到用户组

:: ============================================================
:: whoami - 查看当前用户信息
:: ============================================================

whoami                                  :: 当前用户名
whoami /user                            :: 当前用户 SID
whoami /groups                          :: 用户所属组
whoami /priv                            :: 当前用户权限
whoami /all                             :: 所有信息
```

## 11.2 文件权限管理

```batch
:: ============================================================
:: icacls - 文件权限管理
:: ============================================================

:: 查看权限
icacls file.txt                         :: 查看文件权限

:: 授予权限
icacls file.txt /grant %USERNAME%:F     :: 完全控制
icacls file.txt /grant %USERNAME%:R     :: 只读
icacls file.txt /grant %USERNAME%:W     :: 写入
icacls file.txt /grant %USERNAME%:M     :: 修改
icacls file.txt /grant:"Users":R        :: 授予 Users 组只读

:: 继承权限
icacls file.txt /inheritance:r          :: 禁用继承
icacls file.txt /inheritance:e          :: 启用继承

:: 递归设置目录权限
icacls "C:\MyFolder" /grant "Users":R /t

:: 移除权限
icacls file.txt /remove %USERNAME%      :: 移除用户权限
icacls file.txt /remove:g "Users"       :: 移除组权限

:: 重置权限
icacls file.txt /reset                  :: 重置为默认权限
icacls "C:\MyFolder" /reset /t          :: 递归重置

:: 复制权限
icacls source /save perms.txt           :: 导出权限
icacls dest /restore perms.txt          :: 恢复权限
```

## 11.3 防火墙管理

```batch
:: ============================================================
:: netsh advfirewall - Windows 防火墙
:: ============================================================

:: 查看状态
netsh advfirewall show allprofiles       :: 查看所有配置文件状态
netsh advfirewall show currentprofile    :: 查看当前配置文件
netsh advfirewall firewall show rule name=all  :: 列出所有规则

:: 启用/禁用防火墙
netsh advfirewall set allprofiles state on     :: 开启
netsh advfirewall set allprofiles state off    :: 关闭

:: 创建入站规则
netsh advfirewall firewall add rule name="Open 80" dir=in action=allow protocol=tcp localport=80
netsh advfirewall firewall add rule name="Open SSH" dir=in action=allow protocol=tcp localport=22

:: 创建出站规则
netsh advfirewall firewall add rule name="Block Skype" dir=out action=block program="C:\Program Files\Skype\skype.exe"

:: 允许程序
netsh advfirewall firewall add rule name="MyApp" dir=in action=allow program="C:\MyApp\app.exe"

:: 允许特定 IP 访问
netsh advfirewall firewall add rule name="Allow IP" dir=in action=allow remoteip=192.168.1.0/24 protocol=tcp localport=3389

:: 删除规则
netsh advfirewall firewall delete rule name="Open 80"

:: 导出/导入策略
netsh advfirewall export "C:\fwpolicy.wfw"
netsh advfirewall import "C:\fwpolicy.wfw"

:: ============================================================
:: 端口查询与操作
:: ============================================================

:: 查看端口占用
netstat -ano | find ":80"

:: 检查端口是否可访问
telnet 127.0.0.1 80

:: 通过防火墙打开端口
netsh advfirewall firewall add rule name="TCP 8080" dir=in action=allow protocol=tcp localport=8080
```

---

# 12. 综合实战案例

## 12.1 系统健康检查脚本

```batch
@echo off
REM ============================================================
REM 文件名: health_check.bat
REM 描述: Windows 系统健康检查
REM 用法: health_check.bat [--email admin@example.com]
REM ============================================================
setlocal enabledelayedexpansion

set REPORT_FILE=%TEMP%\health_report.txt
set ISSUES=0
set EMAIL=

REM --- 解析参数 ---
if /i "%1"=="--email" set EMAIL=%2

echo ======================================== > "%REPORT_FILE%"
echo   系统健康检查报告 >> "%REPORT_FILE%"
echo   时间: %DATE% %TIME% >> "%REPORT_FILE%"
echo   主机: %COMPUTERNAME% >> "%REPORT_FILE%"
echo ======================================== >> "%REPORT_FILE%"
echo. >> "%REPORT_FILE%"

echo 正在执行系统健康检查...
echo.

REM --- 1. 系统基本信息 ---
echo [1/7] 系统信息...
echo === 系统信息 === >> "%REPORT_FILE%"
systeminfo | findstr /i "OS Name OS Version System Boot Time" >> "%REPORT_FILE%"
echo. >> "%REPORT_FILE%"

REM --- 2. CPU 检查 ---
echo [2/7] CPU 检查...
echo === CPU 信息 === >> "%REPORT_FILE%"
wmic cpu get name,numberofcores,maxclockspeed,loadpercentage /format:list >> "%REPORT_FILE%"
echo. >> "%REPORT_FILE%"

REM --- 3. 内存检查 ---
echo [3/7] 内存检查...
echo === 内存信息 === >> "%REPORT_FILE%"
for /f "skip=1 tokens=2" %%a in ('wmic os get totalvisiblememorysize') do set /a TOTAL_MEM=%%a/1024 2>nul
for /f "skip=1 tokens=2" %%b in ('wmic os get freephysicalmemory') do set /a FREE_MEM=%%b/1024 2>nul
set /a USED_MEM=%TOTAL_MEM%-%FREE_MEM%
set /a MEM_PERCENT=%USED_MEM%*100/%TOTAL_MEM%

echo 总内存: %TOTAL_MEM% MB >> "%REPORT_FILE%"
echo 已使用: %USED_MEM% MB (%MEM_PERCENT%%) >> "%REPORT_FILE%"
echo 空闲: %FREE_MEM% MB >> "%REPORT_FILE%"

if %MEM_PERCENT% GEQ 90 (
    echo [警告] 内存使用率超过90%% >> "%REPORT_FILE%"
    set /a ISSUES+=1
)
echo. >> "%REPORT_FILE%"

REM --- 4. 磁盘检查 ---
echo [4/7] 磁盘检查...
echo === 磁盘信息 === >> "%REPORT_FILE%"

for /f "skip=1 delims=" %%d in ('wmic logicaldisk where "drivetype=3" get name') do (
    set DISK=%%d
    if not "!DISK!"=="" (
        for /f "tokens=2" %%s in ('wmic logicaldisk where "name='!DISK!'" get size') do set /a SIZE=%%s/1073741824 2>nul
        for /f "tokens=2" %%f in ('wmic logicaldisk where "name='!DISK!'" get freespace') do set /a FREE=%%f/1073741824 2>nul
        set /a USED=!SIZE!-!FREE!
        set /a PERCENT=!USED!*100/!SIZE!

        echo 磁盘 !DISK!: 已用 !USED!GB/!SIZE!GB (!PERCENT!%%) >> "%REPORT_FILE%"
        if !PERCENT! GEQ 85 (
            echo [警告] !DISK! 磁盘使用率超过85%% >> "%REPORT_FILE%"
            set /a ISSUES+=1
        )
    )
)
echo. >> "%REPORT_FILE%"

REM --- 5. 网络检查 ---
echo [5/7] 网络检查...
echo === 网络信息 === >> "%REPORT_FILE%"

ping -n 1 8.8.8.8 >nul 2>&1
if %errorlevel% EQU 0 (
    echo 外网连接: 正常 >> "%REPORT_FILE%"
) else (
    echo [警告] 外网连接异常 >> "%REPORT_FILE%"
    set /a ISSUES+=1
)

ipconfig | findstr /i "IPv4" >> "%REPORT_FILE%"
echo. >> "%REPORT_FILE%"

REM --- 6. 关键服务检查 ---
echo [6/7] 服务检查...
echo === 关键服务状态 === >> "%REPORT_FILE%"

set SERVICES=wuauserv BITS Dhcp Dnscache EventLog Themes

for %%s in (%SERVICES%) do (
    sc query %%s | findstr "RUNNING" >nul
    if %errorlevel% EQU 0 (
        echo %%s: 运行中 >> "%REPORT_FILE%"
    ) else (
        echo %%s: 已停止 >> "%REPORT_FILE%"
    )
)
echo. >> "%REPORT_FILE%"

REM --- 7. 安全检查 ---
echo [7/7] 安全检查...
echo === 安全检查 === >> "%REPORT_FILE%"

REM 检查防火墙状态
netsh advfirewall show allprofiles | findstr "ON" >nul
if %errorlevel% EQU 0 (
    echo 防火墙: 已启用 >> "%REPORT_FILE%"
) else (
    echo [警告] 防火墙未启用 >> "%REPORT_FILE%"
    set /a ISSUES+=1
)

REM 检查更新状态
wmic qfe get hotfixid | findstr "KB" >nul
if %errorlevel% EQU 0 (
    echo 系统更新: 已安装更新 >> "%REPORT_FILE%"
) else (
    echo [注意] 未找到系统更新记录 >> "%REPORT_FILE%"
)

REM --- 输出结果 ---
echo.
echo ========================================
type "%REPORT_FILE%"
echo ========================================

echo.
if %ISSUES% GTR 0 (
    echo [结果] 发现 %ISSUES% 个问题需要关注
) else (
    echo [结果] 系统健康状况良好
)

REM 保存报告
copy "%REPORT_FILE%" "%USERPROFILE%\Desktop\health_report.txt" >nul
echo 报告已保存到桌面

endlocal
```

## 12.2 日志清理脚本

```batch
@echo off
REM ============================================================
REM 文件名: clean_logs.bat
REM 描述: 清理指定天数前的日志文件，可自定义路径
REM 用法: clean_logs.bat [路径] [天数]
REM 示例: clean_logs.bat C:\Logs 7
REM ============================================================
setlocal enabledelayedexpansion

REM --- 参数配置 ---
set TARGET_DIR=%~1
set RETENTION_DAYS=%~2

REM 默认值
if "%TARGET_DIR%"=="" set TARGET_DIR=C:\Logs
if "%RETENTION_DAYS%"=="" set RETENTION_DAYS=30

echo ========================================
echo  日志清理工具
echo ========================================
echo 目标路径: %TARGET_DIR%
echo 保留天数: %RETENTION_DAYS% 天内的日志
echo 将删除: %RETENTION_DAYS% 天前的 .log 和 .txt 文件
echo.

REM --- 检查路径是否存在 ---
if not exist "%TARGET_DIR%" (
    echo [错误] 路径不存在: %TARGET_DIR%
    pause
    exit /b 1
)

REM --- 确认 ---
set /p CONFIRM=确认执行清理？[y/N]:
if /i not "!CONFIRM!"=="y" (
    echo 操作已取消
    pause
    exit /b 0
)

echo.
echo 正在清理中...

REM --- 删除旧日志 ---
for %%t in (log txt tmp bak old) do (
    echo 正在清理 *.%%t 文件...
    forfiles /p "%TARGET_DIR%" /s /m *.%%t /d -%RETENTION_DAYS% /c "cmd /c del /q @path" 2>nul
    if !errorlevel! EQU 0 (
        echo   清理 *.%%t 完成
    )
)

REM --- 删除空目录 ---
echo 正在删除空目录...
for /f "delims=" %%d in ('dir /ad /s /b "%TARGET_DIR%" 2^>nul ^| sort /r') do (
    rd "%%d" 2>nul && echo   删除空目录: %%d
)

echo.
echo ========================================
echo  清理完成！
echo  目录: %TARGET_DIR%
echo  保留: %RETENTION_DAYS% 天内的文件
echo  时间: %DATE% %TIME%
echo ========================================

REM --- 记录日志 ---
echo [%DATE% %TIME%] 清理 %TARGET_DIR%，保留 %RETENTION_DAYS% 天 >> %TEMP%\clean_logs_history.txt

pause
endlocal
```

## 12.3 批量文件处理脚本

```batch
@echo off
REM ============================================================
REM 文件名: batch_process.bat
REM 描述: 批量处理文件的工具集
REM 用法: batch_process.bat [命令] [目录]
REM 命令: rename, backup, count, size
REM ============================================================
setlocal enabledelayedexpansion

set COMMAND=%~1
set WORK_DIR=%~2

if "%COMMAND%"=="" (
    echo 用法: %0 [命令] [目录]
    echo.
    echo 可用命令:
    echo   rename   重命名所有文件（添加前缀）
    echo   backup   备份文件到 Backup 子目录
    echo   count    统计文件数量
    echo   size     统计文件总大小
    echo.
    pause
    exit /b 0
)

if "%WORK_DIR%"=="" set WORK_DIR=%CD%

if not exist "%WORK_DIR%" (
    echo [错误] 目录不存在: %WORK_DIR%
    pause
    exit /b 1
)

cd /d "%WORK_DIR%"

REM ============================================================
REM 批量重命名
REM ============================================================
if /i "%COMMAND%"=="rename" (
    set /p PREFIX=请输入文件名前缀:
    if "!PREFIX!"=="" set PREFIX=file

    set /a count=0
    for %%f in (*.*) do (
        if not "%%f"=="%~nx0" (
            set /a count+=1
            set EXT=%%~xf
            set NEWNAME=!PREFIX!_!count!!EXT!
            ren "%%f" "!NEWNAME!"
            echo   %%f -^> !NEWNAME!
        )
    )
    echo 共重命名 !count! 个文件
)

REM ============================================================
REM 备份文件
REM ============================================================
if /i "%COMMAND%"=="backup" (
    set BACKUP_DIR=%WORK_DIR%\Backup_%DATE:~0,4%%DATE:~5,2%%DATE:~8,2%
    if not exist "!BACKUP_DIR!" mkdir "!BACKUP_DIR!"

    set /a count=0
    for %%f in (*.*) do (
        if not "%%f"=="%~nx0" (
            copy "%%f" "!BACKUP_DIR!" >nul
            set /a count+=1
        )
    )
    echo 已备份 !count! 个文件到 !BACKUP_DIR!
)

REM ============================================================
REM 统计文件数量
REM ============================================================
if /i "%COMMAND%"=="count" (
    echo 文件统计 (%WORK_DIR%):
    echo ================================

    set /a total=0
    for /r . %%f in (*) do (
        set /a total+=1
    )
    echo 总文件数: !total!

    echo.
    echo 按扩展名统计:
    for /r . %%f in (*) do (
        set ext=%%~xf
        if not defined ext set ext=(无扩展名)
        set ext=!ext:.=!
        set /a count_!ext!+=1 2>nul
    )
    for /f "tokens=1,2 delims==_" %%a in ('set count_') do (
        if not "%%b"=="" echo   .%%a: %%b 个文件
    )
)

REM ============================================================
REM 统计文件大小
REM ============================================================
if /i "%COMMAND%"=="size" (
    echo 文件大小统计 (%WORK_DIR%):
    echo ================================

    set /a total=0
    for /r . %%f in (*) do (
        set /a total+=%%~zf
    )

    set /a total_kb=!total!/1024
    set /a total_mb=!total_kb!/1024
    set /a total_gb=!total_mb!/1024

    echo 总大小:
    if !total_gb! GTR 0 (
        echo   !total_gb! GB (!total_mb! MB)
    ) else if !total_mb! GTR 0 (
        echo   !total_mb! MB (!total_kb! KB)
    ) else if !total_kb! GTR 0 (
        echo   !total_kb! KB (!total! 字节)
    ) else (
        echo   !total! 字节
    )

    echo.
    echo 按扩展名:
    for /r . %%f in (*) do (
        set ext=%%~xf
        if not defined ext set ext=(无扩展名)
        set /a size_!ext!+=%%~zf 2>nul
    )
    for /f "tokens=1,2 delims==_" %%a in ('set size_') do (
        if not "%%b"=="" (
            set /a kb=%%b/1024
            set /a mb=!kb!/1024
            if !mb! GTR 0 (
                echo   .%%a: !mb! MB
            ) else (
                echo   .%%a: !kb! KB
            )
        )
    )
)

endlocal
pause
```

## 12.4 Tomcat/Nginx 等应用管理脚本

```batch
@echo off
REM ============================================================
REM 文件名: appctl.bat
REM 描述: Tomcat / Nginx 应用管理脚本模板
REM 用法: appctl.bat {start|stop|restart|status}
REM ============================================================
setlocal enabledelayedexpansion

REM --- 配置区域：修改以下路径 ---
set APP_NAME=MyApp
set APP_HOME=C:\apps\myapp
set CATALINA_HOME=%APP_HOME%\tomcat
set NGINX_HOME=%APP_HOME%\nginx
set JAVA_HOME=C:\Program Files\Java\jdk-17

REM --- PID 文件 ---
set PID_FILE=%TEMP%\%APP_NAME%.pid

REM --- 判断应用类型 ---
if exist "%CATALINA_HOME%\bin\catalina.bat" (
    set APP_TYPE=tomcat
) else if exist "%NGINX_HOME%\nginx.exe" (
    set APP_TYPE=nginx
) else (
    set APP_TYPE=custom
)

REM ============================================================
REM 函数: 启动
REM ============================================================
:start
echo 正在启动 %APP_NAME%...

if "%APP_TYPE%"=="tomcat" (
    call "%CATALINA_HOME%\bin\startup.bat"
    if !errorlevel! EQU 0 (
        echo Tomcat 已启动
    ) else (
        echo [错误] Tomcat 启动失败
        exit /b 1
    )
) else if "%APP_TYPE%"=="nginx" (
    start "nginx" "%NGINX_HOME%\nginx.exe"
    echo Nginx 已启动
) else (
    REM 自定义启动命令
    start "%APP_NAME%" "%APP_HOME%\bin\app.exe"
    echo %APP_NAME% 已启动
)

REM 验证启动
timeout /t 5 /nobreak >nul
call :status
goto :eof

REM ============================================================
REM 函数: 停止
REM ============================================================
:stop
echo 正在停止 %APP_NAME%...

if "%APP_TYPE%"=="tomcat" (
    call "%CATALINA_HOME%\bin\shutdown.bat"
    timeout /t 3 /nobreak >nul
    REM 强制停止（如果未正常关闭）
    taskkill /f /fi "IMAGENAME eq java.exe" /fi "WINDOWTITLE eq %APP_NAME%*" 2>nul
) else if "%APP_TYPE%"=="nginx" (
    "%NGINX_HOME%\nginx.exe" -s stop
) else (
    taskkill /f /fi "IMAGENAME eq app.exe" 2>nul
)

echo %APP_NAME% 已停止
goto :eof

REM ============================================================
REM 函数: 状态
REM ============================================================
:status
echo 检查 %APP_NAME% 状态...

if "%APP_TYPE%"=="tomcat" (
    tasklist /fi "IMAGENAME eq java.exe" 2>nul | findstr /i "java" >nul
    if !errorlevel! EQU 0 (
        for /f "tokens=2" %%p in ('tasklist /fi "IMAGENAME eq java.exe" /nh') do (
            echo Tomcat 运行中 (PID: %%p)
        )
    ) else (
        echo Tomcat 未运行
    )
) else if "%APP_TYPE%"=="nginx" (
    tasklist /fi "IMAGENAME eq nginx.exe" 2>nul | findstr /i "nginx" >nul
    if !errorlevel! EQU 0 (
        echo Nginx 运行中
    ) else (
        echo Nginx 未运行
    )
) else (
    echo 请手动检查 %APP_NAME% 状态
)

goto :eof

REM ============================================================
REM 主逻辑
REM ============================================================
echo ========================================
echo  %APP_NAME% 管理工具
echo  类型: %APP_TYPE%
echo  路径: %APP_HOME%
echo ========================================

set ACTION=%~1
if "%ACTION%"=="" set ACTION=help

if /i "%ACTION%"=="start" (
    call :start
) else if /i "%ACTION%"=="stop" (
    call :stop
) else if /i "%ACTION%"=="restart" (
    call :stop
    timeout /t 2 /nobreak >nul
    call :start
) else if /i "%ACTION%"=="status" (
    call :status
) else (
    echo 用法: %0 {start^|stop^|restart^|status}
    echo.
    echo   start    启动 %APP_NAME%
    echo   stop     停止 %APP_NAME%
    echo   restart  重启 %APP_NAME%
    echo   status   查看状态
)

endlocal
```

## 12.5 定时备份脚本（配合 schtasks）

```batch
@echo off
REM ============================================================
REM 文件名: backup.bat
REM 描述: 增量备份脚本，配合 Windows 计划任务使用
REM
REM 计划任务创建命令:
REM   schtasks /create /tn "DailyBackup" /tr "C:\scripts\backup.bat" ^
REM       /sc daily /st 02:00 /ru SYSTEM /rl highest /f
REM ============================================================
setlocal enabledelayedexpansion

REM --- 配置 ---
set BACKUP_ROOT=D:\Backups
set BACKUP_DIR=%BACKUP_ROOT%\%COMPUTERNAME%_%DATE:~0,4%%DATE:~5,2%%DATE:~8,2%
set LOG_FILE=%BACKUP_ROOT%\backup.log
set ARCHIVE_DAYS=7

REM 要备份的目录（可添加多个）
set SOURCE_DIRS=^
    "C:\Users\%USERNAME%\Documents" ^
    "C:\Users\%USERNAME%\Desktop" ^
    "C:\Projects" ^
    "D:\Database"

REM 排除列表
set EXCLUDE_LIST=%TEMP%\backup_exclude.txt

REM --- 日志函数 ---
echo [%DATE% %TIME%] ==== 备份开始 ==== >> "%LOG_FILE%"

echo ========================================
echo  Windows 自动备份
echo  时间: %DATE% %TIME%
echo  主机: %COMPUTERNAME%
echo ========================================

REM --- 创建备份目录 ---
if not exist "%BACKUP_DIR%" (
    mkdir "%BACKUP_DIR%"
    if !errorlevel! NEQ 0 (
        echo [错误] 无法创建备份目录: %BACKUP_DIR%
        echo [%DATE% %TIME%] [错误] 无法创建备份目录 >> "%LOG_FILE%"
        exit /b 1
    )
)

echo 备份目录: %BACKUP_DIR%
echo.

REM --- 写入排除列表 ---
echo *.tmp > "%EXCLUDE_LIST%"
echo *.log >> "%EXCLUDE_LIST%"
echo *.bak >> "%EXCLUDE_LIST%"
echo Thumbs.db >> "%EXCLUDE_LIST%"
echo ~$* >> "%EXCLUDE_LIST%"
echo .git >> "%EXCLUDE_LIST%"
echo node_modules >> "%EXCLUDE_LIST%"

REM --- 执行备份 ---
set ERROR_COUNT=0
set FILE_COUNT=0

for %%d in (%SOURCE_DIRS%) do (
    set SOURCE=%%~d
    if exist "!SOURCE!" (
        echo 正在备份: !SOURCE!
        echo [%DATE% %TIME%] 备份: !SOURCE! >> "%LOG_FILE%"

        rem 计算目标目录（保留原来的目录结构）
        set DEST=!SOURCE:C:=D:%BACKUP_DIR%!
        set DEST=!DEST:%USERPROFILE%=%BACKUP_DIR%\Profile!

        rem 使用 robocopy 进行增量备份
        robocopy "!SOURCE!" "!DEST!" /e /copyall /r:2 /w:3 ^
            /xf *.tmp *.log *.bak Thumbs.db ^
            /xd .git node_modules ^
            /np /ndl /njh /njs ^
            /log+:%TEMP%\robocopy_log.txt

        set RC=!errorlevel!
        REM robocopy 退出码: 0-7 都算成功（0-7表示有文件复制/跳过）
        if !RC! GEQ 8 (
            set /a ERROR_COUNT+=1
            echo [警告] !SOURCE! 备份有问题 (robocopy 退出码: !RC!)
            echo [%DATE% %TIME%] [警告] !SOURCE! 退出码 !RC! >> "%LOG_FILE%"
        ) else (
            set /a FILE_COUNT+=!RC!
        )
    ) else (
        echo [跳过] 目录不存在: !SOURCE!
        echo [%DATE% %TIME%] [跳过] !SOURCE! >> "%LOG_FILE%"
    )
)

REM --- 压缩备份（可选） ---
echo.
set /p COMPRESS=是否压缩备份文件？[y/N]:
if /i "!COMPRESS!"=="y" (
    echo 正在压缩...
    powershell Compress-Archive -Path "%BACKUP_DIR%\*" -DestinationPath "%BACKUP_DIR%.zip" -Force
    if !errorlevel! EQU 0 (
        echo 压缩完成: %BACKUP_DIR%.zip
        rd /s /q "%BACKUP_DIR%"
    ) else (
        echo [警告] 压缩失败，保留原始文件
    )
)

REM --- 清理旧备份（保留最近 ARCHIVE_DAYS 天） ---
echo.
echo 清理 %ARCHIVE_DAYS% 天前的旧备份...
forfiles /p "%BACKUP_ROOT%" /s /m "%COMPUTERNAME%_*" /d -%ARCHIVE_DAYS% /c "cmd /c echo 删除 @path && rd /s /q @path" 2>nul

REM --- 生成备份报告 ---
echo.
set TOTAL_SIZE=0
for /r "%BACKUP_DIR%" %%f in (*) do set /a TOTAL_SIZE+=%%~zf 2>nul

echo ========================================
echo  备份完成！
echo  目录: %BACKUP_DIR%
echo  文件数: %FILE_COUNT%
echo  错误数: %ERROR_COUNT%
echo ========================================

echo [%DATE% %TIME%] ==== 备份结束 ==== >> "%LOG_FILE%"
echo [%DATE% %TIME%] 备份目录: %BACKUP_DIR% >> "%LOG_FILE%"
echo [%DATE% %TIME%] 文件数: %FILE_COUNT% 错误数: %ERROR_COUNT% >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

REM --- 发送通知（可选） ---
:: 如果配置了邮件，可以使用如下命令发送通知
:: 需要第三方工具如 blat, sendmail, 或 PowerShell
:: powershell Send-MailMessage -To admin@example.com -Subject "备份完成" -Body "备份报告" -SmtpServer smtp.example.com

REM --- 清理临时文件 ---
if exist "%EXCLUDE_LIST%" del "%EXCLUDE_LIST%"

echo.
echo 按任意键退出...
pause >nul

endlocal
```

---

# 13. 附录

## A. 常用 DOS 命令速查表

```batch
:: ============================================================
:: 基础命令
:: ============================================================
dir         列出目录内容
cd / chdir  切换目录
md / mkdir  创建目录
rd / rmdir  删除目录
copy        复制文件
move        移动/重命名文件
del / erase 删除文件
ren / rename 重命名
type        显示文件内容
echo        输出文本
cls         清屏
help        显示帮助
ver         显示版本
vol         显示卷标
date        显示/设置日期
time        显示/设置时间
title       设置窗口标题
color       设置控制台颜色
prompt      更改命令提示符
mode        配置系统设备

:: ============================================================
:: 磁盘命令
:: ============================================================
chkdsk      磁盘检查与修复
chkntfs     显示/修改磁盘启动检查
defrag      磁盘碎片整理
diskpart    磁盘分区管理
format      格式化磁盘
fsutil      文件系统工具
label       管理磁盘卷标
subst       路径映射为盘符

:: ============================================================
:: 系统命令
:: ============================================================
systeminfo  系统详细信息
tasklist    进程列表
taskkill    终止进程
taskmgr     任务管理器（图形界面）
msconfig    系统配置
reg         注册表操作
sc          服务控制
net         网络/服务/用户管理
netsh       网络配置
ipconfig    IP 配置
ping        网络连通测试
tracert     路由追踪
nslookup    DNS 查询
whoami      当前用户
shutdown    关机/重启

:: ============================================================
:: 脚本命令
:: ============================================================
for         循环
if          条件判断
set         设置变量
setlocal    局部变量
endlocal    结束局部变量
call        调用子程序
goto        跳转
shift       参数移位
pause       暂停
rem         注释
exit        退出
start       启动新窗口/程序
timeout     延迟等待
choice      选择输入
```

## B. 错误级别速查表

```batch
:: ============================================================
:: 常见错误级别
:: ============================================================
0          成功
1          功能不正确 / 一般错误
2          系统找不到指定文件
3          系统找不到指定路径
4          系统无法打开文件
5          拒绝访问
8          内存不足
12         访问码无效
87         参数错误
53         网络路径未找到
1450       系统资源不足
160        传递给 C 的代码无效

:: 常用命令的错误级别：
:: find
0          找到匹配
1          未找到匹配
2          参数错误

:: robocopy
0-7        成功
8          部分失败
16         严重错误

:: xcopy
0          成功复制
1          未找到文件
4          初始化错误
5          写入错误
```

## C. 通配符与正则

```batch
:: ============================================================
:: 通配符（DOS 标准）
:: ============================================================
*          匹配任意字符序列（含0个）
?          匹配单个字符

:: 示例
*.txt      所有 txt 文件
???.txt    三个字符名 + .txt
[0-9]*.txt 数字开头的 txt 文件

:: ============================================================
:: findstr 正则（基本）
:: ============================================================
.          任意字符
*          前一个字符0次或多次
^          行首
$          行尾
[abc]      在集合中
[^abc]     不在集合中
[a-z]      范围
\<         词首
\>         词尾
\          转义
```

## D. 注册表相关

```batch
:: ============================================================
:: reg 命令 - 注册表操作
:: ============================================================

reg query HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall
reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion" /v ProductName

:: 添加注册表值
reg add HKCU\Software\MyApp /v Version /t REG_SZ /d "1.0" /f

:: 导入/导出
reg export HKCU\Software\MyApp myapp.reg
reg import myapp.reg

:: 删除
reg delete HKCU\Software\MyApp /f
```

## E. PowerShell 对标 DOS 命令

```powershell
# ============================================================
# DOS 命令与 PowerShell 对比
# ============================================================

# DOS 命令         PowerShell 命令
# --------         ----------------
dir                Get-ChildItem  (ls, dir)
cd                 Set-Location  (cd, sl)
copy               Copy-Item     (cp, copy)
move               Move-Item     (mv, move)
del                Remove-Item   (rm, del, rmdir)
type               Get-Content   (cat, type)
echo               Write-Output  (echo)
set                Set-Variable  (set)
if/for             if/foreach    (语句更丰富)
find               Select-String (sls)
findstr            Select-String
sort               Sort-Object
more               Out-Host -Paging
tasklist           Get-Process   (ps)
taskkill           Stop-Process  (kill)
ping               Test-Connection
netstat            Get-NetTCPConnection
ipconfig           Get-NetIPConfiguration
```

## F. 环境变量一览

```batch
:: ============================================================
:: 常用系统环境变量
:: ============================================================

%ALLUSERSPROFILE%      C:\ProgramData
%APPDATA%              C:\Users\%USERNAME%\AppData\Roaming
%LOCALAPPDATA%         C:\Users\%USERNAME%\AppData\Local
%COMMONPROGRAMFILES%   C:\Program Files\Common Files
%COMPUTERNAME%         计算机名
%COMSPEC%              C:\Windows\System32\cmd.exe
%HOMEDRIVE%            C:
%HOMEPATH%             \Users\%USERNAME%
%HOMESHARE%            用户共享目录
%LOGONSERVER%          登录服务器
%NUMBER_OF_PROCESSORS% CPU 核心数
%OS%                   Windows_NT
%PATH%                 可执行文件搜索路径
%PATHEXT%              可执行文件扩展名
%PROCESSOR_ARCHITECTURE% CPU 架构
%PROCESSOR_IDENTIFIER%  CPU 标识
%PROCESSOR_LEVEL%       CPU 型号
%PROCESSOR_REVISION%    CPU 修订版
%PROGRAMDATA%          C:\ProgramData
%PROGRAMFILES%         C:\Program Files
%PROGRAMFILES(X86)%    C:\Program Files (x86)
%PSModulePath%         PowerShell 模块路径
%PUBLIC%               C:\Users\Public
%SYSTEMDRIVE%          C:
%SYSTEMROOT%           C:\Windows
%TEMP%                 C:\Users\%USERNAME%\AppData\Local\Temp
%TMP%                  同上
%USERDOMAIN%           用户域
%USERNAME%             当前用户名
%USERPROFILE%          C:\Users\%USERNAME%
%WINDIR%               C:\Windows
```

> 文档版本: 1.0 | 最后更新: 2026-06-08
> 如有问题或建议，欢迎改进本文档！
