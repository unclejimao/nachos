# Lab1_线程机制 实习说明

&emsp;&emsp;本实习希望通过修改Nachos系统平台的底层源代码，达到“扩展线程机制”的目标。

## 【实习内容】
### Exercise 1  调研

- 调研Linux或Windows中进程控制块（PCB）的基本实现方式，理解与Nachos的异同。

### Exercise 2  源代码阅读
- 仔细阅读下列源代码，理解Nachos现有的线程机制。
    
    code/threads/main.cc和code/threads/threadtest.cc
    code/threads/thread.h和code/threads/thread.cc

### Exercise 3  扩展线程的数据结构

- 增加“用户ID、线程ID”两个数据成员，并在Nachos现有的线程管理机制中增加对这两个数据成员的维护机制。

### Exercise 4  增加全局线程管理机制
- 在Nachos中增加对线程数量的限制，使得Nachos中最多能够同时存在128个线程；
- 仿照Linux中PS命令，增加一个功能TS(Threads Status)，能够显示当前系统中所有线程的信息和状态。

--------------------------------------
# 完成情况
## Exercise 1  调研

&emsp;&emsp;在Linux中的每一个进程由一个task_struct数据结构来描述。task_struct就是通常所说的进程控制块（PCB）。task_struct容纳了一个进程的所有信息，是对系统进程进行控制的唯一手段，也是最有效的手段。task_struct存放在/include/linux/sched.h中。

&emsp;&emsp;Nachos相对与Linux系统的线程部分来讲，要简单许多。它的PCB仅有几个必须的变量，并且定义了一些最基本的对线程操作的函数。Nachos线程的总数目没有限制，线程的调度比较简单，而且没有实现线程的父子关系等。很多地方需要我们进行完善。

**调研版本：Linux-3.5.4。**

Linux系统的PCB包括了很多参数，主要的参数有：

|序号|参数|描述|
|:---|:---|:---|
|1|进程状态|进程状态是进程调度和交换的依据。Linux下进程设置了5种状态。分别是运行态、可中断态、不可中断态、僵尸状态、暂停态。|
|2|进程调度信息|调度程序利用这些信息决定下一个应该运行的进程。这部分信息包括进程的类别（普通进程还是实时进程）、进程的优先级等。|
|3|标识符|每个进程都有很多标识符来标识它，标识符有进程标识符、用户标识符、组标识符、备份用户标识符、文件系统用户标识符等。标识符可以用于控制进程对系统中文件和设备的访问。|
|4|进程通信相关信息|Linux支持多种不同形式的通信机制。它支持典型的Unix通信机制：信号、管道，也支持System V通信机制：共享内存、信号量和消息队列。|
|5|进程链接信息|Linux中进程有继承关系。除了初始化进程init，其他进程都有一个父进程。每个进程可以通过fork()或clone()系统调用来创建子进程，除了进程标识符等必要的信息外，子进程的task_struct结构中的绝大部分信息都是从父进程中拷贝过来的。系统记录这种父/子、兄/弟关系，使进程间的协作更加方便。task_struct中有许多指针，通过这些指针，系统中所有的task_struct结构就构成了一颗进程树。|
|6|时间和定时器信息|内核需要记录进程在其生存期内使用CPU的时间以便用于统计、计费等有关操作。进程耗费CPU的时间由两部分组成：一是在用户态下耗费的时间，一是在系统态下耗费的时间。这类信息还包括进程剩余的时间片和定时器信息等，以控制相应事件的触发。|
|7|文件系统信息|进程可以打开或关闭文件，文件属于系统资源，Linux内核要对进程使用文件的情况进行记录。|
|8|虚拟内存信息|除了内核线程，每个进程都拥有自己的地址空间，用mm_struct来描述。|
|9|页面管理信息|当物理内存不足时，Linux内存管理子系统需要把内存中部分页面交换到外存，其交换是以页为单位的。这部分结构记录了交换所用到的信息|
|10|对称多处理器信息|与多处理器相关的几个域。|
|11|处理器上下文信息|当进程暂时停止运行时，处理机的状态必须保存在进程的task_struct。当进程被调度重新运行时再从中恢复这些环境，也就是恢复这些寄存器和堆栈的值。|
|12|其他|记录一些其他的必要信息。|

## Exercise2  源代码阅读

### NachOS-3.4/code/threads/main.cc
&emsp;&emsp;main.cc是整个操作系统kernel启动的入口，通过它可以直接调用操作系统的方法。通过程序中的main函数，配以不同的参数，可以调用Nachos操作系统不同部分的各个方法。

```C++
//main.cc是用于初始化操作系统内核的引导程序代码。
//允许直接调用内部操作系统函数，以简化调试和测试。实际上，引导程序代码只是初始化数据结构，并启动一个用户程序来打印登录提示。
//在以后分配部署之前，该文件的大部分都不需要。
//解释了该函数的调用方法和参数作用

//main函数有两个参数，第一个是命令行输入的参数个数，第二个是命令行参数字符串，以char**的形式传入。
//main函数通过逐个解析命令行参数，来完成不同的数据结构的初始化，并调用测试程序
int
main(int argc, char **argv)
{
    //程序主体
}
```

### NachOS-3.4/code/threads/threadtest.cc
&emsp;&emsp;这是一个简单的线程实验的测试用例。用于指导我们如何对线程的修改进行测试的。

```C++
//一个简单的线程分配的测试用例
//创建两个线程，并通过调用thread::yield在它们之间来回切换上下文，以说明线程系统的内部工作。

int testnum = 1;    //测试程序号，对应相应的测试程序。

//SimpleThread()：一个5次循环的程序，每次循环中都让出CPU，让其他就绪的线程执行
void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();     //currentThread是表示当前占用CPU的线程的类，Yield()方法使当前占用CPU的线程释放CPU，并把该线程放入就绪队列尾部
    }
}

//ThreadTest1()：一个测试方法，创建一个分支线程，使之调用SimpleThread()方法，这样两个线程可以交替执行。
//注意这两个线程分别是：
//      第一个线程是ThreadTest()开始执行时创建的原始线程（我们姑且称之为线程0），它调用了ThreadTest1()方法，并在ThreadTest1()方法里面创建了一个叫做“forked thread”的新线程
//      第二个线程就是在ThreadTest1()方法里面创建的叫做“forked thread”的新线程（我们称之为线程1）
void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");    //创建一个叫做“forked thread”的新线程

    t->Fork(SimpleThread, (void*)1);    //这一句调用Thread类的Fork()方法，该方法把当前线程（线程1）放到就绪队列尾部，
                                        //SimpleThread作为参数传入（并给了一个参数 1），并不立刻执行，当线程1占用CPU时，才调用SimpleThread()；
                                        //此时占用CPU的线程仍然是线程0
    SimpleThread(0);    //线程0调用SimpleThread()，传入参数 0，SimpleThread()开始执行，
                        //把当前占用CPU的线程即线程0从CPU取下放至就绪队列尾部；
                        //此时就绪队列中排在第一位的线程是前两句执行完得到的线程1，CPU空闲则线程1上CPU,
                        //线程1执行SimpleThread()，然后让出CPU，把自己放到就绪队列的尾部，以使线程0上CPU
                        //两个线程就这样交替使用CPU，直到循环结束
}

//可以看做一个总控程序，根据main函数传过来testnum参数值来执行不同的测试程序。例如，当testnum==1时，就执行ThreadTest1()。

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

```

### NachOS-3.4/code/threads/thread.h

&emsp;&emsp;定义了管理Thread的数据结构，即Nachos中线程的上下文环境，都被定义在Thread.h中。主要包括当前线程栈顶指针、所有寄存器的状态、栈底、线程状态、名字。当前栈指针和机器状态的定义必须按指定位置，因为Nachos在线程切换时，会按照预先指定的顺序操作线程上下文内存和寄存器。在Thread类中还声明了一些基本的方法，如Fork()、Yield()、Sleep()等等。

```C++
//用于管理线程的数据结构。线程表示程序中代码的顺序执行。因此，线程的状态包括程序计数器、处理器寄存器和执行堆栈。
//注意，因为我们为每个线程分配了一个固定大小的堆栈，所以有可能溢出堆栈——例如，通过递归到一个太深的级别。发生这种情况的最常见原因是在堆栈上分配大型数据结构。
//如果溢出了堆栈，就会出现错误的情况，并且在最坏的情况下，问题可能不会被明确捕获。相反，唯一的症状可能是奇怪的分割错误。（当然，其他问题也会导致SEG错误，因此这并不能确定您的线程堆栈是否太小。）
//如果您发现自己有SEG错误，可以尝试增加线程堆栈的大小——ThreadStackSize.
//在这个接口中，分叉(fork)线程需要两个步骤。我们必须首先为它分配一个数据结构：
//“t=new Thread”
//只有这样我们才能做分叉(fork)：
//"t->fork(f，arg)"

class Thread {
  private:
    // NOTE: DO NOT CHANGE the order of these first two members.
    // THEY MUST be in this position for SWITCH to work.
    int* stackTop;			 // 当前栈顶指针
    void *machineState[MachineStateSize];  // 除栈顶指针外的所有寄存器，最多18个

  public:
    Thread(char* debugName);		// 初始化一个线程
    ~Thread(); 				// 删除一个线程，注意只能在线程不运行时才能调用删除方法

    // 基本线程操作

    void Fork(VoidFunctionPtr func, void *arg); 	// 派生一个线程，放在就绪队列末尾，当它占用CPU时，调用参数列表传入的方法func（以 *arg作为参数）
    void Yield();  				// 当有其他线程准备就绪时，放弃CPU
    void Sleep();  				// 放弃CPU并设置进程状态为BLOCKED
    void Finish();  				// The thread is done executing
    void CheckOverflow();   			// Check if thread has overflowed its stack
    void setStatus(ThreadStatus st) { status = st; }    //  设置进程状态为枚举类型ThreadStatus { JUST_CREATED, RUNNING, READY, BLOCKED }之一

    char* getName() { return (name); }
    void Print() { printf("%s, ", name); }

  private:
    // some of the private data for this class is listed above
    
    int* stack; 	 		// Bottom of the stack 
					// NULL if this is the main thread
					// (If NULL, don't deallocate stack)
    ThreadStatus status;		// ready, running or blocked
    char* name;

    void StackAllocate(VoidFunctionPtr func, void *arg);
    					// Allocate a stack for thread.
					// Used internally by Fork()

#ifdef USER_PROGRAM
// A thread running a user program actually has *two* sets of CPU registers -- 
// one for its state while executing user code, one for its state 
// while executing kernel code.

    int userRegisters[NumTotalRegs];	// user-level CPU register state

  public:
    void SaveUserState();		// save user-level register state
    void RestoreUserState();		// restore user-level register state

    AddrSpace *space;			// User code this thread is running.
#endif
}
```

### NachOS-3.4/code/threads/thread.cc

&emsp;&emsp;Thread.cc中主要是管理Thread的一些事务。主要包括了四个主要的方法。Fork()、Finish()、Yield()、Sleep()。在Thread.h中对它们进行了声明，在Thread.cc中则负责具体的实现。注意到，这里实现的方法大多是都是原子操作，在方法的一开始保存中断层次关闭中断，并在最后恢复原状态。

- Thread()：构造函数，初始化一个新的Thread。

- Fork(VoidFunctionPtr func,int arg):
    
    - func，新线程运行的函数；
    - arg，func函数的参数。它的实现包括一下几步：
        - 分配一个堆栈
        - 初始化堆栈
        - 将线程放入就绪队列

- Finish()：并不是直接收回线程的数据结构和堆栈，因为我们仍在这个堆栈上运行这个线程。做法是将threadToBeDestroyed的值设为当前线程，使得Scheduler的Run()可以调用销毁程序，当我们这个程序退出上下文时，将其销毁。

- Yield()：调用scheduler找到就绪队列中的下一个线程，并让其执行。以达到放弃CPU的效果。

## Exercise 3  扩展线程的数据结构
**增加“用户ID、线程ID”两个数据成员，并在Nachos现有的线程管理机制中增加对这两个数据成员的维护机制。**

- 增加用户ID：

    **设计思路：**
    现有的Nachos代码不支持多用户，我们获取当前Linux系统的用户ID作为Nachos的用户ID。

    **代码修改：**

    在thread.h文件的thread类声明中添加私有的int型成员变量userID、添加公有的成员方法getUserID()用于获取当前的userID。

    代码修改见  [thread.h](./thread.h)

    在thread.cc文件的开始位置，引入头文件“unistd.h”。它是C和C++中提供的对POSIX操作系统的访问功能的头文件，这样，我们就可以在Nachos中直接获取当前Linux的用户ID了。此后在Thread的构造函数中添加语句userID=getuid()，即可获取当前Linux系统的用户ID。在thread.h文件中还实现了getUserID()方法，这个方法很简单，return一个userID即可。

    代码修改见 [thread.cc](./thread.cc)
- 增加线程ID：

    **设计思路：**

    声明一个全局的数组，定义这个数据有128个元素。每个元素的取值为0或1。0表示该数组下标没有作为线程的ID分配出去，1表示以分配。每次创建一个新线程的时候从1开始遍历此数组，得到的第一个值为0元素的数组下标作为线程的ID分配给将要创建的线程。如果遍历一遍数组发现没有可用的ID了，则不创建该线程，并输出提示。另外，由于系统的全局变量都声明和定义在system.h或system.cc中，我们也将该数组定义在这里。

    **代码修改：**

    在thread.h文件的thread类声明中添加int型的私有成员变量threadID、添加公有的成员方法getThreadID()用于获取当前的threadID。
    
    代码修改见  [thread.h](./thread.h)

    在system.cc中，声明可以创建线程数的最大值MaxThread为128、并声明一个threadIDs[]数组用于记录分配和未分配的线程ID数值。

    在system.cc的Initialize()中，定义threadIDs[MaxThread]数组，并将其值都初始化为0。

    代码修改见 [system.cc](./system.cc)

    在thread.cc文件中，定义allocatedThreadID()方法。该方法每次从1开始遍历threadIDs数组，发现的第一个值为零的元素下标作为线程的ID。如果没有可分配的ID，则返回-1。同时，在thread构造函数的一开始，用flag接收一个allocatedThreadID()返回的线程ID。如果为-1，则输出提示，停止创建过程。否则继续创建该线程。并在析构函数中将完成的线程ID的对应的数组值重新赋为0。 

    代码修改见 [thread.cc](./thread.cc)

    随后修改测试用例，验证输出

    测试用例修改见 [threadtest.cc](./threadtest.cc)

    重新```make```后，命令行输入```./nachos -q 2```,运行结果：

    ![](https://raw.githubusercontent.com/unclejimao/pictureBed/master/lab1_exercise3.png)
    ![](https://raw.githubusercontent.com/unclejimao/pictureBed/master/lab1_exercise3_1.png)

## Exercise 4  增加全局线程管理机制

- 增加线程数量

    **设计思路：**

    根据上一个实验，为每个线程分配了一个ID。最大值仍设为128。如果线程数超过128则返回ID为-1。因此在创建新线程时进行一次判断，如果返回的ID为-1，则停止创建即可。

    **代码修改：**

    在Exercise3的基础上，修改thread.cc文件。关键在```ASSERT(flag!=-1)```

    代码修改见 [thread.cc](./thread.cc)

    在threadtest.cc中创建ThreadTest3()，循环创建128个线程，查看输出

    测试用例修改见 [threadtest.cc](./threadtest.cc)

- 增加TS命令参数

    **设计思路：**

    当前的Nachos操作系统中只有READY队列，所以要输出系统中所有的线程，只需要输出READY队列，即可，需要注意的是，总是有一个线程处于正在执行的状态。所以输出中要把这个线程也加上。

    通过阅读代码和查资料了解到，readyList是定义在scheduler.cc中的，但是它是一个私有类型的变量，所以要添加一个公有的get方法获得它。

    同时了解到list.cc中有一个Mapcar(func)的方法。可以对List中的每个元素调用func方法。

    **代码修改：**

    在main.cc中，添加相应方法，使得系统能识别TS命令，在其中将testnum设为3。

    代码修改见 [main.cc](./main.cc)

    在thread.cc中添加MyThreadPrint()的定义，首先将item强制类型转换为Thread类型。之后打印其相关内容。

    代码修改见 [thread.cc](./thread.cc)

    在threadtest.cc中创建ReadyListPrint()函数和ThreadTest4()函数，ReadyListPrint()的主要作用是打印出该线程正在执行时的状态，包括打印出当前线程和readyList中的线程。这里用到了关中断，以防过程中被其他线程中断。

    ThreadTest4()中创建了3个线程，每个线程都执行ReadyListPrint()方法。

    测试用例修改见 [threadtest.cc](./threadtest.cc)