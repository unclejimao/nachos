# Lab3 同步机制 实习说明

本实习希望通过修改Nachos系统平台的底层源代码，达到“扩展同步机制，实现同步互斥实例”的目标。

## 【实习内容】
### Exercise 1  调研
- 调研Linux中实现的同步机制。具体内容见课堂要求。

### Exercise 2  源代码阅读
阅读下列源代码，理解Nachos现有的同步机制。
- code/threads/synch.h和code/threads/synch.cc
- code/threads/synchlist.h和code/threads/synchlist.cc

### Exercise 3  实现锁和条件变量
- 可以使用sleep和wakeup两个原语操作（注意屏蔽系统中断），也可以使用Semaphore作为唯一同步原语（不必自己编写开关中断的代码）。

### Exercise 4  实现同步互斥实例
- 基于Nachos中的信号量、锁和条件变量，采用两种方式实现同步和互斥机制应用（其中使用条件变量实现同步互斥机制为必选题目）。
- 具体可选择“生产者-消费者问题”、“读者-写者问题”、“哲学家就餐问题”、“睡眠理发师问题”等。（也可选择其他经典的同步互斥问题）

# 完成情况
## Exercise 1  调研

&emsp;&emsp;在Linux中进行多线程开发，同步是不可避免的一个问题。在POSIX标准中定义了三种线程同步机制：
- Mutexes（互斥量）
- ConditionVariable（条件变量）
- POSIX Semaphores（信号量）

&emsp;&emsp;**同步互斥**的实现要依赖于院子操作，linux内核中原子操作定义在include/asm/atomic.h文件中，它们都使用汇编语言实现，因为C语言并不能实现这样的操作。原子操作主要用于实现资源计数，很多引用计数（refcnt）就是通过原子操作实现的。原子操作类型定义如下：

```typedef struct { volatile int counter; } atomic_t;```

&emsp;&emsp;volatile修饰字段告诉gcc不要对该类型的数据做优化处理，对它的访问都是对内存的访问，而不是对寄存器的访问。原子操作API包括：atomic_read、atomic_set、atomic_add等。

&emsp;&emsp;Linux内核的**信号量**在概念和原理上与用户态的System V的IPC机制信号量是一样的，但是它绝对不可能在内核之外使用，因此它与System V的IPC机制信号量毫不相干。信号量在创建时需要设置一个初始值，表示同时可以有几个任务可以访问该信号量保护的临界资源。对信号量操作的API有DECLARE_MUTEX(name)、DECLARE_MUTEX_LOCKED(name)、sema_init(sem,val)、init_MUTEX()等。

&emsp;&emsp;**自旋锁**。自旋锁与互斥锁类似，只是自旋锁不会引起调用者睡眠，如果自旋锁已经被别的执行单元保持，调用者就一直循环在那里看是否该自旋锁的保持者已经释放了锁。信号量适用于保持时间较长的情况，它们会导致调用者睡眠，因此只能在进程上下文使用，而自旋锁适合于保持时间非常短的情况，它可以在任何上下文使用。操作自旋锁的API有spin_lock_init()、DEFINE_SPINLOCK()、SPIN_LOCK_UNLOCKED、spin_trylock()、spin_lock()等。

&emsp;&emsp;**大内核锁**。大内核锁本质上也是自旋锁，但又不同于自旋锁，自旋锁不可以递归获得，因为那样会导致死锁，而大内核锁可以递归获得。大内核锁用于保护整个内核，因而整个内核只有一个大内核锁。

## Exercise 2  源代码阅读
### code/threads/synch.h & synch.cc

&emsp;&emsp;这里定义了三种同步：信号量，锁和条件变量。给出了信号量的实现;对于后两者，只给出了接口的过程——它们将作为第一个任务的一部分来实现。


- **Semaphore**

    &emsp;&emsp;这个类中定义了私有变量信号量值(value)和线程等待队列(queue)，以及提供操作的公有方法。P()和V()。

    - P操作：

    1. 当value等于0时，将当前运行线程放入线程等待队列，当前进程进入睡眠状态，并切换到其他线程运行。

    2. 当value大于0时，value--。

    - V操作：

        如果线程等待队列中有等待该信号量的线程，取出其中一个将其设置成就绪态，准备运行。value++。

 

- **Lock**

    &emsp;&emsp;现有的Nachos中，没有给出锁机制的实现，锁的基本结构也只给出了部分内容，其他内容可以视具体实现决定。总体来说，锁的操作有两个，获得锁(Acquire)和释放锁(Release)，他们都是原子操作。

    - Acquire：

        当锁处于BUSY态，进入睡眠状态。当锁处于FREE态，当前进程获得该锁，继续运行。

    - Release：

        释放锁（只能由拥有锁的线程才能释放锁），将锁的状态设置为FREE态，如果有其他线程等待该锁，将其中的一个唤醒，进入就绪态。

 

- **Condition**
    
    &emsp;&emsp;条件变量和信号量和锁机制不一样，它是没有值的（实际上锁机制是一个二元信号量，可以通过信号量来实现）。当一个线程需要的某种条件没有得到满足时，可以将自己作为一个等待条件变量的线程插入所有等待该条件变量的队列，只要条件一旦得到满足，该线程就会被唤醒继续运行。
    
    &emsp;&emsp;条件变量总是和锁机制一起使。它的基本方法有Wait、Signal以及BroadCast。所有这些操作必须在当前线程获得一个锁的前提下，而且所有对一个条件变量进行的操作必须建立在同一个锁的前提下。

    - Wait(Lock *conditionLock) 

        线程等待条件变量

    1. 释放该锁

    2. 进入睡眠状态

    3. 重新申请该锁

    - Signal(Lock *conditionLock)

        从条件变量的等待队列中唤醒一个等待该条件变量的线程。

    - BroadCast(Lock *conditionLock)

        唤醒所有等待该条件变量的线程。
### code/threads/synchlist.h & synchlist.cc

&emsp;&emsp;提供了对一个List的互斥访问，确保在多线程同时访问该List时，不会发生同步错误。

&emsp;&emsp;类中初始化时生成一个Lock对象和List对象。提供了Append()，Remove()和Mapcar()操作。每个操作中都要先获得该锁，然后在对List进行相应的操作。

## Exercise 3  实现锁和条件变量

## Exercise 4  实现同步互斥实例