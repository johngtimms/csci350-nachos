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

using namespace std;

struct ProcessTable {
	spaceId processCount;
	Lock tableLock;
	std::map<spaceId, Process*> processes;
	ProcessTable() {
		processCount = 0;
	}

};

Process *currentProcess;
ProcessTable processTable;

int copyin(unsigned int vaddr, int len, char *buf);
int copyout(unsigned int vaddr, int len, char *buf);

void InitExceptions() {
	cout << "INIT EXCEPTIONS CALLED" << endl;
}

void InitProcess(Process* process) {
	processTable.tableLock.Acquire();
	cout << "InitProcess() CALLED FOR PROCESS: " << process->name << endl;
	processTable.processes[process->processID] = process;
	currentProcess = process;
	processTable.tableLock.Release();
}

// Ensures that the forked thread begins execution at the correct position.
void ForkUserThread(int functionPtr) {
  DEBUG('t', "Setting machine PC to funcPtr for thread %s: 0x%x...\n", currentThread->getName(), functionPtr);
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();
  // update the stack register
  machine->WriteRegister(StackReg, currentThread->space->GetNumPages() * PageSize - 16);
  // Set the program counter to the appropriate place indicated by funcPtr...
  machine->WriteRegister(PCReg, functionPtr);
  machine->WriteRegister(NextPCReg, functionPtr + 4);
  machine->Run();
}

void Fork_Syscall(int functionPtr) {
	currentProcess->threadCount++;
	Thread *thread = new Thread(currentProcess->name);
	thread->space = currentThread->space;
	// if this fails then delete thread and exit function
	if(thread->space->CreateStack() == false)
	{
		delete thread;
    	DEBUG('p', "Create stack failed - not enough memory available.\n");
    	cout << "Fork failed" << endl;
    	return;
	}
	currentProcess->threads->Append(thread);
	thread->Fork(ForkUserThread, functionPtr);
}

bool Exit_Syscall(int status) {
	if(currentThread == currentProcess->processThread) {
		if(currentProcess->threadCount == 0) {
			//exit main thread
			interrupt->Halt();
			return true;
		} else {
			currentThread->Finish();
			return false;
		}
	} else {
		//exiting a forked thread
		currentProcess->threadCount--;
		currentThread->Finish();
		return false;
	}
	/*
	// TEMPLATE FOR EXIT SYSCALL
	processTable.tableLock->Acquire();
	if(processTable.size() == 1 && currentProcess->threadCount == 0) {
   		// CASE: The exiting thread is execThread of the last process running - exit Nachos
   		
   		// Maybe this instead of delete currentThread->space; ???
		// for(unsigned int i = 0; i < (currentThread->space->numPages); i++)
			// currentThread->space->clearPhysicalPage(x); 	
		
		delete currentThread->space; // Clear all the physical pages used in the AddrSpace of this process
        processTable.tableLock->Release();	
		DEBUG('x', "\n Removing last thread of Nachos\n");
		interrupt->Halt();
    } else if(processTable.size() > 1 && currentProcess->threadCount == 0) {
   		// CASE: The exiting thread is the process' execThread (last thread) - exit the process 
		processTable.processes->erase(currentThread->space->process);
		delete currentThread->space; // Clear all the physical pages used in the AddrSpace of this process
		currentThread->space = NULL;	           
        processTable.tableLock->Release();	
		DEBUG('x', "\n Removing last thread in a Process\n");
		currentThread->Finish();
    } else {
   		// CASE: The exiting thread is a thread that was forked in a process

		
		// THREE THINGS TO DO:
		// 1. CHANGE THREAD'S STACK'S PAGES' VALID BITS TO FALSE
				// But there's no way of knowning where a thread's stack begins in currentThread->space->pageTable
		// 2. CLEAR THREAD'S STACK'S PHYSICAL PAGES
				// But there's no way of knowning where a thread's stack begins in space->pageTable
		// currentThread->space->clearStack(currentThread->stackStart); ???

		// 3. DELETE THREAD FROM PROCESS->THREADS
				// currentProcess->threads is a List object with no "remove(Thread* threadToRemove)"

		currentProcess->threadCount--;
		processTable.tableLock->Release();	
		currentThread->Finish();	
	}
	*/
}

void ExecUserThread(spaceId processID) {
	currentThread->space->InitRegisters();	
	currentThread->space->RestoreState();	
	machine->Run();
}

spaceId Exec_Syscall(unsigned int vaddr, int len) {
	// Open file passed into Exec
	char *buf = new char[len+1];
    if(!buf) 
    	return -1;
    if(copyin(vaddr, len, buf) == -1) {
		cout << "Bad pointer passed to Create" << endl;
		delete buf;
		return -1;
    }
    buf[len]='\0';
	OpenFile *executable = fileSystem->Open(buf);
	if(executable == NULL) {
	   cout << "Unable to open file" << endl;
	   return -1;
    }
    // Create an AddrSpace for new file
    AddrSpace *space = new AddrSpace(executable);
    space->CreateStack(); 
    Thread *thread = new Thread(buf);
    // space->CreateStack(thread); // So that a thread can keep track of its8 pages of stack in AddrSpace->pageTable ??
    thread->space = space;
    InitExceptions();
    Process *process = new Process(buf);
    process->space = space;
    process->processThread = thread;
	delete executable;
    InitProcess(process);
	thread->Fork((VoidFunctionPtr)ExecUserThread, process->processID);
    return process->processID;
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

int CreateLock_Syscall() {
	KernelLock *lock = new KernelLock();
	lockTable->tableLock->Acquire();
	int key = lockTable->index;
	lockTable->locks[key] = lock;
	lockTable->index++;
	cout << "Created lock with key: " << key << endl;
	lockTable->tableLock->Release();
	return key;
}

void DestroyLock_Syscall(unsigned int key) {
	KernelLock *lock;
	lockTable->tableLock->Acquire();
	if(key >= 0 && key < lockTable->locks.size())
		lock = lockTable->locks[key];
	else
		cout << "Attempt to destroy lock that doesn't exist" << endl;
	if(lock != NULL && lock->space == currentThread->space) {
		lockTable->locks.erase(key);
		cout << "Destroyed lock with key: " << key << endl;
	}
	lockTable->tableLock->Release();
}

void Acquire_Syscall(unsigned int key) {
	KernelLock *lock;
	lockTable->tableLock->Acquire();
	if(key >= 0 && key < lockTable->locks.size())
		lock = lockTable->locks[key];
	else
		cout << "Attempt to acquire lock that doesn't exist" << endl;
	if(lock != NULL && lock->space == currentThread->space) {
		lock->lock->Acquire();
		cout << "Acquired lock with key: " << key << endl;
	}
	lockTable->tableLock->Release();
}

void Release_Syscall(unsigned int key) {
	KernelLock *lock;
	lockTable->tableLock->Acquire();
	if(key >= 0 && key < lockTable->locks.size())
		lock = lockTable->locks[key];
	else
		cout << "Attempt to release lock that doesn't exist" << endl;
	if(lock != NULL && lock->space == currentThread->space) {
		lock->lock->Release();
		cout << "Released lock with key: " << key << endl;
	}
	lockTable->tableLock->Release();
}

int CreateCondition_Syscall() {
	KernelCondition *condition = new KernelCondition();
	conditionTable->tableLock->Acquire();
	int key = conditionTable->index;
	conditionTable->conditions[key] = condition;
	conditionTable->index++;
	cout << "Created condition with key: " << key << endl;
	conditionTable->tableLock->Release();
	return key;
}

void DestroyCondition_Syscall(unsigned int key) {
	KernelCondition *condition;
	conditionTable->tableLock->Acquire();
	if(key >= 0 && key < conditionTable->conditions.size())
		condition = conditionTable->conditions[key];
	else
		cout << "Attempt to destroy condition that doesn't exist" << endl;
	if(condition != NULL && condition->space == currentThread->space) {
		conditionTable->conditions.erase(key);
		cout << "Destroyed condition with key: " << key << endl;
	}
	conditionTable->tableLock->Release();
}

void Wait_Syscall(unsigned int conditionKey, unsigned int lockKey) {
	KernelCondition *condition;
	KernelLock *lock;
	conditionTable->tableLock->Acquire();
	lockTable->tableLock->Acquire();
	if(conditionKey >= 0 && conditionKey < conditionTable->conditions.size())
		condition = conditionTable->conditions[conditionKey];
	else
		cout << "Attempt to wait by condition that doesn't exist" << endl;
	if(lockKey >= 0 && lockKey < lockTable->locks.size())
		lock = lockTable->locks[lockKey];
	else
		cout << "Condition trying to wait owns lock that doesn't exist" << endl;
	if(condition != NULL && lock != NULL && condition->space == currentThread->space && lock->space == currentThread->space) {
		condition->condition->Wait(lock->lock);
		cout << "Waiting on condition with key: " << conditionKey << endl;
	}
	lockTable->tableLock->Release();
	conditionTable->tableLock->Release();
}

void Signal_Syscall(unsigned int conditionKey, unsigned int lockKey) {
	KernelCondition *condition;
	KernelLock *lock;
	conditionTable->tableLock->Acquire();
	lockTable->tableLock->Acquire();
	if(conditionKey >= 0 && conditionKey < conditionTable->conditions.size())
		condition = conditionTable->conditions[conditionKey];
	else
		cout << "Attempt to signal by condition that doesn't exist" << endl;
	if(lockKey >= 0 && lockKey < lockTable->locks.size())
		lock = lockTable->locks[lockKey];
	else
		cout << "Condition trying to signal owns lock that doesn't exist" << endl;
	if(condition != NULL && lock != NULL && condition->space == currentThread->space && lock->space == currentThread->space) {
		condition->condition->Signal(lock->lock);
		cout << "Condition with key " << conditionKey <<  " signalling" << endl;
	}
	lockTable->tableLock->Release();
	conditionTable->tableLock->Release();
}

void Broadcast_Syscall(unsigned int conditionKey, unsigned int lockKey) {
	KernelCondition *condition;
	KernelLock *lock;
	conditionTable->tableLock->Acquire();
	lockTable->tableLock->Acquire();
	if(conditionKey >= 0 && conditionKey < conditionTable->conditions.size())
		condition = conditionTable->conditions[conditionKey];
	else
		cout << "Attempt to broadcast by condition that doesn't exist" << endl;
	if(lockKey >= 0 && lockKey < lockTable->locks.size())
		lock = lockTable->locks[lockKey];
	else
		cout << "Condition trying to broadcast owns lock that doesn't exist" << endl;
	if(condition != NULL && lock != NULL && condition->space == currentThread->space && lock->space == currentThread->space) {
		condition->condition->Broadcast(lock->lock);
		cout << "Condition with key " << conditionKey << " broadcasting" << endl;
	}
	lockTable->tableLock->Release();
	conditionTable->tableLock->Release();
}

void Print_Syscall(int text, int num) {
	char *buf = new char[100+1];
	if(!buf) 
  		DEBUG('a', "Unable to allocate buffer in Print_Syscall\n");
	if(copyin(text, 100, buf) == -1) {	
		DEBUG('a', "Unable to print in Print_Syscall\n");
		delete[] buf;
	}
	buf[100] = '\0';
	printf(buf, num);
}

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
				if(Exit_Syscall(machine->ReadRegister(4))) {
					delete currentProcess;
					return;
				}
				break;
			case SC_Exec:
				DEBUG('a', "Exec.\n");
				rv = Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(4));
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
				rv = CreateLock_Syscall();
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
				CreateCondition_Syscall();
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
		}
		// Put in the return value and increment the PC
		machine->WriteRegister(2, rv);
		machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
		return;
    } else {
      	cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      	interrupt->Halt();
    }
}

Process::Process(char *processName) {
	name = processName;
	processTable.tableLock.Acquire();
	processID = processTable.processCount;
	processTable.processCount++;
	processTable.tableLock.Release();
	processThread = currentThread;
	threads = new List();
	threadCount = 0;
}

Process::~Process() {
	delete threads;
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