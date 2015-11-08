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
};

struct NetworkLock {
    Lock *lock;
    int processID;
    int threadID;

    NetworkLock(int _processID, int _threadID) {
        lock = new Lock();
        processID = _processID;
        threadID = _threadID;
    }

    ~NetworkLock() {
        delete lock;
    }
};

struct NetworkCondition {
    Condition *condition;
    int processID;
    int threadID;

    NetworkCondition(int _processID, int _threadID) {
        condition = new Condition();
        processID = _processID;
        threadID = _threadID;
    }

    ~NetworkCondition() {
        delete condition;
    }
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

#endif
