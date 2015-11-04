// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "synch.h"
#include <stdlib.h>

extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void*[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if(table) {
	   delete table;
	   table = 0;
    }
    if(lock) {
	   delete lock;
	   lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if there is none.
    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot
    lock->Acquire();
    i = map.Find();
    lock->Release();
    if(i != -1)
	   table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table, and return it.
    void *f = 0;
    if(i >= 0 && i < size) {
	   lock->Acquire();
	   if(map.Test(i)) {
	       map.Clear(i);
	       f = table[i];
	       table[i] = 0;
	   }
	lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------
static void 
SwapHeader (NoffHeader *noffH) {
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------
AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) {
    NoffHeader noffH;
	unsigned int vpn, size, initPages;
	// Don't allocate the input or output to disk files
	fileTable.Put(0);
	fileTable.Put(0);
	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
	 	SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size;

	//numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize, PageSize);
	numPages = divRoundUp(size, PageSize);

	size = numPages * PageSize;

	ASSERT(numPages <= NumPhysPages);
	DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
	// Set up pages for code, init data, unit data
	memoryBitMapLock->Acquire();
	pageTable = new TranslationEntry[numPages];
	int ppn = -1;
	for(vpn = 0; vpn < numPages; vpn++) {
		/*
		// Find a free physical page
		ppn = memoryBitMap->Find();		
		if(ppn == -1) {
		 	printf("Unable to find unused physical page\n");
		 	interrupt->Halt();
		}
		*/
		pageTable[vpn].virtualPage = vpn;
		//pageTable[vpn].physicalPage = ppn;
		pageTable[vpn].valid = FALSE;
		pageTable[vpn].use = FALSE;
		pageTable[vpn].dirty = FALSE;
		pageTable[vpn].readOnly = FALSE;
		/*
		// IPT population code
		ipt[ppn].virtualPage = vpn; 
		ipt[ppn].physicalPage = ppn;
		ipt[ppn].valid = TRUE;
		ipt[ppn].use = FALSE;
		ipt[ppn].dirty = FALSE;
		ipt[ppn].readOnly = FALSE;
		ipt[ppn].owner = this;
		*/
	}
	memoryBitMapLock->Release();
	/*
	for(vpn = 0; vpn < numPages; vpn++)
		bzero(&machine->mainMemory[PageSize * pageTable[vpn].physicalPage], PageSize);
	initPages = divRoundUp(noffH.code.size + noffH.initData.size, PageSize);
	for(vpn = 0; vpn < initPages; vpn++)
		executable->ReadAt(&(machine->mainMemory[pageTable[vpn].physicalPage * PageSize]), PageSize, 40 + (vpn * PageSize));
	*/
	numThreads = 0;
	this->executable = executable;
	codeInitPages = divRoundUp(noffH.code.size + noffH.initData.size, PageSize);
}

void AddrSpace::CreateStack(Thread* thread) {
	DEBUG('t', "In CreateStack()\n");
  	//if(numPages + 8 <= NumPhysPages) {
  		memoryBitMapLock->Acquire();
  		unsigned int vpn;
    	TranslationEntry *newPageTable = new TranslationEntry[numPages + 8];
        thread->stackStart = numPages;
        DEBUG('t', "thread = %s\n", thread->getName());
        DEBUG('t', "thread->stackStart = %d\n", thread->stackStart);
     	// Make deep copy of pageTable to newPageTable
    	for(vpn = 0; vpn < numPages; vpn++) {
      		newPageTable[vpn].virtualPage = pageTable[vpn].virtualPage;
	    	newPageTable[vpn].physicalPage = pageTable[vpn].physicalPage;
	    	newPageTable[vpn].valid = pageTable[vpn].valid;
	    	newPageTable[vpn].use = pageTable[vpn].use;
	    	newPageTable[vpn].dirty = pageTable[vpn].dirty;
	    	newPageTable[vpn].readOnly = pageTable[vpn].readOnly;
	    	// DEBUG('t', "newPageTable[%i].valid = pageTable[%i].valid = %i\n", vpn, vpn, pageTable[vpn].valid);
    	}
    	// Add 8 new pages of stack to newPageTable
    	int ppn = -1;
    	for(vpn = numPages; vpn < numPages + 8; vpn++) {
    		/*
			ppn = memoryBitMap->Find();
			if(ppn == -1) {
			 	printf("Unable to find unused physical page\n");
			 	interrupt->Halt();
			}
			*/
			newPageTable[vpn].virtualPage = vpn;
	     	//newPageTable[vpn].physicalPage = ppn;
	    	newPageTable[vpn].valid = FALSE;
	    	newPageTable[vpn].use = FALSE;
	    	newPageTable[vpn].dirty = FALSE;
	    	newPageTable[vpn].readOnly = FALSE;
	    	// DEBUG('t', "newPageTable[%i].valid = %i\n", vpn, newPageTable[vpn].valid);
	    	/*
	    	// IPT population code
	    	ipt[ppn].virtualPage = vpn; 
			ipt[ppn].physicalPage = ppn;
			ipt[ppn].valid = TRUE;
			ipt[ppn].use = FALSE;
			ipt[ppn].dirty = FALSE;
			ipt[ppn].readOnly = FALSE;
			ipt[ppn].owner = this;
			*/
    	}
    	delete [] pageTable;
    	pageTable = newPageTable;
    	numPages += 8;
    	RestoreState();
    	memoryBitMapLock->Release();
    	//for(int k = 0; k < numPages ; k ++)
            //DEBUG('t', "machine->pageTable[%i].valid = %i\n", k, machine->pageTable[k].valid);
  	//}
}

void AddrSpace::ClearStack(unsigned int stackStart) {
    memoryBitMapLock->Acquire();
    DEBUG('t', "In AddrSpace::ClearStack()\n");
	DEBUG('t', "stackStart: %i\n", stackStart);
	for(unsigned int i = stackStart; i < stackStart + 8; i++) {
		if(pageTable[i].valid == TRUE) {
			pageTable[i].valid = FALSE;
			memoryBitMap->Clear(pageTable[i].physicalPage);
		}
	}
	//RestoreState();
	memoryBitMapLock->Release();
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------
AddrSpace::~AddrSpace() {
	memoryBitMapLock->Acquire();
	for(unsigned int i = 0; i < numPages; i++)
		memoryBitMap->Clear(pageTable[i].physicalPage);
	memoryBitMapLock->Release();
    delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------
void
AddrSpace::InitRegisters() {
    int i;
    for(i = 0; i < NumTotalRegs; i++)
		machine->WriteRegister(i, 0);
    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	
    // Need to also tell MIPS where next instruction is, because of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);
   	// Set the stack register to the end of the address space, where we
   	// allocated the stack; but subtract off a bit, to make sure we don't
   	// accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------
void AddrSpace::SaveState() {
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------
void AddrSpace::RestoreState() {
	// Commented out for Project 3
    //machine->pageTable = pageTable;  
    machine->pageTableSize = numPages;

    for(int i = 0; i < TLBSize ; i++) {
    	if(machine->tlb[i].valid == TRUE && machine->tlb[i].dirty == TRUE) {
    		pageTable[machine->tlb[i].virtualPage].dirty = TRUE;
    		ipt[machine->tlb[i].physicalPage].dirty = TRUE;
    	}
    	machine->tlb[i].valid = FALSE;
    }
}

unsigned int AddrSpace::GetNumPages() {
    return numPages;
}

void AddrSpace::ClearPhysicalPage(int i) {
	memoryBitMapLock->Acquire();
	memoryBitMap->Clear(pageTable[i].physicalPage);
	memoryBitMapLock->Release();
}

void AddrSpace::handlePageFault(int vaddr) {
	int vpn = vaddr / PageSize;
	// Search IPT
	int ppn = -1;
	for(int i = 0 ; i < NumPhysPages; i++) {
		if(ipt[i].virtualPage == vpn && ipt[i].valid == TRUE && ipt[i].space == currentThread->space) {
			ppn = i;
			break;
		}
	}
	// Disable interrupts
	if(ppn = -1)
        ppn = handleIPTMiss(vpn);
    // Load page into TLB
	machine->tlb[currentTLB].virtualPage = ipt[ppn].virtualPage;
	machine->tlb[currentTLB].physicalPage = ppn;
	machine->tlb[currentTLB].valid = TRUE;
	machine->tlb[currentTLB].use = ipt[ppn].use;
	machine->tlb[currentTLB].dirty = ipt[ppn].dirty;
	machine->tlb[currentTLB].readOnly = ipt[ppn].readOnly;
	// Increment TLB index
	currentTLB = (currentTLB + 1) % TLBSize;
	// Enable interrupts
}

int AddrSpace::handleIPTMiss(int vpn) {
	// Allocate page of memory
	int ppn = memoryBitMap->Find();
	if(ppn == -1)
		ppn = handleMemoryFull();
	//bzero(&machine->mainMemory[ppn * PageSize], PageSize); ?
	// If dirty, load page from swapfile into memory
	//if(space->pageTable[vpn].dirty == TRUE)
		// Load from SwapFile
	// Otherwise, load page from executable into memory
	if(vpn < codeInitPages)
		executable->ReadAt(&(machine->mainMemory[ppn * PageSize]), PageSize, 40 + (vpn * PageSize));

	// Update IPT
	ipt[ppn].virtualPage = vpn; 
	ipt[ppn].physicalPage = ppn;
	ipt[ppn].valid = TRUE;
	ipt[ppn].use = FALSE;
	ipt[ppn].dirty = FALSE;
	ipt[ppn].readOnly = FALSE;
	ipt[ppn].space = this;
	// Update Page Table
	pageTable[vpn].physicalPage = ppn;
	pageTable[vpn].valid = TRUE;
	return ppn;
}

int AddrSpace::handleMemoryFull() {
	int ppn = -1;
	if(fifoEviction == TRUE) {

		// FIFO page eviction policy
	} else
		ppn = rand() % NumPhysPages;
		
	if(ipt[ppn].valid == FALSE)
		interrupt->Halt();

	// If page is dirty, write it to the SwapFile
	if(ipt[ppn].dirty == TRUE) {
		// Write to SwapFile
	}
	// Update pageTable in page's addrSpace (dirty and valid bits)
	// If page is in TLB
		ipt[ppn].valid = FALSE;
		// Invalidate the TLB entry
}
