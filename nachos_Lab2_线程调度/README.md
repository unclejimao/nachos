# Lab2 线程调度 实习说明

本实习希望通过修改Nachos系统平台的底层源代码，达到“扩展调度算法”的目标。

## 【实习内容】
### Exercise 1  调研
- 调研Linux或Windows中采用的进程/线程调度算法。具体内容见课堂要求。

### Exercise 2  源代码阅读
仔细阅读下列源代码，理解Nachos现有的线程调度算法。
- code/threads/scheduler.h和code/threads/scheduler.cc
- code/threads/switch.s
- code/machine/timer.h和code/machine/timer.cc

### Exercise 3  线程调度算法扩展
- 扩展线程调度算法，实现基于优先级的抢占式调度算法。

# 完成情况
## Exercise 1  调研



## Exercise 2  源代码阅读

### code/threads/scheduler.h
- scheduler.h定义了线程调度用到的一些数据结构，主要包括了一个就绪队列的列表```readyList```，还有一些关于操作就绪队列的方法，比如找到一个可以运行的线程```FindNextToRun()```，运行一个线程```Run(Thread* nextThread)```，将一个线程设置为Ready并加入就绪队列的末尾```ReadyToRun(Thread* thread)```，打印就绪队列的内容```Print()```等。
### code/threads/scheduler.cc

- scheduler.cc中实现了scheduler.h中的这些函数，值得一提的是Run()函数。它的执行步骤如下：

    1. 保存当前线程为旧线程；

    2. 如果是用户程序，则保存当前CPU寄存器的状态；

    3. 检查当前线程是否有存在栈溢出；

    4. 将nextThread的状态设置为运行态，并作为currentThread运行线程；

    5. 切换到nextThread线程运行 ```SWITCH(oldThread, nextThread);```；

    6. 如果有threadToBeDestroyed线程，则销毁它，并释放它所占用的空间；

    7. 如果是用户程序，恢复当前CPU寄存器的状态。

### code/threads/switch.s

- 这里定义了线程交换的有关内容，线程的上下文切换部分涉及到寄存器的分配，因此这部分内容直接依赖于机器。Nachos目前支持四种类型的机器架构。文件中定义了对这四种机器的两个操作，分别是ThreadRoot()和SWITCH()。

- Nachos中，除了main线程外，所有其他的线程都是从ThreadRoot入口运行的，它的语法是```ThreadRoot(int InitialPC,int InitialArg,int WhenDonePC,intStartupPC)```，其中InitialPC指明新生成线程的入口函数地址，InitialArg是该入口函数的参数，StartupPC是在运行该线程时需要作的一些初始化工作；而WhenDonePC是该线程运行结束时需要作的一些后续工作。在Nachos源代码中，没有任何一个函数和方法显式地调用ThreadRoot函数，ThreadRoot函数只有在被切换时才被调用到。这几个参数是在生成线程分配栈空间时被初始化。

- Switch函数则相对简单，首先保存了原运行线程的状态，其次恢复新运行线程的状态，最后在新运行的线程的栈空间上运行新线程。

### code/machine/timer.h & timer.cc

- 这里定义了一个模拟硬件时间的数据结构。硬件计时器每隔一定的时间产生一个CPU中断，我们可以利用这个中断实现时间分片。

- ```Timer(VoidFunctionPtr timerHandler, int callArg, bool doRandom)```初始化这个模拟时钟。参数timerHandler表示时钟中断处理函数，callArg是timerHandler函数的参数，doRandom表示是否允许中断随机发生，而不是预先制定好。

- 变量初始化完成后，该函数把一个时钟中断插入等待处理的中断队列，当时钟中断时刻到来时，调用TimerHandler函数，调用它的TimerExpired方法，该方法将新的时钟中断插入等待处理中断队列中，然后再调用真正的时钟中断处理函数。

- ```TimerExpired()```该函数将新的时钟中断插入等待处理中断队列中，然后再调用真正的时钟中断处理函数，这样Nachos就可以定时收到时钟中断。

- ```TimerOfNextInterrupt()```该函数返回距离下一次中断发生所需要的时钟周期数。