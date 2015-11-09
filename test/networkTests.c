#include "syscall.h"

int main() {
	int lockOne;
	int lockTwo;
	int conditionOne;

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
	conditionOne = CreateCondition();
	Print("\tCreateCondition test (1/1): Condition ID is %i\n", conditionOne);

	/* Test Wait */
	Acquire(lockTwo);
	Wait(conditionOne, lockTwo);

	/* Test Signal */

	/* Test Broadcast */

	/* Test DestroyCondition */

	/* This is probably a temporary test. I want to
	see what happens to the thread names when I do an exec
	Print("Testing exec from network tests\n", 100);
	Exec("../test/networkTests", 20); */

	/* Test NetHalt */
	Print("\tNetHalt test (1/1): Check that server shuts down\n", 100);
	NetHalt();

	Exit(0);
}

