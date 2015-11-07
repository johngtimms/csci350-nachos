// rpcserver.h

#ifndef RPCSERVER_H
#define RPCSERVER_H

#include "network.h"
#include "synchlist.h"

class RPCServer {
    public:
        RPCServer(NetworkAddress address, double reliability);
        ~RPCServer();

        void ServerHelper();
        void ReceivePacket();
        void PacketSent();

    private:
        Network *network;               // Physical network connection
        // NetworkAddress netAddr;      // Network address of this machine
        Semaphore *packetAvailable;     // V'ed when packet has arrived from network
        Semaphore *packetSent;	        // V'ed when next message can be sent to network
        // Lock *sendLock;              // Only one outgoing message at a time
};

#endif
