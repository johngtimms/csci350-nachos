#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <vector>

using namespace std;

class Process {
public:
	AddrSpace* space;
	spaceId processID;
	Thread* processThread;
	char *name;
	int threadCount;
	vector<Thread*> *threads;
	Process(char *processName);
	~Process();
};
