#include "copyright.h"
#include "system.h"
#include "syscall.h"

using namespace std;

class Process {
public:
	AddrSpace* space;
	spaceId processID;
	Thread* processThread;
	char *name;
	int threadCount;
	List* threads;
	Process(char *processName);
};
