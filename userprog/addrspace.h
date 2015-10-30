// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"

class Thread;

#define UserStackSize		1024 	// Increase this as necessary!
#define MaxOpenFiles        256
#define MaxChildSpaces      256

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space, initializing it with the program stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space
    void InitRegisters();		// Initialize user-level CPU registers, before jumping to user code
    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    unsigned int GetNumPages();
    void CreateStack(Thread* thread);
    void ClearStack(unsigned int stackStart);
    void ClearPhysicalPage(int i);
    void AddrSpace::handlePageFault(int vaddr);
    int AddrSpace::handleIPTMiss(int vpn);
    int AddrSpace::handleMemoryFull();
    Table fileTable;            // Table of openfiles
    int spaceID;
    unsigned int numThreads;
    Thread* processThread;
    //vector<Thread*> threads;
 private:
    TranslationEntry *pageTable;	// Assume linear page table translation for now!
    unsigned int numPages;		// Number of pages in the virtual address space
    OpenFile *executable;
    
};

#endif // ADDRSPACE_H
