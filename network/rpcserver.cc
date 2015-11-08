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
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxCreateLock, &inPktHdr, &inMailHdr, recv);

        // Process the message (identical to original syscall)
        NetworkLock *lock = new NetworkLock();
        lock->threadID = atoi(recv);
        networkLockTable->tableLock->Acquire();
        int key = networkLockTable->index;
        networkLockTable->locks[key] = lock;
        networkLockTable->index++;
        networkLockTable->tableLock->Release();

        // Reply with the key
        sprintf(send, "%d", key);

        // Construct packet header, mail header for the message
        outPktHdr.to = inPktHdr.from;       
        outMailHdr.to = MailboxCreateLock;
        outMailHdr.from = MailboxCreateLock;
        outMailHdr.length = strlen(send) + 1;

        // Send the response message
        bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

        if ( !success )
            printf("WARN: CreateLock failed. Client misconfigured.\n");
    }
}

void RPCServer::Receive_DestroyLock() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    NetworkLock *lock;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxDestroyLock, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int threadID = atoi(strtok(recv,","));
        unsigned int key = atoi(strtok(NULL,","));

        // Process the message (identical to original syscall)
        networkLockTable->tableLock->Acquire();

        if (key >= 0 && key < networkLockTable->locks.size())
            lock = networkLockTable->locks[key];
        else 
            printf("WARN: DestroyLock failed. No such lock.\n");

        if (lock != NULL && lock->threadID == threadID)
            networkLockTable->locks.erase(key);
        else
            printf("WARN: DestroyLock failed. Client does not own lock.\n");

        networkLockTable->tableLock->Release();
    }
}

void RPCServer::Receive_Acquire() {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];

    NetworkLock *lock;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxAcquire, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int threadID = atoi(strtok(recv,","));
        unsigned int key = atoi(strtok(NULL,","));
        DEBUG('rpc', "Acquire key %d\n", key);

        // Process the message (identical to original syscall)
        networkLockTable->tableLock->Acquire();

        if (key >= 0 && key < networkLockTable->locks.size())
            lock = networkLockTable->locks[key];
        else {
            printf("ERROR: Acquire failed. No such lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

        if (lock != NULL && lock->threadID == threadID)
            lock->lock->Acquire();
        else {
            printf("ERROR: Acquire failed. Client does not own lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

        networkLockTable->tableLock->Release();

        // Reply with OK
        strcpy(send, "OK");

        // Construct packet header, mail header for the message
        outPktHdr.to = inPktHdr.from;       
        outMailHdr.to = MailboxAcquire;
        outMailHdr.from = MailboxAcquire;
        outMailHdr.length = strlen(send) + 1;

        // Send the response message
        bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

        if ( !success )
            printf("WARN: Acquire failed. Client misconfigured.\n");
    }
}

void RPCServer::Receive_Release() {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];

    NetworkLock *lock;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxRelease, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int threadID = atoi(strtok(recv,","));
        unsigned int key = atoi(strtok(NULL,","));
         DEBUG('rpc', "Release key %d\n", key);

        // Process the message (identical to original syscall)
        networkLockTable->tableLock->Acquire();

        if (key >= 0 && key < networkLockTable->locks.size())
            lock = networkLockTable->locks[key];
        else {
            printf("ERROR: Release failed. No such lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

        if (lock != NULL && lock->threadID == threadID)
            lock->lock->Release();
        else {
            printf("ERROR: Release failed. Client does not own lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

        networkLockTable->tableLock->Release();

        // Reply with OK
        strcpy(send, "OK");

        // Construct packet header, mail header for the message
        outPktHdr.to = inPktHdr.from;       
        outMailHdr.to = MailboxRelease;
        outMailHdr.from = MailboxRelease;
        outMailHdr.length = strlen(send) + 1;

        // Send the response message
        bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

        if ( !success )
            printf("WARN: Release failed. Client misconfigured.\n");
    }
}

void RPCServer::Receive_CreateCondition() {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxCreateCondition, &inPktHdr, &inMailHdr, recv);

        // Process the message (identical to original syscall)
        NetworkCondition *condition = new NetworkCondition();
        condition->threadID = atoi(recv);
        networkConditionTable->tableLock->Acquire();
        int key = networkConditionTable->index;
        networkConditionTable->conditions[key] = condition;
        networkConditionTable->index++;
        networkConditionTable->tableLock->Release();

        // Reply with the key
        sprintf(send, "%d", key);

        // Construct packet header, mail header for the message
        outPktHdr.to = inPktHdr.from;       
        outMailHdr.to = MailboxCreateCondition;
        outMailHdr.from = MailboxCreateCondition;
        outMailHdr.length = strlen(send) + 1;

        // Send the response message
        bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

        if ( !success )
            printf("WARN: CreateCondition failed. Client misconfigured.\n");
    }
}

void RPCServer::Receive_DestroyCondition() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    NetworkCondition *condition;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxDestroyCondition, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int threadID = atoi(strtok(recv,","));
        unsigned int key = atoi(strtok(NULL,","));

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();

        if (key >= 0 && key < networkConditionTable->conditions.size())
            condition = networkConditionTable->conditions[key];
        else 
            printf("WARN: DestroyCondition failed. No such condition.\n");

        if (condition != NULL && condition->threadID == threadID)
            networkConditionTable->conditions.erase(key);
        else
            printf("WARN: DestroyCondition failed. Client does not own condition.\n");

        networkConditionTable->tableLock->Release();
    }
}

void RPCServer::Receive_Wait() {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];

    NetworkCondition *condition;
    NetworkLock *lock;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxWait, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int threadID = atoi(strtok(recv,","));
        unsigned int conditionKey = atoi(strtok(NULL,","));
        unsigned int lockKey = atoi(strtok(NULL,","));

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();
        networkLockTable->tableLock->Acquire();

        if (conditionKey >= 0 && conditionKey < networkConditionTable->conditions.size())
            condition = networkConditionTable->conditions[conditionKey];
        else {
            printf("ERROR: Wait failed. No such condition. Terminating Nachos.\n");
            interrupt->Halt();
        }

        if (lockKey >= 0 && lockKey < networkLockTable->locks.size())
            lock = networkLockTable->locks[lockKey];
        else {
            printf("ERROR: Wait failed. No such lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

        if (condition != NULL && lock != NULL && condition->threadID == threadID && lock->threadID == threadID) {
            networkConditionTable->tableLock->Release();
            networkLockTable->tableLock->Release();
            condition->condition->Wait(lock->lock);

            // Reply with OK
            strcpy(send, "OK");

            // Construct packet header, mail header for the message
            outPktHdr.to = inPktHdr.from;       
            outMailHdr.to = MailboxWait;
            outMailHdr.from = MailboxWait;
            outMailHdr.length = strlen(send) + 1;

            // Send the response message
            bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

            if ( !success )
                printf("WARN: Wait failed. Client misconfigured.\n");
        } else {
            printf("ERROR: Wait failed. Ownership error. Terminating Nachos.\n");
            interrupt->Halt();
        }
    }
}

void RPCServer::Receive_Signal() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    NetworkCondition *condition;
    NetworkLock *lock;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxSignal, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int threadID = atoi(strtok(recv,","));
        unsigned int conditionKey = atoi(strtok(NULL,","));
        unsigned int lockKey = atoi(strtok(NULL,","));

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();
        networkLockTable->tableLock->Acquire();

        if (conditionKey >= 0 && conditionKey < networkConditionTable->conditions.size())
            condition = networkConditionTable->conditions[conditionKey];
        else {
            printf("ERROR: Signal failed. No such condition. Terminating Nachos.\n");
            interrupt->Halt();
        }

        if (lockKey >= 0 && lockKey < networkLockTable->locks.size())
            lock = networkLockTable->locks[lockKey];
        else {
            printf("ERROR: Signal failed. No such lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

        if (condition != NULL && lock != NULL && condition->threadID == threadID && lock->threadID == threadID)
            condition->condition->Signal(lock->lock);
        else {
            printf("ERROR: Signal failed. Ownership error. Terminating Nachos.\n");
            interrupt->Halt();
        }

        networkConditionTable->tableLock->Release();
        networkLockTable->tableLock->Release();
    }
}

void RPCServer::Receive_Broadcast() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    NetworkCondition *condition;
    NetworkLock *lock;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxBroadcast, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int threadID = atoi(strtok(recv,","));
        unsigned int conditionKey = atoi(strtok(NULL,","));
        unsigned int lockKey = atoi(strtok(NULL,","));

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();
        networkLockTable->tableLock->Acquire();

        if (conditionKey >= 0 && conditionKey < networkConditionTable->conditions.size())
            condition = networkConditionTable->conditions[conditionKey];
        else {
            printf("ERROR: Broadcast failed. No such condition. Terminating Nachos.\n");
            interrupt->Halt();
        }

        if (lockKey >= 0 && lockKey < networkLockTable->locks.size())
            lock = networkLockTable->locks[lockKey];
        else {
            printf("ERROR: Broadcast failed. No such lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

        if (condition != NULL && lock != NULL && condition->threadID == threadID && lock->threadID == threadID)
            condition->condition->Broadcast(lock->lock);
        else {
            printf("ERROR: Broadcast failed. Ownership error. Terminating Nachos.\n");
            interrupt->Halt();
        }

        networkConditionTable->tableLock->Release();
        networkLockTable->tableLock->Release();
    }
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

void RPCServer::Receive_NetHalt() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char buffer[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxNetHalt, &inPktHdr, &inMailHdr, buffer);

        // Halt when receiving a message
        interrupt->Halt();
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
static void DummyReceive_NetHalt(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_NetHalt(); }

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

    Thread *tNetHalt = new Thread("NetHalt thread");
    tNetHalt->Fork(DummyReceive_NetHalt, (int) rpcServer);
}
