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
int nextAvailablePassportClerkIndex = 0;
int nextAvailableCashierClerkIndex = 0;
int nextAvailablePictureClerkIndex = 0;
int nextAvailableSenatorIndex = 0;
int nextAvailableApplicationClerkIndex = 0;

int customerIndexLock, applicationClerkIndexLock, pictureClerkIndexLock, passportClerkIndexLock, cashierIndexLock, senatorIndexLock;

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
	int customerID;

	ClerkState state;
	Customer customer;	

	int lineLock, bribeLineLock, senatorLineLock, clerkLock, moneyLock;
	int lineCV, bribeLineCV, senatorLineCV, clerkCV, breakCV;

} Clerk;

Customer customers[10];
Clerk cashiers[3];
Clerk passportClerks[3];
Clerk pictureClerks[3];
Clerk applicationClerks[3];

void waitInLine(int ssn, ClerkType clerkType);
bool enterLine(int ssn, ClerkType clerkType, int clerkID);
bool enterCashierLine(int ssn, int clerkID);
bool enterPassportLine(int ssn, int clerkID);
bool enterPictureLine(int ssn, int clerkID);
bool enterApplicationLine(int ssn, int clerkID);
int chooseLine(int ssn, ClerkType clerkType);

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

void initClerk(int i){
	
	applicationClerks[i].lineLength = 0;
	applicationClerks[i].bribeLineLength = 0;
	applicationClerks[i].senatorLineLength = 0;
	applicationClerks[i].money = 0;
	applicationClerks[i].state = BUSY;
	
	applicationClerks[i].lineLock = CreateLock();
	applicationClerks[i].bribeLineLock = CreateLock();
	applicationClerks[i].senatorLineLock = CreateLock();
	applicationClerks[i].moneyLock = CreateLock();
	applicationClerks[i].lineCV = CreateCondition();
	applicationClerks[i].bribeLineCV = CreateCondition();
	applicationClerks[i].senatorLineCV = CreateCondition();
	applicationClerks[i].clerkCV = CreateCondition();
	applicationClerks[i].breakCV = CreateCondition();
	

}


void doApplication(int ssn){
	int clerkID;
	waitInLine(ssn, APPLICATION_CLERK);
	clerkID = customers[ssn].clerkID;
	Print("Application Clerk %i has signalled ", clerkID);
	Print("Customer %i to come to their counter", ssn);
	applicationClerks[clerkID].customerID = ssn;
	/* Interaction with clerk */
	Acquire(applicationClerks[clerkID].clerkLock);
    Signal(applicationClerks[clerkID].clerkCV,applicationClerks[clerkID].clerkLock); /* Give incomplete application to Application Clerk */
    Print("Customer %i has given SSN ",ssn);
    Print("%i to ",ssn);
    Print("Application Clerk %i",clerkID);
    Wait(applicationClerks[clerkID].clerkCV,applicationClerks[clerkID].clerkLock);   /* Wait for Application Clerk */
    Signal(applicationClerks[clerkID].clerkCV,applicationClerks[clerkID].clerkLock); /* Accept completed application */

    customers[ssn].hasApp = true;
    Release(applicationClerks[clerkID].clerkLock);
}
void doPicture(int id){
	/*waitInLine(id,pictureClerks);*/
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

	Acquire(customerIndexLock);
	i = nextAvailableCustomerIndex;
	nextAvailableCustomerIndex = nextAvailableCustomerIndex + 1;   /*temporary keeping track of index*/
	Release(customerIndexLock);

	initCustomer(i,false); 
	Print("Running Customer %i\n",i);

	
	

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
    }while(customers[i].certifiedByPassportClerk == false); /* until the customer has been certified, keep him in line with cashier */
  	

	Exit(0);
}

void runApplicationClerk() {
	int i;
	Print("Before Acquire(applicationClerkIndexLock);\n", 0);
	Acquire(applicationClerkIndexLock);
	i = nextAvailableApplicationClerkIndex;
	nextAvailableApplicationClerkIndex = nextAvailableApplicationClerkIndex + 1;
	Release(applicationClerkIndexLock);
	Print("After Release(applicationClerkIndexLock);\n", 0);
	Print("running ApplicationClerk: %i\n",i);
	initClerk(i);
	while(true) {
    	if(applicationClerks[i].senatorLineLength > 0) {
    		Acquire(applicationClerks[i].senatorLineLock);
            Signal(applicationClerks[i].senatorLineCV, applicationClerks[i].senatorLineLock);     /* Signal Customer to exit line */
            Print("ApplicationClerk %i has signalled a Senator to come to their counter", i);
            Acquire(applicationClerks[i].clerkLock); 
            Release(applicationClerks[i].senatorLineLock);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for customer to give application */
            Print("ApplicationClerk %i ", i);
            Print("has recieved SSN %i", applicationClerks[i].customer.SSN);
            Print("from Senator %i\n", applicationClerks[i].customer.SSN);
            Signal(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);    /* Process application */
            Print("ApplicationClerk %i ", i);
            Print("has recorded a completed application from Senator %i\n", applicationClerks[i].customer.SSN);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for Customer to accept completed application */
            /*applicationClerks[i].customer = NULL;  no null in C? */
            Release(applicationClerks[i].clerkLock);
            applicationClerks[i].state = FREE;
    	} else if(applicationClerks[i].bribeLineLength > 0) {
            Acquire(applicationClerks[i].bribeLineLock);
            Signal(applicationClerks[i].bribeLineCV, applicationClerks[i].bribeLineLock);     /* Signal Customer to exit line */
            Print("ApplicationClerk %i has signalled a Customer to come to their counter", i);
            Acquire(applicationClerks[i].clerkLock); 
            Release(applicationClerks[i].bribeLineLock);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for customer to give application */
            Print("ApplicationClerk %i ", i);
            Print("has recieved SSN %i", applicationClerks[i].customer.SSN);
            Print("from Customer %i\n", applicationClerks[i].customer.SSN);
            Signal(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);    /* Process application */
            Print("ApplicationClerk %i ", i);
            Print("has accepted application from Customer %i\n", applicationClerks[i].customer.SSN);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for Customer to accept completed application */
            /*applicationClerks[i].customer = NULL; no null in C? */
            Release(applicationClerks[i].clerkLock);
            applicationClerks[i].state = FREE;
        } else if(applicationClerks[i].lineLength > 0) {
            Acquire(applicationClerks[i].lineLock);
            Signal(applicationClerks[i].lineCV, applicationClerks[i].lineLock);     /* Signal Customer to exit line */
            Print("ApplicationClerk %i has signalled a Customer to come to their counter", i);
            Acquire(applicationClerks[i].clerkLock); 
            Release(applicationClerks[i].lineLock);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for customer to gve application */
            Print("ApplicationClerk %i ", i);
            Print("has recieved SSN %i", applicationClerks[i].customer.SSN);
            Print("from Customer %i\n", applicationClerks[i].customer.SSN);
            Signal(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);    /* Process application */
            Print("ApplicationClerk %i ", i);
            Print("has accepted application from Customer %i\n", applicationClerks[i].customer.SSN);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for Customer to accept completed application */
            /* applicationClerks[i].customer = NULL;  no null in c? */
            Release(applicationClerks[i].clerkLock);
            applicationClerks[i].state = FREE;
        } else {
            Acquire(applicationClerks[i].clerkLock);
            applicationClerks[i].state = BREAK;
            Print("ApplicationClerk %i is going on break\n", i);
            Wait(applicationClerks[i].breakCV, applicationClerks[i].clerkLock);
            Print("ApplicationClerk %i is coming off break\n",i);
            Release(applicationClerks[i].clerkLock);
            applicationClerks[i].state = FREE;
        }
    }
	Exit(0);
}

void runPictureClerk() {
	int i, wait, k;
	Print("Before Acquire(pictureClerkIndexLock);\n",0);
	Acquire(pictureClerkIndexLock);
	i = nextAvailablePictureClerkIndex;
	nextAvailablePictureClerkIndex = nextAvailablePictureClerkIndex + 1;
	Release(pictureClerkIndexLock);
	Print("After Release(pictureClerkIndexLock);\n",0);
	Print("Running PictureClerk: %i\n",i);
	initClerk(i);
	while(true) {
    	if(pictureClerks[i].senatorLineLength > 0) {
    		Acquire(pictureClerks[i].senatorLineLock);
            Signal(pictureClerks[i].senatorLineCV,pictureClerks[i].senatorLineLock);     /* Signal Senator to exit line */
            Print("PictureClerk %i has signalled a Senator to come to their counter", i);
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].senatorLineLock);
            Wait(pictureClerks[i].clerkCV,pictureClerks[i].clerkLock);      /* Wait for Senator to get ready for picture */
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i",pictureClerks[i].customer.SSN);
            Print("from Senator %i\n",pictureClerks[i].customer.SSN);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Senator %i\n",pictureClerks[i].customer.SSN);
            Signal(pictureClerks[i].clerkCV,pictureClerks[i].clerkLock);    /* Give picture back */
            Wait(pictureClerks[i].clerkCV,pictureClerks[i].clerkLock);      /* Wait for Senator to accept picture */
            if(pictureClerks[i].customer.likedPic) {
            	Print("PictureClerk %i ",i);
            	Print("has been told that Senator %i does like their picture\n",pictureClerks[i].customer.SSN);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            }else{
            	Print("PictureClerk %i ",i);
            	Print("has been told that Senator %i does not like their picture\n",pictureClerks[i].customer.SSN);
            }
            /*pictureClerks[i].customer = NULL;  no null in C? */
            Release(pictureClerks[i].clerkLock);
            pictureClerks[i].state = FREE;
    	} else if(pictureClerks[i].bribeLineLength > 0) {
            Acquire(pictureClerks[i].bribeLineLock);
            Signal(pictureClerks[i].bribeLineCV,pictureClerks[i].bribeLineLock);     /* Signal Customer to exit line */
            Print("PictureClerk %i has signalled a Customer to come to their counter", i);
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].bribeLineLock);
            Wait(pictureClerks[i].clerkCV,pictureClerks[i].clerkLock);      /* Wait for customer to get ready for picture */
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i",pictureClerks[i].customer.SSN);
            Print("from Customer %i\n",pictureClerks[i].customer.SSN);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Customer %i\n", pictureClerks[i].customer.SSN);
            Signal(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);    /* Give picture back */
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for Customer to accept picture */
            if(pictureClerks[i].customer.likedPic) {
                Print("PictureClerk %i ",i);
            	Print("has been told that Customer %i does like their picture\n", pictureClerks[i].customer.SSN);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            }else{
            	Print("PictureClerk %i ", i);
            	Print("has been told that Customer %i does not like their picture\n", pictureClerks[i].customer.SSN);
                }
            /*pictureClerks[i].customer = NULL; no null in C? */
            Release(pictureClerks[i].clerkLock);
            pictureClerks[i].state = FREE;
        } else if(pictureClerks[i].lineLength > 0) {
            Acquire(pictureClerks[i].lineLock);
            Signal(pictureClerks[i].lineCV,pictureClerks[i].lineLock);     /* Signal Customer to exit line */
            Print("PictureClerk %i has signalled a Customer to come to their counter", i);
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].lineLock);
            Wait(pictureClerks[i].clerkCV,pictureClerks[i].clerkLock);      /* Wait for customer to get ready for picture */
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i",pictureClerks[i].customer.SSN);
            Print("from Customer %i\n",pictureClerks[i].customer.SSN);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Customer %i\n",pictureClerks[i].customer.SSN);
            Signal(pictureClerks[i].clerkCV,pictureClerks[i].clerkLock);    /* Give picture back */
            Wait(pictureClerks[i].clerkCV,pictureClerks[i].clerkLock);      /* Wait for Customer to accept picture */
            if(pictureClerks[i].customer.likedPic) {
                Print("PictureClerk %i ",i);
            	Print("has been told that Customer %i does like their picture\n",pictureClerks[i].customer.SSN);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            }else{
            	Print("PictureClerk %i ",i);
            	Print("has been told that Customer %i does not like their picture\n",pictureClerks[i].customer.SSN);
                }
            /* pictureClerks[i].customer = NULL;  no null in c? */
            Release(pictureClerks[i].clerkLock);
            pictureClerks[i].state = FREE;
        } else {
            Acquire(pictureClerks[i].clerkLock);
            pictureClerks[i].state = BREAK;
            Print("PictureClerk %i is going on break\n",i);
            Wait(pictureClerks[i].breakCV,pictureClerks[i].clerkLock);
            Print("PictureClerk %i is coming off break\n",i);
            Release(pictureClerks[i].clerkLock);
            pictureClerks[i].state = FREE;
        }
    }
	Exit(0);
}


void runPassportClerk() {
	int i, wait, k;
	Print("Before Acquire(passportClerkIndexLock);\n", 0);
	Acquire(passportClerkIndexLock);
	i = nextAvailablePassportClerkIndex;
	nextAvailablePassportClerkIndex = nextAvailablePassportClerkIndex + 1; /*temporary keeping track of index*/
	Release(passportClerkIndexLock);
	Print("After Release(passportClerkIndexLock);\n", 0);
	Print("Running PassportClerk: %i\n",i);
	initClerk(i);
	while(true) {
    	if(passportClerks[i].senatorLineLength > 0) {
    		Acquire(passportClerks[i].senatorLineLock);
            Signal(passportClerks[i].senatorLineCV, passportClerks[i].senatorLineLock);     /* Signal Senator to exit line */
            Print("PassportClerk %i has signalled a Senator to come to their counter", i);
            Acquire(passportClerks[i].clerkLock); 
            Release(passportClerks[i].senatorLineLock);
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Senator to hand over picture and application */
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i", passportClerks[i].customer.SSN);
            Print("from Senator %i\n", passportClerks[i].customer.SSN);
            if(passportClerks[i].customer.hasApp && passportClerks[i].customer.hasPic) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i has both their application and picture completed\n", passportClerks[i].customer.SSN);
            	 wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++) {             // Process application and picture
                    Yield();
                }
				Print("PassportClerk %i has recorded ", i);
            	Print("Senator %i's passport documentation.\n", passportClerks[i].customer.SSN);
                passportClerks[i].customer.certifiedByPassportClerk = true;
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i does not have both their application and picture completed\n", passportClerks[i].customer.SSN);
            }
            Signal(passportClerks[i].clerkCV, passportClerks[i].clerkLock);    /* Give Senator a passport */
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Senator to accept passport */
            /*passportClerks[i].customer = NULL;  no null in C? */
            Release(passportClerks[i].clerkLock);
            passportClerks[i].state = FREE;
    	} else if(passportClerks[i].bribeLineLength > 0) {
            Acquire(passportClerks[i].bribeLineLock);
            Signal(passportClerks[i].bribeLineCV, passportClerks[i].bribeLineLock);     /* Signal Customer to exit line */
            Print("PassportClerk %i has signalled a Customer to come to their counter", i);
            Acquire(passportClerks[i].clerkLock); 
            Release(passportClerks[i].bribeLineLock);
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);       /* Wait for Customer to hand over picture and application */
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i", passportClerks[i].customer.SSN);
            Print("from Customer %i\n", passportClerks[i].customer.SSN);
            if(passportClerks[i].customer.hasApp && passportClerks[i].customer.hasPic) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i has both their application and picture completed\n", passportClerks[i].customer.SSN);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++) {             // Process application and picture
                    Yield();
                }
				Print("PassportClerk %i has recorded ", i);
            	Print("Customer %i's passport documentation\n", passportClerks[i].customer.SSN);
                passportClerks[i].customer.certifiedByPassportClerk = true;
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i does not have both their application and picture completed\n", passportClerks[i].customer.SSN);
            }
            Signal(passportClerks[i].clerkCV, passportClerks[i].clerkLock);    /* Give Customer a passport */
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Customer to accept passport */
            /*passportClerks[i].customer = NULL; no null in C? */
            Release(passportClerks[i].clerkLock);
            passportClerks[i].state = FREE;
        } else if(passportClerks[i].lineLength > 0) {
            Acquire(passportClerks[i].lineLock);
            Signal(passportClerks[i].lineCV, passportClerks[i].lineLock);     /* Signal Customer to exit line */
            Print("PassportClerk %i has signalled a Customer to come to their counter", i);
            Acquire(passportClerks[i].clerkLock); 
            Release(passportClerks[i].lineLock);
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Customer to hand over picture and application */
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i", passportClerks[i].customer.SSN);
            Print("from Customer %i\n", passportClerks[i].customer.SSN);
            if(passportClerks[i].customer.hasApp && passportClerks[i].customer.hasPic) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i has both their application and picture completed\n", passportClerks[i].customer.SSN);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++) {             // Process application and picture
                    Yield();
                }
				Print("PassportClerk %i has recorded ", i);
            	Print("Customer %i's passport documentation\n", passportClerks[i].customer.SSN);
                passportClerks[i].customer.certifiedByPassportClerk = true;
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i does not have both their application and picture completed\n", passportClerks[i].customer.SSN);
            }
            Signal(passportClerks[i].clerkCV, passportClerks[i].clerkLock);    /* Give Customer a passport */
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Customer to accept passport */
            /* passportClerks[i].customer = NULL;  no null in c? */
            Release(passportClerks[i].clerkLock);
            passportClerks[i].state = FREE;
        } else {
            Acquire(passportClerks[i].clerkLock);
            passportClerks[i].state = BREAK;
            Print("PassportClerk %i is going on break\n", i);
            Wait(passportClerks[i].breakCV, passportClerks[i].clerkLock);
            Print("PassportClerk %i is coming off break\n", i);
            Release(passportClerks[i].clerkLock);
            passportClerks[i].state = FREE;
        }
    }
	Exit(0);
}

void runCashier() {
	int i;
	Print("Before Acquire(cashierIndexLock);\n", 0);
	Acquire(cashierIndexLock);
	i = nextAvailableCashierIndex;
	nextAvailableCashierIndex = nextAvailableCashierIndex + 1;
	Release(cashierIndexLock);
	Print("After Release(cashierIndexLock);\n", 0);
	Print("running Cashier: %i\n",i);
	initClerk(i);
	while(true) {
    	if(cashiers[i].senatorLineLength > 0) {
    		Acquire(cashiers[i].senatorLineLock);
            Signal(cashiers[i].senatorLineCV, cashiers[i].senatorLineLock);     /* Signal Customer to exit line */
            Print("Cashier %i has signalled a Senator to come to their counter", i);
            Acquire(cashiers[i].clerkLock); 
            Release(cashiers[i].senatorLineLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);      /* Wait for customer to give application */
            Print("Cashier %i ", i);
            Print("has recieved SSN %i", cashiers[i].customer.SSN);
            Print("from Senator %i\n", cashiers[i].customer.SSN);
            if(cashiers[i].customer.certifiedByPassportClerk) {
            	Print("Cashier %i has verified that ", i);
            	Print("Senator %i has been certified by a PassportClerk\n", cashiers[i].customer.SSN);
            	if(cashiers[i].customer.hasPaidForPassport == false){
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Senator %i after ceritification\n", cashiers[i].customer.SSN);
		        	cashiers[i].customer.hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Senator %i their completed passport\n", cashiers[i].customer.SSN);
            } else {
            	if(cashiers[i].customer.hasPaidForPassport == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Senator %i before ceritification. ", cashiers[i].customer.SSN);
            		Print("They are going to the back of my line\n").
		        	cashiers[i].customer.hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            }
            Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);    /* Process application */
            Print("Cashier %i has recorded that ", i);
            Print("Senator %i been given their completed passport\n", cashiers[i].customer.SSN);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);      /* Wait for Customer to accept completed application */
            /*cashiers[i].customer = NULL;  no null in C? */
            Release(cashiers[i].clerkLock);
            cashiers[i].state = FREE;
    	}
        if(cashiers[i].bribeLineLength > 0) {
            Acquire(cashiers[i].bribeLineLock);
            Signal(cashiers[i].bribeLineCV, cashiers[i].bribeLineLock);     /* Signal Customer to exit line */
            Print("Cashier %i has signalled a Customer to come to their counter", i);
            Acquire(cashiers[i].clerkLock); 
            Release(cashiers[i].bribeLineLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);      /* Wait for customer to give application */
            Print("Cashier %i ", i);
            Print("has recieved SSN %i", cashiers[i].customer.SSN);
            Print("from Customer %i\n", cashiers[i].customer.SSN);
            if(cashiers[i].customer.certifiedByPassportClerk) {
            	Print("Cashier %i has verified that ", i);
            	Print("Customer %i has been certified by a PassportClerk\n", cashiers[i].customer.SSN);
            	if(cashiers[i].customer.hasPaidForPassport == false){
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i after ceritification\n", cashiers[i].customer.SSN);
		        	cashiers[i].customer.hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Customer %i their completed passport\n", cashiers[i].customer.SSN);
            } else {
            	if(cashiers[i].customer.hasPaidForPassport == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i before ceritification. ", cashiers[i].customer.SSN);
            		Print("They are going to the back of my line\n").
		        	cashiers[i].customer.hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            }
            Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);    /* Process application */
            Print("Cashier %i has recorded that ", i);
            Print("Customer %i been given their completed passport\n", cashiers[i].customer.SSN);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);      /* Wait for Customer to accept completed application */
            /*cashiers[i].customer = NULL; no null in C? */
            Release(cashiers[i].clerkLock);
            cashiers[i].state = FREE;
        } else if(cashiers[i].lineLength > 0) {
            Acquire(cashiers[i].lineLock);
            Signal(cashiers[i].lineCV, cashiers[i].lineLock);     /* Signal Customer to exit line */
            Print("Cashier %i has signalled a Customer to come to their counter", i);
            Acquire(cashiers[i].clerkLock); 
            Release(cashiers[i].lineLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);      /* Wait for customer to gve application */
            Print("Cashier %i ", i);
            Print("has recieved SSN %i", cashiers[i].customer.SSN);
            Print("from Customer %i\n", cashiers[i].customer.SSN);
            if(cashiers[i].customer.certifiedByPassportClerk) {
            	Print("Cashier %i has verified that ", i);
            	Print("Customer %i has been certified by a PassportClerk\n", cashiers[i].customer.SSN);
            	if(cashiers[i].customer.hasPaidForPassport == false){
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i after ceritification\n", cashiers[i].customer.SSN);
		        	cashiers[i].customer.hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Customer %i their completed passport\n", cashiers[i].customer.SSN);
            } else {
            	if(cashiers[i].customer.hasPaidForPassport == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i before ceritification. ", cashiers[i].customer.SSN);
            		Print("They are going to the back of my line\n").
		        	cashiers[i].customer.hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            }
            Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);    /* Process application */
            Print("Cashier %i has recorded that ", i);
            Print("Customer %i been given their completed passport\n", cashiers[i].customer.SSN);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);      /* Wait for Customer to accept completed application */
            /* cashiers[i].customer = NULL;  no null in c? */
            Release(cashiers[i].clerkLock);
            cashiers[i].state = FREE;
        } else {
            Acquire(cashiers[i].clerkLock);
            cashiers[i].state = BREAK;
            Print("Cashier %i is going on break\n", i);
            Wait(cashiers[i].breakCV, cashiers[i].clerkLock);
            Print("Cashier %i is coming off break\n",i);
            Release(cashiers[i].clerkLock);
            cashiers[i].state = FREE;
        }
    }
	Exit(0);
}

void test1() {
	Print("inside test1\n",0);
	Acquire(senatorOutsideLineLock);
	Print("test 1 acquired lock\n",0);
	Wait(senatorOutsideLineCV,senatorOutsideLineLock);
	Print("test 1 is past wait\n",0);
	Release(senatorOutsideLineLock);
	Exit(0);
}

void test2() {
	Print("inside test2\n",0);
	Acquire(senatorOutsideLineLock);
	Print("test 2 acquired lock\n",0);
	/*Wait(senatorOutsideLineCV, senatorOutsideLineLock);*/
	Signal(senatorOutsideLineCV,senatorOutsideLineLock);
	Release(senatorOutsideLineLock);
	Exit(0);
}

int main() {
	int k;
	senatorOutsideLineLock = CreateLock();
	senatorOutsideLineCV = CreateCondition();
	senatorInsideLock = CreateLock();
	customerOutsideLineLock = CreateLock();
	customerOutsideLineCV = CreateCondition();
	customerIndexLock = CreateLock();
	applicationClerkIndexLock = CreateLock();
	pictureClerkIndexLock = CreateLock();
	passportClerkIndexLock = CreateLock();
	senatorIndexLock = CreateLock();
	cashierIndexLock = CreateLock();
	
	numCustomers = 10;
	numPassportClerks = 3;
	numPictureClerks = 3;
	numApplicationClerks = 3;

	
	

	
	/*
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

	*/
	Fork(&runApplicationClerk);
	Fork(&runCustomer);
	Fork(&runCustomer);
	
	
}

int chooseLine(int ssn, ClerkType clerkType) {
	int i, minLength, clerkID;
	bool canBribe;
	minLength = 51;
	canBribe = false;

	if(customers[ssn].money > 500) {
		canBribe = true;
	}
	/* Choose the shortest line possible */
	switch(clerkType) {
		case APPLICATION_CLERK:
			for(i = 0; i < numApplicationClerks; i++) {
				if(applicationClerks[i].lineLength < minLength) {
					customers[ssn].clerkID = i;
					minLength = applicationClerks[i].lineLength;
					customers[ssn].didBribe = false;
				}
				if(canBribe) {
					if(applicationClerks[i].bribeLineLength < minLength) {
						customers[ssn].clerkID = i;
						minLength = applicationClerks[i].bribeLineLength;
						customers[ssn].didBribe = true;
					}
				}
			}
			break;
		case PICTURE_CLERK:
			for(i = 0; i < numPictureClerks; i++) {
				if(pictureClerks[i].lineLength < minLength) {
					customers[ssn].clerkID = i;
					minLength = pictureClerks[i].lineLength;
					customers[ssn].didBribe = false;
				}
				if(canBribe) {
					if(pictureClerks[i].bribeLineLength < minLength) {
						customers[ssn].clerkID = i;
						minLength = pictureClerks[i].bribeLineLength;
						customers[ssn].didBribe = true;
					}
				}
			}
			break;
		case PASSPORT_CLERK:
			for(i = 0; i < numPassportClerks; i++) {
				if(passportClerks[i].lineLength < minLength) {
					customers[ssn].clerkID = i;
					minLength = passportClerks[i].lineLength;
					customers[ssn].didBribe = false;
				}
				if(canBribe) {
					if(passportClerks[i].bribeLineLength < minLength) {
						customers[ssn].clerkID = i;
						minLength = passportClerks[i].bribeLineLength;
						customers[ssn].didBribe = true;
					}
				}
			}
			break;
		case CASHIER:
			for(i = 0; i < numCashiers; i++) {
				if(cashiers[i].lineLength < minLength) {
					customers[ssn].clerkID = i;
					minLength = cashiers[i].lineLength;
					customers[ssn].didBribe = false;
				}
				if(canBribe) {
					if(cashiers[i].bribeLineLength < minLength) {
						customers[ssn].clerkID = i;
						minLength = cashiers[i].bribeLineLength;
						customers[ssn].didBribe = true;
					}
				}
			}
			break;
	}
}

bool enterApplicationLine(int ssn, int clerkID) { 
	/* Stand in line */
	Print("Customer %i about to stand in application clerk line\n",ssn);
	if(applicationClerks[clerkID].state != FREE) {
		if(customers[ssn].isSenator) {
			Acquire(applicationClerks[clerkID].senatorLineLock);
			applicationClerks[clerkID].senatorLineLength++;
			Print("Senator %i has gotten in senator line for ",ssn);
			Print("Application Clerk %i\n",clerkID);
			Wait(applicationClerks[clerkID].senatorLineCV, applicationClerks[clerkID].senatorLineLock);
			applicationClerks[clerkID].senatorLineLength--;
		} else if (customers[ssn].didBribe) {
			Acquire(applicationClerks[clerkID].bribeLineLock);
			applicationClerks[clerkID].bribeLineLength++;
			Print("Customer %i has gotten in bribe line for ",ssn);
			Print("Application Clerk %i\n",clerkID);
			Wait(applicationClerks[clerkID].bribeLineCV, applicationClerks[clerkID].bribeLineLock);
			applicationClerks[clerkID].bribeLineLength--;
			Print("test? after wait",0);
		} else {
			Acquire(applicationClerks[clerkID].lineLock);
			applicationClerks[clerkID].lineLength++;
			Print("Customer %i has gotten in regular line for ",ssn);
			Print("Application Clerk %i\n",clerkID);
			Wait(applicationClerks[clerkID].lineCV, applicationClerks[clerkID].lineLock);
			applicationClerks[clerkID].lineLength--;
			Print("test? after wait",0);
		}
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock);
	if(senatorInside && !customers[ssn].isSenator) {
		Release(senatorInsideLock);
		Release(applicationClerks[clerkID].bribeLineLock);
		Release(applicationClerks[clerkID].lineLock);
		Acquire(customerOutsideLineLock);
		Print("Customer %i is going outside the Passport Office because there is a Senator present.\n",ssn);
		customersOutside += 1;
		Wait(customerOutsideLineCV, customerOutsideLineLock);
		customersOutside -= 1;
		Release(customerOutsideLineLock);
		return true;
	} else {
		Release(senatorInsideLock);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		applicationClerks[clerkID].state = BUSY;
		Release(applicationClerks[clerkID].bribeLineLock);
		Release(applicationClerks[clerkID].lineLock);
		Release(applicationClerks[clerkID].senatorLineLock);
		return false;
	}
}

bool enterPictureLine(int ssn, int clerkID) { 
	/* Stand in line */
	if(pictureClerks[clerkID].state != FREE) {
		if(customers[ssn].isSenator) {
			Acquire(pictureClerks[clerkID].senatorLineLock);
			pictureClerks[clerkID].senatorLineLength++;
			/*printf("%s has gotten in senator line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(pictureClerks[clerkID].senatorLineCV, pictureClerks[clerkID].senatorLineLock);
			pictureClerks[clerkID].senatorLineLength--;
		} else if (customers[ssn].didBribe) {
			Acquire(pictureClerks[clerkID].bribeLineLock);
			pictureClerks[clerkID].bribeLineLength++;
			/*printf("%s has gotten in bribe line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(pictureClerks[clerkID].bribeLineCV, pictureClerks[clerkID].bribeLineLock);
			pictureClerks[clerkID].bribeLineLength--;
		} else {
			Acquire(pictureClerks[clerkID].lineLock);
			pictureClerks[clerkID].lineLength++;
			/*printf("%s has gotten in regular line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(pictureClerks[clerkID].lineCV, pictureClerks[clerkID].lineLock);
			pictureClerks[clerkID].lineLength--;
		}
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock);
	if(senatorInside && !customers[ssn].isSenator) {
		Release(senatorInsideLock);
		Release(pictureClerks[clerkID].bribeLineLock);
		Release(pictureClerks[clerkID].lineLock);
		Acquire(customerOutsideLineLock);
		/*printf("%s is going outside the Passport Office because there is a Senator present.\n", currentThread->getName());*/
		customersOutside += 1;
		Wait(customerOutsideLineCV, customerOutsideLineLock);
		customersOutside -= 1;
		Release(customerOutsideLineLock);
		return true;
	} else {
		Release(senatorInsideLock);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		pictureClerks[clerkID].state = BUSY;
		Release(pictureClerks[clerkID].bribeLineLock);
		Release(pictureClerks[clerkID].lineLock);
		Release(pictureClerks[clerkID].senatorLineLock);
		return false;
	}
}

bool enterPassportLine(int ssn, int clerkID) { 
	/* Stand in line */
	if(passportClerks[clerkID].state != FREE) {
		if(customers[ssn].isSenator) {
			Acquire(passportClerks[clerkID].senatorLineLock);
			passportClerks[clerkID].senatorLineLength++;
			/*printf("%s has gotten in senator line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(passportClerks[clerkID].senatorLineCV, passportClerks[clerkID].senatorLineLock);
			passportClerks[clerkID].senatorLineLength--;
		} else if(customers[ssn].didBribe) {
			Acquire(passportClerks[clerkID].bribeLineLock);
			passportClerks[clerkID].bribeLineLength++;
			/*printf("%s has gotten in bribe line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(passportClerks[clerkID].bribeLineCV, passportClerks[clerkID].bribeLineLock);
			passportClerks[clerkID].bribeLineLength--;
		} else {
			Acquire(passportClerks[clerkID].lineLock);
			passportClerks[clerkID].lineLength++;
			/*printf("%s has gotten in regular line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(passportClerks[clerkID].lineCV, passportClerks[clerkID].lineLock);
			passportClerks[clerkID].lineLength--;
		}
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock);
	if(senatorInside && !customers[ssn].isSenator) {
		Release(senatorInsideLock);
		Release(passportClerks[clerkID].bribeLineLock);
		Release(passportClerks[clerkID].lineLock);
		Acquire(customerOutsideLineLock);
		/*printf("%s is going outside the Passport Office because there is a Senator present.\n", currentThread->getName());*/
		customersOutside += 1;
		Wait(customerOutsideLineCV, customerOutsideLineLock);
		customersOutside -= 1;
		Release(customerOutsideLineLock);
		return true;
	} else {
		Release(senatorInsideLock);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		passportClerks[clerkID].state = BUSY;
		Release(passportClerks[clerkID].bribeLineLock);
		Release(passportClerks[clerkID].lineLock);
		Release(passportClerks[clerkID].senatorLineLock);
		return false;
	}
}

bool enterCashierLine(int ssn, int clerkID) { 
	/* Stand in line */
	if(cashiers[clerkID].state != FREE) {
		if(customers[ssn].isSenator) {
			Acquire(cashiers[clerkID].senatorLineLock);
			cashiers[clerkID].senatorLineLength++;
			/*printf("%s has gotten in senator line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(cashiers[clerkID].senatorLineCV, cashiers[clerkID].senatorLineLock);
			cashiers[clerkID].senatorLineLength--;
		} else if(customers[ssn].didBribe) {
			Acquire(cashiers[clerkID].bribeLineLock);
			cashiers[clerkID].bribeLineLength++;
			/*printf("%s has gotten in bribe line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(cashiers[clerkID].bribeLineCV, cashiers[clerkID].bribeLineLock);
			cashiers[clerkID].bribeLineLength--;
		} else {
			Acquire(cashiers[clerkID].lineLock);
			cashiers[clerkID].lineLength++;
			/*printf("%s has gotten in regular line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(cashiers[clerkID].lineCV, cashiers[clerkID].lineLock);
			cashiers[clerkID].lineLength--;
		}
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock);
	if(senatorInside && !customers[ssn].isSenator) {
		Release(senatorInsideLock);
		Release(cashiers[clerkID].bribeLineLock);
		Release(cashiers[clerkID].lineLock);
		Acquire(customerOutsideLineLock);
		/*printf("%s is going outside the Passport Office because there is a Senator present.\n", currentThread->getName());*/
		customersOutside += 1;
		Wait(customerOutsideLineCV, customerOutsideLineLock);
		customersOutside -= 1;
		Release(customerOutsideLineLock);
		return true;
	} else {
		Release(senatorInsideLock);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		cashiers[clerkID].state = BUSY;
		Release(cashiers[clerkID].bribeLineLock);
		Release(cashiers[clerkID].lineLock);
		Release(cashiers[clerkID].senatorLineLock);
		return false;
	}
}

bool enterLine(int ssn, ClerkType clerkType, int clerkID) { 
	switch(clerkType) {
		case APPLICATION_CLERK:
			return enterApplicationLine(ssn, clerkID);
			break;
		case PICTURE_CLERK:
			return enterPictureLine(ssn, clerkID);
			break;
		case PASSPORT_CLERK:
			return enterPassportLine(ssn, clerkID);
			break;
		case CASHIER:
			return enterCashierLine(ssn, clerkID);
			break;
		}
}

void waitInLine(int ssn, ClerkType clerkType) {
	bool senatord;
	int clerkID;
	senatord = false;
	chooseLine(ssn, clerkType);
	clerkID = customers[ssn].clerkID;
	Print("Customer %i chose ",ssn);
	Print("Clerk %i's line\n",clerkID);
	senatord = enterLine(ssn, clerkType, clerkID);
	while(senatord) {
		clerkID = chooseLine(ssn, clerkType);
		senatord = enterLine(ssn, clerkType, clerkID);
	}
	if(customers[ssn].didBribe) {
		customers[ssn].money = customers[ssn].money - 500;
		switch(clerkType) {
			case APPLICATION_CLERK:
				Acquire(applicationClerks[clerkID].moneyLock);
				applicationClerks[clerkID].money = applicationClerks[clerkID].money + 500;
				Release(applicationClerks[clerkID].moneyLock);
				break;
			case PICTURE_CLERK:
				Acquire(pictureClerks[clerkID].moneyLock);
				pictureClerks[clerkID].money = pictureClerks[clerkID].money + 500;
				Release(pictureClerks[clerkID].moneyLock);
				break;
			case PASSPORT_CLERK:
				Acquire(passportClerks[clerkID].moneyLock);
				passportClerks[clerkID].money = passportClerks[clerkID].money + 500;
				Release(passportClerks[clerkID].moneyLock);
				break;
			case CASHIER:
				Acquire(cashiers[clerkID].moneyLock);
				cashiers[clerkID].money = cashiers[clerkID].money + 500;
				Release(cashiers[clerkID].moneyLock);
				break;
		}
	}
}