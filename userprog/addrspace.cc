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
	unsigned int i, size, initPages;
	// Don't allocate the input or output to disk files
	fileTable.Put(0);
	fileTable.Put(0);
	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
	 	SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
	//numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize, PageSize);
	numPages = divRoundUp(size, PageSize);	// Pages for code, init data and unit data only
	size = numPages * PageSize;
	ASSERT(numPages <= NumPhysPages);
	DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
	// Set up pages for code, init data, unit data and execThread
	mmBitMapLock->Acquire();
	pageTable = new TranslationEntry[numPages + 8]; // Add 8 pages for execThread stack
	int ppn;
	for(i = 0; i < numPages; i++) {
		pageTable[i].virtualPage = i;
		ppn = mmBitMap->Find();		// Find a free physical page
		if(ppn == -1) {
			printf("Unable to find unused physical page\n");
			interrupt->Halt();
		}
		pageTable[i].physicalPage = ppn;
		pageTable[i].valid = TRUE;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;  // if the code segment was entirely on a separate page, we could set its pages to be read-only
	}
	mmBitMapLock->Release();
	mmLock->Acquire();
	// Zero out address space
	for(i = 0; i < (numPages); i++)
		bzero(&machine->mainMemory[PageSize * pageTable[i].physicalPage], PageSize);
	// Number of pages to copy to main memory
	initPages = divRoundUp(noffH.code.size + noffH.initData.size, PageSize);
	for(i = 0; i < initPages; i++)
		executable->ReadAt(&(machine->mainMemory[pageTable[i].physicalPage * PageSize]), PageSize, 40 + (i * PageSize));
	mmLock->Release();
}


//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------
AddrSpace::~AddrSpace() {
	mmBitMapLock->Acquire();
	for(unsigned int i = 0; i < numPages; i++)
		mmBitMap->Clear(pageTable[i].physicalPage);
	mmBitMapLock->Release();
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
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

unsigned int AddrSpace::GetNumPages() {
    return numPages;
}

bool AddrSpace::CreateStack(Thread* thread) {
  	if(numPages + 8 <= NumPhysPages) {
  		mmBitMapLock->Acquire();
  		unsigned int i;
    	TranslationEntry *newPageTable = new TranslationEntry[numPages + 8];
    	
        thread->stackStart = numPages; 
    	// So that AddrSpace::clearStack(int stackStart) can be called in Exit_Syscall() ?
    	
     	// Make deep copy of pageTable to newPageTable
    	for(i = 0; i < numPages; i++) {
    		DEBUG('t',"Old virtual page: %i\n",pageTable[i].virtualPage);
      		newPageTable[i].virtualPage = pageTable[i].virtualPage;
	    	newPageTable[i].physicalPage = pageTable[i].physicalPage;
	    	DEBUG('t',"valid: %i\n",pageTable[i].valid);
	    	newPageTable[i].valid = pageTable[i].valid;
	    	newPageTable[i].use = pageTable[i].use;
	    	newPageTable[i].dirty = pageTable[i].dirty;
	    	newPageTable[i].readOnly = pageTable[i].readOnly;
    	}
    	// Add 8 new pages of stack to newPageTable
    	int ppn;
    	for(i = numPages; i < numPages + 8; i++) {
    		DEBUG('t',"New virtual page: %i\n",i);
	    	newPageTable[i].virtualPage = i;
			ppn = mmBitMap->Find();
			if(ppn == -1) {
				printf("Unable to find unused physical page\n");
				interrupt->Halt();
			}
	    	newPageTable[i].physicalPage = i;
	    	newPageTable[i].valid = TRUE;
	    	newPageTable[i].use = FALSE;
	    	newPageTable[i].dirty = FALSE;
	    	newPageTable[i].readOnly = FALSE;
    	}
        delete[] pageTable; 
    	pageTable = newPageTable;
    	numPages = numPages + 8;
    	machine->pageTable = pageTable;
    	mmBitMapLock->Release();
    	for(int k = 0; k < numPages ; k ++){
            DEBUG('t', "addrspace: number #%i valid:%i\n",k,machine->pageTable[k].valid);
        }
  	  	return true;
  	} else {
    	DEBUG('a', "No room available on the stack");
    	return false;
    }
}

void AddrSpace::AllocateStack()			//Allocate 8 pages of Stack for a new thread
{
	mmBitMapLock->Acquire();

	ASSERT(numPages <= NumPhysPages);

	for (unsigned int x=numPages; x < numPages + 8; x++)
	{
		pageTable[x].virtualPage = x;
		pageTable[x].physicalPage = mmBitMap -> Find();	//Returns a Free Page
		ASSERT(pageTable[x].physicalPage != -1)			//Returns -1, if no free page available
		pageTable[x].valid = TRUE;
		pageTable[x].use = FALSE;
		pageTable[x].dirty = FALSE;
		pageTable[x].readOnly = FALSE;  // if the code segment was entirely on 
						// a separate page, we could set its 
						// pages to be read-only
	}		

	//update numPages to now include 8 pages for the allocated stack;

	numPages+=8;

	//addressSpaceSize+=8 * PageSize;

//	machine -> pageTableSize = numPages;

	
	mmBitMapLock->Release();

}

// Potentially called by Exit_Syscall()
void AddrSpace::clearStack(int stackStart) {
	for(unsigned int i = stackStart; i < stackStart + 8; i++) {
		pageTable[i].valid = FALSE;
		mmBitMap->Clear(pageTable[i].physicalPage);
	}
}

void AddrSpace::clearPhysicalPage(int i)
{
	mmBitMap->Clear(pageTable[i].physicalPage);
}
