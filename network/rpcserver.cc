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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         char *name = strtok(NULL,",");
         
         int foundKey = -1;
         std::map<int, NetworkLock*>::iterator iterator;
         for(iterator = networkLockTable->locks.begin(); iterator != networkLockTable->locks.end(); iterator++) {
         if(strcmp(iterator->second->name, name) == 0) { //if names are equal
         foundKey = iterator->first;
         break;
         }
         }
         if(foundKey != -1) {
         DEBUG('r', "CreateLock Received - key: %i name: %s (Already Created)\n", foundKey, name);
         SendResponse(inPktHdr.from, inMailHdr.from, foundKey);
         */
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
        
        /*
         // Process the message (identical to original syscall)
         NetworkLock *lock = new NetworkLock(name);
         */
        // This is Client-to-Server, no other server has it, so we create a new lock (just like the original syscall)
        lock = new NetworkLock(lockName);
        networkLockTable->tableLock->Acquire();
        networkLockTable->locks[lockName] = lock;
        networkLockTable->tableLock->Release();
        
        /*
         // Reply with the key
         DEBUG('r', "CreateLock Received - key: %i name: %s (New)\n", key, name);
         SendResponse(inPktHdr.from, inMailHdr.from, key);
         */
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         unsigned int key = atoi(strtok(NULL,","));
         */
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
            
            /*
             lock = networkLockTable->locks[key];
             if (lock != NULL) {
             networkLockTable->locks.erase(key);
             DEBUG('r', "DestroyLock Received - key: %d name: %s mailbox: %d\n", key, lock->name, ClientMailbox(machineID, processID, threadID));
             } else
             printf("WARN: DestroyLock failed. No such lock.\n");
             */
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         unsigned int key = atoi(strtok(NULL,","));
         
         // Process the message (identical to original syscall)
         networkLockTable->tableLock->Acquire();
         
         lock = networkLockTable->locks[key];
         if (lock != NULL) {
         lock->Acquire(machineID, processID, threadID);
         DEBUG('r', "Acquire Received - key: %d name: %s mailbox: %d\n", key, lock->name, ClientMailbox(machineID, processID, threadID));
         // Response is sent from Acquire()
         */
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         
         unsigned int key = atoi(strtok(NULL,","));
         
         // Process the message (identical to original syscall)
         networkLockTable->tableLock->Acquire();
         
         lock = networkLockTable->locks[key];
         if (lock != NULL) {
         lock->Release(machineID, processID, threadID);
         DEBUG('r', "Release Received - key: %d name: %s mailbox: %d\n", key, lock->name, ClientMailbox(machineID, processID, threadID));
         // Response (to any threads waiting to Acquire) is sent from lock->Release()
         // NO RESPONSE is sent to the thread making this actual call
         } else {
         printf("ERROR: Release failed. No such lock. Terminating Nachos.\n");
         interrupt->Halt();
         */
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
                // It does, tell the server with Wait/Signal/Broadcast to go ahead
                SendResponse(serverMailbox, -1, serverMachine);
            } else {
                // It does not, tell the server
                SendResponse(serverMailbox, -2, serverMachine);
            }

            // Wait for the server to tell us it's done
            postOffice->Receive(MailboxServerLockSubordinate, &inPktHdr, &inMailHdr, conf); 

            // Double-check this worked
            if ( strcmp(test,conf) != 0 ) {
                printf("ERROR: Subordinate failure. Nachos halting.\n");
                interrupt->Halt();
            }
        } else {
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         char *name = strtok(NULL,",");
         
         int foundKey = -1;
         std::map<int, NetworkCondition*>::iterator iterator;
         for(iterator = networkConditionTable->conditions.begin(); iterator != networkConditionTable->conditions.end(); iterator++) {
         if(strcmp(iterator->second->name, name) == 0) { //if names are equal
         foundKey = iterator->first;
         break;
         }
         }
         if(foundKey != -1) {
         DEBUG('r', "CreateCondition Received - key: %i name: %s (Already Created)\n", foundKey, name);
         SendResponse(inPktHdr.from, inMailHdr.from, foundKey);
         */
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
        
        /*
         // Process the message (identical to original syscall)
         NetworkCondition *condition = new NetworkCondition(name);
         */
        // This is Client-to-Server, no other server has it, so we create a new condition (just like the original syscall)
        condition = new NetworkCondition(conditionName);
        networkConditionTable->tableLock->Acquire();
        networkConditionTable->conditions[conditionName] = condition;
        networkConditionTable->tableLock->Release();
        
        /*
         // Reply with the key
         DEBUG('r', "CreateCondition Received - key: %i name: %s (New)\n", key, name);
         SendResponse(inPktHdr.from, inMailHdr.from, key);
         */
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         unsigned int key = atoi(strtok(NULL,","));
         */
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
            
            /*
             condition = networkConditionTable->conditions[key];
             if (condition != NULL) {
             DEBUG('r', "DestroyCondition Received - key: %d name: %s mailbox: %d\n", key, condition->name, ClientMailbox(machineID, processID, threadID));
             */
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         
         unsigned int conditionKey = atoi(strtok(NULL,","));
         unsigned int lockKey = atoi(strtok(NULL,","));
         DEBUG('r', "Wait Received - conditionKey %d lockKey %d (machine: %d process: %d thread %d)\n", conditionKey, lockKey, machineID, processID, threadID);
         
         
         // Process the message (identical to original syscall)
         networkConditionTable->tableLock->Acquire();
         networkLockTable->tableLock->Acquire();
         */
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
                }
                
                condition->Wait(-mailbox, lock);
                
                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // Tell the subordinate we're done with the lock
                if (subordinateActive) {
                    SendQuery(MailboxServerLockSubordinate, -1, "sub", "Wait (subordinate done)"); // Sending a bogus mailbox
                }


                // Success response to the original client is sent from Wait()
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }
        /*
         DEBUG('r', "Wait Received - conditionKey: %d conditionName: %s lockKey: %d lockName: %s mailbox: %d\n", conditionKey, condition->name, lockKey, lock->name, ClientMailbox(machineID, processID, threadID));
         condition->Wait(machineID, processID, threadID, lock);
         // Response sent from NetworkCondition::Wait()
         */
        
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         unsigned int conditionKey = atoi(strtok(NULL,","));
         unsigned int lockKey = atoi(strtok(NULL,","));
         
         // Process the message (identical to original syscall)
         networkConditionTable->tableLock->Acquire();
         networkLockTable->tableLock->Acquire();
         */
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
                }
                
                condition->Signal(-mailbox, lock);
                
                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();

                // Tell the subordinate we're done with the lock
                if (subordinateActive) {
                    SendQuery(MailboxServerLockSubordinate, -1, "sub", "Signal (subordinate done"); // Sending a bogus mailbox
                }

                // Success response to the original client is sent from Signal()
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }
        /*
         DEBUG('r', "Signal Received - conditionKey: %d conditionName: %s lockKey: %d lockName: %s mailbox: %d\n", conditionKey, condition->name, lockKey, lock->name, ClientMailbox(machineID, processID, threadID));
         condition->Signal(machineID, processID, threadID, lock);
         // Response(s) sent from Signal()
         */
        
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         unsigned int conditionKey = atoi(strtok(NULL,","));
         unsigned int lockKey = atoi(strtok(NULL,","));
         
         
         // Process the message (identical to original syscall)
         networkConditionTable->tableLock->Acquire();
         networkLockTable->tableLock->Acquire();
         */
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
                }
                
                condition->Broadcast(-mailbox, lock);
                
                networkConditionTable->tableLock->Release();
                networkLockTable->tableLock->Release();


                // Tell the subordinate we're done with the lock
                if (subordinateActive) {
                    SendQuery(MailboxServerLockSubordinate, -1, "sub", "Broadcast (subordinate done)"); // Sending a bogus mailbox
                }

                // Success response to the original client is sent from Broadcast()
                continue;
            } else {
                SendResponse(serverMailbox, -2, serverMachine);   // Tell the querying server it's not here
                continue;
            }
        }
        
        /*
         DEBUG('r', "Broadcast Received - conditionKey: %d conditionName: %s lockKey: %d lockName: %s mailbox: %d\n", conditionKey, condition->name, lockKey, lock->name, ClientMailbox(machineID, processID, threadID));
         condition->Broadcast(machineID,processID, threadID, lock);
         // Response(s) sent from Broadcast()
         */
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         char *name = strtok(NULL,",");
         int foundKey = -1;
         
         
         std::map<int, NetworkMV*>::iterator iterator;
         for(iterator = networkMVTable->mvs.begin(); iterator != networkMVTable->mvs.end(); iterator++) {
         if(strcmp(iterator->second->name, name) == 0) { //if names are equal
         foundKey = iterator->first;
         break;
         }
         }
         
         if(foundKey != -1) {
         DEBUG('r', "CreateMV Received - key: %i name: %s (Already Created)\n", foundKey, name);
         SendResponse(inPktHdr.from, inMailHdr.from, foundKey);
         */
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
        
        /*
         // Process the message (identical to original syscall)
         NetworkMV *mv = new NetworkMV(name);
         */
        // This is Client-to-Server, no other server has it, so we create a new mv (just like the original syscall)
        mv = new NetworkMV(mvName);
        networkMVTable->tableLock->Acquire();
        networkMVTable->mvs[mvName] = mv;
        networkMVTable->tableLock->Release();
        
        /*
         // Reply with the key
         DEBUG('r', "CreateMV Received - key: %i name: %s (New)\n", key, name);
         SendResponse(inPktHdr.from, inMailHdr.from, key);
         */
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         unsigned int key = atoi(strtok(NULL,","));
         
         */
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
            
            /*
             mv = networkMVTable->mvs[key];
             if (mv != NULL) {
             networkMVTable->mvs.erase(key);
             DEBUG('r', "DestroyMV Received - key: %d name: %s mailbox: %d\n", key, mv->name, ClientMailbox(machineID, processID, threadID));
             } else
             printf("WARN: DestroyMV failed. No such MV.\n");
             networkMVTable->tableLock->Release();
             */
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         unsigned int key = atoi(strtok(NULL,","));
         */
        int mailbox = inMailHdr.from;
        std::string mvName(recv);
        
        // Check if the mv exists
        NetworkMV *mv = networkMVTable->mvs[mvName];
        
        /*
         mv = networkMVTable->mvs[key];
         if (mv != NULL) {
         DEBUG('r', "GetMV Received - key: %d name: %s value: %d mailbox: %d\n", key, mv->name, mv->value, ClientMailbox(machineID, processID, threadID));
         SendResponse(inPktHdr.from, inMailHdr.from, mv->value);
         } else
         printf("WARN: GetMV failed. No such MV.\n");
         */
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
        /*
         int processID = atoi(strtok(recv,","));
         int threadID = atoi(strtok(NULL,","));
         int machineID = inPktHdr.from;
         unsigned int key = atoi(strtok(NULL,","));
         unsigned int value = atoi(strtok(NULL,","));
         
         */
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
            
            /*
             mv = networkMVTable->mvs[key];
             if (mv != NULL) {
             DEBUG('r', "SetMV Received - key: %d name: %s value: %d mailbox: %d\n", key, mv->name, value, ClientMailbox(machineID, processID, threadID));
             mv->value = value;
             } else
             printf("WARN: GetMV failed. No such MV.\n");
             */
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
    
    /*
     // Have to add 1 because process and thread and machineID can both be 0, which would create overlap
     sprintf(str1, "%d", (machineID + 1));
     sprintf(str2, "%d", (process + 1));
     sprintf(str3, "%d", (thread + 1)); //not working properly
     
     strcat(str1, str2);
     strcat(str1, str3); //not working properly
     */
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
    //DEBUG('r', "Send response machine %d mailbox %d response %d \"%s\"\n", machineID, mailbox, response, send);
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

/*
 NetworkLock::NetworkLock(char *_name) {
 mailboxID = -1;
 name = new char[strlen(_name)+1]; //deep copy
 strcpy(name, _name); //deep copy
 */
NetworkLock::NetworkLock(std::string _name) {
    mailbox = -1;
    name = _name;
    queue = new List;
}

NetworkLock::~NetworkLock() {
    //delete name;
    delete queue;
}

/*
 void NetworkLock::Acquire(int machineID, int process, int thread) {
 // Do the regular Lock::Acquire stuff
 IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
 int mailbox = RPCServer::ClientMailbox(machineID, process, thread); // get mailbox id of client calling acquire
 if (mailboxID == -1) {                               // lock is available
 mailboxID = mailbox;
 DEBUG('r', "Acquire(): Success - name: %s mailbox: %d (machine: %d process: %d)\n", name, mailbox, machineID, process);
 RPCServer::SendResponse(machineID, mailboxID, -1);
 } else if (mailboxID != mailbox) {                    // lock is busy
 DEBUG('r', "Acquire(): Waiting to acquire - name: %s mailbox: %d (machine: %d process: %d)\n", name, mailbox, machineID, process);
 queue->Append((void *) mailbox);                 // add mailbox ID to the wait queue
 
 } else {
 printf("WARN: Acquire() Failed - name: %s mailbox: %d (machine: %d process: %d)\n", name, mailbox, machineID, process);
 */
void NetworkLock::Acquire(int _mailbox) {
    // Do the regular Lock::Acquire stuff
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if (mailbox == -1) {                                // lock is available
        mailbox = _mailbox;
        DEBUG('r', "Acquire - %d getting %s (no delay)\n", mailbox, name.c_str());
        RPCServer::SendResponse(mailbox, -1);           // notify the thread calling Acquire() (just like Release() would do)
    } else if (mailbox != _mailbox) {                   // lock is busy
        queue->Append((void *) _mailbox);               // add mailbox to the wait queue
        DEBUG('r', "Acquire - %d wants %s but %d has it (delaying)\n", _mailbox, name.c_str(), mailbox);
    } else {
        printf("WARN: Acquire duplicate - %d has and wants %s (no delay)\n", mailbox, name.c_str());
        RPCServer::SendResponse(mailbox, -1);           // notify the thread calling Acquire() (just like Release() would do)
    }
    (void) interrupt->SetLevel(oldLevel);               // re-enable interrupts
}

/*
 void NetworkLock::Release(int machineID, int process, int thread) {
 // Do the regular Lock::Release stuff
 IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
 int mailbox = RPCServer::ClientMailbox(machineID, process, thread); //get mailbox id of client calling release
 if (mailboxID == mailbox) {
 if (queue->IsEmpty()) {
 mailboxID = -1; //no mailbox's are waiting
 }
 else {
 mailboxID = (int) queue->Remove();           // wake up a waiting mailbox
 DEBUG('r', "Release(): Success - name: %s mailbox: %d (machine: %d process: %d)\n", name, machineID, mailbox, process);
 
 // temporary way to get machineID from mailbox
 int machineToAcquire = mailboxID;
 while (machineToAcquire >= 10)
 machineToAcquire /= 10;
 machineToAcquire--;
 
 DEBUG('r', "Release(): Sending acquire request to mailbox: %d (machine: %d process: %d)\n", mailboxID, machineToAcquire, process);
 RPCServer::SendResponse(machineToAcquire, mailboxID, -1);
 }
 } else {
 printf("WARN: Release() Failed - name: %s mailbox: %d (machine: %d)\n", name, mailbox, machineID);
 */
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

/*
 bool NetworkLock::HasAcquired(int mailbox) {
 return (mailboxID == mailbox);
 */
bool NetworkLock::HasAcquired(int _mailbox) {
    return (mailbox == _mailbox);
}

//-----------------------------------------------------------------------------------------------//
// Create NetworkCondition
//-----------------------------------------------------------------------------------------------//

/*
 NetworkCondition::NetworkCondition(char* _name) {
 name = new char[strlen(_name)+1]; //deep copy
 strcpy(name, _name); //deep copy
 */
NetworkCondition::NetworkCondition(std::string _name) {
    name = _name;
    conditionLock = NULL;
    queue = new List;
}

NetworkCondition::~NetworkCondition() {
    delete conditionLock;
    delete queue;
    //delete name;
}

/*
 void NetworkCondition::Wait(int machineID, int process, int thread, NetworkLock* lock) {
 IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
 int mailbox = RPCServer::ClientMailbox(machineID, process, thread); //get mailbox id of client calling wait
 if (lock != NULL) {
 if (lock->HasAcquired(mailbox)) {
 if (conditionLock == NULL)  {                    // condition hasn't been assigned to a lock yet
 conditionLock = lock;
 DEBUG('r', "Wait(): CV assigned lock to lock: %s mailbox: %d (machine: %d process: %d)\n", lock->name, mailbox, machineID, process);
 }
 if (conditionLock == lock) {                    // ok to wait
 queue->Append((void *) mailbox);             // add mailbox to wait queue
 conditionLock->Release(machineID, process, thread);    // release waiting lock
 DEBUG('r', "Wait(): Success - conditionName: %s lockName: %s mailbox %d (machine: %d process: %d)\n", name, lock->name, mailbox, machineID, process);
 */
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

/*
 void NetworkCondition::Signal(int machineID, int process, int _thread, NetworkLock* lock) {
 IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
 int mailbox = RPCServer::ClientMailbox(machineID, process, _thread); //get mailbox id of client calling signal
 if (lock != NULL) {
 if (lock->HasAcquired(mailbox)) {
 if (conditionLock == lock) {
 int mailboxToSignal = (int) queue->Remove();         // remove one waiting mailbox from queue
 DEBUG('r', "Signal(): Success - conditionName: %s lockName: %s mailbox: %d (machine: %d process: %d)\n", name, lock->name, mailbox, machineID, process);
 RPCServer::SendResponse(machineID, mailbox, -1);
 
 // TEMPORARY WAY TO GET MACHINE ID FROM MAILBOX
 int machineIDToSignal = mailboxToSignal;
 while (machineIDToSignal >= 10)
 machineIDToSignal /= 10;
 machineIDToSignal--;
 
 DEBUG('r', "Wait(): Sending release request to lockName: %s mailbox: %d (machine: %d process: %d)\n", lock->name, mailboxToSignal, machineIDToSignal, process);
 RPCServer::SendResponse(machineIDToSignal, mailboxToSignal, -1);
 // That's why the second response is sent (so Signal() knows to go ahead and the thread signaled will be stuck
 // MUST Release() following this call to Signal() in exception.cc, otherwise Release())
 // Similarly, the thread signaled is still sitting in Wait() in exception.cc, and it should Acquire() there
 */
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

/*
 void NetworkCondition::Broadcast(int machineID, int process, int _thread, NetworkLock* lock) {
 int mailbox = RPCServer::ClientMailbox(machineID, process, _thread); //get mailbox id of client calling wait
 if (lock != NULL) {
 if (lock->HasAcquired(mailbox)) {
 if (conditionLock == lock) {
 while (!queue->IsEmpty()) {                // signal all mailbox's waiting
 // Do the regular Condition::Signal stuff
 IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
 if (lock != NULL) {
 if (lock->HasAcquired(mailbox)) {
 if (conditionLock == lock) {
 int mailboxToSignal = (int) queue->Remove();         // remove one waiting mailbox from queue
 
 //Temporary way to get machine ID from mailbox
 int machineIDToSignal = mailboxToSignal;
 while (machineIDToSignal >= 10)
 machineIDToSignal /= 10;
 machineIDToSignal--;
 
 DEBUG('r', "Broadcast(): Sending signal request to mailbox: %d (machine: %d process: %d)\n", mailboxToSignal, machineIDToSignal, process);
 RPCServer::SendResponse(machineIDToSignal, mailboxToSignal, -1);
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
 DEBUG('r', "Broadcast(): Success - conditionName: %s lockName: %s mailbox: %d (machine: %d process: %d)\n", name, lock->name, mailbox, machineID, process );
 RPCServer::SendResponse(machineID, mailbox, -1); //signal back to thread waiting in exception.cc
 */
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

/*
 NetworkMV::NetworkMV(char* _name) {
 */
NetworkMV::NetworkMV(std::string _name) {
    value = 0;
    name = _name;
    //name = new char[strlen(_name)+1]; //deep copy
    //strcpy(name, _name); //deep copy*/
}

/*
 NetworkMV::~NetworkMV() {
 delete name;
 }
 
 */
NetworkMV::~NetworkMV() {}

int NetworkMV::getMV() {
    return value;
}

void NetworkMV::setMV(int _value) {
    value = _value;
}