// main.cc
//	Bootstrap code to initialize the operating system kernel.
//
// Allows direct calls into internal operating system functions,
// to simplify debugging and testing.  In practice, the
// bootstrap code would just initialize data structures,
// and start a user program to print the login prompt.
//
// 	Most of this file is not needed until later assignments.
//
// Usage: nachos -d <debugflags> -rs <random seed #>
//		-s -x <nachos file> -c <consoleIn> <consoleOut>
//		-f -cp <unix file> <nachos file>
//		-p <nachos file> -r <nachos file> -l -D -t
//              -n <network reliability> -m <machine id>
//              -o <other machine id>
//              -z
//
//    -d causes certain debugging messages to be printed (cf. utility.h)
//    -rs causes Yield to occur at random (but repeatable) spots
//    -z prints the copyright message
//
//  USER_PROGRAM
//    -s causes user programs to be executed in single-step mode
//    -x runs a user program
//    -c tests the console
//
//  FILESYS
//    -f causes the physical disk to be formatted
//    -cp copies a file from UNIX to Nachos
//    -p prints a Nachos file to stdout
//    -r removes a Nachos file from the file system
//    -l lists the contents of the Nachos directory
//    -D prints the contents of the entire file system
//    -t tests the performance of the Nachos file system
//
//  NETWORK
//    -n sets the network reliability
//    -m sets this machine's host id (needed for the network)
//    -o runs a simple test of the Nachos network software
//
//  NOTE -- flags are ignored until the relevant assignment.
//  Some of the flags are interpreted here; some in system.cc.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#define MAIN
#include "copyright.h"
#undef MAIN

#include "utility.h"
#include "system.h"

#ifdef THREADS
extern int testnum;
#endif

// External functions used by this file

extern void ThreadTest(void), Copy(char *unixFile, char *nachosFile);
extern void Print(char *file), PerformanceTest(void);
extern void StartProcess(char *file), ConsoleTest(char *in, char *out);
extern void MailTest(int networkID);

//----------------------------------------------------------------------
// main
// 	Bootstrap the operating system kernel.
//
//	Check command line arguments
//	Initialize data structures
//	(optionally) Call test procedure
//
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------

int main(int argc, char **argv)
{
	int argCount; // the number of arguments
								// for a particular command

	DEBUG('t', "Entering main");
	(void)Initialize(argc, argv);

#ifdef THREADS

	// 例如输入命令为“./nachos -q 6”
	//-->argc=3, argv={"./nachos","-q","6"}
	//argv是一个二维数组，里面保存的是char*指针。开始时，argv指向"./nachos",
	//for循环中argc--, argv++使得argv指针指向第二个参数串（本例中即"-q"),argc--消耗一个参数串总数
	//因此for循环第一次循环的switch()是从第二个参数串开始判断，argv[0][1]指的是每个参数串"-"后面跟的字符（本例中为"q"）
	//每循环依次，argv就指向下一个参数串，并消耗一个参数串总数；直到没有参数串时停止循环。
	for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount)
	{
		argCount = 1;
		switch (argv[0][1])
		{
		case 'q':									 //当检测到'q'时，后面紧跟的数字要转换成int型，输入给ThreadTest()做switch()判断
			testnum = atoi(argv[1]); //当前检测到'q'，argv指向的是"-q",argv[0]是"-q",argv[1]就是后面紧跟的参数串"6"
			argCount++;							 //argCount置为2，使得下一次for循环直接跳过"-q"后面紧跟的"6",指向下一个参数串（如果有的话）
			break;
		default:
			testnum = 1; //如果没有输入"-q"参数，直接默认执行ThreadTest1(),作为测试
			break;
		}
	}

	ThreadTest();
#endif

	for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount)
	{
		argCount = 1;
		if (!strcmp(*argv, "-z")) // 如果命令行参数中存在"-z",那就print copyright
			printf(copyright);
#ifdef USER_PROGRAM
		if (!strcmp(*argv, "-x"))
		{ // 如果命令行参数中存在"-x",那就run a user program
			ASSERT(argc > 1);
			StartProcess(*(argv + 1)); // 开始执行用户程序，并将紧跟"-x"的参数串作为参数传入
			argCount = 2; // 跳过'-x'和后面紧跟的参数串
		}
		else if (!strcmp(*argv, "-c"))
		{ // 如果命令行参数中存在"-c",那就test the console
			if (argc == 1)
				ConsoleTest(NULL, NULL);
			else
			{
				ASSERT(argc > 2);
				ConsoleTest(*(argv + 1), *(argv + 2));
				argCount = 3;
			}
			interrupt->Halt(); // once we start the console, then
												 // Nachos will loop forever waiting
												 // for console input
		}
#endif // USER_PROGRAM
#ifdef FILESYS
		if (!strcmp(*argv, "-cp"))
		{ // 如果命令行参数中存在"-cp",那就copy from UNIX to Nachos
			ASSERT(argc > 2);
			Copy(*(argv + 1), *(argv + 2));
			argCount = 3;
		}
		else if (!strcmp(*argv, "-p"))
		{ // 如果命令行参数中存在"-p",那就print a Nachos file
			ASSERT(argc > 1);
			Print(*(argv + 1));
			argCount = 2;
		}
		else if (!strcmp(*argv, "-r"))
		{ // 如果命令行参数中存在"-r",那就remove Nachos file
			ASSERT(argc > 1);
			fileSystem->Remove(*(argv + 1));
			argCount = 2;
		}
		else if (!strcmp(*argv, "-l"))
		{ // 如果命令行参数中存在"-l",那就list Nachos directory
			fileSystem->List();
		}
		else if (!strcmp(*argv, "-D"))
		{ // 如果命令行参数中存在"-D",那就print entire filesystem
			fileSystem->Print();
		}
		else if (!strcmp(*argv, "-t"))
		{ // 如果命令行参数中存在"-t",那就performance test
			PerformanceTest();
		}
#endif // FILESYS
#ifdef NETWORK
		if (!strcmp(*argv, "-o"))
		{
			ASSERT(argc > 1);
			Delay(2); // delay for 2 seconds
								// to give the user time to
								// start up another nachos
			MailTest(atoi(*(argv + 1)));
			argCount = 2;
		}
#endif // NETWORK
	}

	currentThread->Finish(); // NOTE: if the procedure "main"
			// returns, then the program "nachos"
			// will exit (as any other normal program
			// would).  But there may be other
			// threads on the ready list.  We switch
			// to those threads by saying that the
			// "main" thread is finished, preventing
			// it from returning.
	return (0); // Not reached...
}
