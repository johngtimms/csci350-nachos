#include "syscall.h"

int main() {
	int lock;
	int locktwo;

	Print("Network Tests\n", 14);

	/* Test NetPrint */
	NetPrint("First Message\n", 14);
	NetPrint("Second Message\n", 15);

	/* Test CreateLock */
	lock = CreateLock();
	Print("Created Lock\n", 13);
	locktwo = CreateLock();

	/* Test (premature) Release */

	/* Test Acquire */

	/* Test Release */

	/* Test DestroyLock */
	DestroyLock(locktwo);
	Print("Destroyed Lock\n", 15);

	/* This is probably a temporary test. I want to
	see what happens to the thread names when I do an exec*/
	Print("Testing exec from network tests\n", 32);
	Exec("../test/networkTests", 20);

	/* Test NetHalt */
	NetHalt();

	Exit(0);
}

