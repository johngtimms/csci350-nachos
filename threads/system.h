// system.h 
// All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"

#include "../vm/ipt.h"


// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization; called before anything else
extern void Cleanup();				// Cleanup, called when Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  	// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;				// performance metrics
extern Timer *timer;					// the hardware alarm clock
extern bool fifoEviction;

#ifdef USER_PROGRAM
#include "machine.h"
#include "structs.h"
#include "bitmap.h"
#include "openfile.h"
#include "list.h"
extern Machine *machine;	// user program memory and registers
extern LockTable *lockTable;
extern ConditionTable *conditionTable;
extern BitMap *memoryBitMap;	// BitMap representing physical memory
extern Lock *memoryBitMapLock;	// Lock for mmBitMap
extern Lock *forkLock;
extern ProcessTable *processTable;
extern Lock *processTableLock;
extern int currentTLB;
extern IPTEntry *ipt;
extern OpenFile *swapfile;
extern BitMap *swapfileBitMap;
extern List *fifo;
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk *synchDisk;
#endif

#ifdef NETWORK
    #include "post.h"
    #include "rpcserver.h"

    extern PostOffice* postOffice;
    extern RPCServer* rpcServer;
    extern int machineName;
    extern int destinationName;
    extern NetworkLockTable* networkLockTable;
    extern NetworkConditionTable* networkConditionTable;
    extern NetworkMVTable* networkMVTable;

    extern int threadIndex;
    extern Lock *threadIndexLock;
#endif

#endif // SYSTEM_H
