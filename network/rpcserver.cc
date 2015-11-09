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
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxCreateLock, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));

        // Process the message (identical to original syscall)
        NetworkLock *lock = new NetworkLock(inPktHdr.from, processID);
        networkLockTable->tableLock->Acquire();
        int key = networkLockTable->index;
        networkLockTable->locks[key] = lock;
        networkLockTable->index++;
        networkLockTable->tableLock->Release();

        // Reply with the key
        DEBUG('r', "CreateLock process %d thread %d key %d (new)\n", processID, threadID, key);
        SendResponse(inPktHdr.from, inMailHdr.from, key);
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
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        unsigned int key = atoi(strtok(NULL,","));
        DEBUG('r', "DestroyLock process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkLockTable->tableLock->Acquire();

        DEBUG('r', "DestroyLock networkLockTable size %d\n", networkLockTable->locks.size());
        if (networkLockTable->locks.find(key) != networkLockTable->locks.end())
            lock = networkLockTable->locks[key];
        else 
            printf("WARN: DestroyLock failed. No such lock.\n"); // TODO this isn't accurate, not sure what needs to be done to fix

        if (lock != NULL && lock->IsOwner(processID))
            networkLockTable->locks.erase(key);
        else
            printf("WARN: DestroyLock failed. Client does not own lock.\n");

        networkLockTable->tableLock->Release();
    }
}

void RPCServer::Receive_Acquire() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    NetworkLock *lock;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxAcquire, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        unsigned int key = atoi(strtok(NULL,","));
        DEBUG('r', "Acquire process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkLockTable->tableLock->Acquire();

        if (networkLockTable->locks.find(key) != networkLockTable->locks.end())
            lock = networkLockTable->locks[key];
        else {
            printf("ERROR: Acquire failed. No such lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

        if (lock != NULL)
            lock->Acquire(processID, threadID);
            // Response is sent from Acquire()
        else {
            printf("ERROR: Acquire failed. Lock null. Terminating Nachos.\n");
            interrupt->Halt();
        }

        networkLockTable->tableLock->Release();
    }
}

void RPCServer::Receive_Release() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    NetworkLock *lock;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxRelease, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        unsigned int key = atoi(strtok(NULL,","));
        DEBUG('r', "Release process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkLockTable->tableLock->Acquire();

        if (networkLockTable->locks.find(key) != networkLockTable->locks.end())
            lock = networkLockTable->locks[key];
        else {
            printf("ERROR: Release failed. No such lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

        if (lock != NULL)
            lock->Release(processID, threadID);
            // Response (to any threads waiting to Acquire) is sent from lock->Release()
            // NO RESPONSE is sent to the thread making this actual call
        else {
            printf("ERROR: Release failed. Lock null. Terminating Nachos.\n");
            interrupt->Halt();
        }

        networkLockTable->tableLock->Release();
    }
}

void RPCServer::Receive_CreateCondition() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxCreateCondition, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));

        // Process the message (identical to original syscall)
        NetworkCondition *condition = new NetworkCondition(inPktHdr.from, processID);
        networkConditionTable->tableLock->Acquire();
        int key = networkConditionTable->index;
        networkConditionTable->conditions[key] = condition;
        networkConditionTable->index++;
        networkConditionTable->tableLock->Release();

        // Reply with the key
        DEBUG('r', "CreateCondition process %d thread %d key %d (new)\n", processID, threadID, key);
        SendResponse(inPktHdr.from, inMailHdr.from, key);
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
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        unsigned int key = atoi(strtok(NULL,","));
        DEBUG('r', "DestroyCondition process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();

        if(networkConditionTable->conditions.find(key) != networkConditionTable->conditions.end())
            condition = networkConditionTable->conditions[key];
        else 
            printf("WARN: DestroyCondition failed. No such condition.\n");

        if (condition != NULL && condition->IsOwner(processID))
            networkConditionTable->conditions.erase(key);
        else
            printf("WARN: DestroyCondition failed. Client does not own condition.\n");

        networkConditionTable->tableLock->Release();
    }
}

void RPCServer::Receive_Wait() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    NetworkCondition *condition;
    NetworkLock *lock;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxWait, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        unsigned int conditionKey = atoi(strtok(NULL,","));
        unsigned int lockKey = atoi(strtok(NULL,","));
        DEBUG('r', "Wait process %d thread %d conditionKey %d lockKey %d\n", processID, threadID, conditionKey, lockKey);

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();
        networkLockTable->tableLock->Acquire();

        if(networkConditionTable->conditions.find(conditionKey) != networkConditionTable->conditions.end())
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

        if (condition != NULL && lock != NULL) {
            condition->Wait(processID, threadID, lock);
            // Response sent from Wait()
        } else {
            printf("ERROR: Wait failed. Condition or lock null. Terminating Nachos.\n");
            interrupt->Halt();
        }

        networkConditionTable->tableLock->Release();
        networkLockTable->tableLock->Release();
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
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        unsigned int conditionKey = atoi(strtok(NULL,","));
        unsigned int lockKey = atoi(strtok(NULL,","));
        DEBUG('r', "Signal process %d thread %d conditionKey %d lockKey %d\n", processID, threadID, conditionKey, lockKey);

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();
        networkLockTable->tableLock->Acquire();

        if(networkConditionTable->conditions.find(conditionKey) != networkConditionTable->conditions.end())
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

        if (condition != NULL && lock != NULL)
            condition->Signal(processID, threadID, lock);
        else {
            printf("ERROR: Signal failed. Condition or lock null. Terminating Nachos.\n");
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
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        unsigned int conditionKey = atoi(strtok(NULL,","));
        unsigned int lockKey = atoi(strtok(NULL,","));
        DEBUG('r', "Broadcast process %d thread %d conditionKey %d lockKey %d\n", processID, threadID, conditionKey, lockKey);

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();
        networkLockTable->tableLock->Acquire();

        if(networkConditionTable->conditions.find(conditionKey) != networkConditionTable->conditions.end())
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

        if (condition != NULL && lock != NULL)
            condition->Broadcast(processID, threadID, lock);
        else {
            printf("ERROR: Broadcast failed. Condition or lock null. Terminating Nachos.\n");
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
// When replying to the client, must reply to the individual process/thread box.
//-----------------------------------------------------------------------------------------------//
int RPCServer::ClientMailbox(int process, int thread) {
    char str1[2];
    char str2[2];

    // Have to add 1 because process and thread can both be 0, which would create overlap
    sprintf(str1, "%d", (process + 1));
    sprintf(str2, "%d", (thread + 1));
    strcat(str1, str2);

    DEBUG('r', "Client Mailbox %d process %d thread %d\n", atoi(str1), process, thread);

    return atoi(str1);
}

//-----------------------------------------------------------------------------------------------//
// A lot of calls need to reply "OK" when they're done, this promotes code reuse.
// It also handles numeric responses.
// For "OK" pass -1 as the response. For a numeric response, pass a number as the response.
//-----------------------------------------------------------------------------------------------//
void RPCServer::SendResponse(int machineID, int mailbox, int response) {
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    char send[MaxMailSize];

    // Construct response
    if (response == -1)
        strcpy(send, "OK");
    else
        sprintf(send, "%d", response);

    DEBUG('r', "Send response machine %d mailbox %d response %d \"%s\"\n", machineID, mailbox, response, send);

    // Construct packet header, mail header for the message
    outPktHdr.to = machineID;       
    outMailHdr.to = mailbox;
    outMailHdr.from = 100; // No syscall ever needs to reply to a response
    outMailHdr.length = strlen(send) + 1;

    // Send the response message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

    if ( !success )
        printf("WARN: SendResponse failed. Client misconfigured.\n");
    else
        DEBUG('r', "Send response success\n");
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

//-----------------------------------------------------------------------------------------------//
// Create new NetworkLock
//-----------------------------------------------------------------------------------------------//

NetworkLock::NetworkLock(int _machineID, int process) {
    machineID = _machineID;
    processID = process;
    threadID = -1;
    queue = new List;
}

NetworkLock::~NetworkLock() {
    delete queue;
}

void NetworkLock::Acquire(int process, int thread) {
    // Only the owner process may access this lock
    if (process != processID) {
        printf("ERROR: Acquire failed. Owner error. Terminating Nachos.\n");
        interrupt->Halt();
    }

    // Do the regular Lock::Acquire stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (threadID == -1) {                               // lock is available
        threadID = thread;
        DEBUG('r', "Acquire success machine %d process %d thread %d\n", machineID, process, thread);
        RPCServer::SendResponse(machineID, RPCServer::ClientMailbox(processID, threadID), -1);
    } else if (threadID != thread) {                    // lock is busy
        queue->Append((void *) thread);                 // add thread to the wait queue
        DEBUG('r', "Acquire waiting process %d thread %d\n", process, thread);
    } else {
        printf("WARN: Acquire duplicate process %d thread %d\n", process, thread);
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

void NetworkLock::Release(int process, int thread) {
    // Only the owner process may access this lock
    if (process != processID) {
        printf("ERROR: Acquire failed. Owner error. Terminating Nachos.\n");
        interrupt->Halt();
    }

    // Do the regular Lock::Release stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (threadID == thread) {
        if (queue->IsEmpty())                           // no threads waiting
            threadID = -1;
        else {
            threadID = (int) queue->Remove();           // wake up a waiting thread
            DEBUG('r', "Acquire success via release process %d thread %d\n", processID, threadID);
            RPCServer::SendResponse(machineID, RPCServer::ClientMailbox(processID, threadID), -1);
        }
    } else {
        printf("WARN: Release without acquire process %d thread %d\n", process, thread);
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

bool NetworkLock::IsOwner(int process) {
    return (processID == process);
}

bool NetworkLock::HasAcquired(int thread) {
    return (threadID == thread);
}

//-----------------------------------------------------------------------------------------------//
// Create new NetworkCondition
//-----------------------------------------------------------------------------------------------//

NetworkCondition::NetworkCondition(int _machineID, int process) {
    machineID = _machineID;
    processID = process;
    conditionLock = NULL;
    queue = new List;
}

NetworkCondition::~NetworkCondition() {
    delete conditionLock;
    delete queue;
}

void NetworkCondition::Wait(int process, int thread, NetworkLock* lock) {
    // Only the owner process may access this condition
    if (process != processID) {
        printf("ERROR: Wait failed. Owner error. Terminating Nachos.\n");
        interrupt->Halt();
    }

    // Do the regular Condition::Wait stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (lock != NULL) {
        if (lock->IsOwner(process) && lock->HasAcquired(thread)) {
            if (conditionLock == NULL)                      // condition hasn't been assigned to a lock yet
                conditionLock = lock;
                DEBUG('r', "Wait assigned lock process %d thread %d\n", processID, thread);
            if (conditionLock == lock) {                    // ok to wait
                queue->Append((void *) thread);             // add thread to wait queue
                conditionLock->Release(process, thread);    // release waiting lock
                DEBUG('r', "Wait waiting process %d thread %d\n", processID, thread);
            } else {
                printf("ERROR: Wait failed. Wrong lock. Terminating Nachos.\n");
                interrupt->Halt();
            }
        } else {
            printf("ERROR: Wait failed. Unaquired lock. Terminating Nachos.\n");
            interrupt->Halt();
        }
    } else {
        printf("ERROR: Wait failed. Lock null. Terminating Nachos.\n");
        interrupt->Halt();
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

void NetworkCondition::Signal(int process, int _thread, NetworkLock* lock) {
    // Only the owner process may access this condition
    if (process != processID) {
        printf("ERROR: Signal failed. Owner error. Terminating Nachos.\n");
        interrupt->Halt();
    }

    // Do the regular Condition::Signal stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (lock != NULL) {
        if (lock->IsOwner(process) && lock->HasAcquired(_thread)) {
            if (conditionLock == lock) {
                int thread = (int) queue->Remove();         // remove one waiting thread from queue
                DEBUG('r', "Signal thread process %d thread %d thread %d (given)\n", processID, thread, _thread);
                RPCServer::SendResponse(machineID, RPCServer::ClientMailbox(processID, thread), -1);
                RPCServer::SendResponse(machineID, RPCServer::ClientMailbox(processID, _thread), -1);
                // MUST Release() following this call to Signal() in exception.cc, otherwise the thread signaled will be stuck
                // That's why the second response is sent (so Signal() knows to go ahead and Release())
                // Similarly, the thread signaled is still sitting in Wait() in exception.cc, and it should Acquire() there
            } else {
                printf("ERROR: Signal failed. Wrong lock. Terminating Nachos.\n");
                interrupt->Halt();
            }
        } else {
            printf("ERROR: Signal failed. Unaquired lock. Terminating Nachos.\n");
            interrupt->Halt();
        }
    } else {
        printf("ERROR: Signal failed. Lock null. Terminating Nachos.\n");
        interrupt->Halt();
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

void NetworkCondition::Broadcast(int process, int _thread, NetworkLock* lock) {
    // Only the owner process may access this condition
    if (process != processID) {
        printf("ERROR: Broadcast failed. Owner error. Terminating Nachos.\n");
        interrupt->Halt();
    }

    // Do the regular Condition::Broadcast stuff
    // MUST Release() following this call to Broadcast() in exception.cc, otherwise the thread(s) signaled will be stuck
    if (lock != NULL) {
        if (lock->IsOwner(process) && lock->HasAcquired(_thread)) {
            if (conditionLock == lock) {
                while (!queue->IsEmpty())                   // signal all threads waiting
                    Signal(process, _thread, lock);
            } else {
                printf("ERROR: Broadcast failed. Wrong lock. Terminating Nachos.\n");
                interrupt->Halt();
            }
        } else {
            printf("ERROR: Signal failed. Unaquired lock. Terminating Nachos.\n");
            interrupt->Halt();
        }
    } else {
        printf("ERROR: Broadcast failed. Lock null. Terminating Nachos.\n");
        interrupt->Halt();
    }
}

bool NetworkCondition::IsOwner(int process) {
    return (processID == process);
}
