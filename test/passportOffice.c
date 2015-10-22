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

int customerIndexLock, applicationClerkIndexLock, pictureClerkIndexLock, passportClerkIndexLock, cashierIndexLock;

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


void doApplication(int id){
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

void runPassportClerk() {
	int i;

	Acquire(passportClerkIndexLock);
	i = nextAvailablePassportClerkIndex;
	nextAvailablePassportClerkIndex = nextAvailablePassportClerkIndex + 1; /*temporary keeping track of index*/
	Release(passportClerkIndexLock);

	Print("running PassportClerk: %i\n",i);
	initClerk(i);	
	
	
	Exit(0);
}

void runApplicationClerk() {
	int i;

	Print("before acquire app clerk index lock \n",0);
	Acquire(applicationClerkIndexLock);
	i = nextAvailableApplicationClerkIndex;
	nextAvailableApplicationClerkIndex = nextAvailableApplicationClerkIndex + 1; /*temporary keeping track of index*/
	Release(applicationClerkIndexLock);
	Print("after release app clerk index lock \n",0);

	Print("running ApplicationClerk: %i\n",i);
	initClerk(i);
	

	Exit(0);
}

void runPictureClerk() {
	int i, wait, k;
	Print("before acquire picture clerk index lock \n",0);
	Acquire(pictureClerkIndexLock);
	i = nextAvailablePictureClerkIndex;
	nextAvailablePictureClerkIndex = nextAvailablePictureClerkIndex + 1; /*temporary keeping track of index*/
	Release(pictureClerkIndexLock);
	Print("after release picture clerk index lock \n",0);

	Print("running PictureClerk: %i\n",i);
	initClerk(i);
	

	while(true) {
    	if(pictureClerks[i].senatorLineLength > 0) {
    		Acquire(pictureClerks[i].senatorLineLock);
            Signal(pictureClerks[i].senatorLineCV,pictureClerks[i].senatorLineLock);     /* Signal Customer to exit line */
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].senatorLineLock);
            Wait(pictureClerks[i].clerkCV,pictureClerks[i].clerkLock);      /* Wait for customer to get ready for picture */
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i",pictureClerks[i].customer.SSN);
            Print("from Senator %i\n",pictureClerks[i].customer.SSN);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Senator %i\n",pictureClerks[i].customer.SSN);
            Signal(pictureClerks[i].clerkCV,pictureClerks[i].clerkLock);    /* Give picture back */
            Wait(pictureClerks[i].clerkCV,pictureClerks[i].clerkLock);      /* Wait for Customer to accept picture */
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
    	}
        if(pictureClerks[i].bribeLineLength > 0) {
            Acquire(pictureClerks[i].bribeLineLock);
            Signal(pictureClerks[i].bribeLineCV,pictureClerks[i].bribeLineLock);     /* Signal Customer to exit line */
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].bribeLineLock);
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
            /*pictureClerks[i].customer = NULL; no null in C? */
            Release(pictureClerks[i].clerkLock);
            pictureClerks[i].state = FREE;
        } else if(pictureClerks[i].lineLength > 0) {
            Acquire(pictureClerks[i].lineLock);
            Signal(pictureClerks[i].lineCV,pictureClerks[i].lineLock);     /* Signal Customer to exit line */
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
            Print("PictureClerk %i taking a break\n",i);
            Wait(pictureClerks[i].breakCV,pictureClerks[i].clerkLock);
            Print("PictureClerk %i back from break\n",i);
            Release(pictureClerks[i].clerkLock);
            pictureClerks[i].state = FREE;
        }
    }
	
	Exit(0);
}

void test(){
	Acquire(senatorOutsideLineLock);
	Wait(senatorOutsideLineCV, senatorOutsideLineLock);
	Release(senatorOutsideLineLock);
}

void test1(){
	Print("inside test1",0);
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
	cashierClerkIndexLock = CreateLock();
	
	numCustomers = 10;
	numPassportClerks = 3;
	numPictureClerks = 3;
	numApplicationClerks = 3;

	/*
	Print("before test\n",0);
	Fork(&test);
	Print("after test\n",0);
	Fork(&test1);
	*/

	
	
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

clerkID chooseLine(int ssn, ClerkType clerkType) {
	int i, minLength, clerkID;
	bool canBribe;
	minLength = 51;
	canBribe = false;

	if(customers[ssn].money > 500) 
		canBribe = true;
	// Choose the shortest line possible
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

/* ONLY COMPLETE FOR APPLICATIONCLERK */
bool enterLine(int ssn, ClerkType clerkType, int clerkID) { 
	// Stand in line
	if(applicationClerks[clerkID].state != FREE) {
		if(customers[ssn].isSenator) {
			Acquire(applicationClerks[clerkID].senatorLineLock);
			applicationClerks[clerkID].senatorLineLength++;
			/*printf("%s has gotten in senator line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(applicationClerks[clerkID].senatorLineCV, applicationClerks[clerkID].senatorLineLock);
			applicationClerks[clerkID].senatorLineLength--;
		} else if (didBribe) {
			Acquire(applicationClerks[clerkID].bribeLineLock);
			applicationClerks[clerkID].bribeLineLength++;
			/*printf("%s has gotten in bribe line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(applicationClerks[clerkID].bribeLineCV, applicationClerks[clerkID].bribeLineLock);
			applicationClerks[clerkID].bribeLineLength--;
		} else {
			Acquire(applicationClerks[clerkID].lineLock);
			applicationClerks[clerkID].lineLength++;
			/*printf("%s has gotten in regular line for %s.\n", currentThread->getName(), clerk->getName());*/
			Wait(applicationClerks[clerkID].lineCV, applicationClerks[clerkID].lineLock);
			applicationClerks[clerkID].lineLength--;
		}
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock);
	if(senatorInside && !customers[ssn].isSenator) {
		Release(senatorInsideLock);
		Release(applicationClerks[clerkID].bribeLineLock);
		Release(applicationClerks[clerkID].lineLock);
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
		applicationClerks[clerkID].state = BUSY;
		Release(applicationClerks[clerkID].bribeLineLock);
		Release(applicationClerks[clerkID].lineLock);
		Release(applicationClerks[clerkID].senatorLineLock);
		return false;
	}
}


void waitInLine(int ssn, ClerkType clerkType) {
	bool senatord
	int clerkID;
	senatord = false;
	chooseLine(ssn, clerkType);
	clerkID = customers[ssn].clerkID;
	senatord = enterLine(ssn, clerkType, clerkID);
	while(senatord) {
		clerkID = chooseLine(ssn, clerkType);
		senatord = enterLine(ssn, clerkType, clerkID);
	}
	if(customers[ssn].didBribe) {
		customers[ssn].money = customers[ssn] - 500;
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