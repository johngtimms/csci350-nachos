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
        int mailbox = inMailHdr.from;
        char *lockName = recv;

        // Check if the lock already exists
        NetworkLock *lock = networkLockTable->locks.at(lockName);

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxCreateLock + 100;

            if ( lock != NULL ) {
                DEBUG('r', "CreateLock (Found Remote) - mailbox %d name %s\n", mailbox, lockName);
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                SendResponse(-mailbox, -1);                         // Send a response to the original client
                continue;
            } else {
                SendResponse(replyToMachine, replyToMailbox, -2);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( lock != NULL ) {
            // We DO have it
            DEBUG('r', "CreateLock (Found Local) - mailbox %d name %s\n", mailbox, lockName);
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            int result = SendQuery(MailboxCreateLock, mailbox, lockName, "CreateLock");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, so we create a new lock (just like the original syscall)
        NetworkLock *lock = new NetworkLock(mailbox, lockName);
        networkLockTable->tableLock->Acquire();
        networkLockTable->locks.at(lockName) = lock;
        networkLockTable->tableLock->Release();

        // Reply with success
        DEBUG('r', "CreateLock (New) - mailbox %d name %s\n", mailbox, lockName);
        SendResponse(mailbox, -1);
    }
}

void RPCServer::Receive_DestroyLock() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxDestroyLock, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        char *lockName = recv;

        // Check if the lock exists
        NetworkLock *lock = networkLockTable->locks.at(lockName);

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxDestroyLock + 100;

            if ( lock != NULL ) {
                DEBUG('r', "DestroyLock (Found Remote) - mailbox %d name %s\n", mailbox, lockName);
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                
                // Process the message (identical to the original syscall)
                networkLockTable->tableLock->Acquire();
                networkLockTable->locks.erase(lockName);
                networkLockTable->tableLock->Release();

                // Send success
                SendResponse(-mailbox, -1);
                continue;
            } else {
                SendResponse(replyToMachine, replyToMailbox, -2);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( lock != NULL ) {
            // We DO have it
            DEBUG('r', "DestroyLock (Found Local) - mailbox %d name %s\n", mailbox, lockName);

            // Process the message (identical to the original syscall)
            networkLockTable->tableLock->Acquire();
            networkLockTable->locks.erase(lockName);
            networkLockTable->tableLock->Release();

            // Send success
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            int result = SendQuery(MailboxDestroyLock, mailbox, lockName, "DestroyLock");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        DEBUG('r', "WARN: DestroyLock failed. No such lock - mailbox %d name %s\n", mailbox, lockName);
        SendResponse(mailbox, -2);
    }
}

void RPCServer::Receive_Acquire() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxAcquire, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        char *lockName = recv;

        // Check if the lock already exists
        NetworkLock *lock = networkLockTable->locks.at(lockName);

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxAcquire + 100;

            if ( lock != NULL ) {
                DEBUG('r', "Acquire (Found Remote) - mailbox %d name %s\n", mailbox, lockName);
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkLockTable->tableLock->Acquire();
                lock->Acquire(-mailbox)
                networkLockTable->tableLock->Release();

                // Success response to the original client is sent from Acquire()
                continue;
            } else {
                SendResponse(replyToMachine, replyToMailbox, -2);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( lock != NULL ) {
            // We DO have it
            DEBUG('r', "Acquire (Found Local) - mailbox %d name %s\n", mailbox, lockName);

            // Handle it
            networkLockTable->tableLock->Acquire();
            lock->Acquire(mailbox)
            networkLockTable->tableLock->Release();

            // Success response is sent from Acquire()
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            int result = SendQuery(MailboxAcquire, mailbox, lockName, "Acquire");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        DEBUG('r', "ERROR: Acquire failed. Terminating Nachos. No such lock - mailbox %d name %s\n", mailbox, lockName);
        SendResponse(mailbox, -2);
        interrupt->Halt();
    }
}

void RPCServer::Receive_Release() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxRelease, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        char *lockName = recv;

        // Check if the lock already exists
        NetworkLock *lock = networkLockTable->locks.at(lockName);

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxRelease + 100;

            if ( lock != NULL ) {
                DEBUG('r', "Release (Found Remote) - mailbox %d name %s\n", mailbox, lockName);
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkLockTable->tableLock->Acquire();
                lock->Release(-mailbox);
                networkLockTable->tableLock->Release();

                // Success response to the original client is sent from Release()
                continue;
            } else {
                SendResponse(replyToMachine, replyToMailbox, -2);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( lock != NULL ) {
            // We DO have it
            DEBUG('r', "Release (Found Local) - mailbox %d name %s\n", mailbox, lockName);

            // Handle it
            networkLockTable->tableLock->Acquire();
            lock->Release(mailbox);
            networkLockTable->tableLock->Release();

            // Success response is sent from Release()
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            int result = SendQuery(MailboxRelease, mailbox, lockName, "Release");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        DEBUG('r', "ERROR: Release failed. Terminating Nachos. No such lock - mailbox %d name %s\n", mailbox, lockName);
        SendResponse(mailbox, -2);
        interrupt->Halt();
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
        int mailbox = inMailHdr.from;
        char *conditionName = recv;

        // Check if the condition already exists
        NetworkCondition *condition = networkConditionTable->conditions.at(conditionName);

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxCreateCondition + 100;

            if ( condition != NULL ) {
                DEBUG('r', "CreateCondition (Found Remote) - mailbox %d name %s\n", mailbox, conditionName);
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                SendResponse(-mailbox, -1);                         // Send a response to the original client
                continue;
            } else {
                SendResponse(replyToMachine, replyToMailbox, -2);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( condition != NULL ) {
            // We DO have it
            DEBUG('r', "CreateCondition (Found Local) - mailbox %d name %s\n", mailbox, conditionName);
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            int result = SendQuery(MailboxCreateCondition, mailbox, conditionName, "CreateCondition");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, so we create a new condition (just like the original syscall)
        NetworkCondition *condition = new NetworkCondition(mailbox, conditionName);
        networkConditionTable->tableLock->Acquire();
        networkConditionTable->conditions.at(conditionName) = condition;
        networkConditionTable->tableLock->Release();

        // Reply with success
        DEBUG('r', "CreateCondition (New) - mailbox %d name %s\n", mailbox, conditionName);
        SendResponse(mailbox, -1);
    }
}

void RPCServer::Receive_DestroyCondition() {    
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxDestroyCondition, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        char *conditionName = recv;

        // Check if the condition exists
        NetworkCondition *condition = networkConditionTable->conditions.at(conditionName);

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxDestroyCondition + 100;

            if ( condition != NULL ) {
                DEBUG('r', "DestroyCondition (Found Remote) - mailbox %d name %s\n", mailbox, conditionName);
                
                // Process the message (identical to the original syscall)
                networkConditionTable->tableLock->Acquire();
                networkConditionTable->conditions.erase(conditionName);
                networkConditionTable->tableLock->Release();

                // Send success
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                SendResponse(-mailbox, -1);                         // Send a response to the original client
                continue;
            } else {
                SendResponse(replyToMachine, replyToMailbox, -2);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( condition != NULL ) {
            // We DO have it
            DEBUG('r', "DestroyCondition (Found Local) - mailbox %d name %s\n", mailbox, conditionName);

            // Process the message (identical to the original syscall)
            networkConditionTable->tableLock->Acquire();
            networkConditionTable->conditions.erase(conditionName);
            networkConditionTable->tableCondition->Release();

            // Send success
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            int result = SendQuery(MailboxCreateCondition, mailbox, conditionName, "DestroyCondition");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        DEBUG('r', "WARN: DestroyCondition failed. No such condition - mailbox %d name %s\n", mailbox, conditionName);
        SendResponse(mailbox, -2);
    }
}

void RPCServer::Receive_Wait() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxWait, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        char *conditionName = strtok(recv,",");
        char *lockName = strtok(NULL,",");

        // Check if the condition already exists (and get the lock, because we assume they are on the same server)
        NetworkCondition *condition = networkConditionTable->conditions.at(conditionName);
        NetworkLock *lock = networkLockTable->locks.at(lockName);

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxWait + 100;

            if ( condition != NULL ) {
                DEBUG('r', "Wait (Found Remote) - mailbox %d name %s\n", mailbox, conditionName);
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkConditionTable->tableLock->Acquire();
                networkLockTable->tableLock->Acquire();

                if (lock == NULL) {
                    DEBUG('r', "ERROR: Wait (Remote) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName);
                    SendResponse(-mailbox, -2);
                    interrupt->Halt();
                }

                condition->Wait(-mailbox, lock);

                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // Success response to the original client is sent from Wait()
                continue;
            } else {
                SendResponse(replyToMachine, replyToMailbox, -2);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( condition != NULL ) {
            // We DO have it
            DEBUG('r', "Wait (Found Local) - mailbox %d name %s\n", mailbox, conditionName);

            // Handle it
            networkConditionTable->tableLock->Acquire();
            networkLockTable->tableLock->Acquire();

            if (lock == NULL) {
                DEBUG('r', "ERROR: Wait (Local) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName);
                SendResponse(mailbox, -2);
                interrupt->Halt();
            }

            condition->Wait(mailbox, lock);

            networkConditionTable->tableLock->Release();
            networkLockTable->tableLock->Release();

            // Success response to the original client is sent from Wait()
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            int result = SendQuery(MailboxWait, mailbox, conditionName, "Wait");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        DEBUG('r', "ERROR: Wait failed. Terminating Nachos. No such condition - mailbox %d name %s\n", mailbox, conditionName);
        SendResponse(mailbox, -2);
        interrupt->Halt();
    }
}

void RPCServer::Receive_Signal() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxWait, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        char *conditionName = strtok(recv,",");
        char *lockName = strtok(NULL,",");

        // Check if the condition already exists (and get the lock, because we assume they are on the same server)
        NetworkCondition *condition = networkConditionTable->conditions.at(conditionName);
        NetworkLock *lock = networkLockTable->locks.at(lockName);

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxWait + 100;

            if ( condition != NULL ) {
                DEBUG('r', "Signal (Found Remote) - mailbox %d name %s\n", mailbox, conditionName);
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkConditionTable->tableLock->Acquire();
                networkLockTable->tableLock->Acquire();

                if (lock == NULL) {
                    DEBUG('r', "ERROR: Signal (Remote) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName);
                    SendResponse(-mailbox, -2);
                    interrupt->Halt();
                }

                condition->Signal(-mailbox, lock);

                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // Success response to the original client is sent from Signal()
                continue;
            } else {
                SendResponse(replyToMachine, replyToMailbox, -2);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( condition != NULL ) {
            // We DO have it
            DEBUG('r', "Signal (Found Local) - mailbox %d name %s\n", mailbox, conditionName);

            // Handle it
            networkConditionTable->tableLock->Acquire();
            networkLockTable->tableLock->Acquire();

            if (lock == NULL) {
                DEBUG('r', "ERROR: Signal (Local) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName);
                SendResponse(mailbox, -2);
                interrupt->Halt();
            }

            condition->Signal(mailbox, lock);

            networkConditionTable->tableLock->Release();
            networkLockTable->tableLock->Release();

            // Success response to the original client is sent from Signal()
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            int result = SendQuery(MailboxSignal, mailbox, conditionName, "Signal");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        DEBUG('r', "ERROR: Signal failed. Terminating Nachos. No such condition - mailbox %d name %s\n", mailbox, conditionName);
        SendResponse(mailbox, -2);
        interrupt->Halt();
    }
}

void RPCServer::Receive_Broadcast() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxWait, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        char *conditionName = strtok(recv,",");
        char *lockName = strtok(NULL,",");

        // Check if the condition already exists (and get the lock, because we assume they are on the same server)
        NetworkCondition *condition = networkConditionTable->conditions.at(conditionName);
        NetworkLock *lock = networkLockTable->locks.at(lockName);

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxWait + 100;

            if ( condition != NULL ) {
                DEBUG('r', "Broadcast (Found Remote) - mailbox %d name %s\n", mailbox, conditionName);
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkConditionTable->tableLock->Acquire();
                networkLockTable->tableLock->Acquire();

                if (lock == NULL) {
                    DEBUG('r', "ERROR: Broadcast (Remote) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName);
                    SendResponse(-mailbox, -2);
                    interrupt->Halt();
                }

                condition->Broadcast(-mailbox, lock);

                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // Success response to the original client is sent from Broadcast()
                continue;
            } else {
                SendResponse(replyToMachine, replyToMailbox, -2);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( condition != NULL ) {
            // We DO have it
            DEBUG('r', "Broadcast (Found Local) - mailbox %d name %s\n", mailbox, conditionName);

            // Handle it
            networkConditionTable->tableLock->Acquire();
            networkLockTable->tableLock->Acquire();

            if (lock == NULL) {
                DEBUG('r', "ERROR: Broadcast (Local) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName);
                SendResponse(mailbox, -2);
                interrupt->Halt();
            }

            condition->Broadcast(mailbox, lock);

            networkConditionTable->tableLock->Release();
            networkLockTable->tableLock->Release();

            // Success response to the original client is sent from Broadcast()
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            int result = SendQuery(MailboxBroadcast, mailbox, conditionName, "Broadcast");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        DEBUG('r', "ERROR: Broadcast failed. Terminating Nachos. No such condition - mailbox %d name %s\n", mailbox, conditionName);
        SendResponse(mailbox, -2);
        interrupt->Halt();
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
// When replying to the client, must reply to the individual process/thread mailbox.
//-----------------------------------------------------------------------------------------------//
int RPCServer::ClientMailbox(int machine, int process, int thread) {
    char str1[2];
    char str2[2];
    char str3[2];
    
    // Have to add 1 because process, thread, and machine can all be 0, which would create overlap
    sprintf(str1, "%d", (process + 1));
    sprintf(str2, "%d", (thread + 1));
    sprintf(str3, "%d", (machine + 1));

    strcat(str1, str2);
    strcat(str1, str3);
    int mailbox = atoi(str1);

    DEBUG('p', "mailbox calculated %d", mailbox);

    return mailbox;
}

//-----------------------------------------------------------------------------------------------//
// A lot of calls need to reply "yes" or "no" when they're done, this promotes code reuse.
// It also handles numeric responses.
// For "yes" pass -1 as the response. 
// For "no" pass -2 as the response.
// For a numeric response, pass a number greater than or equal to zero as the response.
//-----------------------------------------------------------------------------------------------//
void RPCServer::SendResponse(int mailbox, int response, int machine) {
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    char send[MaxMailSize];

    // Construct response
    if (response == -2)
        strcpy(send, "no");
    else if (response == -1)
        strcpy(send, "yes");
    else if (response >= 0)
        sprintf(send, "%d", response);
    else {
        printf("ERROR: Invalid response. Mailbox %d. Response %d. Terminating Nachos.\n", mailbox, response);
        interrupt->Halt();
    }

    // Calculate machine ID from the mailbox if this is a Server-to-Client response
    if (machine == -1) {
        machine = (mailbox % 10000) - 1;
    }

    DEBUG('r', "Send response machine %d mailbox %d response %d send \"%s\"\n", machine, mailbox, response, send);

    // Construct packet header, mail header for the message
    outPktHdr.to = machine;       
    outMailHdr.to = mailbox;
    outMailHdr.from = -1; // No syscall ever needs to reply to a response
    outMailHdr.length = strlen(send) + 1;

    // Send the response message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

    if ( !success )
        printf("WARN: SendResponse failed. Client misconfigured.\n");
    else
        DEBUG('r', "Send response success\n");
}

//-----------------------------------------------------------------------------------------------//
// SendQuery - Sends a Server-to-Server query
//
// Automatically queries all servers 
//
// mailboxTo is one of the defined mailbox numbers for RPCs
// mailboxFrom is the Server-to-Client mailbox, which will be negated
// The negation of mailboxFrom tells the queried server this is a Server-to-Server call
// query is the original query from the client
// identifier is a description for debugging
//-----------------------------------------------------------------------------------------------//
bool RPCServer::SendQuery(int mailboxTo, int mailboxFrom, char *query, char *identifier) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];
    char test[MaxMailSize]; strcpy(test, "yes");

    // If I am the only server, return false
    if (numServers == 1) {
        return 1;
    }

    // Send the query with no modifications
    sprintf(send, "%s", query);

    // Send the query to all servers until out of servers or get a "yes"
    for (int serverToQuery = 0; serverToQuery <= 4; serverToQuery++) {
        // Don't send a query to ourselves
        if (machineName == serverToQuery) {
            continue;
        }

        // Construct the packet header, mail header for the message
        outPktHdr.to = serverToQuery;
        outMailHdr.to = mailboxTo;
        outMailHdr.from = -mailboxFrom; // MAILBOX IS NEGATED HERE. DO NOT PRE-NEGATE.
        outMailHdr.length = strlen(send) + 1;

        // Send the query message
        bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

        // Check that the send worked
        if ( !success )
            printf("WARN: %s (SendQuery) failed. Server %d misconfigured.\n", identifier, serverToQuery);

        // Get the response back
        // Use mailboxTo + 100 to listen for "yes" or "no"
        int mailbox = mailboxTo + 100;
        postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);

        // If we get a "yes", then return 0. Otherwise keep trying.
        // A "yes" means the other server is handling the request
        if ( strcmp(test,recv) )
            return 0;
    }

    // Query was ultimately unsuccessful 
    return 1;
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

    if (numServers > 0 && numServers < 6) {
        printf("SERVER RUNNING - numServers is %d\n", numServers);
    } else {
        printf("OTHER SERVERS UNKNOWN. NACHOS HALTING.\n");
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
