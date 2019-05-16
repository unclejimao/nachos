// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"
#include "synch.h"


// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	    
        //******************************************************************************************************************************************//
        // *Begin* These lines are added for Lab1 Exerise3 

        // printf("*** thread %d looped %d times\n", which, num);
        printf("User-ID%d Cruuent thread %s whose thread ID is %d loop %d times\n",currentThread->getUserID(),currentThread->getName(),currentThread->getThreadID(),num);

        // *End* These lines are added for Lab1 Exerise3 
        //******************************************************************************************************************************************//
        currentThread->Yield();
    }
}


//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}

//******************************************************************************************************************************************//
// *Begin* These lines are added for Lab1 Exerise3 

void
ThreadTest2(){

    DEBUG('t', "Entering ThreadTest2");

    Thread *t1 = new Thread("forked t1");
    Thread *t2 = new Thread("forked t2");
    Thread *t3 = new Thread("forked t3");
    Thread *t4 = new Thread("forked t4");

    t1->Fork(SimpleThread,(void*)(t1->getThreadID()));
    t2->Fork(SimpleThread,(void*)(t2->getThreadID()));
    t3->Fork(SimpleThread,(void*)(t3->getThreadID()));
    t4->Fork(SimpleThread,(void*)(t4->getThreadID()));

    SimpleThread(0);
}

// *End* These lines are added for Lab1 Exerise3 
//******************************************************************************************************************************************//

//******************************************************************************************************************************************//
// *Begin* These lines are added for Lab1 Exerise4

void
ThreadTest3(){

    DEBUG('t', "Entering ThreadTest3");
    for (int j = 0; j <= 128; j++)
    {
        printf("This is the %d th loop...\n",j);
        Thread *t = new Thread("forked thread");
        printf("-*- User-ID%d is generating a new thread: %s%d \n",t->getUserID(),t->getName(),t->getThreadID());
    }
    
}

void
ReadyListPrint(int what){

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf("Name %s, threadID %d, status: %s.\n",currentThread->getName(),currentThread->getThreadID(),currentThread->getStatus());
    
    List *list = new List();
    list = scheduler->getReadyList();

    if (!list->IsEmpty())
    {
        list->Mapcar(MyThreadPrint);
    }
    currentThread->Yield();
    interrupt->SetLevel(oldLevel);

void
ThreadTest4(){
    DEBUG('t', "Entering ThreadTe/* code */t4");

    Thread *t1 = new Thread("forked t1");
    Thread *t2 = new Thread("forked t2");
    Thread *t3 = new Thread("forked t3");

    t1->Fork(ReadyListPrint,(void*)0);
    t2->Fork(ReadyListPrint,(void*)0);
    t3->Fork(ReadyListPrint,(void*)0);
}

// *End* These lines are added for Lab1 Exerise4 
//******************************************************************************************************************************************//


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
        case 1:
            ThreadTest1();
            break;
        //******************************************************************************************************************************************//
        // *Begin* These lines are added for Lab1 Exerise3 

        case 2:
            ThreadTest2();
            break;

        // *End* These lines are added for Lab1 Exerise3 
        //******************************************************************************************************************************************//
        
        //******************************************************************************************************************************************//
        // *Begin* These lines are added for Lab1 Exerise4

        case 3:
            ThreadTest3();
            break;
        
        case 4:
            ThreadTest4();
            break;

        // *End* These lines are added for Lab1 Exerise4 
        //******************************************************************************************************************************************//

        default:
            printf("No test specified.\n");
            break;
    }
}

