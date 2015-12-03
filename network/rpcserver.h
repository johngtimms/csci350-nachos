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
        static int ClientMailbox(int _machine, int process, int thread);

        // Server-to-Client and Server-to-Server response messages
        // For Server-to-Client responses, mailbox is obtained from a call to ClientMailbox
        // For Server-to-Server messages, mailbox is one of the defined mailbox numbers above
        // plus 100, because the receiving server will be waiting there for "yes" or "no" (see SendQuery)
        // response is either "yes" or "no" (pass -1 or -2, respectively)
        // if an int greater than or equal to zero is passed as the response, it will be sent unmodified (so that GetMV will work)
        // Clients will interpret "yes" as success and "no" as an error
        // Sending "yes" to a server tells it the sending server can handle the query
        // Sending "no" to a server tells it the sending server cannot handle the query
        // machine calculated from mailbox for Server-to-Client calls, for Server-to-Server calls it comes from machine
        static void SendResponse(int mailbox, int response, int _machine = -1);

        // Server-to-Server query messages
        // mailboxTo is one of the defined mailbox numbers above
        // mailboxFrom is the Server-to-Client mailbox obtained from ClientMailbox, which will be negated
        // The negation of mailboxFrom tells the receiving server this is a Server-to-Server call
        // query is the original query from the client
        // identifier is a description for debugging
        static bool SendQuery(int mailboxTo, int mailboxFrom, std::string query, char *identifier);
};

class NetworkLock {
    public:
        NetworkLock(std::string _name);
        ~NetworkLock();
        void Acquire(int _mailbox);
        void Release(int _mailbox);
        bool HasAcquired(int _mailbox);
        std::string getName() { return name; }

    private:
        int mailbox; // If this is set, it means the lock is held
        std::string name;
        List *queue;
};

class NetworkCondition {
    public:
        NetworkCondition(std::string _name);
        ~NetworkCondition();
        void Wait(int mailbox, NetworkLock *lock);
        void Signal(int mailbox, NetworkLock *lock, bool callingClientMessage = true);
        void Broadcast(int mailbox, NetworkLock *lock);
        
    private:
        std::string name;
        NetworkLock *conditionLock;
        List *queue;
};

class NetworkMV {
    public:
        NetworkMV(std::string _name);
        ~NetworkMV();
        int getMV();
        void setMV(int _value);
       
    private:
        std::string name;
        int value;
};

struct NetworkLockTable {
    Lock *tableLock;
    std::map<std::string, NetworkLock*> locks;

    NetworkLockTable() {
        tableLock = new Lock();
    }

    ~NetworkLockTable() {
        delete tableLock;
    }
};

struct NetworkConditionTable {
    Lock *tableLock;
    std::map<std::string, NetworkCondition*> conditions;

    NetworkConditionTable() {
        tableLock = new Lock();
    }

    ~NetworkConditionTable() {
        delete tableLock;
    }
};

struct NetworkMVTable {
    Lock *tableLock;
    std::map<std::string, NetworkMV*> mvs;

    NetworkMVTable() {
        tableLock = new Lock();
    }

    ~NetworkMVTable() {
        delete tableLock;
    }
};

#endif
