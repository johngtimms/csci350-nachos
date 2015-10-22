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

int nextAvailableCustomerIndex = 0;
int clerkCounter = 0;
int nextAvailablePassportClerkIndex = 0;
int nextAvailableCashierClerkIndex = 0;
int nextAvailablePictureClerkIndex = 0;
int nextAvailableSenatorIndex = 0;
int nextAvailableApplicationClerkIndex = 0;

bool runningTest1 = false;
bool runningTest2 = false;
bool runningTest3 = false;
bool runningTest4 = false;
bool runningTest5 = false;
bool runningTest6 = false;
bool runningTest7a = false;
bool runningTest7b = false;
typedef enum {FREE, BUSY, BREAK} ClerkState;
typedef enum {APPLICATION_CLERK, PICTURE_CLERK, PASSPORT_CLERK, CASHIER} ClerkType;
int amounts[] = {100, 600, 1100, 1600}; /*random amounts of money customer can start with */


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

typedef struct Clerk {
	
	int lineLength;
	int bribeLineLength;
	int senatorLineLength;
	int money;

	ClerkState state;
	Customer customer;	

	int lineLock, bribeLineLock, senatorLineLock, clerkLock, moneyLock;
	int lineCV, bribeLineCV, senatorLineCV, clerkCV, breakCV;

} Clerk;

Customer customers[10];
Clerk passportClerks[3];
Clerk pictureClerks[3];
Clerk applicationClerks[3];

void initCustomer(int i, bool _isSenator){
	customers[i].isSenator = _isSenator;
    customers[i].clerkID = -1;
    customers[i].SSN = i;
    customers[i].money = amounts[(int)(Rand() % 4)];
    customers[i].hasApp = false;
    customers[i].hasPic = false;
    customers[i].certifiedByPassportClerk = false;
    customers[i].hasPassport = false;
    customers[i].seenApp = false;
    customers[i].seenPic = false;
    customers[i].likedPic = false;
    customers[i].hasPaidForPassport = false;
}

void initApplicationClerk(int i){
	
	applicationClerks[i].lineLength = 0;
	applicationClerks[i].bribeLineLength = 0;
	applicationClerks[i].senatorLineLength = 0;
	applicationClerks[i].money = 0;
	applicationClerks[i].state = BUSY;
	/*
	applicationClerks[i].lineLock = CreateLock();
	applicationClerks[i].bribeLineLock = CreateLock();
	applicationClerks[i].senatorLineLock = CreateLock();
	applicationClerks[i].moneyLock = CreateLock();
	applicationClerks[i].lineCV = CreateCondition();
	applicationClerks[i].bribeLineCV = CreateCondition();
	applicationClerks[i].senatorLineCV = CreateCondition();
	applicationClerks[i].clerkCV = CreateCondition();
	applicationClerks[i].breakCV = CreateCondition();
	*/

}

void initPassportClerk(int i){
	
	passportClerks[i].lineLength = 0;
	passportClerks[i].bribeLineLength = 0;
	passportClerks[i].senatorLineLength = 0;
	passportClerks[i].money = 0;
	passportClerks[i].state = BUSY;
    /*
	passportClerks[i].lineLock = CreateLock();
	passportClerks[i].bribeLineLock = CreateLock();
	passportClerks[i].senatorLineLock = CreateLock();
	passportClerks[i].moneyLock = CreateLock();
	passportClerks[i].lineCV = CreateCondition();
	passportClerks[i].bribeLineCV = CreateCondition();
	passportClerks[i].senatorLineCV = CreateCondition();
	passportClerks[i].clerkCV = CreateCondition();
	passportClerks[i].breakCV = CreateCondition();
	*/

}


/*
Customer newCustomer(int index, bool _isSenator){
	Customer customer;
	
	customer.isSenator = _isSenator;
	customer.clerkID = -1;
	customer.SSN = index;
	money = amounts[(int)(Rand() % 4)];
	
	customer.hasApp = false;
	customer.hasPic = false;
	customer.certifiedByPassportClerk = false;
	customer.hasPassport = false;
	customer.seenApp = false;
	customer.seenPic = false;
	customer.likedPic = false;
	customer.hasPaidForPassport = false;
	
	return &(customer);
}
*/

void doApplication(int id){
}
void doPicture(int id){
	waitInLine(id,pictureClerks);
}
void doPassport(int id){
	/*
	waitInLine(customer, passportClerks, numPassportClerks);
	Clerk* clerk = passportClerks[clerkID];
	cout<<clerk.getName()<<" has signalled "<<this.getName()<<" to come to their counter."<<endl;
	clerk.clerkLock.Acquire();
    clerk.customer = this;
    //cout << getName() << " currently with " << clerk.getName() << endl;
    clerk.clerkCV.Signal(clerk.clerkLock); 
    clerk.clerkCV.Wait(clerk.clerkLock); 
    if(hasApp && hasPic) {
        //cout << getName() << " accepted passport from " << clerk.getName() << endl;
        //certifiedByPassportClerk = true; passport clerk should be setting this
        //cout << getName() << " finished with " << clerk.getName() << endl;
        clerk.clerkCV.Signal(clerk.clerkLock);
        clerk.clerkLock.Release();
    } else {
        clerk.clerkCV.Signal(clerk.clerkLock);
        clerk.clerkLock.Release();
        int wait = rand() % ((100 - 20) + 1) + 20;
        for(int i = 0; i < wait; i++) 
            Yield();
    }
    */

}
void doCashier(int id){
}


void runCustomer() {
	int i;
	i = nextAvailableCustomerIndex;
	initCustomer(i,false); 
	
	nextAvailableCustomerIndex = nextAvailableCustomerIndex + 1;   /*temporary keeping track of index*/

	Acquire(senatorOutsideLineLock);
	Acquire(senatorInsideLock);

	if (senatorsOutside > 0 || senatorInside) {
		Release(senatorInsideLock);
		Release(senatorOutsideLineLock);
		Acquire(customerOutsideLineLock);
		customersOutside += 1;
		Wait(customerOutsideLineCV,customerOutsideLineLock);
		customersOutside -= 1;
		Release(customerOutsideLineLock);
	}
	
	Release(senatorInsideLock);
	Release(senatorOutsideLineLock);
	
	if(Rand() % 10 == 0 && runningTest1 == false) /* Customer has 1 in 10 chance of going to Passport Clerk first*/
        doPassport(customers[i].SSN);
    /* Randomly decide whether to go to AppClerk or PicClerk first */
    if(runningTest1) { /*if running test 1, then just go to the application clerk*/
    	doApplication(customers[i].SSN);
    	return;
    } else if(runningTest3 || runningTest5) { /* go straight to cashier for these tests */
    	doCashier(customers[i].SSN);
    	return;
    } else if(Rand() % 2 == 0) {
        doApplication(customers[i].SSN);
        doPicture(customers[i].SSN);
    } else {
        doPicture(customers[i].SSN);
        doApplication(customers[i].SSN);
    }
	if(Rand() % 10 == 0 && runningTest1 == false) /* Customer has 1 in 10 chance of going to Cashier before PassportClerk */
        doCashier(customers[i].SSN);
    doPassport(customers[i].SSN);
    do{
    	doCashier(customers[i].SSN);
    }while(customers[i].certifiedByPassportClerk == true); /* until the customer has been certified, keep him in line with cashier */
  	

	Exit(0);
}

void runPassportClerk() {
	int i;
	i = nextAvailablePassportClerkIndex;
	Print("running passportClerk: %i\n",i);
	initPassportClerk(i);	
	
	nextAvailablePassportClerkIndex = nextAvailablePassportClerkIndex + 1; /*temporary keeping track of index*/
	Exit(0);
}

void runApplicationClerk() {
	int i;
	i = nextAvailableApplicationClerkIndex;
	Print("running applicationClerk: %i\n",i);
	initApplicationClerk(i);
	nextAvailableApplicationClerkIndex = nextAvailableApplicationClerkIndex + 1; /*temporary keeping track of index*/

	Exit(0);
}

void runPictureClerk() {
	int i;
	i = nextAvailablePictureClerkIndex;
	Print("running applicationClerk: %i\n",i);
	initApplicationClerk(i);
	nextAvailablePictureClerkIndex = nextAvailablePictureClerkIndex + 1; /*temporary keeping track of index*/

	Exit(0);
}

int main() {
	int k;
	senatorOutsideLineLock = CreateLock();
	senatorOutsideLineCV = CreateCondition();
	senatorInsideLock = CreateLock();
	customerOutsideLineLock = CreateLock();
	customerOutsideLineCV = CreateCondition();
	
	numCustomers = 10;
	numPassportClerks = 3;
	numPictureClerks = 3;
	numApplicationClerks = 3;


	for(k = 0 ; k < numPassportClerks ; k++) {
		Fork(&runPassportClerk);
	}

	for(k = 0 ; k < numPictureClerks ; k++) {
		Fork(&runPictureClerk);
	}

	for(k = 0 ; k < numApplicationClerks ; k++) {
		Fork(&runApplicationClerk);
	}

	for(k = 0 ; k < numCustomers ; k++) {
		Fork(&runCustomer);
	}
	
	

}



/*

void waitInLine(Customer customer, Clerk** clerks, int numClerks){

}
void chooseLine(Clerk** clerks, int numClerks){

}
bool enterLine(Clerk* clerk){

}


*/




