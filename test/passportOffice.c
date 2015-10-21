#include "syscall.h"

typedef int bool;
#define true 1
#define false 0

struct Customer;
struct Clerk;
struct Manager;


typedef struct Customer {
	
	int money;
	int clerkID;
	int SSN;
	bool didBribe;		
	bool isSenator;
	bool hasApp;
	bool hasPic;
	bool certifiedByPassportClerk;
	bool hasPassport;
	bool hasPaidForPassport;
	bool seenApp;
	bool seenPic;
	bool likedPic;

} Customer;

Customer* newCustomer(int index, bool _isSenator){
	struct Customer *customer;
	
	customer->isSenator = _isSenator;
	customer->clerkID = -1;
	customer->SSN = index;
	/*money = amounts[(int)(rand() % 4)];*/
	
	customer->hasApp = false;
	customer->hasPic = false;
	customer->certifiedByPassportClerk = false;
	customer->hasPassport = false;
	customer->seenApp = false;
	customer->seenPic = false;
	customer->likedPic = false;
	customer->hasPaidForPassport = false;
	
	return customer;
}

void runCustomer(Customer customer){

	Exit(0);
}

int main() {

	Customer *customer = newCustomer(0, false);
	
	Fork(&runCustomer);
	
	

}


/*
void doApplication(struct Customer customer){

}
void doPicture(){

}
void doPassport(){

}
void doCashier(){

}

void waitInLine(Clerk** clerks, int numClerks){

}
void chooseLine(Clerk** clerks, int numClerks){

}
bool enterLine(Clerk* clerk){

}
*/






