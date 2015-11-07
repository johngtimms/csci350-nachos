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

class RPCServer {
    public:
        RPCServer();
        ~RPCServer();

        void Receive_NetPrint();
};

// class RPCServer {
//     public:
//         RPCServer(NetworkAddress address, double reliability);
//         ~RPCServer();

//         void ServerHelper();
//         void ReceivePacket();
//         void PacketSent();

//     private:
//         Network *network;               // Physical network connection
//         // NetworkAddress netAddr;      // Network address of this machine
//         Semaphore *packetAvailable;     // V'ed when packet has arrived from network
//         Semaphore *packetSent;	        // V'ed when next message can be sent to network
//         // Lock *sendLock;              // Only one outgoing message at a time
// };

#endif
