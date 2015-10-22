#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <vector>

using namespace std;

class Process {
public:
	AddrSpace* space;
	int processID;
	Thread* processThread;
	char *name;
	int threadCount;
	vector<Thread*> *threads;
	Process(char *processName);
	~Process();
};
