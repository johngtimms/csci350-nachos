// rpcserver.h

#ifndef RPCSERVER_H
#define RPCSERVER_H

#include "network.h"
#include "synchlist.h"

// The network-backed syscalls are mapped to the following mailboxes:
#define MailboxCreateLock           0
#define MailboxDestroyLock          1
#define MailboxAcquire              2
#define MailboxRelease              3
#define MailboxCreateCondition      4
#define MailboxDestroyCondition     5
#define MailboxWait                 6
#define MailboxSignal               7
#define MailboxBroadcast            8
#define MailboxNetPrint             9
#define MailboxNetHalt              10
#define MailboxCreateMV             11
#define MailboxDestroyMV            12
#define MailboxGetMV                13
#define MailboxSetMV                14

class RPCServer {
    public:
        RPCServer();
        ~RPCServer();

        // These functions handle calls from clients
        void Receive_CreateLock();
        void Receive_DestroyLock();
        void Receive_Acquire();
        void Receive_Release();
        void Receive_CreateCondition();
        void Receive_DestroyCondition();
        void Receive_Wait();
        void Receive_Signal();
        void Receive_Broadcast();
        void Receive_NetPrint();
        void Receive_NetHalt();
        void Receive_CreateMV();
        void Receive_DestroyMV();
        void Receive_GetMV();
        void Receive_SetMV();

        // To prevent overlap, mailbox IDs are calculated
        static int ClientMailbox(int machine, int process, int thread);

        // Server-to-Client and Server-to-Server response messages
        // For Server-to-Client responses, mailbox is obtained from a call to ClientMailbox
        // For Server-to-Server messages, mailbox is one of the defined mailbox numbers above
        // plus 100, because the receiving server will be waiting there for "yes" or "no"
        // response is either "yes" or "no" (pass 0 or 1)
        // Clients will interpret "yes" as success and "no" as an error
        // Sending "yes" to a server tells it the sending server can handle the query
        // Sending "no" to a server tells it the sending server cannot handle the query
        static void SendResponse(int mailbox, int response);

        // Server-to-Server query messages
        // mailboxTo is one of the defined mailbox numbers above
        // mailboxFrom is the negation of the Server-to-Client mailbox obtained from ClientMailbox
        // The negation of mailboxFrom tells the receiving server this is a Server-to-Server call
        // query is the original query from the client
        static bool SendQuery(int mailboxTo, int mailboxFron, int query);
};

class NetworkLock {
    public:
        char* name;
        NetworkLock(int _machineID, int process, char *_name);
        ~NetworkLock();
        void Acquire(int _machineID, int process, int thread);
        void Release(int _machineID, int process, int thread);
        bool IsOwner(int _machineID);
        bool HasAcquired(int mailbox);

    private:
        int machineID;
        int processID;
        int threadID;
        int mailboxID;
        List *queue;
};

class NetworkCondition {
    public:
        char* name;
        NetworkCondition(int _machineID, int process, char *_name);
        ~NetworkCondition();
        void Wait(int _machineID, int process, int thread, NetworkLock *lock);
        void Signal(int _machineID, int process, int _thread, NetworkLock *lock);
        void Broadcast(int _machineID, int process, int _thread, NetworkLock *lock);
        bool IsOwner(int _machineID);
        
    private:
        int machineID;
        int processID;
        int mailboxID;
        NetworkLock *conditionLock;
        List *queue;
};

class NetworkMV {
    public:
        int value;
        char* name;
        NetworkMV(int _machineID, int process, char *_name);
        ~NetworkMV();
        bool IsOwner(int process);
       
    private:
        int machineID;
        int processID;
};

struct NetworkLockTable {
    int index;
    Lock *tableLock;
    std::map<int, NetworkLock*> locks;

    NetworkLockTable() {
        index = 0;
        tableLock = new Lock();
    }

    ~NetworkLockTable() {
        delete tableLock;
    }
};

struct NetworkConditionTable {
    int index;
    Lock *tableLock;
    std::map<int, NetworkCondition*> conditions;

    NetworkConditionTable() {
        index = 0;
        tableLock = new Lock();
    }

    ~NetworkConditionTable() {
        delete tableLock;
    }
};

struct NetworkMVTable {
    int index;
    Lock *tableLock;
    std::map<int, NetworkMV*> mvs;

    NetworkMVTable() {
        index = 0;
        tableLock = new Lock();
    }

    ~NetworkMVTable() {
        delete tableLock;
    }
};

#endif
