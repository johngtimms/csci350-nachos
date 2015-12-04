#include "syscall.h"

void TestWait() {
	Acquire("lockOne", -1);
	Wait("conditionOne", -1, "lockOne", -1); /* lockOne will be released */
	Print("\tWait test (2/2): Finished waiting\n", 100);
	Release("lockOne", -1); /* release lockOne again, Wait re-acquired after Signal. */
	Exit(0);
}

void TestSignal() {
	Acquire("lockOne", -1);
	Print("\tWait test (1/2): Wait DID release the lock\n", 100);
	Signal("conditionOne", -1, "lockOne", -1); /* lockOne will be released */
    Release("lockOne", -1);
	Print("\tSignal test (1/2): Finished signaling\n", 100);
	Exit(0);
}

void TestBroadcast() {
	Acquire("lockOne", -1);
	Broadcast("conditionOne", -1, "lockOne", -1);
    Release("lockOne", -1);
	Print("\tBroadcast test (?/5): Finished broadcasting\n", 100);
	Exit(0);
}

void TestBroadcastHelper() {
	Acquire("lockOne", -1);
	Wait("conditionOne", -1, "lockOne", -1); /* lockOne will be released half-way through Wait, then gotten again */
	Print("\tBroadcast test (?/5): Finished waiting\n", 100);
	Release("lockOne", -1); /* release lockOne again, Wait re-acquired after Signal. */
	Exit(0);
}

int main() {
	Print("Network Tests\n", 100);

	/* Test NetPrint */
	Print("\tNetPrint test (1/1): Check server console for \"Hello World!\"\n", 100);
	NetPrint("Hello World!\n", 13);

	/* Test CreateLock */
	CreateLock("lockOne", -1);
	CreateLock("lockTwo", -1);
	Print("\tCreateLock test (1/1): Two locks created\n", 100);

	/* Test (premature) Release */
	Release("lockOne", -1);
	Print("\tRelease test (1/2): Check server console for \"WARN: Release ...\"\n", 100);

	/* Test Acquire */
	Acquire("lockOne", -1);
	Print("\tAcquire test (1/1): Lock acquired\n", 100);

	/* Test Release */
	Release("lockOne", -1);
	Print("\tRelease test (2/2): Lock released\n", 100);

	/* Test DestroyLock */
	DestroyLock("lockOne", -1);
	DestroyLock("lockTwo", -1);
	Print("\tDestroyLock test (1/1): Destroyed two locks\n", 100);

	/* Test CreateCondition */
	CreateCondition("conditionOne", -1);
	Print("\tCreateCondition test (1/1): One condition created\n", 100);

	/* Test Wait / Signal */
	CreateLock("lockOne", -1);
	Fork(&TestWait);
	Fork(&TestSignal);
	Acquire("lockOne", -1);	/* Somehow lockOne is being acquired before it's apropriate */
	Print("\tSignal test (2/2): Signal DID release the lock\n", 100);
	Release("lockOne", -1);

	/* Test Broadcast */
	Fork(&TestBroadcastHelper);
	Fork(&TestBroadcastHelper);
	Acquire("lockOne", -1);
	Print("\tBroadcast test (1/5): Acquired lock to prove waits DID release\n", 100);
	Release("lockOne", -1);
	Fork(&TestBroadcast);
	Acquire("lockOne", -1);
	Print("\tBroadcast test (5/5): Broadcast DID release the lock\n", 100);
	Release("lockOne", -1);

	/* Test DestroyCondition */
	DestroyCondition("conditionOne", -1);
	Print("\tDestroyCondition test (1/1): Destroyed one condition\n", 100);
	Acquire("lockOne", -1); /* Don't destroy a lock held by someone else */
	DestroyLock("lockOne", -1);

	/* Test NetHalt */
	Print("\tNetHalt test (1/1): Check that server shuts down\n", 100);
	NetHalt();

	Exit(0);
}
