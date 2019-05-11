# NachOs_4.1 编译过程
## 编译环境
- VMware® Workstation 15 Pro
- Ubuntu 18.04.1 LTS 64位
- g++-4.4
## 依赖库的安装
在64位系统编译32位程序需要安装一些依赖库文件，可以参考下面的链接进行提前安装，也可以根据错误输出提示安装所需要的库。

> [64位系统下配置32位开发环境](http://sixarm.com/about/ubuntu-apt-get-install-ia32-for-32-bit-on-64-bit.html)

我的库文件提前装好了，因此这里不再赘述。
## 编译目标

    ./NachOS-4.1/code/build.linux/Makefile

## debug过程
- 进入```/NachOS-4.1/code/build.linux```目录，依次执行```make clean```和```make distclean```命令

    这两句打扫编译环境，清理之前编译（如果编译过的话）产生的中间文件，每次编译前都最好执行者两个命令。两个命令输出如下：


      make clean
    > /bin/rm -f bitmap.o debug.o libtest.o sysdep.o interrupt.o stats.o timer.o console.o machine.o mipssim.o translate.o network.o disk.o alarm.o kernel.o main.o scheduler.o synch.o thread.o addrspace.o exception.o synchconsole.o directory.o filehdr.o filesys.o pbitmap.o openfile.o synchdisk.o post.o switch.o</br>
    > /bin/rm -f swtch.s
      
      make distclean
     > /bin/rm -f bitmap.o debug.o libtest.o sysdep.o interrupt.o stats.o timer.o console.o machine.o mipssim.o translate.o network.o disk.o alarm.o kernel.o main.o scheduler.o synch.o thread.o addrspace.o exception.o synchconsole.o directory.o filehdr.o filesys.o pbitmap.o openfile.o synchdisk.o post.o switch.o</br>
     > /bin/rm -f swtch.s</br>
     > /bin/rm -f nachos</br>
     > /bin/rm -f DISK_?</br>
     > /bin/rm -f core</br>
     > /bin/rm -f SOCKET_?</br>

- 执行```make depend```命令

      make depend

    输出如下错误：
    > ../lib/sysdep.h:15: fatal error: iostream.h: No such file or directory

    **错误原因：**</br>
    error输出中提到的文件```sysdep.h```第15行：

    ```C
    #include "iostream.h"
    ```
    是C语言的写法，在C++中，该头文件的引用标准写法应该是：
    ```C++
    #include <iostream>
    using namespace std;
    ```
    **解决方法：**</br>
    将```sysdep.h```第15行改成标准引用头文件写法即可。</br>

- 修改完后依次执行下面3个命令

      make clean
      make distclean
      make depend

    输出结果如下，说明成功生成 Makefile 依赖文件：

      g++-4.4 -I../network -I../filesys -I../userprog -I../threads -I../machine -I../lib -DFILESYS_STUB -DRDATA -DSIM_FIX -DTUT -Dx86 -DLINUX -DCHANGED -M ../lib/bitmap.cc ../lib/debug.cc ../lib/hash.cc ../lib/libtest.cc ../lib/list.cc ../lib/sysdep.cc ../machine/interrupt.cc ../machine/stats.cc ../machine/timer.cc ../machine/console.cc ../machine/machine.cc ../machine/mipssim.cc ../machine/translate.cc ../machine/network.cc ../machine/disk.cc ../threads/alarm.cc ../threads/kernel.cc ../threads/main.cc ../threads/scheduler.cc ../threads/synch.cc ../threads/synchlist.cc ../threads/thread.cc ../userprog/addrspace.cc ../userprog/exception.cc ../userprog/synchconsole.cc ../filesys/directory.cc ../filesys/filehdr.cc ../filesys/filesys.cc ../filesys/pbitmap.cc ../filesys/openfile.cc ../filesys/synchdisk.cc  ../network/post.cc > makedep
      ed - Makefile.dep < eddep
      rm eddep makedep 

- 执行```make```命令

      make

    输出如下错误：

      error: unrecognized command line option "-fwritable-strings"

    **错误原因：**</br>
    gcc 4.0以上版本都不再支持```-fwritable-strings```选项。</br>
    > GCC no longer accepts the -fwritable-strings option. Use named character arrays when you need a writable string.
    >
    > http://gcc.gnu.org/gcc-4.0/changes.html</br>

    **解决方法：**</br>
    将 Makefile 文件第203行：</br>
    ```makefile
      CFLAGS = -ftemplate-depth-100 -Wno-deprecated -g -Wall -fwritable-strings $(INCPATH) $(DEFINES) $(HOSTCFLAGS) -DCHANGED
    ```
    改为：（即删掉其中```-fwritable-strings```）
    ```makefile
      CFLAGS = -ftemplate-depth-100 -Wno-deprecated -g -Wall $(INCPATH) $(DEFINES) $(HOSTCFLAGS) -DCHANGED
    ```
- 修改后继续执行```make```命令

      make

    输出如下错误：
      
      ../lib/list.cc:240: error: there are no arguments to ‘IsEmpty’ that depend on a template parameter, so a declaration of ‘IsEmpty’ must be available
      ../lib/list.cc:240: note: (if you use ‘-fpermissive’, G++ will accept your code, but allowing the use of an undeclared name is deprecated)
      ../lib/list.cc:241: error: ‘first’ was not declared in this scope
      ../lib/list.cc:242: error: ‘last’ was not declared in this scope
      ../lib/list.cc:243: error: ‘first’ was not declared in this scope
      ../lib/list.cc:251: error: ‘numInList’ was not declared in this scope
      ../lib/list.cc:255: error: ‘last’ was not declared in this scope
      ../lib/list.cc:258: error: ‘numInList’ was not declared in this scope
      ../lib/list.cc: In member function ‘void SortedList<T>::SanityCheck() const’:
      ../lib/list.cc:341: error: ‘first’ was not declared in this scope
      ../lib/list.cc:341: error: ‘last’ was not declared in this scope
      ../lib/list.cc: In member function ‘void SortedList<T>::SelfTest(T*, int)’:
      ../lib/list.cc:371: error: there are no arguments to ‘RemoveFront’ that depend on a template parameter, so a declaration of ‘RemoveFront’ must be available
      ../lib/list.cc:374: error: there are no arguments to ‘IsEmpty’ that depend on a template parameter, so a declaration of ‘IsEmpty’ must be available

    **错误原因：**</br>

    参考 https://blog.csdn.net/lonelysky/article/details/52304378</br>

    **解决方法：**</br>
    为报错的所有函数如```IsEmpty(),RemoveFront()等```和变量```first,last等```都增加```this```指针：即将报错文件中的函数修改为下面的形式：
    ```C++
    this->first;
    this->last;
    this->IsEmpty();
    this->RemoveFront()
    ...
    ```
- 修改后继续执行```make```命令

      make

    输出如下错误：

      ...
      g++-4.4 -ftemplate-depth-100 -Wno-deprecated -g -Wall -I../network -I../filesys -I../userprog -I../threads -I../machine -I../lib -DFILESYS_STUB -DRDATA -DSIM_FIX -DTUT -Dx86 -DLINUX -DCHANGED -c ../threads/thread.cc
      ../threads/thread.cc: In member function ‘void Thread::Fork(void (*)(void*), void*)’:
      ../threads/thread.cc:98: error: cast from ‘void (*)(void*)’ to ‘int’ loses precision
      ../threads/thread.cc: In member function ‘void Thread::StackAllocate(void (*)(void*), void*)’:
      ../threads/thread.cc:345: error: cast from ‘void (*)()’ to ‘int’ loses precision
      ../threads/thread.cc: In member function ‘void Thread::SelfTest()’:
      ../threads/thread.cc:430: warning: deprecated conversion from string constant to ‘char*’

    **错误原因：**</br>
    上面error提示意思是从```void*```到```int ```的转换丢失精度。</br>
    这是因为在64位机器上，指针占用8个字节，而int 占用4个字节，因此从void指针转换为int会丢失精度。</br>

    **解决方法：**</br>
    找到报错文件所在的代码，将从```void*```到```int ```的转换改为从```void*```到```long ```的转换:
    ```C++
    98    DEBUG(dbgThread, "Forking thread: " << name << " f(a): " << (int) func << " " << arg);
    345   *(--stackTop) = (int) ThreadRoot;
    ```
    改为：
    ```C++
    98    DEBUG(dbgThread, "Forking thread: " << name << " f(a): " << (long) func << " " << arg);
    345   *(--stackTop) = (long) ThreadRoot;
    ```

- 修改后继续执行```make```命令

      make

    输出如下错误：

      swtch.s: Assembler messages:
      swtch.s:7: Error: invalid instruction suffix for `push'
      swtch.s:9: Error: invalid instruction suffix for `push'
      swtch.s:10: Error: operand type mismatch for `call'
      swtch.s:11: Error: operand type mismatch for `call'
      swtch.s:12: Error: operand type mismatch for `call'
      swtch.s:14: Error: invalid instruction suffix for `pop'

    **错误原因：**</br>

    64位系统和32位系统的as命令对于某些汇编指令的处理支持不一样造成的。

    **解决方法：**</br>

    在报错的文件头部加上```.code32```即可：

    ![](https://raw.githubusercontent.com/unclejimao/pictureBed/master/%E6%B1%87%E7%BC%96%E9%94%99%E8%AF%AF.png)

- 修改后继续执行```make```命令

      make


    没有报错，得到输出结果如下，说明编译成功：

      /lib/cpp  -P -I../network -I../filesys -I../userprog -I../threads -I../machine -I../lib -Dx86 -DLINUX ../threads/switch.s > swtch.s
      as -o switch.o swtch.s
      g++-4.4 bitmap.o debug.o libtest.o sysdep.o interrupt.o stats.o timer.o console.o machine.o mipssim.o translate.o network.o disk.o alarm.o kernel.o main.o scheduler.o synch.o thread.o addrspace.o exception.o synchconsole.o directory.o filehdr.o filesys.o pbitmap.o openfile.o synchdisk.o post.o switch.o  -o nachos

- 在该目录下执行```./nachos```命令，有如下输出，说明nachos安装成功：

      tests summary: ok:0
      Machine halting!

      Ticks: total 10, idle 0, system 10, user 0
      Disk I/O: reads 0, writes 0
      Console I/O: reads 0, writes 0
      Paging: faults 0
      Network I/O: packets received 0, sent 0
