#ifndef STRUCTS_H
#define STRUCTS_H

#include "synch.h"
#include <map>

struct KernelLock {
	Lock *lock;
	AddrSpace *space;
	bool isToBeDeleted;
	KernelLock() {
    	lock = new Lock();
    	space = NULL;
    	isToBeDeleted = true;
	}
	~KernelLock() {
    	if(lock)
        	delete lock;
    	if(space)
        	delete space;
	}
};

struct KernelCondition {
	Condition *condition;
	AddrSpace *space;
	bool isToBeDeleted;
	KernelCondition() {
	    condition = new Condition();
	    space = NULL;
	    isToBeDeleted = true;
	}
	~KernelCondition() {
	    if(condition)
	        delete condition;
	    if(space)
	        delete space;
	}
};

struct LockTable {
	int index;
	Lock *tableLock;
	std::map<int, KernelLock*> locks;
	LockTable() {
		index = 0;
		tableLock = new Lock();
	}
	~LockTable() {
		delete tableLock;
	}
};

struct ConditionTable {
	int index;
	Lock *tableLock;
	std::map<int, KernelCondition*> conditions;
	ConditionTable() {
		index = 0;
		tableLock = new Lock();
	}
	~ConditionTable() {
		delete tableLock;
	}
};

struct ProcessTable {
	int processID;
	//Lock *tableLock;
	int numProcesses;
	map<int, AddrSpace*> processes;
	ProcessTable() {
		processID = 0;
		numProcesses = 0;
		//tableLock = new Lock("processTableLock");
	}

};
#endif
