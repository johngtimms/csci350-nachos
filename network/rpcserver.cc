// rpcserver.cc

#include "system.h"
#include "network.h"
#include "rpcserver.h"
#include "interrupt.h"

// Setting up an RPCServer object may be unnecessary, but I can't think of a
// more convenient way to get the threads (started in RunServer) working.
RPCServer::RPCServer() { }
RPCServer::~RPCServer() { }

//-----------------------------------------------------------------------------------------------//
// Following are functions designed to be spun up as threads that 
// respond to the networking requests.
//-----------------------------------------------------------------------------------------------//

void RPCServer::Receive_CreateLock() {

}

void RPCServer::Receive_DestroyLock() {

}

void RPCServer::Receive_Acquire() {

}

void RPCServer::Receive_Release() {

}

void RPCServer::Receive_CreateCondition() {

}

void RPCServer::Receive_DestroyCondition() {

}

void RPCServer::Receive_Wait() {

}

void RPCServer::Receive_Signal() {

}

void RPCServer::Receive_Broadcast() {

}

void RPCServer::Receive_NetPrint() {
	PacketHeader inPktHdr;
	MailHeader inMailHdr;
	char buffer[MaxMailSize];

	for (;;) {
		// Wait for a mailbox message
		postOffice->Receive(MailboxNetPrint, &inPktHdr, &inMailHdr, buffer);

		// Print the message
		printf(buffer);
		fflush(stdout);
	}
}

//-----------------------------------------------------------------------------------------------//
// Dummy functions to allow forking / callbacks (also used in post.cc)
//-----------------------------------------------------------------------------------------------//

static void DummyReceive_CreateLock(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_CreateLock(); }
static void DummyReceive_DestroyLock(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_DestroyLock(); }
static void DummyReceive_Acquire(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_Acquire(); }
static void DummyReceive_Release(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_Release(); }
static void DummyReceive_CreateCondition(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_CreateCondition(); }
static void DummyReceive_DestroyCondition(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_DestroyCondition(); }
static void DummyReceive_Wait(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_Wait(); }
static void DummyReceive_Signal(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_Signal(); }
static void DummyReceive_Broadcast(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_Broadcast(); }
static void DummyReceive_NetPrint(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_NetPrint(); }

//-----------------------------------------------------------------------------------------------//
// RunServer() is called from main.cc when the "--server" flag is used.
// It starts threads to handle each networked syscall. Each thread checks its assigned mailbox.
//-----------------------------------------------------------------------------------------------//

void RunServer() {
	if (machineName > -1) {
		printf("SERVER RUNNING - MACHINE NAME IS %d\n", machineName);
	} else {
		printf("SERVER HAS NO MACHINE NAME. NACHOS HALTING.\n");
		interrupt->Halt();
	}

	// Note: rpcServer defined in system.cc

	Thread *tCreateLock = new Thread("CreateLock thread");
	tCreateLock->Fork(DummyReceive_CreateLock, (int) rpcServer);

	Thread *tDestroyLock = new Thread("DestroyLock thread");
	tDestroyLock->Fork(DummyReceive_DestroyLock, (int) rpcServer);

	Thread *tAcquire = new Thread("Acquire thread");
	tAcquire->Fork(DummyReceive_Acquire, (int) rpcServer);

	Thread *tRelease = new Thread("Release thread");
	tRelease->Fork(DummyReceive_Release, (int) rpcServer);

	Thread *tCreateCondition = new Thread("CreateCondition thread");
	tCreateCondition->Fork(DummyReceive_CreateCondition, (int) rpcServer);

	Thread *tDestroyCondition = new Thread("DestroyCondition thread");
	tDestroyCondition->Fork(DummyReceive_DestroyCondition, (int) rpcServer);

	Thread *tWait = new Thread("Wait thread");
	tWait->Fork(DummyReceive_Wait, (int) rpcServer);

	Thread *tSignal = new Thread("Signal thread");
	tSignal->Fork(DummyReceive_Signal, (int) rpcServer);

	Thread *tBroadcast = new Thread("Broadcast thread");
	tBroadcast->Fork(DummyReceive_Broadcast, (int) rpcServer);

	Thread *tNetPrint = new Thread("NetPrint thread");
	tNetPrint->Fork(DummyReceive_NetPrint, (int) rpcServer);
}
