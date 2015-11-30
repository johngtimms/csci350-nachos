// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include "process.h"

#ifdef NETWORK
#include "network.h"
#include "rpcserver.h"
#endif

using namespace std;

int copyin(unsigned int vaddr, int len, char *buf);
int copyout(unsigned int vaddr, int len, char *buf);
void Create_Syscall(unsigned int vaddr, int len);
int Open_Syscall(unsigned int vaddr, int len);
void Write_Syscall(unsigned int vaddr, int len, int id);
int Read_Syscall(unsigned int vaddr, int len, int id);
void Close_Syscall(int fd);

char* concatenate(char str[], int len, int num) {
    len = len - 1; /*sizeof() returns 1 more then actual size*/
    /*char numstr = (char)(((int) '0') + num);*/
    //printf("String passed in with length %i is %s\n",len,str);
    /*int len = sizeof(str); this is calculating length wrong, so instead we're passing in the length as paramter*/ 
    /*str[len] = numstr;
    /*str[len + 1] = '\0';*/

    /* hundreds place */
    str[len] = '0' + num / 100;
    /* tens place */
    str[len + 1] = '0' + (num % 100) / 10;
    /* one's place */
    str[len + 2] = '0' + num % 10;
    /* null terminator */
    str[len + 3] = '\0';
   // printf("Converted string is %s\n",str);
    return str;
}

// Ensures that the forked thread begins execution at the correct position.
void ForkUserThread(int functionPtr) {
    forkLock->Acquire();
    DEBUG('t', "In ForkUserThread\n");
    DEBUG('t', "Setting machine PC to funcPtr for thread %s: 0x%x...\n", currentThread->getName(), functionPtr);
    // Set the program counter to the appropriate place indicated by funcPtr...
    machine->WriteRegister(PCReg, functionPtr);
    machine->WriteRegister(NextPCReg, functionPtr + 4);
    currentThread->space->RestoreState();
    // update the stack register
    machine->WriteRegister(StackReg, currentThread->space->GetNumPages() * PageSize - 16);
    forkLock->Release();
    machine->Run();
}

void Fork_Syscall(int functionPtr) {
	DEBUG('t', "In Fork_Syscall\n");

	Thread *thread = new Thread("forked thread");

	// Give the thread an ID (needed for networking)
	threadIndexLock->Acquire();
	thread->setID(++threadIndex);
	threadIndexLock->Release();

	// Finish creatng thread
	thread->space = currentThread->space;
  	thread->space->CreateStack(thread);
  	thread->space->numThreads++;
	thread->Fork((VoidFunctionPtr)ForkUserThread, functionPtr);
	currentThread->Yield();
}

void Exit_Syscall(int status) {
    processTableLock->Acquire();
    DEBUG('t', "In Exit_Syscall\n");
    DEBUG('t', "currentThread = %s\n", currentThread->getName());
    DEBUG('t', "processTable->numProcesses = %i\n", processTable->numProcesses);
    DEBUG('t', "currentThread->space->numThreads = %i\n", currentThread->space->numThreads);
    // CASE 1: The exiting thread is execThread of the last process running - exit Nachos
    if(processTable->numProcesses == 1 && currentThread->space->numThreads == 1) {
        // Clear all the physical pages used in the AddrSpace of this process
        // for(unsigned int i = 0; i < currentThread->space->GetNumPages(); i++)
        // currentThread->space->ClearPhysicalPage(i);
        delete currentThread->space;
        currentThread->space = NULL;
        processTableLock->Release();
        // delete kernelLockTable & kernelCVTable
        DEBUG('t', "Exit_Syscall Case 1 (Last thread in nachos called Exit)\n");
        //printf("machine->ReadRegister(4) =  %d\n", machine->ReadRegister(4));
        printf("Terminating Nachos\n");
        interrupt->Halt();
    }
    // CASE 2: The exiting thread is a process' last running thread - exit the process
    else if(processTable->numProcesses > 1 && currentThread->space->numThreads == 1) {
        processTable->processes.erase(currentThread->space->spaceID);
        processTable->numProcesses--;
        for(unsigned int i = 0; i < NumPhysPages; i++)
            if(ipt[i].space == currentThread->space)
                ipt[i].valid = false;
        // Delete KernelLocks & KernelCVs belonging to this space
        // Clear all the physical pages used in the AddrSpace of this process
        delete currentThread->space;
        currentThread->space = NULL;
        processTableLock->Release();
        DEBUG('t', "Exit_Syscall Case 2 (Last thread belonging to process called Exit)\n");
        currentThread->Finish();
    }
    // CASE 3: The exiting thread is a thread that was forked in a process
    else if(processTable->numProcesses >= 1 && currentThread->space->numThreads > 1) {
        currentThread->space->ClearStack(currentThread->stackStart);
        currentThread->space->numThreads--;
        processTableLock->Release();
        DEBUG('t', "Exit_Syscall Case 3 (Forked thread called Exit)\n");
        currentThread->Finish();
    } else
        DEBUG('t', "IN ELSE\n");
}

void ExecUserThread(int processID) {
    DEBUG('t', "In ExecUserSyscall\n");
    // Initialize registers for new file
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
}

int Exec_Syscall(unsigned int vaddr, int len) {
    processTableLock->Acquire();
    DEBUG('t', "In Exec_Syscall\n");
    if(len <= 0)
        printf("len <= 0 in Exec_Syscall\n");
    char *buf = new char[len + 1];
    if(copyin(vaddr, len, buf) == -1) {
        printf("copyin() failed in Exec_Syscall\n");
        delete buf;
        return -1;
    }
    buf[len] = '\0';
    OpenFile *executable = fileSystem->Open(buf);
    if(executable == NULL) {
        cout << "Unable to open file: " << buf << endl;
        return -1;
    }
    // Create an AddrSpace (Process) for new file
    AddrSpace *space = new AddrSpace(executable);
    // Create "main" Thread of new process
    Thread *thread = new Thread("0"); // Name the first thread in the new process 0 (needed for networking)
    
    threadIndexLock->Acquire();
    thread->setID(++threadIndex);
    threadIndexLock->Release();
    
    // New process bookkeeping
    thread->space = space;
    thread->space->CreateStack(thread);
    space->processThread = thread;
    space->numThreads++;
    space->spaceID = processTable->processID;
    //space.threads.push_back(thread);
    // Update Process Table
    processTable->processID++;
    processTable->processes[space->spaceID] = space;
    processTable->numProcesses++;
    //delete executable; // Commented out for project 3
    processTableLock->Release();
    // Fork new process
    thread->Fork((VoidFunctionPtr)ExecUserThread, space->spaceID);
    currentThread->Yield();
    return space->spaceID;
}

#ifndef NETWORK

int CreateLock_Syscall() {
    KernelLock *lock = new KernelLock();
    lock->space = currentThread->space;
    lockTable->tableLock->Acquire();
    int key = lockTable->index;
    lockTable->locks[key] = lock;
    lockTable->index++;
    DEBUG('l', "Created lock with key: %i\n",key);
    lockTable->tableLock->Release();
    return key;
}

void DestroyLock_Syscall(unsigned int key) {
    KernelLock *lock;
    lockTable->tableLock->Acquire();
    if(lockTable->locks.find(key) != lockTable->locks.end())
        lock = lockTable->locks[key];
    else
        DEBUG('l', "Attempt to destroy lock that doesn't exist\n");
    if(lock != NULL && lock->space == currentThread->space) {
        lockTable->locks.erase(key);
        DEBUG('l', "Destroyed lock with key: %i\n", key);
    }
    lockTable->tableLock->Release();
}

void Acquire_Syscall(unsigned int key) {
    KernelLock *lock;
    lockTable->tableLock->Acquire();
    if(lockTable->locks.find(key) != lockTable->locks.end())
        lock = lockTable->locks[key];
    else
        DEBUG('l', "Attempt to acquire lock that doesn't exist\n");
    if(lock != NULL && lock->space == currentThread->space) {
        lock->lock->Acquire();
        DEBUG('l', "Acquired lock with key: %i\n",key);
    }
    lockTable->tableLock->Release();
}

void Release_Syscall(unsigned int key) {
    KernelLock *lock;
    lockTable->tableLock->Acquire();
    if(lockTable->locks.find(key) != lockTable->locks.end())
        lock = lockTable->locks[key];
    else
        DEBUG('l', "Attempt to release lock that doesn't exist\n");
    if(lock != NULL && lock->space == currentThread->space) {
        lock->lock->Release();
        DEBUG('l', "Released lock with key: %i\n",key);
    }
    lockTable->tableLock->Release();
}

int CreateCondition_Syscall() {
    KernelCondition *condition = new KernelCondition();
    condition->space = currentThread->space;
    conditionTable->tableLock->Acquire();
    int key = conditionTable->index;
    conditionTable->conditions[key] = condition;
    conditionTable->index++;
    DEBUG('l', "Created condition with key: %i\n", key);
    conditionTable->tableLock->Release();
    return key;
}

void DestroyCondition_Syscall(unsigned int key) {
    KernelCondition *condition;
    conditionTable->tableLock->Acquire();
    if(conditionTable->conditions.find(key) != conditionTable->conditions.end())
        condition = conditionTable->conditions[key];
    else
        DEBUG('l', "Attempt to destroy condition that doesn't exist\n");
    if(condition != NULL && condition->space == currentThread->space) {
        conditionTable->conditions.erase(key);
        DEBUG('l', "Destroyed condition with key: %i\n", key);
    }
    conditionTable->tableLock->Release();
}

void Wait_Syscall(unsigned int conditionKey, unsigned int lockKey) {
    KernelCondition *condition;
    KernelLock *lock;
    conditionTable->tableLock->Acquire();
    lockTable->tableLock->Acquire();
    if(conditionTable->conditions.find(conditionKey) != conditionTable->conditions.end())
        condition = conditionTable->conditions[conditionKey];
    else
        DEBUG('l', "Attempt to wait by condition that doesn't exist\n");
    if(lockKey >= 0 && lockKey < lockTable->locks.size())
        lock = lockTable->locks[lockKey];
    else
        DEBUG('l', "Condition trying to wait owns lock that doesn't exist\n");
    if(condition != NULL && lock != NULL && condition->space == currentThread->space && lock->space == currentThread->space) {
        lockTable->tableLock->Release();
        conditionTable->tableLock->Release();
        DEBUG('l', "Waiting on condition with key: %i, and lock key: %i\n", conditionKey,lockKey);
        condition->condition->Wait(lock->lock);
    }
}

void Signal_Syscall(unsigned int conditionKey, unsigned int lockKey) {
    KernelCondition *condition;
    KernelLock *lock;
    conditionTable->tableLock->Acquire();
    lockTable->tableLock->Acquire();
    if(conditionTable->conditions.find(conditionKey) != conditionTable->conditions.end())
        condition = conditionTable->conditions[conditionKey];
    else
        DEBUG('l', "Attempt to signal by condition that doesn't exist\n");
    if(lockKey >= 0 && lockKey < lockTable->locks.size())
        lock = lockTable->locks[lockKey];
    else
        DEBUG('l', "Condition trying to signal owns lock that doesn't exist\n");
    if(condition != NULL && lock != NULL && condition->space == currentThread->space && lock->space == currentThread->space) {
        condition->condition->Signal(lock->lock);
        DEBUG('l', "Condition with key %i signalling\n", conditionKey);
    }
    lockTable->tableLock->Release();
    conditionTable->tableLock->Release();
}

void Broadcast_Syscall(unsigned int conditionKey, unsigned int lockKey) {
    KernelCondition *condition;
    KernelLock *lock;
    conditionTable->tableLock->Acquire();
    lockTable->tableLock->Acquire();
    if(conditionTable->conditions.find(conditionKey) != conditionTable->conditions.end())
        condition = conditionTable->conditions[conditionKey];
    else
        DEBUG('l', "Attempt to broadcast by condition that doesn't exist\n");
    if(lockKey >= 0 && lockKey < lockTable->locks.size())
        lock = lockTable->locks[lockKey];
    else
        DEBUG('l', "Condition trying to broadcast owns lock that doesn't exist\n");
    if(condition != NULL && lock != NULL && condition->space == currentThread->space && lock->space == currentThread->space) {
        condition->condition->Broadcast(lock->lock);
        DEBUG('l', "Condition with key %i broadcasting\n", conditionKey);
    }
    lockTable->tableLock->Release();
    conditionTable->tableLock->Release();
}

#endif

void Print_Syscall(int text, int num) {
    DEBUG('t', "In Print_Syscall\n");
    char *buf = new char[100 + 1];
    if(!buf)
        DEBUG('t', "Unable to allocate buffer in Print_Syscall\n");
    if(copyin(text, 100, buf) == -1) {
        DEBUG('t', "Unable to print in Print_Syscall\n");
        delete[] buf;
    }
    buf[100] = '\0';
    printf(buf, num);
}

int Rand_Syscall() {
    return rand();
}

#ifdef NETWORK

// Sends a request with the process's ID, gets a lock ID back
int CreateLock_Syscall(unsigned int vaddr, int len, int index) {
    char *name = new char[len+1];
    if( copyin(vaddr,len,name) == -1 ) {
        printf("%s","Bad pointer passed to CreateMV\n");
        delete name;
        return -1;
    }
    name[len] = '\0';
    concatenate(name,len,index);
    
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];
    
    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    int mailbox = RPCServer::ClientMailbox(machineName, processID, threadID);
    
    sprintf(send, "%d,%d,%s", processID, threadID, name);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxCreateLock;
    outMailHdr.from = mailbox; // need a reply, send my mailbox
    outMailHdr.length = strlen(send) + 1;
    DEBUG('l', "outMailHr.to: %i, mailbox: %i\n", MailboxCreateLock, mailbox);
    // Send the request message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: CreateLock failed. Server misconfigured.\n");
    
    // Get the response back
    postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);
    int key = atoi(recv);
    DEBUG('z', "CreateLock process %d thread %d key: %i\n", processID, threadID, key);
    return key;
}

void DestroyLock_Syscall(unsigned int key) {
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    char send[MaxMailSize];
    
    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    DEBUG('z', "DestroyLock process %d thread %d\n", processID, threadID);
    sprintf(send, "%d,%d,%d", processID, threadID, key);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxDestroyLock;
    outMailHdr.from = 0; // no reply needed
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: DestroyLock failed. Server misconfigured.\n");
}

void Acquire_Syscall(unsigned int key) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];
    
    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    int mailbox = RPCServer::ClientMailbox(machineName, processID, threadID);

    DEBUG('z', "Acquire - mailbox %d key %i\n", mailbox, key);
    sprintf(send, "%d,%d,%d", processID, threadID, key);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxAcquire;
    outMailHdr.from = mailbox; // need a reply
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: Acquire failed. Server misconfigured.\n");

    // Get the response back
    postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);

    DEBUG('z', "Acquire SUCCESS - process %d thread %d key %d\n", processID, threadID, key);
    
    char test[MaxMailSize];
    strcpy(test, "OK");
    if ( strcmp(test,recv) )
        printf("WARN: Acquire failed. Recieved bad server message.\n");
}

void Release_Syscall(unsigned int key) {
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    char send[MaxMailSize];
    
    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    int mailbox = RPCServer::ClientMailbox(machineName, processID, threadID);
    DEBUG('z', "Release - process %d thread %d key %d\n", processID, threadID, key);
    sprintf(send, "%d,%d,%d", processID, threadID, key);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxRelease;
    outMailHdr.from = 0; // no reply needed
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: Release failed. Server misconfigured.\n");
}

int CreateCondition_Syscall(unsigned int vaddr, int len, int index) {
    char *name = new char[len+1];
    if( copyin(vaddr,len,name) == -1 ) {
        printf("%s","Bad pointer passed to CreateMV\n");
        delete name;
        return -1;
    }
    name[len] = '\0';
    concatenate(name,len,index);
    
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];
    
    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    int mailbox = RPCServer::ClientMailbox(machineName, processID, threadID);
    DEBUG('z', "CreateCondition - process %d thread %d\n", processID, threadID);
    sprintf(send, "%d,%d,%s", processID, threadID, name);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxCreateCondition;
    outMailHdr.from = mailbox; // need a reply
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: CreateCondition failed. Server misconfigured.\n");

    // Get the response back
    postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);
    int key = atoi(recv);
    return key;
}

void DestroyCondition_Syscall(unsigned int key) {
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    char send[MaxMailSize];
    
    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    DEBUG('z', "DestroyCondition - process %d thread %d key %d\n", processID, threadID, key);
    sprintf(send, "%d,%d,%d", processID, threadID, key);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxDestroyCondition;
    outMailHdr.from = 0; // no reply needed
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: DestroyCondition failed. Server misconfigured.\n");
}

void Wait_Syscall(unsigned int conditionKey, unsigned int lockKey) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];
    char test[MaxMailSize]; strcpy(test, "OK");
    bool success;
    
    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    int mailbox = RPCServer::ClientMailbox(machineName, processID, threadID);
    DEBUG('z', "Wait - process %d thread %d condition %d lock %d\n", processID, threadID, conditionKey, lockKey);
    sprintf(send, "%d,%d,%d,%d", processID, threadID, conditionKey, lockKey);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxWait;
    outMailHdr.from = mailbox; // need a reply
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: Wait failed. Server misconfigured.\n");

    // Get the response back
    postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);
    
    if ( strcmp(test,recv) )
        printf("WARN: Wait failed. Recieved bad server message.\n");
    
    // After getting the CV, we need to acquire the lock
    DEBUG('z', "Acquire (after Wait) - process %d thread %d condition %d lock %d\n", processID, threadID, conditionKey, lockKey);
    sprintf(send, "%d,%d,%d", processID, threadID, lockKey);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxAcquire;
    outMailHdr.from = mailbox; // need a reply
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: Acquire (after Wait) failed. Server misconfigured.\n");

    // Get the response back
    postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);

	DEBUG('z', "Acquire (after Wait) SUCCESS - process %d thread %d condition %d lock %d\n", processID, threadID, conditionKey, lockKey);
    
    if ( strcmp(test,recv) )
        printf("WARN: Acquire (after Wait) failed. Recieved bad server message.\n");
}

void Signal_Syscall(unsigned int conditionKey, unsigned int lockKey) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];
    char test[MaxMailSize]; strcpy(test, "OK");
    bool success;
    
    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    int mailbox = RPCServer::ClientMailbox(machineName, processID, threadID);
    DEBUG('z', "Signal - process %d thread %d condition %d lock %d\n", processID, threadID, conditionKey, lockKey);
    sprintf(send, "%d,%d,%d,%d", processID, threadID, conditionKey, lockKey);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxSignal;
    outMailHdr.from = mailbox; // need a reply
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: Signal failed. Server misconfigured.\n");

    // Get the response back
    postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);
    
    if ( strcmp(test,recv) )
        printf("WARN: Signal failed. Recieved bad server message.\n");
     
    /* Peppy - i don't think signal should be releasing the lock, so im commenting this next part
                I could be wrong though...
                
    // After signaling, need to release the lock so the waiting thread can acquire
    DEBUG('z', "Release (after Signal) process %d thread %d\n", processID, threadID);
    sprintf(send, "%d,%d,%d", processID, threadID, lockKey);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxRelease;
    outMailHdr.from = 0; // no reply needed
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: Release (after Signal) failed. Server misconfigured.\n");
    */
}

void Broadcast_Syscall(unsigned int conditionKey, unsigned int lockKey) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];
    char test[MaxMailSize]; strcpy(test, "OK");
    bool success;
    
    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    int mailbox = RPCServer::ClientMailbox(machineName, processID, threadID);
    DEBUG('z', "Broadcast - process %d thread %d condition %d lock %d\n", processID, threadID, conditionKey, lockKey);
    sprintf(send, "%d,%d,%d,%d", processID, threadID, conditionKey, lockKey);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxBroadcast;
    outMailHdr.from = mailbox; // Need a reply
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
        printf("WARN: Broadcast failed. Server misconfigured.\n");

    // Get the response back
    postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);
    
    if ( strcmp(test,recv) )
        printf("WARN: Broadcast failed. Recieved bad server message.\n");
    
    // After broadcasting, need to release the lock so the waiting thread can acquire
    DEBUG('z', "Release (after Broadcast) process %d thread %d\n", processID, threadID);
    sprintf(send, "%d,%d,%d", processID, threadID, lockKey);
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxRelease;
    outMailHdr.from = 0; // no reply needed
    outMailHdr.length = strlen(send) + 1;
    
    // Send the request message
    success = postOffice->Send(outPktHdr, outMailHdr, send);
    
    if ( !success )
    	printf("WARN: Release (after Broadcast) failed. Server misconfigured.\n");
}

void NetPrint_Syscall(int text, int num) {
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    
    char *buf = new char[100 + 1];
    copyin(text, 100, buf);
    buf[100] = '\0';
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxNetPrint;
    outMailHdr.from = MailboxNetPrint;
    outMailHdr.length = strlen(buf) + 1;
    
    // Make sure the message is not too long
    // Example of a 39 byte message, the max that can be sent: "Lorem ipsum dolor sit amet turpis duis."
    if (outMailHdr.length > 40) {
        printf("ERROR: NetPrint failed. Can't send %d bytes. Terminating Nachos.\n", outMailHdr.length);
        interrupt->Halt();
    }
    
    // Send the message
    bool success = postOffice->Send(outPktHdr, outMailHdr, buf);
    
    if ( !success ) {
        printf("ERROR: NetPrint failed. Server misconfigured. Terminating Nachos.\n");
        interrupt->Halt();
    }
    
    delete[] buf;
}

void NetHalt_Syscall() {
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    
    char *buf = "NoOp"; // No need to send a real message
    
    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;
    outMailHdr.to = MailboxNetHalt;
    outMailHdr.from = MailboxNetHalt;
    outMailHdr.length = strlen(buf) + 1;
    
    // Send the message
    bool success = postOffice->Send(outPktHdr, outMailHdr, buf);
    
    if ( !success ) {
        printf("ERROR: NetHalt failed. Server misconfigured. Terminating Nachos.\n");
        interrupt->Halt();
    }
    
    delete[] buf;
}

int CreateMV_Syscall(unsigned int vaddr, int len, int index) {
    char *name = new char[len+1];
    if( copyin(vaddr,len,name) == -1 ) {
        printf("%s","Bad pointer passed to CreateMV\n");
        delete name;
        return -1;
    }
    name[len] = '\0';
    concatenate(name,len,index);

    
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];

    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    int mailbox = RPCServer::ClientMailbox(machineName, processID, threadID);
    DEBUG('z', "CreateMV process %d thread %d\n", processID, threadID);
    sprintf(send, "%d,%d,%s", processID, threadID, name);

    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;     
    outMailHdr.to = MailboxCreateMV;
    outMailHdr.from = mailbox; // need a reply, send my mailbox
    outMailHdr.length = strlen(send) + 1;

    // Send the request message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

    if ( !success )
      printf("WARN: CreateMV failed. Server misconfigured.\n");

    // Get the response back
    postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);
    int key = atoi(recv);
    DEBUG('z', "CreateMV successful with key: %i\n", key);
    return key;
}

void DestroyMV_Syscall(int key) {
	PacketHeader outPktHdr;
    MailHeader outMailHdr;
    char send[MaxMailSize];

    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    DEBUG('z', "DestroyMV - process %d thread %d key %d\n", processID, threadID, key);
    sprintf(send, "%d,%d,%d", processID, threadID, key);

    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;     
    outMailHdr.to = MailboxDestroyMV;
    outMailHdr.from = 0; // no reply needed
    outMailHdr.length = strlen(send) + 1;

    // Send the request message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

    if ( !success )
      printf("WARN: DestroyMV failed. Server misconfigured.\n");
}

int GetMV_Syscall(int key) {
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];

    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    int mailbox = RPCServer::ClientMailbox(machineName, processID, threadID);
    DEBUG('z', "GetMV - process %d thread %d key %d\n", processID, threadID, key);
    sprintf(send, "%d,%d,%d", processID, threadID, key);

    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;     
    outMailHdr.to = MailboxGetMV;
    outMailHdr.from = mailbox; 
    outMailHdr.length = strlen(send) + 1;

    // Send the request message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

    if ( !success )
      printf("WARN: GetMV failed. Server misconfigured.\n");

  	// Get the response back
    postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);
    int value = atoi(recv);
    return value;
}

void SetMV_Syscall(int key, int value) {
	PacketHeader outPktHdr;
    MailHeader outMailHdr;
    char send[MaxMailSize];

    // Form the request message
    int processID = currentThread->space->spaceID;
    int threadID = currentThread->getID();
    DEBUG('z', "SetMV - process %d thread %d key %d value %d\n", processID, threadID, key, value);
    sprintf(send, "%d,%d,%d,%d", processID, threadID, key, value);

    // Construct packet header, mail header for the message
    outPktHdr.to = destinationName;     
    outMailHdr.to = MailboxSetMV;
    outMailHdr.from = 0; // no reply needed
    outMailHdr.length = strlen(send) + 1;

    // Send the request message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

    if ( !success )
      printf("WARN: SetMV failed. Server misconfigured.\n");
}

#endif

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv = 0; 	// the return value from a syscall
    if(which == SyscallException) {
        switch (type) {
            default:
                DEBUG('a', "Unknown syscall - shutting down.\n");
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_Exit:
                DEBUG('a', "Exiting.\n");
                Exit_Syscall(machine->ReadRegister(4));
                break;
            case SC_Exec:
                DEBUG('a', "Exec.\n");
                rv = Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Join:
                DEBUG('a', "Join.\n");
                //Join_Syscall();
                break;
            case SC_Create:
                DEBUG('a', "Create syscall.\n");
                Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Open:
                DEBUG('a', "Open syscall.\n");
                rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Write:
                DEBUG('a', "Write syscall.\n");
                Write_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
                break;
            case SC_Read:
                DEBUG('a', "Read syscall.\n");
                rv = Read_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
                break;
            case SC_Close:
                DEBUG('a', "Close syscall.\n");
                Close_Syscall(machine->ReadRegister(4));
                break;
            case SC_Fork:
                DEBUG('a', "Fork.\n");
                Fork_Syscall(machine->ReadRegister(4));
                break;
            case SC_Yield:
                DEBUG('a', "Yield.\n");
                currentThread->Yield();
                break;
            case SC_CreateLock:
                DEBUG('a', "CreateLock syscall.\n");
                rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
                break;
            case SC_DestroyLock:
                DEBUG('a', "DestroyLock syscall.\n");
                DestroyLock_Syscall(machine->ReadRegister(4));
                break;
            case SC_Acquire:
                DEBUG('a', "Acquire syscall.\n");
                Acquire_Syscall(machine->ReadRegister(4));
                break;
            case SC_Release:
                DEBUG('a', "Release syscall.\n");
                Release_Syscall(machine->ReadRegister(4));
                break;
            case SC_CreateCondition:
                DEBUG('a', "CreateCondition syscall.\n");
                rv = CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
                break;
            case SC_DestroyCondition:
                DEBUG('a', "DestroyCondition syscall.\n");
                DestroyCondition_Syscall(machine->ReadRegister(4));
                break;
            case SC_Wait:
                DEBUG('a', "Wait syscall.\n");
                Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Signal:
                DEBUG('a', "Signal syscall.\n");
                Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Broadcast:
                DEBUG('a', "Broadcast syscall.\n");
                Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Print:
                Print_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Rand:
                DEBUG('a', "Random number syscall.\n");
                rv = Rand_Syscall();
                break;
            #ifdef NETWORK
    			case SC_NetPrint:
    				NetPrint_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    				break;
                case SC_NetHalt:
                    NetHalt_Syscall();
                    break;
                case SC_CreateMV:
                	rv = CreateMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
                	break;
                case SC_DestroyMV:
                	DestroyMV_Syscall(machine->ReadRegister(4));
                	break;
                case SC_GetMV:
                	rv = GetMV_Syscall(machine->ReadRegister(4));
                	break;
                case SC_SetMV:
                	SetMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                	break;
            #endif
		}
		// Put in the return value and increment the PC
		machine->WriteRegister(2, rv);
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 4);
		return;
    } else if(which == PageFaultException) {
        rv = machine->ReadRegister(BadVAddrReg);
        currentThread->space->handlePageFault(rv);
        //printf("rv: %d\n", rv);
    } else {
        cout << "Unexpected user mode exception - which:" << which << "  type:" << type << endl;
        interrupt->Halt();
    }
}

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n = 0;			// The number of bytes copied in
    int *paddr = new int;
    
    while ( n >= 0 && n < len) {
        result = machine->ReadMem( vaddr, 1, paddr );
        while(!result) // FALL 09 CHANGES
        {
            result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
        }
        buf[n++] = *paddr;
        if ( !result ) {
            //translation failed
            return -1;
        }
        vaddr++;
    }
    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n = 0;			// The number of bytes copied in
    while ( n >= 0 && n < len) {
        // Note that we check every byte's address
        result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );
        
        if ( !result ) {
            //translation failed
            return -1;
        }
        vaddr++;
    }
    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    
    if (!buf)
        return;
    if( copyin(vaddr,len,buf) == -1 ) {
        printf("%s","Bad pointer passed to Create\n");
        delete buf;
        return;
    }
    buf[len]='\0';
    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id
    
    if(!buf) {
        printf("%s","Can't allocate kernel buffer in Open\n");
        return -1;
    }
    if(copyin(vaddr, len, buf) == -1) {
        printf("%s","Bad pointer passed to Open\n");
        delete[] buf;
        return -1;
    }
    
    buf[len]='\0';
    
    f = fileSystem->Open(buf);
    delete[] buf;
    
    if(f) {
        if((id = currentThread->space->fileTable.Put(f)) == -1)
            delete f;
        return id;
    }
    else
        return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output
    
    if(id == ConsoleInput) 
        return;
    if(!(buf = new char[len])) {
        printf("%s","Error allocating kernel buffer for write!\n");
        return;
    } else {
        if (copyin(vaddr,len,buf) == -1) {
            printf("%s","Bad pointer passed to to write: data not written\n");
            delete[] buf;
            return;
        }
    }
    if(id == ConsoleOutput) {
        for (int ii=0; ii<len; ii++) {
            printf("%c",buf[ii]);
        }
        
    } else {
        if ((f = (OpenFile *) currentThread->space->fileTable.Get(id))) {
            f->Write(buf, len);
        } else {
            printf("%s","Bad OpenFileId passed to Write\n");
            len = -1;
        }
    }
    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output
    
    if(id == ConsoleOutput) 
        return -1;
    if(!(buf = new char[len])) {
        printf("%s","Error allocating kernel buffer in Read\n");
        return -1;
    }
    if(id == ConsoleInput) {
        //Reading from the keyboard
        scanf("%s", buf);
        if(copyout(vaddr, len, buf) == -1 ) {
            printf("%s","Bad pointer passed to Read: data not copied\n");
        }
    } else {
        if((f = (OpenFile *) currentThread->space->fileTable.Get(id))) {
            len = f->Read(buf, len);
            if(len > 0) {
                //Read something from the file. Put into user's address space
                if(copyout(vaddr, len, buf) == -1) {
                    printf("%s","Bad pointer passed to Read: data not copied\n");
                }
            }
        } else {
            printf("%s","Bad OpenFileId passed to Read\n");
            len = -1;
        }
    }
    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);
    if(f) {
        delete f;
    } else {
        printf("%s","Tried to close an unopen file\n");
    }
}
