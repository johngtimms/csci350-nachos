// synch.h 
//  Data structures for synchronizing threads.
//
//  Three kinds of synchronization are defined here: semaphores,
//  locks, and condition variables.  The implementation for
//  semaphores is given; for the latter two, only the procedure
//  interface is given -- they are to be implemented as part of 
//  the first assignment.
//
//  Note that all the synchronization objects take a "name" as
//  part of the initialization.  This is solely for debugging purposes.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// synch.h -- synchronization primitives.  

#ifndef SYNCH_H
#define SYNCH_H

#include "copyright.h"
#include "thread.h"
#include "list.h"

class Semaphore {
  public:
    Semaphore(char* debugName, int initialValue);   // set initial value
    ~Semaphore();                       // de-allocate semaphore
    char* getName() { return name;}         // debugging assist
    
    void P();    // these are the only operations on a semaphore
    void V();    // they are both *atomic*
    
  private:
    char* name;        // useful for debugging
    int value;         // semaphore value, always >= 0
    List *queue;       // threads waiting in P() for the value to be > 0
};

class Lock {
  public:
    Lock();
    Lock(char* debugName);
    ~Lock();
    void Acquire();
    void Release();
    bool isHeldByCurrentThread();
  private:
    char* name;
    Thread *lockOwner;
    List *queue;
};

class Condition {
  public:
    Condition();
    Condition(char* debugName);
    ~Condition();
    void Wait(Lock *conditionLock);
    void Signal(Lock *conditionLock);
    void Broadcast(Lock *conditionLock);
    char* getName() { return name; }
  private:
    char* name;
    Lock *lock;
    List *queue;
};
#endif // SYNCH_H