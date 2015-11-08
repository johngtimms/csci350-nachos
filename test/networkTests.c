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

	/* Test NetHalt */
	NetHalt();

	Exit(0);
}

