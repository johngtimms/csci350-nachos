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
        int machineID = inPktHdr.from;
        char *name = strtok(NULL,",");
        
        int foundKey = -1;
        std::map<int, NetworkLock*>::iterator iterator;
        for(iterator = networkLockTable->locks.begin(); iterator != networkLockTable->locks.end(); iterator++) {
            if(iterator->second->name == name) {
                foundKey = iterator->first;
                break;
            }
        }
        if(foundKey != -1) {
            DEBUG('r', "CreateLock - found Lock already created with key: %i\n", foundKey);
            SendResponse(inPktHdr.from, inMailHdr.from, foundKey);
            continue;
        }

        // Process the message (identical to original syscall)
        NetworkLock *lock = new NetworkLock(inPktHdr.from, processID, name);
        networkLockTable->tableLock->Acquire();
        int key = networkLockTable->index;
        networkLockTable->locks[key] = lock;
        networkLockTable->index++;
        networkLockTable->tableLock->Release();

        // Reply with the key
        DEBUG('r', "CreateLock - machineID %i name %s key %d (new)\n", machineID, name, key);
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
        DEBUG('r', "DestroyLock - process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkLockTable->tableLock->Acquire();

        lock = networkLockTable->locks[key];
        if (lock != NULL) {
            //if (lock->IsOwner(processID))
                networkLockTable->locks.erase(key);
            /*else
              printf("WARN: DestroyLock failed. Client does not own lock.\n");  */
        } else
            printf("WARN: DestroyLock failed. No such lock.\n");

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
        int machineID = inPktHdr.from;
        
        unsigned int key = atoi(strtok(NULL,","));
        DEBUG('r', "Acquire - process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkLockTable->tableLock->Acquire();

        lock = networkLockTable->locks[key];
        if (lock != NULL) {
            lock->Acquire(machineID, processID, threadID);
            // Response is sent from Acquire()
        } else {
            printf("ERROR: Acquire failed. No such lock. Terminating Nachos.\n");
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
        int machineID = inPktHdr.from;
        
        unsigned int key = atoi(strtok(NULL,","));
        DEBUG('r', "Release - process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkLockTable->tableLock->Acquire();

        lock = networkLockTable->locks[key];
        if (lock != NULL)
            lock->Release(machineID, processID, threadID);
            // Response (to any threads waiting to Acquire) is sent from lock->Release()
            // NO RESPONSE is sent to the thread making this actual call
        else {
            printf("ERROR: Release failed. No such lock. Terminating Nachos.\n");
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
        char *name = strtok(NULL,",");
        
        int foundKey = -1;
        std::map<int, NetworkCondition*>::iterator iterator;
        for(iterator = networkConditionTable->conditions.begin(); iterator != networkConditionTable->conditions.end(); iterator++) {
            if(iterator->second->name == name) {
                foundKey = iterator->first;
                break;
            }
        }
        if(foundKey != -1) {
            DEBUG('r', "CreateCondition - found Condition already created with key: %i\n", foundKey);
            SendResponse(inPktHdr.from, inMailHdr.from, foundKey);
            continue;
        }

        // Process the message (identical to original syscall)
        NetworkCondition *condition = new NetworkCondition(inPktHdr.from, processID, name);
        networkConditionTable->tableLock->Acquire();
        int key = networkConditionTable->index;
        networkConditionTable->conditions[key] = condition;
        networkConditionTable->index++;
        networkConditionTable->tableLock->Release();

        // Reply with the key
        DEBUG('r', "CreateCondition - process %d thread %d key %d (new)\n", processID, threadID, key);
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
        DEBUG('r', "DestroyCondition - process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();

        condition = networkConditionTable->conditions[key];
        if (condition != NULL) {
            //if (condition->IsOwner(processID))
                networkConditionTable->conditions.erase(key);
            //else
            //   printf("WARN: DestroyCondition failed. Client does not own condition.\n"); 
        } else {
           printf("WARN: DestroyCondition failed. No such condition.\n"); 
        }

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
        int machineID = inPktHdr.from;
        
        unsigned int conditionKey = atoi(strtok(NULL,","));
        unsigned int lockKey = atoi(strtok(NULL,","));
        DEBUG('r', "Wait - process %d thread %d conditionKey %d lockKey %d\n", processID, threadID, conditionKey, lockKey);

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();
        networkLockTable->tableLock->Acquire();

        condition = networkConditionTable->conditions[conditionKey];
        if (condition == NULL) {
            printf("ERROR: Wait failed. No such condition. Terminating Nachos.\n");
            interrupt->Halt();
        }

        lock = networkLockTable->locks[lockKey];
        if (lock == NULL) {
            printf("ERROR: Wait failed. No such lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

       // if (condition->IsOwner(machineID) && lock->IsOwner(machineID)) {
            condition->Wait(machineID, processID, threadID, lock);
            // Response sent from Wait()
       /* } else {
            printf("ERROR: Wait failed. Condition or lock owner error. Terminating Nachos.\n");
            interrupt->Halt();
        }*/

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

        DEBUG('r', "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");

        // Read the message
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        int machineID = inPktHdr.from;
        unsigned int conditionKey = atoi(strtok(NULL,","));
        unsigned int lockKey = atoi(strtok(NULL,","));
        DEBUG('r', "Signal - process %d thread %d conditionKey %d lockKey %d\n", processID, threadID, conditionKey, lockKey);

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();
        networkLockTable->tableLock->Acquire();

        condition = networkConditionTable->conditions[conditionKey];
        if (condition == NULL) {
            printf("ERROR: Signal failed. No such condition. Terminating Nachos.\n");
            interrupt->Halt();
        }

        lock = networkLockTable->locks[lockKey];
        if (lock == NULL) {
            printf("ERROR: Signal failed. No such lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

        //if (condition->IsOwner(machineID) && lock->IsOwner(machineID)) {
            condition->Signal(machineID, processID, threadID, lock);
            // Response(s) sent from Signal()
       /* } else {
            printf("ERROR: Signal failed. Condition or lock owner error. Terminating Nachos.\n");
            interrupt->Halt();
        }*/

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
        int machineID = inPktHdr.from;
        unsigned int conditionKey = atoi(strtok(NULL,","));
        unsigned int lockKey = atoi(strtok(NULL,","));
        DEBUG('r', "Broadcast - process %d thread %d conditionKey %d lockKey %d\n", processID, threadID, conditionKey, lockKey);

        // Process the message (identical to original syscall)
        networkConditionTable->tableLock->Acquire();
        networkLockTable->tableLock->Acquire();

        condition = networkConditionTable->conditions[conditionKey];
        if (condition == NULL) {
            printf("ERROR: Broadcast failed. No such condition. Terminating Nachos.\n");
            interrupt->Halt();
        }

        lock = networkLockTable->locks[lockKey];
        if (lock == NULL) {
            printf("ERROR: Broadcast failed. No such lock. Terminating Nachos.\n");
            interrupt->Halt();
        }

       // if (condition->IsOwner(machineID) && lock->IsOwner(machineID)) {
            condition->Broadcast(machineID,processID, threadID, lock);
            // Response(s) sent from Broadcast()
       /* } else {
            printf("ERROR: Broadcast failed. Condition or lock owner error. Terminating Nachos.\n");
            interrupt->Halt();
        }*/

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

void RPCServer::Receive_CreateMV() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxCreateMV, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        char *name = strtok(NULL,",");
        
        int foundKey = -1;
        std::map<int, NetworkMV*>::iterator iterator;
        for(iterator = networkMVTable->mvs.begin(); iterator != networkMVTable->mvs.end(); iterator++) {
            if(iterator->second->name == name) {
                foundKey = iterator->first;
                break;
            }
        }
        if(foundKey != -1) {
            DEBUG('r', "CreateMV - found MV already created with key: %i\n", foundKey);
            SendResponse(inPktHdr.from, inMailHdr.from, foundKey);
            continue;
        }

        // Process the message (identical to original syscall)
        NetworkMV *mv = new NetworkMV(inPktHdr.from, processID, name);
        networkMVTable->tableLock->Acquire();
        int key = networkMVTable->index;
        //networkMVTable->mvs[key] = mv->Get(processID);
        networkMVTable->mvs[key] = mv;
        networkMVTable->index++;
        networkMVTable->tableLock->Release();

        // Reply with the key
        DEBUG('r', "CreateMV - process %d thread %d key %d (new)\n", processID, threadID, key);
        SendResponse(inPktHdr.from, inMailHdr.from, key);
    }
}

void RPCServer::Receive_DestroyMV() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    NetworkMV *mv;

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxDestroyMV, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        unsigned int key = atoi(strtok(NULL,","));
        DEBUG('r', "DestroyMV - process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkMVTable->tableLock->Acquire();

        mv = networkMVTable->mvs[key];
        if (mv != NULL) {
            //if (mv->IsOwner(processID))
                networkMVTable->mvs.erase(key);
           // else
           //   printf("WARN: DestroyMV failed. Client does not own MV.\n");  
        } else
            printf("WARN: DestroyMV failed. No such MV.\n");
        networkMVTable->tableLock->Release();
    }
}

void RPCServer::Receive_GetMV() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    NetworkMV *mv;
    int value;
    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxGetMV, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        unsigned int key = atoi(strtok(NULL,","));
        DEBUG('r', "GetMV - process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkMVTable->tableLock->Acquire();

        mv = networkMVTable->mvs[key];
        if (mv != NULL) {
            SendResponse(inPktHdr.from, inMailHdr.from, mv->value);
        } else
            printf("WARN: GetMV failed. No such MV.\n");

        networkMVTable->tableLock->Release();
    }
}

void RPCServer::Receive_SetMV() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    NetworkMV *mv;
    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxSetMV, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int processID = atoi(strtok(recv,","));
        int threadID = atoi(strtok(NULL,","));
        unsigned int key = atoi(strtok(NULL,","));
        unsigned int value = atoi(strtok(NULL,","));
        DEBUG('r', "GetMV - process %d thread %d key %d\n", processID, threadID, key);

        // Process the message (identical to original syscall)
        networkMVTable->tableLock->Acquire();

        mv = networkMVTable->mvs[key];
        if (mv != NULL) {
            mv->value = value;
        } else
            printf("WARN: GetMV failed. No such MV.\n");

        networkMVTable->tableLock->Release();
    }
}

//-----------------------------------------------------------------------------------------------//
// When replying to the client, must reply to the individual process/thread box.
//-----------------------------------------------------------------------------------------------//
int RPCServer::ClientMailbox(int machineID, int process, int thread) {
    char str1[2];
    char str2[2];
    char str3[2];
    
    // Have to add 1 because process and thread and machineID can both be 0, which would create overlap
    sprintf(str1, "%d", (process + 1));
    sprintf(str2, "%d", (thread + 1));
    sprintf(str3, "%d", (machineID + 1));

    strcat(str1, str2);
    strcat(str1, str3);
    int mailbox = atoi(str1);

    return mailbox;
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
static void DummyReceive_CreateMV(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_CreateMV(); }
static void DummyReceive_DestroyMV(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_DestroyMV(); }
static void DummyReceive_GetMV(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_GetMV(); }
static void DummyReceive_SetMV(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_SetMV(); }

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

    Thread *tCreateMV = new Thread("CreateMV thread");
    tCreateMV->Fork(DummyReceive_CreateMV, (int) rpcServer);

    Thread *tDestroyMV = new Thread("DestroyMV thread");
    tDestroyMV->Fork(DummyReceive_DestroyMV, (int) rpcServer);

    Thread *tGetMV = new Thread("GetMV thread");
    tGetMV->Fork(DummyReceive_GetMV, (int) rpcServer);

    Thread *tSetMV = new Thread("SetMV thread");
    tSetMV->Fork(DummyReceive_SetMV, (int) rpcServer);
}

//-----------------------------------------------------------------------------------------------//
// Create NetworkLock
//-----------------------------------------------------------------------------------------------//

NetworkLock::NetworkLock(int _machineID, int process, char* _name) {
    machineID = _machineID;
    processID = process;
    threadID = -1;
    mailboxID = -1;
    name = _name;
    queue = new List;
}

NetworkLock::~NetworkLock() {
    delete queue;
}

void NetworkLock::Acquire(int _machineID, int process, int thread) {
    // no more owners
    /*if (machineID != _machineID) {
        printf("ERROR: Acquire failed. Owner error. Terminating Nachos.\n");
        interrupt->Halt();
    }*/
    int mailbox = RPCServer::ClientMailbox(_machineID, process, thread); //get mailbox id of client calling acquire
    
    // Do the regular Lock::Acquire stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (mailboxID == -1) {                               // lock is available
        mailboxID = mailbox;
        DEBUG('r', "Acquire success machine %d process %d thread %d\n", machineID, process, thread);
        RPCServer::SendResponse(machineID, mailboxID, -1);
    } else if (threadID != thread) {                    // lock is busy
        queue->Append((void *) mailbox);                 // add mailbox ID to the wait queue
        DEBUG('r', "Acquire waiting process %d thread %d\n", process, thread);
    } else {
        printf("WARN: Acquire duplicate process %d thread %d\n", process, thread);
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

void NetworkLock::Release(int _machineID, int process, int thread) {
    // no more owners
    /*if (machineID != _machineID) {
        printf("ERROR: Acquire failed. Owner error. Terminating Nachos.\n");
        interrupt->Halt();
    }
    */
    int mailbox = RPCServer::ClientMailbox(_machineID, process, thread); //get mailbox id of client calling release
    
    // Do the regular Lock::Release stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (mailboxID == mailbox) {
        if (queue->IsEmpty()) {
            mailboxID = -1; //no mailbox's are waiting
        }
        else {
            mailboxID = (int) queue->Remove();           // wake up a waiting mailbox
            DEBUG('r', "Acquire success via release process %d thread %d mailbox %d\n", process, threadID, mailboxID);
            RPCServer::SendResponse(machineID, mailboxID, -1);
        }
    } else {
        printf("WARN: Release without acquire process %d thread %d\n", process, thread);
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

// bool NetworkLock::IsOwner(int _machineID) {
//     return (machineID == _machineID);
// }

bool NetworkLock::HasAcquired(int mailbox) {
    return (mailboxID == mailbox);
}

//-----------------------------------------------------------------------------------------------//
// Create NetworkCondition
//-----------------------------------------------------------------------------------------------//

NetworkCondition::NetworkCondition(int _machineID, int process, char* _name) {
    machineID = _machineID;
    processID = process;
    name = _name;
    conditionLock = NULL;
    queue = new List;
}

NetworkCondition::~NetworkCondition() {
    delete conditionLock;
    delete queue;
}

void NetworkCondition::Wait(int _machineID, int process, int thread, NetworkLock* lock) {
    // Only the owner process may access this condition
    /*
    if (process != processID) {
        printf("ERROR: Wait failed. Owner error. Terminating Nachos.\n");
        interrupt->Halt();
    }
    */
    
    int mailbox = RPCServer::ClientMailbox(_machineID, process, thread); //get mailbox id of client calling wait

    // Do the regular Condition::Wait stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (lock != NULL) {
        if (/*lock->IsOwner(_machineID) && */lock->HasAcquired(mailbox)) {
            if (conditionLock == NULL)                      // condition hasn't been assigned to a lock yet
                conditionLock = lock;
                DEBUG('r', "Wait assigned lock process %d thread %d\n", processID, thread);
            if (conditionLock == lock) {                    // ok to wait
                queue->Append((void *) mailbox);             // add mailbox to wait queue
                conditionLock->Release(_machineID, process, thread);    // release waiting lock
                DEBUG('r', "Wait waiting process %d thread %d\n", processID, thread);
            } else {
                printf("ERROR: Wait failed. Wrong lock. Terminating Nachos.\n");
                interrupt->Halt();
            }
        } else {
            printf("ERROR: Wait failed. Unacquired lock. Process: %i, thread: %i, lock: %i Terminating Nachos.\n",process,thread,lock->name);
            interrupt->Halt();
        }
    } else {
        printf("ERROR: Wait failed. Lock null. Terminating Nachos.\n");
        interrupt->Halt();
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

void NetworkCondition::Signal(int _machineID, int process, int _thread, NetworkLock* lock) {
    // Only the owner process may access this condition
    /*
    if (process != processID) {
        printf("ERROR: Signal failed. Owner error. Terminating Nachos.\n");
        interrupt->Halt();
    }
    */
    
    int mailbox = RPCServer::ClientMailbox(_machineID, process, _thread); //get mailbox id of client calling signal

    // Do the regular Condition::Signal stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (lock != NULL) {
        if (/*lock->IsOwner(_machineID) && */lock->HasAcquired(mailbox)) {
            if (conditionLock == lock) {
                int thread = (int) queue->Remove();         // remove one waiting thread from queue
                DEBUG('r', "Signal thread process %d thread %d thread %d (given)\n", processID, thread, _thread);
                RPCServer::SendResponse(machineID, mailbox, -1);
                RPCServer::SendResponse(machineID, mailbox, -1);
                // That's why the second response is sent (so Signal() knows to go ahead and the thread signaled will be stuck
                // MUST Release() following this call to Signal() in exception.cc, otherwise Release())
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

void NetworkCondition::Broadcast(int _machineID, int process, int _thread, NetworkLock* lock) {
    // Only the owner process may access this condition
    /*
    if (process != processID) {
        printf("ERROR: Broadcast failed. Owner error. Terminating Nachos.\n");
        interrupt->Halt();
    }
    */
    
    int mailbox = RPCServer::ClientMailbox(_machineID, process, _thread); //get mailbox id of client calling wait

    // Do the regular Condition::Broadcast stuff
    // MUST Release() following this call to Broadcast() in exception.cc, otherwise the thread(s) signaled will be stuck
    if (lock != NULL) {
        if (/*lock->IsOwner(_machineID) && */lock->HasAcquired(mailbox)) {
            if (conditionLock == lock) {
                while (!queue->IsEmpty())                   // signal all mailbox's waiting
                    Signal(_machineID, process, _thread, lock);
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

/*
bool NetworkCondition::IsOwner(int _machineID) {
    return (machineID == _machineID);
}
*/

//-----------------------------------------------------------------------------------------------//
// Create NetworkMV
//-----------------------------------------------------------------------------------------------//

NetworkMV::NetworkMV(int _machineID, int process, char* _name) {
    machineID = _machineID;
    processID = process;
    value = 0;
    name = _name;
}

NetworkMV::~NetworkMV() {}

/*bool NetworkMV::IsOwner(int process) {
    return (processID == process);
}
*/
