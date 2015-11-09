#include "syscall.h"

int lockOne;
int lockTwo;
int conditionOne;

void TestWait() {
	Print("qqqtw\n", 100);
	Acquire(lockOne); /* this becomes thread 1
	and gives the following output:                --a-1-- Acquire - process 0 thread 1 key 2 
	somehow / somewhere in this vicinity we get the error from the post office:

		ERROR: Condition list empty cond was signaled, but lock is NULL! Given lock is list lock.

	this occurs AFTER fork(&TestSignal) is called!

	*/
	Print("\t\t condition %d", conditionOne); Print(" lock %d\n", lockOne);
	Wait(conditionOne, lockOne); /* lockOne will be released */
	Print("\tWait test (2/2): Finished waiting\n", 100);
	Exit(0);
}

void TestSignal() {
	Print("qqqts\n", 100);
	Acquire(lockOne);
	Print("\tWait test (1/2): Wait DID release the lock\n", 100);
	Signal(conditionOne, lockOne); /* lockOne will be released */
	Print("\tSignal test (1/2): Finished signaling\n", 100);
	Exit(0);
}

void TestBroadcast() {
	Print("qqqtb\n", 100);
	Acquire(lockOne);
	Broadcast(conditionOne, lockOne);
	Print("\tBroadcast test (?/5): Finished broadcasting\n", 100);
	Exit(0);
}

void TestBroadcastHelper() {
	Print("qqqtbh\n", 100);
	Acquire(lockOne);
	Wait(conditionOne, lockOne);
	Print("\tBroadcast test (?/5): Finished waiting\n", 100);
	Exit(0);
}

int main() {
	Print("Network Tests\n", 100);

	/* Test NetPrint */
	Print("\tNetPrint test (1/1): Check server console for \"Hello World!\"\n", 100);
	NetPrint("Hello World!\n", 13);

	/* Test CreateLock */
	lockOne = CreateLock();
	lockTwo = CreateLock();
	Print("\tCreateLock test (1/1): Two lock IDs are %i", lockOne); Print(", %i\n", lockTwo);

	/* Test (premature) Release */
	Release(lockOne);
	Print("\tRelease test (1/2): Check server console for \"WARN: Release ...\"\n", 100);

	/* Test Acquire */
	Acquire(lockOne);
	Print("\tAcquire test (1/1): Lock acquired\n", 100);

	/* Test Release */
	Release(lockOne);
	Print("\tRelease test (2/2): Lock released\n", 100);

	/* Test DestroyLock */
	DestroyLock(lockOne);
	DestroyLock(lockTwo);
	Print("\tDestroyLock test (1/1): Destroyed two locks\n", 100);

	/* Test CreateCondition */
	Print("www0\n", 100);
	conditionOne = CreateCondition();
	Print("\tCreateCondition test (1/1): Condition ID is %i\n", conditionOne);

	/* Test Wait / Signal */
	Print("www1\n", 100);
	lockOne = CreateLock();
	Print("www2\n", 100);
	Fork(&TestWait);
	Print("www3a\n", 100);
	Print("www3b\n", 100);
	Print("www3c\n", 100);
	Print("www3d\n", 100);
	Print("www3e\n", 100);
	Print("www3f\n", 100);
	Print("www3g\n", 100);
	Fork(&TestSignal);
	/* post office error appears exactly here */
	Print("www4\n", 100);
	Acquire(lockOne);
	Print("www5\n", 100);
	Print("\tSignal test (2/2): Signal DID release the lock\n", 100);

	/* Test Broadcast */
	Fork(&TestBroadcastHelper);
	Fork(&TestBroadcastHelper);
	Acquire(lockOne);
	Print("\tBroadcast test (1/5): Acquired lock to prove waits DID release\n", 100);
	Release(lockOne);
	Fork(&TestBroadcast);
	Acquire(lockOne);
	Print("\tBroadcast test (5/5): Broadcast DID release the lock\n", 100);

	/* Test DestroyCondition */
	DestroyCondition(conditionOne);
	Print("\tDestroyCondition test (1/1): Destroyed one condition\n", 100);
	DestroyLock(lockOne);

	/* Test NetHalt */
	Print("\tNetHalt test (1/1): Check that server shuts down\n", 100);
	NetHalt();

	Exit(0);
}
