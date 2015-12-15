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
        std::string lockName(recv);

        // Check if the lock already exists
        NetworkLock *lock = networkLockTable->locks[lockName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxCreateLock + 100;

            if ( lock != NULL ) {
                DEBUG('r', "Receive_CreateLock (remote) - %d creating %s\n", mailbox, lockName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it
                SendResponse(-mailbox, -1);                         // Send a response to the original client
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }

        // This is Client-to-Server
        if ( lock != NULL ) {
            // We DO have it
            DEBUG('r', "Receive_CreateLock (local) - %d creating %s\n", mailbox, lockName.c_str());
            SendResponse(mailbox, -1);
            continue;
        } else {
            // We DO NOT have it, we have to check with other servers
            bool result = SendQuery(MailboxCreateLock, mailbox, lockName, "CreateLock");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, so we create a new lock (just like the original syscall)
        lock = new NetworkLock(lockName);
        networkLockTable->tableLock->Acquire();
        networkLockTable->locks[lockName] = lock;
        networkLockTable->tableLock->Release();

        // Reply with success
        DEBUG('r', "Receive_CreateLock (new) - %d creating %s\n", mailbox, lockName.c_str());
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
                DEBUG('r', "Receive_DestroyLock (remote) - %d destroying %s\n", mailbox, lockName.c_str());
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
            DEBUG('r', "Receive_DestroyLock (local) - %d destroying %s\n", mailbox, lockName.c_str());

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
        printf("WARN: Receive_DestroyLock failed. No such lock - mailbox %d name %s\n", mailbox, lockName.c_str());
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
                DEBUG('r', "Receive_Acquire (remote) - %d acquiring %s\n", mailbox, lockName.c_str());
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
            DEBUG('r', "Receive_Acquire (local) - %d acquiring %s\n", mailbox, lockName.c_str());

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
        printf("ERROR: Receive_Acquire failed. Terminating Nachos. No such lock - mailbox %d name %s\n", mailbox, lockName.c_str());
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

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxRelease + 100;

            if ( lock != NULL ) {
                DEBUG('r', "Receive_Release (remote) - %d releasing %s\n", mailbox, lockName.c_str());
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
            DEBUG('r', "Receive_Release (local) - %d releasing %s\n", mailbox, lockName.c_str());

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
        printf("ERROR: Receive_Release failed. Terminating Nachos. No such lock - mailbox %d name %s\n", mailbox, lockName.c_str());
        SendResponse(mailbox, -2);
        interrupt->Halt();
    }
}

void RPCServer::Receive_ServerLockSubordinate() {
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char recv[MaxMailSize];
    char conf[MaxMailSize];
    char test[MaxMailSize]; strcpy(test, "sub"); // Special message (other than "yes") to prevent confusion

    for (;;) {
        // Wait for a mailbox message
        postOffice->Receive(MailboxServerLockSubordinate, &inPktHdr, &inMailHdr, recv); 

        // Read the message
        int mailbox = inMailHdr.from;                           // The mailbox that's touching the Condition, that *SHOULD* be holding the lock
        std::string lockName(recv);                             // The lock name we hope is on this server
        int serverMachine = inPktHdr.from;                      // The server that has the Condition
        int serverMailbox = MailboxServerLockSubordinate + 100; // The mailbox that will be used to talk back to the server

        // Make sure we haven't accidentally interrupted a confirmation to another subordinate
        if ( strcmp(test,recv) == 0 ) {
            DEBUG('r', "Receive_ServerLockSubordinate (intercepted) - Server %d is asking for %s on behalf of %d\n", serverMachine, lockName.c_str(), mailbox);
            SendResponse(serverMailbox, -2, serverMachine);
            continue;
        }

        // Acquire the NetworkLockTable lock
        networkLockTable->tableLock->Acquire();

        // Check for the lock on this server
        NetworkLock *lock = networkLockTable->locks[lockName];

        // Check if we have the lock
        if ( lock != NULL ) {
            // We do, check that the mailbox owns the lock
            bool hasAcquired = lock->HasAcquired(mailbox);

            if (hasAcquired) {
                DEBUG('r', "Receive_ServerLockSubordinate (have it) - Server %d is asking for %s on behalf of %d\n", serverMachine, lockName.c_str(), mailbox);
                // It does, tell the server with Wait/Signal/Broadcast to go ahead
                SendResponse(serverMailbox, -1, serverMachine);
            } else {
                DEBUG('r', "Receive_ServerLockSubordinate (have it, cheat) - Server %d is asking for %s on behalf of %d\n", serverMachine, lockName.c_str(), mailbox);
                // It does not, tell the server
                SendResponse(serverMailbox, -2, serverMachine);
            }

            // // Wait for the server to tell us it's done
            // postOffice->Receive(MailboxServerLockSubordinate, &inPktHdr, &inMailHdr, conf); 

            // DEBUG('r', "Receive_ServerLockSubordinate (server finished) - Server %d is asking for %s on behalf of %d\n", serverMachine, lockName.c_str(), mailbox);
                            
            // SendResponse(115, -1, inPktHdr.from);     // this should be the mailbox it's wanting

            // // Double-check this worked
            // if ( strcmp(test,conf) != 0 ) {
            //     printf("ERROR: Subordinate failure. Nachos halting.\n");
            //     interrupt->Halt();
            // }
        } else {
            DEBUG('r', "Receive_ServerLockSubordinate (don't have it) - Server %d is asking for %s on behalf of %d\n", serverMachine, lockName.c_str(), mailbox);
            // Tell the server we do not have the lock
            SendResponse(serverMailbox, -2, serverMachine);
        }

        // Release the LockTable lock
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
        int mailbox = inMailHdr.from;
        std::string conditionName(recv);

        // Check if the condition already exists
        NetworkCondition *condition = networkConditionTable->conditions[conditionName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxCreateCondition + 100;

            if ( condition != NULL ) {
                DEBUG('r', "Receive_CreateCondition (remote) - %d creating %s\n", mailbox, conditionName.c_str());
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
            DEBUG('r', "Receive_CreateCondition (local) - %d creating %s\n", mailbox, conditionName.c_str());
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
        DEBUG('r', "Receive_CreateCondition (new) - %d creating %s\n", mailbox, conditionName.c_str());
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
                DEBUG('r', "Receive_DestroyCondition (remote) - %d destroying %s\n", mailbox, conditionName.c_str());
                
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
            DEBUG('r', "Receive_DestroyCondition (local) - %d destroying %s\n", mailbox, conditionName.c_str());

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
        printf("WARN: Receive_DestroyCondition failed. No such condition - mailbox %d name %s\n", mailbox, conditionName.c_str());
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
        std::string together(recv);
        std::string conditionName(strtok(recv, ","));
        std::string lockName(strtok(NULL, ","));

        // Check if the condition exists (and get the lock, because we assume they are on the same server)
        NetworkCondition *condition = networkConditionTable->conditions[conditionName];
        NetworkLock *lock = networkLockTable->locks[lockName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {

            // We CANNOT assume that the lock and the condition were created on the same server
            // It might be good to create a "lock swap" that pulls the lock over to the condition server
            // when the lock is being assigned to the condition
            // The other option would be a way to talk to the other server to ensure that the lock matches
            // and is held, but then you woulnd't be able to maintain immutability on the lock while the server
            // that held the condition is working

            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxWait + 100;

            bool subordinateActive = false;

            if ( condition != NULL ) {
                DEBUG('r', "Receive_Wait (remote) - %d waiting on %s with %s\n", mailbox, conditionName.c_str(), lockName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkConditionTable->tableLock->Acquire();
                networkLockTable->tableLock->Acquire();

                if (lock == NULL) {
                    subordinateActive = true;

                    // SendQuery to find the server that has it
                    bool result = SendQuery(MailboxServerLockSubordinate, mailbox, lockName, "Wait (subordinate)");

                    if (!result) {
                        printf("ERROR: Receive_Wait (remote) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
                        SendResponse(-mailbox, -2);
                        interrupt->Halt();          
                    }

                    // Dummy lock to satisfy Wait()
                    lock = new NetworkLock(lockName);
                    lock->Acquire(-mailbox, false);
                }

                condition->Wait(-mailbox, lock);

                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // // Tell the subordinate we're done with the lock
                // if (subordinateActive) {
                //     SendQuery(MailboxServerLockSubordinate, -1, "sub", "Wait (subordinate done)"); // Sending a bogus mailbox
                // }

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
            DEBUG('r', "Receive_Wait (local) - %d waiting on %s with %s\n", mailbox, conditionName.c_str(), lockName.c_str());

            // Handle it
            networkConditionTable->tableLock->Acquire();
            networkLockTable->tableLock->Acquire();

            if (lock == NULL) {
                printf("ERROR: Receive_Wait (local) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
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
            DEBUG('r', "Receive_Wait (checking remotes) - %d waiting on %s with %s\n", mailbox, conditionName.c_str(), lockName.c_str());
            bool result = SendQuery(MailboxWait, mailbox, together, "Wait");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("ERROR: Receive_Wait failed. Terminating Nachos. No such condition - mailbox %d name %s\n", mailbox, conditionName.c_str());
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
        postOffice->Receive(MailboxSignal, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        std::string together(recv);
        std::string conditionName(strtok(recv, ","));
        std::string lockName(strtok(NULL, ","));

        // Check if the condition exists (and get the lock, because we assume they are on the same server)
        NetworkCondition *condition = networkConditionTable->conditions[conditionName];
        NetworkLock *lock = networkLockTable->locks[lockName];

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxSignal + 100;

            bool subordinateActive = false;

            if ( condition != NULL ) {
                DEBUG('r', "Receive_Signal (remote) - %d signaling %s with %s\n", mailbox, conditionName.c_str(), lockName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkConditionTable->tableLock->Acquire();
                networkLockTable->tableLock->Acquire();

                if (lock == NULL) {
                    subordinateActive = true;

                    // SendQuery to find the server that has it
                    bool result = SendQuery(MailboxServerLockSubordinate, mailbox, lockName, "Signal (subordinate");
                    
                    if (!result) {
                        printf("ERROR: Receive_Signal (remote) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
                        SendResponse(-mailbox, -2);
                        interrupt->Halt();
                    }

                    // Dummy lock to satisfy Signal()
                    lock = new NetworkLock(lockName);
                    lock->Acquire(-mailbox, false);
                }

                condition->Signal(-mailbox, lock);

                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // // Tell the subordinate we're done with the lock
                // if (subordinateActive) {
                //     SendQuery(MailboxServerLockSubordinate, -1, "sub", "Signal (subordinate done"); // Sending a bogus mailbox
                // }

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
            DEBUG('r', "Receive_Signal (local) - %d signaling %s with %s\n", mailbox, conditionName.c_str(), lockName.c_str());

            // Handle it
            networkConditionTable->tableLock->Acquire();
            networkLockTable->tableLock->Acquire();

            if (lock == NULL) {
                printf("ERROR: Receive_Signal (local) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
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
            bool result = SendQuery(MailboxSignal, mailbox, together, "Signal");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("ERROR: Receive_Signal failed. Terminating Nachos. No such condition - mailbox %d name %s\n", mailbox, conditionName.c_str());
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
        postOffice->Receive(MailboxBroadcast, &inPktHdr, &inMailHdr, recv);

        // Read the message
        int mailbox = inMailHdr.from;
        std::string together(recv);
        std::string conditionName(strtok(recv, ","));
        std::string lockName(strtok(NULL, ","));

        // Check if the condition exists (and get the lock, because we assume they are on the same server)
        NetworkCondition *condition = networkConditionTable->conditions[conditionName];
        NetworkLock *lock = networkLockTable->locks[lockName];

        bool subordinateActive = false;

        // Check if this is a Server-to-Server query
        if (mailbox < 0) {
            int serverMachine = inPktHdr.from;
            int serverMailbox = MailboxBroadcast + 100;

            if ( condition != NULL ) {
                DEBUG('r', "Receive_Broadcast (remote) - %d broadcasting to %s with %s\n", mailbox, conditionName.c_str(), lockName.c_str());
                SendResponse(serverMailbox, -1, serverMachine);     // Tell the querying server we have it

                // Handle it
                networkConditionTable->tableLock->Acquire();
                networkLockTable->tableLock->Acquire();

                if (lock == NULL) {
                    subordinateActive = true;

                    // SendQuery to find the server that has it
                    bool result = SendQuery(MailboxServerLockSubordinate, mailbox, lockName, "Broadcast (subordinate)");

                    if (!result) {
                        printf("ERROR: Receive_Broadcast (remote) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
                        SendResponse(-mailbox, -2);
                        interrupt->Halt();          
                    }

                    // Dummy lock to satisfy Broadcast()
                    lock = new NetworkLock(lockName);
                    lock->Acquire(-mailbox, false);
                }

                condition->Broadcast(-mailbox, lock);

                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // // Tell the subordinate we're done with the lock
                // if (subordinateActive) {
                //     SendQuery(MailboxServerLockSubordinate, -1, "sub", "Broadcast (subordinate done)"); // Sending a bogus mailbox
                // }

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
            DEBUG('r', "Receive_Broadcast (local) - %d broadcasting to %s with %s\n", mailbox, conditionName.c_str(), lockName.c_str());

            // Handle it
            networkConditionTable->tableLock->Acquire();
            networkLockTable->tableLock->Acquire();

            if (lock == NULL) {
                printf("ERROR: Receive_Broadcast (local) failed. Terminating Nachos. No such lock - mailbox %d name %s\n", -mailbox, lockName.c_str());
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
            bool result = SendQuery(MailboxBroadcast, mailbox, together, "Broadcast");
            if (result)
                continue; // One of the other servers had it, we can finish
        }

        // This is Client-to-Server, no other server has it, this is an error
        printf("ERROR: Receive_Broadcast failed. Terminating Nachos. No such condition - mailbox %d name %s\n", mailbox, conditionName.c_str());
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
        std::string together(recv);
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
            bool result = SendQuery(MailboxSetMV, mailbox, together, "SetMV");
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
        printf("ERROR: Invalid response given to SendResponse. Mailbox %d. Response %d. Terminating Nachos.\n", mailbox, response);
        interrupt->Halt();
    }

    // Calculate machine ID from the mailbox if this is a Server-to-Client response
    if (_machine == -1) {
        _machine = (mailbox / 10000) - 10;
    }

    DEBUG('r', "SendResponse - machine %d mailbox %d response %d send \"%s\"\n", _machine, mailbox, response, send);

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

    DEBUG('r', "SendQuery (start) - mailboxTo %d mailboxFrom %d query %s identifier %s\n", mailboxTo, mailboxFrom, query.c_str(), identifier);

    // If I am the only server, return false
    if (numServers == 1) {
        return false;
    }

    // Send the query with no modifications
    sprintf(send, "%s", query.c_str());

    // Send the query to all servers until out of servers or get a "yes"
    for (int serverToQuery = 0; serverToQuery < numServers; serverToQuery++) {
        // Don't send a query to ourselves
        if (machineName == serverToQuery) {
            continue;
        }

        // Construct the packet header, mail header for the message
        outPktHdr.to = serverToQuery;
        outMailHdr.to = mailboxTo;
        outMailHdr.from = -mailboxFrom; // MAILBOX IS NEGATED HERE. DO NOT PRE-NEGATE.
        outMailHdr.length = strlen(send) + 1;

        DEBUG('r', "SendQuery (query %d) - mailboxTo %d mailboxFrom %d query %s identifier %s\n", serverToQuery, mailboxTo, mailboxFrom, query.c_str(), identifier);

        // Send the query message
        bool success = postOffice->Send(outPktHdr, outMailHdr, send); 

        // Check that the send worked
        if ( !success )
            printf("WARN: %s (SendQuery) failed. Server %d misconfigured.\n", identifier, serverToQuery);

        // Get the response back
        // Use mailboxTo + 100 to listen for "yes" or "no"
        int mailbox = mailboxTo + 100;
        postOffice->Receive(mailbox, &inPktHdr, &inMailHdr, recv);

        // If we get a "yes", then return true. Otherwise keep trying.
        // A "yes" means the other server is handling the request
        if ( strcmp(test,recv) == 0 ) {
            DEBUG('r', "SendQuery (true %d) - mailboxTo %d mailboxFrom %d query %s identifier %s\n", serverToQuery, mailboxTo, mailboxFrom, query.c_str(), identifier);
            return true;
        } else {
            DEBUG('r', "SendQuery (false %d) - mailboxTo %d mailboxFrom %d query %s identifier %s\n", serverToQuery, mailboxTo, mailboxFrom, query.c_str(), identifier);
        }
    }

    DEBUG('r', "SendQuery (false) - mailboxTo %d mailboxFrom %d query %s identifier %s\n", mailboxTo, mailboxFrom, query.c_str(), identifier);

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
static void DummyReceive_ServerLockSubordinate(int arg) { RPCServer* rpcs = (RPCServer *) arg; rpcs->Receive_ServerLockSubordinate(); }

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

    Thread *tServerLockSubordinate = new Thread("ServerLockSubordinate thread");
    tServerLockSubordinate->Fork(DummyReceive_ServerLockSubordinate, (int) rpcServer);
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

void NetworkLock::Acquire(int _mailbox, bool messageCaller) {
    // Do the regular Lock::Acquire stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (mailbox == -1) {                                // lock is available
        mailbox = _mailbox;
        DEBUG('r', "Acquire - %d getting %s (no delay)\n", mailbox, name.c_str());
        if (messageCaller)                              // prevent messaging the subordinate
            RPCServer::SendResponse(mailbox, -1);       // notify the thread calling Acquire() (just like Release() would do)
    } else if (mailbox != _mailbox) {                   // lock is busy
        queue->Append((void *) _mailbox);               // add mailbox to the wait queue
        DEBUG('r', "Acquire - %d wants %s but %d has it (delaying)\n", _mailbox, name.c_str(), mailbox);
    } else {
        printf("WARN: Acquire duplicate - %d has and wants %s (no delay)\n", mailbox, name.c_str());
        RPCServer::SendResponse(mailbox, -1);           // notify the thread calling Acquire() (just like Release() would do)
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

void NetworkLock::Release(int _mailbox, bool messageCaller) {    
    // Do the regular Lock::Release stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (mailbox == _mailbox) {
        if (queue->IsEmpty()) {
            mailbox = -1;                               // no mailboxes are waiting
            DEBUG('r', "Release - %d released %s (now free)\n", _mailbox, name.c_str());
            if (messageCaller)
                RPCServer::SendResponse(_mailbox, -1);  // notify the thread calling Release()
        } else { 
            mailbox = (int) queue->Remove();            // wake up a waiting thread (referenced by mailbox)
            DEBUG('r', "Release - %d released %s (now going to %d)\n", _mailbox, name.c_str(), mailbox);
            if (messageCaller)
                RPCServer::SendResponse(_mailbox, -1);  // notify the thread calling Release()
            DEBUG('r', "Acquire - %d getting %s after delay (%d had it)\n", mailbox, name.c_str(), _mailbox);
            RPCServer::SendResponse(mailbox, -1);       // notify the waiting thread
        }
    } else {
        if (mailbox == -1)
            printf("WARN: Release without acquire - %d released %s (is free)\n", _mailbox, name.c_str());
        else
            printf("WARN: Release without acquire - %d released %s (%d has it)\n", _mailbox, name.c_str(), mailbox);
        RPCServer::SendResponse(_mailbox, -1);          // notify the thread calling Release()
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
                DEBUG('r', "Wait - %d assigned %s to %s\n", mailbox, lock->getName().c_str(), name.c_str());
            }
            if (conditionLock == lock) {                    // ok to wait
                queue->Append((void *) mailbox);            // add mailbox to wait queue
                DEBUG('r', "Wait - %d waiting for %s with %s (will be released)\n", mailbox, name.c_str(), lock->getName().c_str());
                conditionLock->Release(mailbox, false);     // release mailbox's hold on the lock, don't confirm to the client calling Wait
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

void NetworkCondition::Signal(int mailbox, NetworkLock *lock, bool messageCaller) {
    // Do the regular Condition::Signal stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);       // disable interrupts
    if (lock != NULL) {
        if (lock->HasAcquired(mailbox)) {
            if (conditionLock == lock) {
                int _mailbox = (int) queue->Remove();       // remove one waiting thread from queue
                if (messageCaller) {
                    DEBUG('r', "Signal - %d signaled %d through %s with %s\n", mailbox, _mailbox, name.c_str(), lock->getName().c_str());
                    RPCServer::SendResponse(mailbox, -1);   // Send response to the client that called Signal()
                    DEBUG('r', "Wait - %d was signaled by %d through %s with %s\n", _mailbox, mailbox, name.c_str(), lock->getName().c_str());
                    RPCServer::SendResponse(_mailbox, -1);      // Send response to the client that called Wait()
                } else {
                    DEBUG('r', "Wait - %d was broadcast to by %d through %s with %s\n", _mailbox, mailbox, name.c_str(), lock->getName().c_str());
                    RPCServer::SendResponse(_mailbox, -1);      // Send response to the client that called Wait()
                }
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
                DEBUG('r', "Broadcast - %d broadcast through %s with %s\n", mailbox, name.c_str(), lock->getName().c_str());
                RPCServer::SendResponse(mailbox, -1);       // Send response to the client that called Broadcast()
                while (!queue->IsEmpty())                   // signal all mailbox's waiting
                    Signal(mailbox, lock, false);           // Don't send duplicate success responses
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
