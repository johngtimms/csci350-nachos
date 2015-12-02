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

        DEBUG('r', "0");

        // Wait for a mailbox message
        postOffice->Receive(MailboxCreateLock, &inPktHdr, &inMailHdr, recv);

        DEBUG('r', "1");

        // Read the message
        int mailbox = inMailHdr.from;
        std::string lockName(recv);

        // DEBUG('r', "{%d}",mailbox);
        // DEBUG('r', "2");

        // printf("\t CreateLock - there are %d locks\n", networkLockTable->locks.size());
        // for (std::map<char*, NetworkLock*>::iterator it = networkLockTable->locks.begin(); it != networkLockTable->locks.end(); ++it) {
        //     printf("\t CreateLock - %s / %s\n", it->first, it->second);
        //     printf("\t RELEASE - %d \n", it->second == NULL);

        //     if (it->first == lockName) {
        //         printf("HERE!!\n");
        //     }
        // }

        // Check if the lock already exists
        NetworkLock *lock = networkLockTable->locks[lockName];

        DEBUG('r', "3");

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            DEBUG('r', "a");
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxCreateLock + 100;

            if ( lock != NULL ) {
                DEBUG('n', "CreateLock (Found Remote) - mailbox %d name %s\n", mailbox, lockName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                SendResponse(-mailbox, -1);                         // Send a response to the original client
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

DEBUG('r', "4");
        // This is Client-to-Server
        if ( lock != NULL ) {
            DEBUG('r', "a");
            // We DO have it
            DEBUG('n', "CreateLock (Found Local) - mailbox %d name %s\n", mailbox, lockName.c_str());
            SendResponse(mailbox, -1);
            DEBUG('r', "b");
            continue;
        } else {
            DEBUG('r', "z");
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxCreateLock, mailbox, lockName, "CreateLock");
            DEBUG('r', "[%d]",result);
            if (result)
                continue; // One of the other servers had it, we can finish
        }

DEBUG('r', "5");
        // This is Client-to-Server, no other server has it, so we create a new lock (just like the original syscall)
        lock = new NetworkLock(lockName);
        networkLockTable->tableLock->Acquire();
        networkLockTable->locks[lockName] = lock;
        networkLockTable->tableLock->Release();

        printf("\t CreateLock - there are %d locks\n", networkLockTable->locks.size());
        for (std::map<std::string, NetworkLock*>::iterator it = networkLockTable->locks.begin(); it != networkLockTable->locks.end(); ++it) {
            printf("\t CreateLock - %s / %s\n", it->first.c_str(), it->second);
            printf("\t RELEASE - %d \n", it->second == NULL);

            if (it->first == lockName) {
                printf("HERE!!\n");
            }
        }

        DEBUG('r', "6");

        // Reply with success
        DEBUG('n', "CreateLock (New) - mailbox %d name %s\n", mailbox, lockName.c_str());
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
        std::string lockName(recv);

        // Check if the lock exists
        NetworkLock *lock = networkLockTable->locks[lockName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxDestroyLock + 100;

            if ( lock != NULL ) {
                DEBUG('n', "DestroyLock (Found Remote) - mailbox %d name %s\n", mailbox, lockName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                
                // Process the message (identical to the original syscall)
                networkLockTable->tableLock->Acquire();
                networkLockTable->locks.erase(lockName);
                networkLockTable->tableLock->Release();

                // Send success
                SendResponse(-mailbox, -1);
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( lock != NULL ) {
            // We DO have it
            DEBUG('n', "DestroyLock (Found Local) - mailbox %d name %s\n", mailbox, lockName.c_str());

            // Process the message (identical to the original syscall)
            networkLockTable->tableLock->Acquire();
            networkLockTable->locks.erase(lockName);
            networkLockTable->tableLock->Release();

            // Send success
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxDestroyLock, mailbox, lockName, "DestroyLock");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("WARN: DestroyLock failed. No such lock - mailbox %d name %s\n", mailbox, lockName.c_str());
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
        std::string lockName(recv);

        // Check if the lock exists
        NetworkLock *lock = networkLockTable->locks[lockName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxAcquire + 100;

            if ( lock != NULL ) {
                DEBUG('r', "Acquire (Found Remote) - mailbox %d name %s\n", mailbox, lockName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkLockTable->tableLock->Acquire();
                lock->Acquire(-mailbox);
                networkLockTable->tableLock->Release();

                // Success response to the original client is sent from Acquire()
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( lock != NULL ) {
            // We DO have it
            DEBUG('r', "Acquire (Found Local) - mailbox %d name %s\n", mailbox, lockName.c_str());

            // Handle it
            networkLockTable->tableLock->Acquire();
            lock->Acquire(mailbox);
            networkLockTable->tableLock->Release();

            // Success response is sent from Acquire()
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxAcquire, mailbox, lockName, "Acquire");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("ERROR: Acquire failed. Terminating Nachos. No such lock - mailbox %d name %s\n", mailbox, lockName.c_str());
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
        std::string lockName(recv);

        // Check if the lock exists
        NetworkLock *lock = networkLockTable->locks[lockName];

        // DEBUG('r', "RELEASE - there are %d locks\n", networkLockTable->locks.size());
        // for (int i = 0; i < networkLockTable->locks.size(), i++) {
        //     DEBUG('r', "RELEASE - %s", (*networkLockTable->locks[i]).getName());
        // }

        DEBUG('r', "RELEASE - there are %d locks\n", networkLockTable->locks.size());
        for (std::map<std::string, NetworkLock*>::iterator it = networkLockTable->locks.begin(); it != networkLockTable->locks.end(); ++it) {
            DEBUG('r', "RELEASE - %s / %s\n", it->first.c_str(), it->first.c_str());
            printf("\t RELEASE - %d \n", it->second == NULL);

            if (it->first == lockName) {
                printf("HERE!!\n");
                lock = it->second;
            }
        }

        NetworkLock *lockNull = networkLockTable->locks["locknull"];
        NetworkLock *lockOne = networkLockTable->locks["lockOne"];
        NetworkLock *lockTwo = networkLockTable->locks["lockTwo"];

        printf("\t null %d \n", lockNull == NULL);
        printf("\t one %d \n", lockOne == NULL);
        printf("\t two %d \n", lockTwo == NULL);

        // for (std::map<char,int>::iterator it=mymap.begin(); it!=mymap.end(); ++it)
        //     std::cout << it->first << " => " << it->second << '\n';

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxRelease + 100;

            if ( lock != NULL ) {
                DEBUG('r', "Release (Found Remote) - mailbox %d name %s\n", mailbox, lockName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkLockTable->tableLock->Acquire();
                lock->Release(-mailbox);
                networkLockTable->tableLock->Release();

                // Success response to the original client is sent from Release()
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( lock != NULL ) {
            // We DO have it
            DEBUG('r', "Release (Found Local) - mailbox %d name %s\n", mailbox, lockName.c_str());

            // Handle it
            networkLockTable->tableLock->Acquire();
            lock->Release(mailbox);
            networkLockTable->tableLock->Release();

            // Success response is sent from Release()
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxRelease, mailbox, lockName, "Release");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("ERROR: Release failed. Terminating Nachos. No such lock - mailbox %d name %s\n", mailbox, lockName.c_str());
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
        std::string conditionName(recv);

        // Check if the condition already exists
        NetworkCondition *condition = networkConditionTable->conditions[conditionName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxCreateCondition + 100;

            if ( condition != NULL ) {
                DEBUG('r', "CreateCondition (Found Remote) - mailbox %d name %s\n", mailbox, conditionName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                SendResponse(-mailbox, -1);                         // Send a response to the original client
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( condition != NULL ) {
            // We DO have it
            DEBUG('r', "CreateCondition (Found Local) - mailbox %d name %s\n", mailbox, conditionName.c_str());
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxCreateCondition, mailbox, conditionName, "CreateCondition");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, so we create a new condition (just like the original syscall)
        condition = new NetworkCondition(conditionName);
        networkConditionTable->tableLock->Acquire();
        networkConditionTable->conditions[conditionName] = condition;
        networkConditionTable->tableLock->Release();

        // Reply with success
        DEBUG('r', "CreateCondition (New) - mailbox %d name %s\n", mailbox, conditionName.c_str());
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
        std::string conditionName(recv);

        // Check if the condition exists
        NetworkCondition *condition = networkConditionTable->conditions[conditionName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxDestroyCondition + 100;

            if ( condition != NULL ) {
                DEBUG('r', "DestroyCondition (Found Remote) - mailbox %d name %s\n", mailbox, conditionName.c_str());
                
                // Process the message (identical to the original syscall)
                networkConditionTable->tableLock->Acquire();
                networkConditionTable->conditions.erase(conditionName);
                networkConditionTable->tableLock->Release();

                // Send success
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                SendResponse(-mailbox, -1);                         // Send a response to the original client
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( condition != NULL ) {
            // We DO have it
            DEBUG('r', "DestroyCondition (Found Local) - mailbox %d name %s\n", mailbox, conditionName.c_str());

            // Process the message (identical to the original syscall)
            networkConditionTable->tableLock->Acquire();
            networkConditionTable->conditions.erase(conditionName);
            networkConditionTable->tableLock->Release();

            // Send success
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxCreateCondition, mailbox, conditionName, "DestroyCondition");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("WARN: DestroyCondition failed. No such condition - mailbox %d name %s\n", mailbox, conditionName.c_str());
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
        std::string conditionName(strtok(recv, ","));
        std::string lockName(strtok(NULL, ","));

        // Check if the condition exists (and get the lock, because we assume they are on the same server)
        NetworkCondition *condition = networkConditionTable->conditions[conditionName];
        NetworkLock *lock = networkLockTable->locks[lockName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxWait + 100;

            if ( condition != NULL ) {
                DEBUG('r', "Wait (Found Remote) - mailbox %d name %s\n", mailbox, conditionName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkConditionTable->tableLock->Acquire();
                networkLockTable->tableLock->Acquire();

                if (lock == NULL) {
                    printf("ERROR: Wait (Remote) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
                    SendResponse(-mailbox, -2);
                    interrupt->Halt();
                }

                condition->Wait(-mailbox, lock);

                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // Success response to the original client is sent from Wait()
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( condition != NULL ) {
            // We DO have it
            DEBUG('r', "Wait (Found Local) - mailbox %d name %s\n", mailbox, conditionName.c_str());

            // Handle it
            networkConditionTable->tableLock->Acquire();
            networkLockTable->tableLock->Acquire();

            if (lock == NULL) {
                printf("ERROR: Wait (Local) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
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
            bool result = SendQuery(MailboxWait, mailbox, conditionName, "Wait");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("ERROR: Wait failed. Terminating Nachos. No such condition - mailbox %d name %s\n", mailbox, conditionName.c_str());
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
        std::string conditionName(strtok(recv, ","));
        std::string lockName(strtok(NULL, ","));

        // Check if the condition exists (and get the lock, because we assume they are on the same server)
        NetworkCondition *condition = networkConditionTable->conditions[conditionName];
        NetworkLock *lock = networkLockTable->locks[lockName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxWait + 100;

            if ( condition != NULL ) {
                DEBUG('r', "Signal (Found Remote) - mailbox %d name %s\n", mailbox, conditionName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkConditionTable->tableLock->Acquire();
                networkLockTable->tableLock->Acquire();

                if (lock == NULL) {
                    printf("ERROR: Signal (Remote) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
                    SendResponse(-mailbox, -2);
                    interrupt->Halt();
                }

                condition->Signal(-mailbox, lock);

                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // Success response to the original client is sent from Signal()
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( condition != NULL ) {
            // We DO have it
            DEBUG('r', "Signal (Found Local) - mailbox %d name %s\n", mailbox, conditionName.c_str());

            // Handle it
            networkConditionTable->tableLock->Acquire();
            networkLockTable->tableLock->Acquire();

            if (lock == NULL) {
                printf("ERROR: Signal (Local) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
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
            bool result = SendQuery(MailboxSignal, mailbox, conditionName, "Signal");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("ERROR: Signal failed. Terminating Nachos. No such condition - mailbox %d name %s\n", mailbox, conditionName.c_str());
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
        std::string conditionName(strtok(recv, ","));
        std::string lockName(strtok(NULL, ","));

        // Check if the condition exists (and get the lock, because we assume they are on the same server)
        NetworkCondition *condition = networkConditionTable->conditions[conditionName];
        NetworkLock *lock = networkLockTable->locks[lockName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxWait + 100;

            if ( condition != NULL ) {
                DEBUG('r', "Broadcast (Found Remote) - mailbox %d name %s\n", mailbox, conditionName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkConditionTable->tableLock->Acquire();
                networkLockTable->tableLock->Acquire();

                if (lock == NULL) {
                    printf("ERROR: Broadcast (Remote) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
                    SendResponse(-mailbox, -2);
                    interrupt->Halt();
                }

                condition->Broadcast(-mailbox, lock);

                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // Success response to the original client is sent from Broadcast()
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( condition != NULL ) {
            // We DO have it
            DEBUG('r', "Broadcast (Found Local) - mailbox %d name %s\n", mailbox, conditionName.c_str());

            // Handle it
            networkConditionTable->tableLock->Acquire();
            networkLockTable->tableLock->Acquire();

            if (lock == NULL) {
                printf("ERROR: Broadcast (Local) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
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
            bool result = SendQuery(MailboxBroadcast, mailbox, conditionName, "Broadcast");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("ERROR: Broadcast failed. Terminating Nachos. No such condition - mailbox %d name %s\n", mailbox, conditionName.c_str());
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


        // // Test sending
        // PacketHeader outPktHdr;
        // MailHeader outMailHdr;
        // char send[MaxMailSize];

        // sprintf(send, "101510");

        // outPktHdr.to = 0;
        // outMailHdr.to = 20;
        // outMailHdr.from = 21;
        // outMailHdr.length = strlen(send) + 1;

        // postOffice->Send(outPktHdr, outMailHdr, send);
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
        int mailbox = inMailHdr.from;
        std::string mvName(recv);

        // Check if the mv already exists
        NetworkMV *mv = networkMVTable->mvs[mvName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxCreateMV + 100;

            if ( mv != NULL ) {
                DEBUG('r', "CreateMV (Found Remote) - mailbox %d name %s\n", mailbox, mvName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                SendResponse(-mailbox, -1);                         // Send a response to the original client
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( mv != NULL ) {
            // We DO have it
            DEBUG('r', "CreateMV (Found Local) - mailbox %d name %s\n", mailbox, mvName.c_str());
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxCreateMV, mailbox, mvName, "CreateMV");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, so we create a new mv (just like the original syscall)
        mv = new NetworkMV(mvName);
        networkMVTable->tableLock->Acquire();
        networkMVTable->mvs[mvName] = mv;
        networkMVTable->tableLock->Release();

        // Reply with success
        DEBUG('r', "CreateMV (New) - mailbox %d name %s\n", mailbox, mvName.c_str());
        SendResponse(mailbox, -1);
    }
}

void RPCServer::Receive_DestroyMV() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxDestroyMV, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        std::string mvName(recv);

        // Check if the mv exists
        NetworkMV *mv = networkMVTable->mvs[mvName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxDestroyMV + 100;

            if ( mv != NULL ) {
                DEBUG('r', "DestroyMV (Found Remote) - mailbox %d name %s\n", mailbox, mvName.c_str());
                
                // Process the message (identical to the original syscall)
                networkMVTable->tableLock->Acquire();
                networkMVTable->mvs.erase(mvName);
                networkMVTable->tableLock->Release();

                // Send success
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                SendResponse(-mailbox, -1);                         // Send a response to the original client
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( mv != NULL ) {
            // We DO have it
            DEBUG('r', "DestroyMV (Found Local) - mailbox %d name %s\n", mailbox, mvName.c_str());

            // Process the message (identical to the original syscall)
            networkMVTable->tableLock->Acquire();
            networkMVTable->mvs.erase(mvName);
            networkMVTable->tableLock->Release();

            // Send success
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxCreateMV, mailbox, mvName, "DestroyMV");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("WARN: DestroyMV failed. No such mv - mailbox %d name %s\n", mailbox, mvName.c_str());
        SendResponse(mailbox, -2);
    }
}

void RPCServer::Receive_GetMV() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxGetMV, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        std::string mvName(recv);

        // Check if the mv exists
        NetworkMV *mv = networkMVTable->mvs[mvName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxGetMV + 100;

            if ( mv != NULL ) {
                DEBUG('r', "GetMV (Found Remote) - mailbox %d name %s\n", mailbox, mvName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkMVTable->tableLock->Acquire();
                int value = mv->getMV();
                networkMVTable->tableLock->Release();

                // Send success
                SendResponse(-mailbox, value);
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( mv != NULL ) {
            // We DO have it
            DEBUG('r', "GetMV (Found Local) - mailbox %d name %s\n", mailbox, mvName.c_str());

            // Handle it
            networkMVTable->tableLock->Acquire();
            int value = mv->getMV();
            networkMVTable->tableLock->Release();

            // Send success
            SendResponse(mailbox, value);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxGetMV, mailbox, mvName, "GetMV");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("ERROR: GetMV failed. Terminating Nachos. No such mv - mailbox %d name %s\n", mailbox, mvName.c_str());
        SendResponse(mailbox, -2);
        interrupt->Halt();
    }
}

void RPCServer::Receive_SetMV() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxSetMV, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        std::string mvName(strtok(recv, ","));
        int mvValue = atoi(strtok(NULL,","));

        // Check if the mv exists
        NetworkMV *mv = networkMVTable->mvs[mvName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxSetMV + 100;

            if ( mv != NULL ) {
                DEBUG('r', "SetMV (Found Remote) - mailbox %d name %s\n", mailbox, mvName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkMVTable->tableLock->Acquire();
                mv->setMV(mvValue);
                networkMVTable->tableLock->Release();

                // Send success
                SendResponse(-mailbox, -1);
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( mv != NULL ) {
            // We DO have it
            DEBUG('r', "SetMV (Found Local) - mailbox %d name %s\n", mailbox, mvName.c_str());

            // Handle it
            networkMVTable->tableLock->Acquire();
            mv->setMV(mvValue);
            networkMVTable->tableLock->Release();

            // Send success
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxSetMV, mailbox, mvName, "SetMV");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("ERROR: SetMV failed. Terminating Nachos. No such mv - mailbox %d name %s\n", mailbox, mvName.c_str());
        SendResponse(mailbox, -2);
        interrupt->Halt();
    }
}

//-----------------------------------------------------------------------------------------------//
// When replying to the client, must reply to the individual process/thread mailbox.
//-----------------------------------------------------------------------------------------------//
int RPCServer::ClientMailbox(int _machine, int process, int thread) {
    char str1[2];
    char str2[2];
    char str3[2];
    
    // Have to add 10 because process, thread, and _machine can all be 0, which would create overlap
    // Use 10 instead of 1 so that SendResponse can easily extract the machine ID
    sprintf(str1, "%d", (_machine + 10));
    sprintf(str2, "%d", (process + 10));
    sprintf(str3, "%d", (thread + 10));

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
void RPCServer::SendResponse(int mailbox, int response, int _machine) {
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
    if (_machine == -1) {
        _machine = (mailbox / 10000) - 10;
    }

    DEBUG('n', "Send response machine %d mailbox %d response %d send \"%s\"\n", _machine, mailbox, response, send);

    // Construct packet header, mail header for the message
    outPktHdr.from = 0; // TEST
    outPktHdr.to = _machine;       
    outMailHdr.to = mailbox;
    outMailHdr.from = -1; // No syscall ever needs to reply to a response
    outMailHdr.length = strlen(send) + 1;

    // Send the response message
    bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

    if ( !success )
        printf("WARN: SendResponse failed. Client misconfigured.\n");
    else
        DEBUG('n', "Send response success\n");
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
bool RPCServer::SendQuery(int mailboxTo, int mailboxFrom, std::string query, char *identifier) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char send[MaxMailSize];
    char recv[MaxMailSize];
    char test[MaxMailSize]; strcpy(test, "yes");

    // If I am the only server, return false
    if (numServers == 1) {
        return false;
    }

    // Send the query with no modifications
    sprintf(send, "%s", query.c_str());

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
            return true;
    }

    // Query was ultimately unsuccessful 
    return false;
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

NetworkLock::NetworkLock(std::string _name) {
    mailbox = -1;
    name = _name;
    queue = new List;
}

NetworkLock::~NetworkLock() {
    delete queue;
}

void NetworkLock::Acquire(int _mailbox) {
    // Do the regular Lock::Acquire stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (mailbox == -1) {                                // lock is available
        mailbox = _mailbox;
        DEBUG('r', "Acquire success (Acquire) - mailbox %d name %s\n", mailbox, name.c_str());
        RPCServer::SendResponse(mailbox, -1);
    } else if (mailbox != _mailbox) {                    // lock is busy
        queue->Append((void *) _mailbox);                // add mailbox to the wait queue
        DEBUG('r', "Acquire waiting - mailbox %d name %s\n", mailbox, name.c_str());
    } else {
        printf("WARN: Acquire duplicate - mailbox %d name %s\n", mailbox, name.c_str());
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

void NetworkLock::Release(int _mailbox) {    
    // Do the regular Lock::Release stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (mailbox == _mailbox) {
        if (queue->IsEmpty()) {
            mailbox = -1;                               // no mailboxes are waiting
        } else {
            mailbox = (int) queue->Remove();            // wake up a waiting thread (referenced by mailbox)
            DEBUG('r', "Acquire success (Release) - mailbox %d name %s\n", mailbox, name.c_str());
            RPCServer::SendResponse(mailbox, -1);
        }
    } else {
        printf("WARN: Release without acquire - mailbox %d name %s\n", mailbox, name.c_str());
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

bool NetworkLock::HasAcquired(int _mailbox) {
    return (mailbox == _mailbox);
}

//-----------------------------------------------------------------------------------------------//
// Create NetworkCondition
//-----------------------------------------------------------------------------------------------//

NetworkCondition::NetworkCondition(std::string _name) {
    name = _name;
    conditionLock = NULL;
    queue = new List;
}

NetworkCondition::~NetworkCondition() {
    delete conditionLock;
    delete queue;
}

void NetworkCondition::Wait(int mailbox, NetworkLock *lock) {
    // Do the regular Condition::Wait stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);       // disable interrupts
    if (lock != NULL) {
        if (lock->HasAcquired(mailbox)) {
            if (conditionLock == NULL) {                    // condition hasn't been assigned to a lock yet
                conditionLock = lock;
                DEBUG('r', "Wait assigned - mailbox %d name %s lock %s\n", mailbox, name.c_str(), lock->getName().c_str());
            } else if (conditionLock == lock) {             // ok to wait
                queue->Append((void *) mailbox);            // add mailbox to wait queue
                conditionLock->Release(mailbox);            // release mailbox's hold on the lock
                DEBUG('r', "Wait waiting - mailbox %d name %s\n", mailbox, name.c_str());
            } else {
                printf("ERROR: Wait failed. Wrong lock. Terminating Nachos.\n");
                interrupt->Halt();
            }
        } else {
            printf("ERROR: Wait failed. Terminating Nachos. Unacquired lock - mailbox %d name %s\n", mailbox, name.c_str());
            interrupt->Halt();
        }
    } else {
        printf("ERROR: Wait failed. Lock null. Terminating Nachos.\n");
        interrupt->Halt();
    }
    (void) interrupt->SetLevel(oldLevel);                   // re-enable interrupts
}

void NetworkCondition::Signal(int mailbox, NetworkLock *lock) {
    // Do the regular Condition::Signal stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);       // disable interrupts
    if (lock != NULL) {
        if (lock->HasAcquired(mailbox)) {
            if (conditionLock == lock) {
                mailbox = (int) queue->Remove();        // remove one waiting thread from queue
                DEBUG('r', "Signal thread - mailbox %d name %s\n", mailbox, name.c_str());
                RPCServer::SendResponse(mailbox, -1);
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
    (void) interrupt->SetLevel(oldLevel);                   // re-enable interrupts
}

void NetworkCondition::Broadcast(int mailbox, NetworkLock *lock) {
    // Do the regular Condition::Broadcast stuff
    // MUST Release() following this call to Broadcast() in exception.cc, otherwise the thread(s) signaled will be stuck
    if (lock != NULL) {
        if (lock->HasAcquired(mailbox)) {
            if (conditionLock == lock) {
                while (!queue->IsEmpty())                   // signal all mailbox's waiting
                    Signal(mailbox, lock);
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

//-----------------------------------------------------------------------------------------------//
// Create NetworkMV
//-----------------------------------------------------------------------------------------------//

NetworkMV::NetworkMV(std::string _name) {
    value = 0;
    name = _name;
}

NetworkMV::~NetworkMV() {}

int NetworkMV::getMV() {
    return value;
}

void NetworkMV::setMV(int _value) {
    value = _value;
}
