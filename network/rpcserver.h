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

        static int ClientMailbox(int process, int thread);
        static void SendResponse(int machineID, int mailbox, int response);
};

class NetworkLock {
    public:
        NetworkLock(int _machineID, int process, char *name);
        ~NetworkLock();
        void Acquire(int _machineID, int process, int thread);
        void Release(int _machineID, int process, int thread);
        bool IsOwner(int _machineID);
        bool HasAcquired(int mailbox);
        char *name;

    private:
        int machineID;
        int processID;
        int threadID;
        int mailboxID;
        List *queue;
};

class NetworkCondition {
    public:
        NetworkCondition(int _machineID, int process, char *name);
        ~NetworkCondition();
        void Wait(int _machineID, int process, int thread, NetworkLock *lock);
        void Signal(int _machineID, int process, int _thread, NetworkLock *lock);
        void Broadcast(int _machineID, int process, int _thread, NetworkLock *lock);
        bool IsOwner(int _machineID);
        char *name;

    private:
        int machineID;
        int processID;
        int mailboxID;
        NetworkLock *conditionLock;
        List *queue;
};

class NetworkMV {
    public:
        NetworkMV(int _machineID, int process, char *name);
        ~NetworkMV();
        bool IsOwner(int process);
        int value;
        char *name;
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
