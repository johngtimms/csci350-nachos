// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------
Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------
Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------
void
Semaphore::P() // try/wait
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    while(value == 0) { 			// semaphore not available
	   queue->Append((void *) currentThread);	// so go to sleep
	   currentThread->Sleep();
    } 
    value--; 					// semaphore available, consume its value
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------
void
Semaphore::V() //increment/signal
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    thread = (Thread *)queue->Remove();
    if(thread != NULL)	   // make thread ready, consuming the V immediately
	   scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

Lock::Lock(char* debugName) 
{
    name = debugName;
    lockOwner = NULL;
    queue = new List;
}

Lock::~Lock() 
{
    delete queue;
}

void 
Lock::Acquire()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if(lockOwner == NULL) {         // Lock is available
        lockOwner = currentThread;
        //printf(">Thread %s has acquired %s\n", currentThread->getName(), name);
    } else if (lockOwner != currentThread) {                    // Lock is busy
        queue->Append((void *)currentThread);       // add currentThread to wait queue
        currentThread->Sleep();                     // put currentThread to sleep
        //printf(">Thread %s is waiting to acquire %s (going to sleep)\n", currentThread->getName(), name);
    } else
        printf("Warning: %s has already acquired %s\n", currentThread->getName(), name);
    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

void 
Lock::Release() 
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if(currentThread == lockOwner) {
        //printf(">Thread %s is releasing Lock %s\n", currentThread->getName(), name);    
        if(queue->IsEmpty())     // No threads waiting
            lockOwner = NULL;
        else {
            lockOwner = (Thread*) queue->Remove();  // Wake up a waiting thread
            scheduler->ReadyToRun(lockOwner);
            //printf(">Thread %s has acquired %s\n", lockOwner->getName(), name);
        }
    } else {
        printf("Warning: %s hasn't acquired %s\n", currentThread->getName(), name);
    }
    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

bool
Lock::isHeldByCurrentThread() 
{
    return currentThread == lockOwner;
}

Condition::Condition(char* debugName) 
{
    name = debugName;
    lock = NULL;
    queue = new List;
}

Condition::~Condition() 
{ 
    delete queue;
}

void 
Condition::Wait(Lock* conditionLock) 
{   
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if(conditionLock != NULL) {
        if(lock == NULL)               // condition hasn't been assigned to a lock yet
            lock = conditionLock;
        if(lock == conditionLock) {    // OK to wait
            queue->Append((void*)currentThread);        // add currentThread to wait queue
            //printf(">Thread %s is waiting to be signalled by Condition %s\n", currentThread->getName(), name);
            lock->Release();                            // release waitingLock
            currentThread->Sleep();                     // put currentThread to sleep (wait for condition variable to signal)
            lock->Acquire();                            // reacquire waitingLock
        } else {
            printf("Warning: Incorrect Lock\n");
        }
    } else {
        printf("Warning: conditionLock pointer is NULL\n");
    }
    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

void 
Condition::Signal(Lock* conditionLock) 
{ 
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if(lock != NULL && conditionLock == lock) {
        thread = (Thread*) queue->Remove();     // remove one waiting thread from queue
        scheduler->ReadyToRun(thread);          // wake up waiting thread
        //printf(">Condition %s has signalled Thread %s\n", name, thread->getName());
    } else {
        printf("Warning: Condition signalling wrong lock\n");
    }
    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

void 
Condition::Broadcast(Lock* conditionLock) 
{ 
    if(lock != NULL && conditionLock == lock) {
        while(!queue->IsEmpty())            // signal all threads waiting 
            Signal(conditionLock);
    } else {
        printf("Warning: Condition signalling wrong lock\n");
    }
}