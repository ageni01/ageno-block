---
cover: /assets/images/cover2.jpg
icon: pen-to-square
date: 2026-01-2
category:
  - Java多线程
tag:
  - IO
  - 多线程
star: true
sticky: true
---

# Java多线程
线程是操作系统进行资源分配的最小单位，线程之间可以并发执行。
线程的创建：
1. 继承Thread类
2. 实现Runnable接口
3. 使用Callable接口
4. 使用FutureTask

---

创建线程的步骤：
1. 创建一个实现Runnable接口的类，实现run()方法，定义线程要执行的任务。
2. 创建Runnable接口的实现类，创建Runnable接口的实现类对象。
3. 创建Thread类对象，构造方法中传递Runnable接口的实现类对象。
4. 创建Thread类对象，调用start()方法启动线程。
5. 当调用start()方法启动一个线程时，JVM会为这个线程创建一个栈空间，并调用run()方法，为线程分配任务。


---

创建线程的代码：
```
public class MyThread implements Runnable{
  public void run(){
    System.out.println("线程任务开始执行...");
    System.out.println("线程任务执行完毕...");
    System.out.println("线程任务开始执行...");
    System.out.println("线程任务执行完毕...");
    System.out.println("线程任务开始执行...");
  }
  public static void main(String[] args) {
    MyThread mt = new MyThread();
    Thread t = new Thread(mt);
    t.start();
    System.out.println("main方法结束...");
  }
}
```
