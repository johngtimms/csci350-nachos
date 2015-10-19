#include "copyright.h"
#include "system.h"
#include "syscall.h"

using namespace std;

class Process {
public:
	AddrSpace* space;
	Thread* processThread;
	char *name;
	int threadCount;
	spaceId processID;
	List* threads;
	Process(char *processName);
};
