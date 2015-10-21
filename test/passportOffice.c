#include "syscall.h"

typedef int bool;
#define true 1
#define false 0

struct Customer;
struct Clerk;
struct Manager;

int senatorOutsideLineLock;
int senatorOutsideLineCV;
int senatorsOutside = 0;
int senatorInsideLock;
bool senatorInside = false;
int customerOutsideLineLock;
int customerOutsideLineCV;
int customersOutside = 0;
int numCustomers, numApplicationClerks, numPictureClerks, numPassportClerks, numCashiers, numSenators;

int customerCounter = 0;

bool runningTest1 = false;
bool runningTest2 = false;
bool runningTest3 = false;
bool runningTest4 = false;
bool runningTest5 = false;
bool runningTest6 = false;
bool runningTest7a = false;
bool runningTest7b = false;


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

Customer* customers[10];

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

void doApplication(Customer *customer){

}
void doPicture(Customer *customer){

}
void doPassport(Customer *customer){
	/*
	waitInLine(customer, passportClerks, numPassportClerks);
	Clerk* clerk = passportClerks[clerkID];
	cout<<clerk->getName()<<" has signalled "<<this->getName()<<" to come to their counter."<<endl;
	clerk->clerkLock->Acquire();
    clerk->customer = this;
    //cout << getName() << " currently with " << clerk->getName() << endl;
    clerk->clerkCV->Signal(clerk->clerkLock); 
    clerk->clerkCV->Wait(clerk->clerkLock); 
    if(hasApp && hasPic) {
        //cout << getName() << " accepted passport from " << clerk->getName() << endl;
        //certifiedByPassportClerk = true; passport clerk should be setting this
        //cout << getName() << " finished with " << clerk->getName() << endl;
        clerk->clerkCV->Signal(clerk->clerkLock);
        clerk->clerkLock->Release();
    } else {
        clerk->clerkCV->Signal(clerk->clerkLock);
        clerk->clerkLock->Release();
        int wait = rand() % ((100 - 20) + 1) + 20;
        for(int i = 0; i < wait; i++) 
            Yield();
    }
    */

}
void doCashier(Customer *customer){

}

void runCustomer(){
	Customer *customer = newCustomer(customerCounter, false);
	customers[customerCounter] = customer;
	Print("running customer: %i\n",customerCounter);
	customerCounter = customerCounter + 1;
	/*Exit(0);*/
}



int main() {
	int k;
	senatorOutsideLineLock = CreateLock();
	senatorOutsideLineCV = CreateCondition();
	senatorInsideLock = CreateLock();
	customerOutsideLineLock = CreateLock();
	customerOutsideLineCV = CreateCondition();
	
	numCustomers = 10;

	
	
	for(k = 0 ; k < numCustomers ; k++) {
		Fork(&runCustomer);
	}
	

}



/*

void waitInLine(Customer *customer, Clerk** clerks, int numClerks){

}
void chooseLine(Clerk** clerks, int numClerks){

}
bool enterLine(Clerk* clerk){

}


*/




