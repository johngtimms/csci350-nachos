#include "syscall.h"

void TestWait() {
	Acquire("lockOne");
	Wait("conditionOne", "lockOne"); /* lockOne will be released */
	Print("\tWait test (2/2): Finished waiting\n", 100);
	Release("lockOne"); /* release lockOne again, Wait re-acquired after Signal. */
	Exit(0);
}

void TestSignal() {
	Acquire("lockOne");
	Print("\tWait test (1/2): Wait DID release the lock\n", 100);
	Signal("conditionOne", "lockOne"); /* lockOne will be released */
	Print("\tSignal test (1/2): Finished signaling\n", 100);
	Exit(0);
}

void TestBroadcast() {
	Acquire("lockOne");
	Broadcast("conditionOne", "lockOne");
	Print("\tBroadcast test (?/5): Finished broadcasting\n", 100);
	Exit(0);
}

void TestBroadcastHelper() {
	Acquire("lockOne");
	Wait("conditionOne", "lockOne"); /* lockOne will be released half-way through Wait, then gotten again */
	Print("\tBroadcast test (?/5): Finished waiting\n", 100);
	Release("lockOne"); /* release lockOne again, Wait re-acquired after Signal. */	
	Exit(0);
}

int main() {
	Print("Network Tests\n", 100);

	/* Test NetPrint */
	Print("\tNetPrint test (1/1): Check server console for \"Hello World!\"\n", 100);
	NetPrint("Hello World!\n", 13);

	/* Test CreateLock */
	CreateLock("lockOne");
	CreateLock("lockTwo");
	Print("\tCreateLock test (1/1): Two locks created\n", 100);

	/* Test (premature) Release */
	Release("lockOne");
	Print("\tRelease test (1/2): Check server console for \"WARN: Release ...\"\n", 100);

	/* Test Acquire */
	Acquire("lockOne");
	Print("\tAcquire test (1/1): Lock acquired\n", 100);

	/* Test Release */
	Release("lockOne");
	Print("\tRelease test (2/2): Lock released\n", 100);

	/* Test DestroyLock */
	DestroyLock("lockOne");
	DestroyLock("lockTwo");
	Print("\tDestroyLock test (1/1): Destroyed two locks\n", 100);

	/* Test CreateCondition */
	CreateCondition("conditionOne");
	Print("\tCreateCondition test (1/1): One condition created\n", 100);

	/* Test Wait / Signal */
	CreateLock("lockOne");
	Fork(&TestWait);
	Fork(&TestSignal);
	Acquire("lockOne");	/* Somehow lockOne is being acquired before it's apropriate */
	Print("\tSignal test (2/2): Signal DID release the lock\n", 100);
	Release("lockOne");

	/* Test Broadcast */
	Fork(&TestBroadcastHelper);
	Fork(&TestBroadcastHelper);
	Acquire("lockOne");
	Print("\tBroadcast test (1/5): Acquired lock to prove waits DID release\n", 100);
	Release("lockOne");
	Fork(&TestBroadcast);
	Acquire("lockOne");
	Print("\tBroadcast test (5/5): Broadcast DID release the lock\n", 100);
	Release("lockOne");

	/* Test DestroyCondition */
	DestroyCondition("conditionOne");
	Print("\tDestroyCondition test (1/1): Destroyed one condition\n", 100);
	Acquire("lockOne"); /* Don't destroy a lock held by someone else */
	DestroyLock("lockOne");

	/* Test NetHalt */
	Print("\tNetHalt test (1/1): Check that server shuts down\n", 100);
	NetHalt();

	Exit(0);
}
