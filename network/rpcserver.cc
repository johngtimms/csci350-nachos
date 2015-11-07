// rpcserver.cc

#include "system.h"
#include "network.h"
#include "rpcserver.h"
#include "interrupt.h"

// Dummy functions (see post.cc for alternate implementation)
static void DummyServerHelper(int arg)
{ RPCServer* rpcs = (RPCServer *) arg; rpcs->ServerHelper(); }
static void DummyReceivePacket(int arg)
{ RPCServer* rpcs = (RPCServer *) arg; rpcs->ReceivePacket(); }
static void DummyPacketSent(int arg)
{ RPCServer* rpcs = (RPCServer *) arg; rpcs->PacketSent(); }

// Called from system.cc to set up an RPCServer object
RPCServer::RPCServer(NetworkAddress address, double reliability) {
	// These semaphores will be raised by the Network when
	// appropriate, then ServerHelper will handle them
	packetAvailable = new Semaphore("packet available", 0);
	packetSent = new Semaphore("packet sent", 0);

	// Set up the network
	network = new Network(address, reliability, DummyReceivePacket, DummyPacketSent, (int) this);
}

// TODO document
RPCServer::~RPCServer() {
	delete network;
	delete packetAvailable;
	delete packetSent;

    // delete sendLock; // TODO what's this for??
}

// TODO document
void RPCServer::ServerHelper() {
	printf("Hello, I am a server helper.\n");



 //    PacketHeader pktHdr;
 //    MailHeader mailHdr;
 //    char *buffer = new char[MaxPacketSize];

 //    for (;;) {
 //        // first, wait for a message
 //        packetAvailable->P();	
 //        pktHdr = network->Receive(buffer);

 //        mailHdr = *(MailHeader *)buffer;
 //        if (DebugIsEnabled('n')) {
	//     printf("Putting mail into mailbox: ");
	//     PrintHeader(pktHdr, mailHdr);
 //        }

	// // check that arriving message is legal!
	// ASSERT(0 <= mailHdr.to && mailHdr.to < numBoxes);
	// ASSERT(mailHdr.length <= MaxMailSize);

	// // put into mailbox
 //        boxes[mailHdr.to].Put(pktHdr, mailHdr, buffer + sizeof(MailHeader));
 //    }
}

// The constructor sets up the Network. When the Network gets a packet, it knows
// to call this function. ServerHelper() handles the packetAvailable semaphore.
void RPCServer::ReceivePacket() { 
    packetAvailable->V(); 
}

// The constructor sets up the Network. When the Network sends a packet, it knows
// to call this function. ServerHelper() handles the packetSent semaphore.
void RPCServer::PacketSent() { 
    packetSent->V();
}

// The server function called by main.cc
void RunServer() {
	printf("Hello, I am a server.\n");

    Thread *t = new Thread("server helper");
    t->Fork(DummyServerHelper, (int) rpcServer); // Note: rpcServer defined in system.cc

    // Not sure how to handle closing the server? Is there required shut down behavior?
	// interrupt->Halt();
}
