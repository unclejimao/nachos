// synchlist.h
//	Data structures for synchronized access to a list.
//  用于同步访问列表的数据结构
//
//	Implemented by surrounding the List abstraction
//	with synchronization routines.
//  通过使用同步例程包围List抽象来实现。
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef SYNCHLIST_H
#define SYNCHLIST_H

#include "copyright.h"
#include "list.h"
#include "synch.h"

// The following class defines a "synchronized list" -- a list for which:
// these constraints hold:
//	1. Threads trying to remove an item from a list will
//	wait until the list has an element on it.
//	2. One thread at a time can access list data structures

class SynchList
{
public:
  SynchList();  // initialize a synchronized list
  ~SynchList(); // de-allocate a synchronized list

  void Append(void *item);           // append item to the end of the list,
                                     // and wake up any thread waiting in remove
  void *Remove();                    // remove the first item from the front of
                                     // the list, waiting if the list is empty
  void Mapcar(VoidFunctionPtr func); // apply function to every item in the list

private:
  List *list;           // the unsynchronized list
  Lock *lock;           // enforce mutual exclusive access to the list
  Condition *listEmpty; // wait in Remove if the list is empty
};

#endif // SYNCHLIST_H
