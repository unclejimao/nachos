# Lab2 线程调度 实习说明

本实习希望通过修改Nachos系统平台的底层源代码，达到“扩展调度算法”的目标。

## 实习内容
### Exercise 1  调研
- 调研Linux或Windows中采用的进程/线程调度算法。具体内容见课堂要求。

### Exercise 2  源代码阅读
仔细阅读下列源代码，理解Nachos现有的线程调度算法。
- code/threads/scheduler.h和code/threads/scheduler.cc
- code/threads/switch.s
- code/machine/timer.h和code/machine/timer.cc

### Exercise 3  线程调度算法扩展
- 扩展线程调度算法，实现基于优先级的抢占式调度算法。
