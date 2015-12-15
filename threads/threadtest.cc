///////////////////////////////////////////////////////////////////////
// CSCI 350 - Fall 2015
// Assignment 1 Part 2
// 
// Developers: nroubal@usc.edu sisay@usc.edu timms@usc.edu
//
// Contents:
// 	- Passport Office declarations and variables
//	- Passport Office classes (headers and source)
//		- Customer
//		- ApplicationClerk
//		- PictureClerk
//		- PassportClerk
//		- Cashier
//		- Manager
//	- Thread behavior runners
//	- Passport Office code and menu
//
///////////////////////////////////////////////////////////////////////

#include "copyright.h"
#include "system.h"
#include "synch.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

void TestCode();

//
// Forward declarations
//

class Customer;
class Clerk;
class Manager;

//
// Variables
//

enum ClerkState {FREE, BUSY, BREAK};
enum ClerkType {APPLICATION_CLERK, PICTURE_CLERK, PASSPORT_CLERK, CASHIER};
int amounts[] = {100, 600, 1100, 1600};
int numCustomers, numApplicationClerks, numPictureClerks, numPassportClerks, numCashiers, numSenators;
Customer** customers;
Customer** senators;
Clerk** applicationClerks;
Clerk** pictureClerks;
Clerk** passportClerks;
Clerk** cashiers;
Manager *manager;
Lock *senatorOutsideLineLock;
Condition *senatorOutsideLineCV;
int senatorsOutside = 0;
Lock *senatorInsideLock;
bool senatorInside = false;
Lock *customerOutsideLineLock;
Condition *customerOutsideLineCV;
int customersOutside = 0;
bool runningTest1 = false;
bool runningTest2 = false;
bool runningTest3 = false;
bool runningTest4 = false;
bool runningTest5 = false;
bool runningTest6 = false;
bool runningTest7a = false;
bool runningTest7b = false;

//
// Customer class header
//

class Customer : public Thread {
	
	public:
		char* name;
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


		Customer(char* _name, int i, bool _isSenator);
		void doApplication();
		void doPicture();
		void doPassport();
		void doCashier();
		void waitInLine(Clerk** clerks, int numClerks);
		void chooseLine(Clerk** clerks, int numClerks);
		bool enterLine(Clerk* clerk);
};

//
// Clerk class header (only for inheritance)
//

class Clerk : public Thread { 

	public:
		char* name;
		int lineLength;
		int bribeLineLength;
		int senatorLineLength;
		int money;

		ClerkState state;
		Customer *customer;	// Only used for picture clerks

		Lock *lineLock, *bribeLineLock, *senatorLineLock, *clerkLock, *moneyLock;
		Condition *lineCV, *bribeLineCV, *senatorLineCV, *clerkCV, *breakCV;

		Clerk(char* _name);
};

//
// Manager class header
//

class Manager : public Thread {
	
	public:
		int totalMoneyMade;
		
		Manager(char* debugName);
		void managerMain();
};

//
// Customer class source
//

Customer::Customer(char* _name, int i, bool _isSenator) : Thread(_name) {
    name = _name;
	isSenator = _isSenator;
    clerkID = -1;
    SSN = i;
    money = amounts[(int)(rand() % 4)];
    hasApp = false;
    hasPic = false;
    certifiedByPassportClerk = false;
    hasPassport = false;
    seenApp = false;
    seenPic = false;
    likedPic = false;
    hasPaidForPassport = false;
}

void Customer::doApplication() {
	this->waitInLine(applicationClerks, numApplicationClerks);
	Clerk* clerk = applicationClerks[clerkID];
	cout<<clerk->getName()<<" has signalled "<<this->getName()<<" to come to their counter."<<endl;
	clerk->customer = this;
    // Interaction with clerk
    clerk->clerkLock->Acquire();
    clerk->clerkCV->Signal(clerk->clerkLock); // Give incomplete application to Application Clerk
    printf("%s has given SSN %i to %s.\n", this->name, this->SSN,clerk->name);
    clerk->clerkCV->Wait(clerk->clerkLock);   // Wait for Application Clerk
    clerk->clerkCV->Signal(clerk->clerkLock); // Accept completed application

    hasApp = true;
    clerk->clerkLock->Release();
}

void Customer::doPicture() {
	this->waitInLine(pictureClerks, numPictureClerks);
	Clerk* clerk = pictureClerks[clerkID];
	cout<<clerk->getName()<<" has signalled "<<this->getName()<<" to come to their counter."<<endl;
    // Interaction with clerk
    // JGT- didn't make any changes here, but recursivly calling doPicture() isn't a good solution
    seenPic = false;
    clerk->clerkLock->Acquire();
    clerk->customer = this;
    clerk->clerkCV->Signal(clerk->clerkLock); // Customer with Clerk
    printf("%s has given SSN to %s.\n", name, clerk->name);
    clerk->clerkCV->Wait(clerk->clerkLock);   // Wait for Picture Clerk
    seenPic = true;
    if(rand() % 4 == 0 && !senatorInside) {// Customer decides whether they don't like picture
        likedPic = false;
    	printf("%s does not like their picture from %s.\n", name, clerk->name);
        clerk->clerkCV->Signal(clerk->clerkLock);
        clerk->clerkLock->Release();
        doPicture();
    } else {
        likedPic = true;
        printf("%s does like their picture from %s.\n", name, clerk->name);
        clerk->clerkCV->Signal(clerk->clerkLock);
        hasPic = true;
        clerk->clerkLock->Release();
    }
}

void Customer::doPassport() {
	this->waitInLine(passportClerks, numPassportClerks);
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
}

void Customer::doCashier() {
	this->waitInLine(cashiers, numCashiers);	// ---------- Cashier doesn't have a bribe line -------------------
	Clerk* clerk = cashiers[clerkID];
	cout<<clerk->getName()<<" has signalled "<<this->getName()<<" to come to their counter."<<endl;
	   // Interaction with cashier
    clerk->clerkLock->Acquire();
    clerk->customer = this;
    //cout << getName() << " currently with " << clerk->getName() << endl;
    clerk->clerkCV->Signal(clerk->clerkLock);
    clerk->clerkCV->Wait(clerk->clerkLock); 
    if(certifiedByPassportClerk && hasPaidForPassport) {
        cout << clerk->getName() << " has recorded that "<<this->getName()<<" has been given their completed passport." << endl;
        money = money - 100;
        hasPassport = true;
        //cout << getName() << " finished with " << clerk->getName() << endl;
        clerk->clerkCV->Signal(clerk->clerkLock);
        clerk->clerkLock->Release();
        cout<<getName()<<" is leaving the Passport Office"<<endl;
    } else {
        clerk->clerkCV->Signal(clerk->clerkLock);
        clerk->clerkLock->Release();
        int wait = rand() % ((100 - 20) + 1) + 20;
        for(int i = 0; i < wait; i++) 
            Yield();
    }
}

// Changes clerkID and didBribe through chooseLine. Changes money.
void Customer::waitInLine(Clerk** clerks, int numClerks) {
	bool senatord = false;

	this->chooseLine(clerks, numClerks);
	senatord = this->enterLine(clerks[clerkID]);

	while (senatord) {
		this->chooseLine(clerks, numClerks);
		senatord = this->enterLine(clerks[clerkID]);
	}

	if (didBribe) {
		money -= 500;
		clerks[clerkID]->moneyLock->Acquire();
		clerks[clerkID]->money += 500;
		clerks[clerkID]->moneyLock->Release();
	}
}

// Changes clerkID and didBribe
void Customer::chooseLine(Clerk** clerks, int numClerks) {
	int minLength = 51;
	bool canBribe = false;

	if (money > 500) 
		canBribe = true;

	// Choose the shortest line possible
	for (int i = 0; i < numClerks; i++) {
		if (clerks[i]->lineLength < minLength) {
			clerkID = i;
			minLength = clerks[i]->lineLength;
			didBribe = false;
		}

		if (canBribe) {
			if (clerks[i]->bribeLineLength < minLength) {
				clerkID = i;
				minLength = clerks[i]->bribeLineLength;
				didBribe = true;
			}
		}
	}
}

// Makes no changes
bool Customer::enterLine(Clerk* clerk) {
	// Stand in line
	if (clerk->state != FREE) {
		if(isSenator) {
			clerk->senatorLineLock->Acquire();
			clerk->senatorLineLength++;
			printf("%s has gotten in senator line for %s.\n", currentThread->getName(), clerk->getName());
			clerk->senatorLineCV->Wait(clerk->senatorLineLock);
			clerk->senatorLineLength--;
		}
		else if (didBribe) {
			clerk->bribeLineLock->Acquire();
			clerk->bribeLineLength++;
			printf("%s has gotten in bribe line for %s.\n", currentThread->getName(), clerk->getName());
			clerk->bribeLineCV->Wait(clerk->bribeLineLock);
			clerk->bribeLineLength--;
		} else {
			clerk->lineLock->Acquire();
			clerk->lineLength++;
			printf("%s has gotten in regular line for %s.\n", currentThread->getName(), clerk->getName());
			clerk->lineCV->Wait(clerk->lineLock);
			clerk->lineLength--;
		}
	}

	// Called out of line, make sure it wasn't because of a senator
	senatorInsideLock->Acquire();
	if (senatorInside && !isSenator) {
		senatorInsideLock->Release();
		clerk->bribeLineLock->Release();
		clerk->lineLock->Release();
		customerOutsideLineLock->Acquire();
		printf("%s is going outside the Passport Office because there is a Senator present.\n", currentThread->getName());
		customersOutside += 1;
		customerOutsideLineCV->Wait(customerOutsideLineLock);
		customersOutside -= 1;
		customerOutsideLineLock->Release();
		return true;
	} else {
		senatorInsideLock->Release();

		// Change the clerk to BUSY before releasing the clerk's line lock
		clerk->state = BUSY;
		clerk->bribeLineLock->Release();
		clerk->lineLock->Release();
		clerk->senatorLineLock->Release();

		return false;
	}
}

//
// Clerk class source
//

Clerk::Clerk(char* _name) : Thread(_name) {
	name = _name;
	money = 0;
	state = BUSY;

	lineLength = 0;
	bribeLineLength = 0;
	senatorLineLength = 0;
	lineLock = new Lock("lineLock");
	bribeLineLock = new Lock("bribeLineLock");
	senatorLineLock = new Lock("senatorLineLock");
	lineCV = new Condition("lineCV");
	bribeLineCV = new Condition("bribeLineCV");
	senatorLineCV = new Condition("senatorLineCV");
	clerkLock = new Lock("clerkLock");
	moneyLock = new Lock("moneyLock");
	clerkCV = new Condition("clerkCV");
	breakCV = new Condition("breakCV");
}

//
// Manager class source
//

Manager::Manager(char* debugName) : Thread(debugName) {
	totalMoneyMade = 0;
}

void Manager::managerMain() {
	//int totalMoneyMade = 0;
	while(true) {
		bool allClerksOnBreak = true; 
		int appClerkMoneyTotal = 0;
		int picClerkMoneyTotal = 0;
		int passportClerkMoneyTotal = 0;
		int cashierMoneyTotal = 0;

		for(int k = 0; k < numApplicationClerks; k ++) { //loop through all app clerks
			Clerk *thisClerk = applicationClerks[k];
			thisClerk->moneyLock->Acquire();
			appClerkMoneyTotal += thisClerk->money;  //get this clerks money total
			thisClerk->moneyLock->Release();
			if(thisClerk->state != BREAK) { //check if not on break 
				allClerksOnBreak = false; 
			} else { //if on break
				thisClerk->bribeLineLock->Acquire();
				thisClerk->lineLock->Acquire();
				if((thisClerk->bribeLineLength + thisClerk->lineLength) >= 3 || thisClerk->senatorLineLength > 0) { //check if >=3 customers are in line, or if a senator is in line
					thisClerk->breakCV->Signal(thisClerk->clerkLock); //if so, then wake up clerk 
					cout<<"Manager has woken up an ApplicationClerk"<<endl;
					allClerksOnBreak = false; //all clerks are no longer are on break
				}
				thisClerk->bribeLineLock->Release();
				thisClerk->lineLock->Release();
			}

		}
		// if(runningTest2) //test 2 just needed to read clerk's money once
		// 	break;

		for(int k = 0; k < numPictureClerks; k ++) { //loop through all pic clerks
			Clerk *thisClerk = pictureClerks[k];
			thisClerk->moneyLock->Acquire();
			picClerkMoneyTotal += thisClerk->money;  //get this clerks money total
			thisClerk->moneyLock->Release();
			if(thisClerk->state != BREAK) { //check if not on break 
				allClerksOnBreak = false; 
			} else { //if on break
				thisClerk->bribeLineLock->Acquire();
				thisClerk->lineLock->Acquire();
				if((thisClerk->bribeLineLength + thisClerk->lineLength) >= 3 || thisClerk->senatorLineLength > 0) { //check if >=3 customers are in line, or if a senator is in line
					thisClerk->breakCV->Signal(thisClerk->clerkLock); //if so, then wake up clerk 
					cout<<"Manager has woken up a PictureClerk"<<endl;
					allClerksOnBreak = false; //all clerks are no longer are on break
				}
				thisClerk->bribeLineLock->Release();
				thisClerk->lineLock->Release();
			}
		}
		for(int k = 0; k < numPassportClerks; k ++) { //loop through all passport clerks
			Clerk *thisClerk = passportClerks[k];
			thisClerk->moneyLock->Acquire();
			passportClerkMoneyTotal += thisClerk->money;  //get this clerks money total
			thisClerk->moneyLock->Release();
			if(thisClerk->state != BREAK) { //check if not on break 
				allClerksOnBreak = false; 
			} else { //if on break
				thisClerk->bribeLineLock->Acquire();
				thisClerk->lineLock->Acquire();
				if((thisClerk->bribeLineLength + thisClerk->lineLength) >= 3 || thisClerk->senatorLineLength > 0) { //check if >=3 customers are in line, or if a senator is in line
					thisClerk->breakCV->Signal(thisClerk->clerkLock); //if so, then wake up clerk 
					cout<<"Manager has woken up a PassportClerk"<<endl;
					allClerksOnBreak = false; //all clerks are no longer are on break
				}
				thisClerk->bribeLineLock->Release();
				thisClerk->lineLock->Release();
			}
		}
		for(int k = 0; k < numCashiers; k ++) { //loop through all cashiers
			Clerk *thisClerk = cashiers[k];
			thisClerk->moneyLock->Acquire();
			cashierMoneyTotal += thisClerk->money;  //get this clerks money total
			thisClerk->moneyLock->Release();
			if(thisClerk->state != BREAK) { //check if not on break 
				allClerksOnBreak = false; 
			} else { //if on break
				thisClerk->bribeLineLock->Acquire();
				thisClerk->lineLock->Acquire();
				if((thisClerk->bribeLineLength + thisClerk->lineLength) >= 3 || thisClerk->senatorLineLength > 0) { //check if >=3 customers are in line, or if a senator is in line
					thisClerk->breakCV->Signal(thisClerk->clerkLock); //if so, then wake up clerk 
					cout<<"Manager has woken up a Cashier"<<endl;
					allClerksOnBreak = false; //all clerks are no longer are on break
				}
				thisClerk->bribeLineLock->Release();
				thisClerk->lineLock->Release();
			}

		}

		if(runningTest3 || runningTest4) //dont need manager to output anything if running test 3/4
			break;

		if(allClerksOnBreak && !runningTest2) //if all clerks are on break
			continue;

		//output money statements

		for(int k = 0 ; k < 100 ; k ++ ) {
			currentThread->Yield();
		}
		if(runningTest5 || runningTest7a || runningTest7b) //test 5/7 should just keep checking if manager needs to wake cashier clerks
			continue;
		cout << "Manager: App clerks have made a total of $" << appClerkMoneyTotal << endl;
		cout << "Manager: Pic clerks have made a total of $" << picClerkMoneyTotal << endl;
		cout << "Manager: Passport clerks have made a total of $" << passportClerkMoneyTotal << endl;
		cout << "Manager: Cashier have made a total of $" << cashierMoneyTotal << endl;
		totalMoneyMade = appClerkMoneyTotal + picClerkMoneyTotal + passportClerkMoneyTotal;
		cout << "Manager: Passport Office has made a total of $" << totalMoneyMade << endl;
		if(runningTest2)
			break;
	}
}

//
// Thread behavior runners
// 

void runCustomer(int id) {
    Customer *customer = customers[id];

    senatorOutsideLineLock->Acquire();
    senatorInsideLock->Acquire();

	if (senatorsOutside > 0 || senatorInside) {
		senatorInsideLock->Release();
		senatorOutsideLineLock->Release();
		customerOutsideLineLock->Acquire();
		customersOutside += 1;
		customerOutsideLineCV->Wait(customerOutsideLineLock);
		customersOutside -= 1;
		customerOutsideLineLock->Release();
	}

	senatorInsideLock->Release();
	senatorOutsideLineLock->Release();

	if(rand() % 10 == 0 && runningTest1 == false) // Customer has 1 in 10 chance of going to Passport Clerk first
        customer->doPassport();
    // Randomly decide whether to go to AppClerk or PicClerk first
    if(runningTest1) { //if running test 1, then just go to the application clerk
    	customer->doApplication();
    	return;
    } else if(runningTest3 || runningTest5) { // go straight to cashier for these tests
    	customer->doCashier();
    	return;
    } else if(rand() % 2 == 0) {
        customer->doApplication();
        customer->doPicture();
    } else {
        customer->doPicture();
        customer->doApplication();
    }
	if(rand() % 10 == 0 && runningTest1 == false) // Customer has 1 in 10 chance of going to Cashier before PassportClerk
        customer->doCashier();
    customer->doPassport();
    do{
    	customer->doCashier();
    }while(customer->certifiedByPassportClerk == false); //until the customer has been certified, keep him in line with cashier
  
}


void runSenator(int id) {
	Customer *senator = senators[id];
	
	senatorOutsideLineLock->Acquire();
	senatorInsideLock->Acquire();

	// Senators wait on other senators outside
	if (senatorsOutside > 0 || senatorInside) {
		senatorsOutside += 1;
		senatorInsideLock->Release();
		senatorOutsideLineCV->Wait(senatorOutsideLineLock);
		senatorInsideLock->Acquire();
		senatorsOutside -= 1;
	}
		
	// NOTICE: Don't Signal() senatorOutsideLineCV except for when a senator leaves.
	senatorInside = true;
	senatorInsideLock->Release();
	senatorOutsideLineLock->Release();
		
	// Senator entering, alert all lines to empty
	for(int i = 0; i < numApplicationClerks; i++) {
		Clerk *clerk = applicationClerks[i];
		clerk->lineLock->Acquire();
		clerk->lineCV->Broadcast(clerk->lineLock);
		clerk->lineLock->Release();
		clerk->bribeLineLock->Acquire();
		clerk->bribeLineCV->Broadcast(clerk->bribeLineLock);
		clerk->bribeLineLock->Release();
	}
		
	for(int i = 0; i < numPictureClerks; i++) {
		Clerk *clerk = pictureClerks[i];
		clerk->lineLock->Acquire();
		clerk->lineCV->Broadcast(clerk->lineLock);
		clerk->lineLock->Release();
		clerk->bribeLineLock->Acquire();
		clerk->bribeLineCV->Broadcast(clerk->bribeLineLock);
		clerk->bribeLineLock->Release();
	}
		
	for(int i = 0; i < numPassportClerks; i++) {
		Clerk *clerk = passportClerks[i];
		clerk->lineLock->Acquire();
		clerk->lineCV->Broadcast(clerk->lineLock);
		clerk->lineLock->Release();
		clerk->bribeLineLock->Acquire();
		clerk->bribeLineCV->Broadcast(clerk->bribeLineLock);
		clerk->bribeLineLock->Release();
	}
	
	for(int i = 0; i < numCashiers; i++) {
		Clerk *clerk = cashiers[i];
		clerk->lineLock->Acquire();
		clerk->lineCV->Broadcast(clerk->lineLock);
		clerk->lineLock->Release();
		clerk->bribeLineLock->Acquire();
		clerk->bribeLineCV->Broadcast(clerk->bribeLineLock);
		clerk->bribeLineLock->Release();
	}
	
    // Randomly decide whether to go to AppClerk or PicClerk first
    if(rand() % 2 == 1) {
        senator->doApplication();
        senator->doPicture();
    } else {
        senator->doPicture();
        senator->doApplication();
    }
	
    senator->doPassport();
    senator->doCashier();

    // Add code here to Broadcast() when a senator leaves
    senatorOutsideLineLock->Acquire();
    senatorInsideLock->Acquire();
    customerOutsideLineLock->Acquire();
	senatorInside = false;
	senatorOutsideLineCV->Broadcast(senatorOutsideLineLock);
	senatorInsideLock->Release();
	senatorOutsideLineLock->Release();
	if(senatorsOutside == false) { //if there are no other senators outside, then customers can come in
		customerOutsideLineCV->Broadcast(customerOutsideLineLock);
	}
	customerOutsideLineLock->Release();
}

void runApplicationClerk(int id) {
    Clerk *thisClerk = applicationClerks[id];
    // cout << thisClerk->getName() << " currently running." << endl;
    while(true) {
    	if(thisClerk->senatorLineLength > 0) {
    		thisClerk->senatorLineLock->Acquire();
            thisClerk->senatorLineCV->Signal(thisClerk->senatorLineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->senatorLineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to turn in application
            printf("%s has received SSN %i from %s.\n", thisClerk->name, thisClerk->customer->SSN,thisClerk->customer->getName());
            int wait = rand() % ((100 - 20) + 1) + 20;
            for(int i = 0; i < wait; i++)               // Process application
                currentThread->Yield();
            printf("%s has recored a complete application for %s.\n", thisClerk->name,thisClerk->customer->getName());
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give complete application back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept application
            thisClerk->customer = NULL;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
    	}
        else if(thisClerk->bribeLineLength > 0) {
            thisClerk->bribeLineLock->Acquire();
            thisClerk->bribeLineCV->Signal(thisClerk->bribeLineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->bribeLineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to turn in application
            printf("%s has received SSN %i from %s.\n", thisClerk->name, thisClerk->customer->SSN,thisClerk->customer->getName());
            int wait = rand() % ((100 - 20) + 1) + 20;
            for(int i = 0; i < wait; i++)               // Process application
                currentThread->Yield();
            printf("%s has recorded a complete application for %s.\n", thisClerk->name,thisClerk->customer->getName());
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give complete application back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept application
            printf("%s has received $500 from %s.\n", thisClerk->name,thisClerk->customer->getName());            
            thisClerk->customer = NULL;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        } else if(thisClerk->lineLength > 0) {
            thisClerk->lineLock->Acquire();
            thisClerk->lineCV->Signal(thisClerk->lineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->lineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to turn in application
            printf("%s has received SSN %i from %s.\n", thisClerk->name, thisClerk->customer->SSN,thisClerk->customer->getName());
            int wait = rand() % ((100 - 20) + 1) + 20;
            for(int i = 0; i < wait; i++)               // Process application
                thisClerk->Yield();
            printf("%s has recored a complete application for %s.\n", thisClerk->name,thisClerk->customer->getName());
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give complete application back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept application
            thisClerk->customer = NULL;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        } else {
            thisClerk->clerkLock->Acquire();
            thisClerk->state = BREAK;
            cout << thisClerk->getName() << " taking a break." << endl;
            thisClerk->breakCV->Wait(thisClerk->clerkLock);
            cout << thisClerk->getName() << " back from break." << endl;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        }
    }
}

void runPictureClerk(int id) {
    Clerk *thisClerk = pictureClerks[id];
    // cout << thisClerk->getName() << " currently running." << endl;
    while(true) {
    	if(thisClerk->senatorLineLength > 0) {
    		thisClerk->senatorLineLock->Acquire();
            thisClerk->senatorLineCV->Signal(thisClerk->senatorLineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->senatorLineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to get ready for picture
            printf("%s has received SSN %i from %s.\n", thisClerk->name, thisClerk->customer->SSN,thisClerk->customer->getName());
            printf("%s has taken a picture of %s.\n", thisClerk->name,thisClerk->customer->getName());
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give picture back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept picture
            if(thisClerk->customer->likedPic) {
                cout << thisClerk->getName() << " has been told that " << thisClerk->customer->getName() << " does like their picture." << endl;
                int wait = rand() % ((100 - 20) + 1) + 20;
                for(int i = 0; i < wait; i++)               // Process picture
                    currentThread->Yield();
            }else{
            cout << thisClerk->getName() << " has been told that " << thisClerk->customer->getName() << " does not like their picture." << endl;
            }
            thisClerk->customer = NULL;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
    	}
        if(thisClerk->bribeLineLength > 0) {
            thisClerk->bribeLineLock->Acquire();
            thisClerk->bribeLineCV->Signal(thisClerk->bribeLineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->bribeLineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to get ready for picture
            printf("%s has received SSN %i from %s.\n", thisClerk->name, thisClerk->customer->SSN,thisClerk->customer->getName());
            printf("%s has taken a picture of %s.\n", thisClerk->name,thisClerk->customer->getName());
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give picture back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept picture
            if(thisClerk->customer->likedPic) {
                cout << thisClerk->getName() << " has been told that " << thisClerk->customer->getName() << " does like their picture." << endl;
                int wait = rand() % ((100 - 20) + 1) + 20;
                for(int i = 0; i < wait; i++)               // Process picture
                    currentThread->Yield();
            }else{
            cout << thisClerk->getName() << " has been told that " << thisClerk->customer->getName() << " does not like their picture." << endl;
            }
            thisClerk->customer = NULL;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        } else if(thisClerk->lineLength > 0) {
            thisClerk->lineLock->Acquire();
            thisClerk->lineCV->Signal(thisClerk->lineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->lineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to get ready for picture
            printf("%s has received SSN %i from %s.\n", thisClerk->name, thisClerk->customer->SSN,thisClerk->customer->getName());
            printf("%s has taken a picture of %s.\n", thisClerk->name,thisClerk->customer->getName());
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give picture back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept picture
            cout << thisClerk->getName() << " - " << thisClerk->customer->getName() << endl;
            if(thisClerk->customer->likedPic) {
                cout << thisClerk->getName() << " has been told that " << thisClerk->customer->getName() << " does like their picture." << endl;
                int wait = rand() % ((100 - 20) + 1) + 20;
                for(int i = 0; i < wait; i++)               // Process picture
                    currentThread->Yield();
            }else{
            cout << thisClerk->getName() << " has been told that " << thisClerk->customer->getName() << " does not like their picture." << endl;
            }
            thisClerk->customer = NULL;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        } else {
            thisClerk->clerkLock->Acquire();
            thisClerk->state = BREAK;
            cout << thisClerk->getName() << " taking a break." << endl;
            thisClerk->breakCV->Wait(thisClerk->clerkLock);
            cout << thisClerk->getName() << " back from break." << endl;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        }
    }
}

void runPassportClerk(int id) {
    Clerk *thisClerk = passportClerks[id];
    // cout << thisClerk->getName() << " currently running." << endl;
    while(true) {
    	if(thisClerk->senatorLineLength > 0) {
    		thisClerk->senatorLineLock->Acquire();
            thisClerk->senatorLineCV->Signal(thisClerk->senatorLineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->senatorLineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to turn in application and picture
            printf("%s has received SSN %i from %s.\n", thisClerk->name, thisClerk->customer->SSN,thisClerk->customer->getName());
            if(thisClerk->customer->hasApp && thisClerk->customer->hasPic) {
            	cout<<"inside if";
                int wait = rand() % ((100 - 20) + 1) + 20; 
                cout << thisClerk->getName() << " started recording " << thisClerk->customer->getName();
                cout << "'s documents.'" << endl;
                for(int i = 0; i < wait; i++)               // Process application and picture
                    currentThread->Yield();
                cout << thisClerk->getName() << " finished recording " << thisClerk->customer->getName();
                cout << "'s passport documents." << endl;
                thisClerk->customer->certifiedByPassportClerk = true;
            } else {
            	printf("%s has determined that %s does not have both their application and picture completed.\n", thisClerk->name,thisClerk->customer->getName());
            }
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give passport to customer
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept passport
            thisClerk->customer = NULL;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
    	}
        else if(thisClerk->bribeLineLength > 0) {
            thisClerk->bribeLineLock->Acquire();
            thisClerk->bribeLineCV->Signal(thisClerk->bribeLineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->bribeLineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to turn in application and picture
            printf("%s has received SSN %i from %s.\n", thisClerk->name, thisClerk->customer->SSN,thisClerk->customer->getName());
            if(thisClerk->customer->hasApp && thisClerk->customer->hasPic) {
                int wait = rand() % ((100 - 20) + 1) + 20; 
                cout << thisClerk->getName() << " started recording " << thisClerk->customer->getName();
                cout << "'s documents." << endl;
                for(int i = 0; i < wait; i++)               // Process application and picture
                    currentThread->Yield();
                cout << thisClerk->getName() << " finished recording " << thisClerk->customer->getName();
                cout << "'s passport documents.'" << endl;
                thisClerk->customer->certifiedByPassportClerk = true;
            } else {
                printf("%s has determined that %s does not have both their application and picture completed.\n", thisClerk->name,thisClerk->customer->getName());
            }
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give passport to customer
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept passport
            thisClerk->customer = NULL;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        } else if(thisClerk->lineLength > 0) {
            thisClerk->lineLock->Acquire();
            thisClerk->lineCV->Signal(thisClerk->lineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->lineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to turn in application
            printf("%s has received SSN %i from %s.\n", thisClerk->name, thisClerk->customer->SSN,thisClerk->customer->getName());
            if(thisClerk->customer->hasApp && thisClerk->customer->hasPic) {
                int wait = rand() % ((100 - 20) + 1) + 20; 
                for(int i = 0; i < wait; i++)               // Process application and picture
                    currentThread->Yield();
                cout << thisClerk->getName() << " finished recording " << thisClerk->customer->getName();
                cout << "'s passport documents.'" << endl;
                thisClerk->customer->certifiedByPassportClerk = true;
            } else {
                printf("%s has determined that %s does not have both their application and picture completed.\n", thisClerk->name,thisClerk->customer->getName());
            }
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give complete application back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept application
            thisClerk->customer = NULL;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        } else {
            thisClerk->clerkLock->Acquire();
            thisClerk->state = BREAK;
            cout << thisClerk->getName() << " taking a break." << endl;
            thisClerk->breakCV->Wait(thisClerk->clerkLock);
            cout << thisClerk->getName() << " back from break." << endl;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        }
    }
}

void runCashier(int id) {
	Clerk *thisCashier = cashiers[id];
    // cout << thisCashier->getName() << " currently running." << endl;
    while(true) {
    	if(thisCashier->senatorLineLength > 0) {
    		thisCashier->senatorLineLock->Acquire();
            thisCashier->senatorLineCV->Signal(thisCashier->senatorLineLock);     // Signal Customer to exit line
            thisCashier->clerkLock->Acquire();
            thisCashier->senatorLineLock->Release();
            thisCashier->clerkCV->Wait(thisCashier->clerkLock);      // Wait for customer to pay
            printf("%s has received SSN %i from %s.\n", thisCashier->name, thisCashier->customer->SSN,thisCashier->customer->getName());
            if(thisCashier->customer->certifiedByPassportClerk) {
            	printf("%s has verified that %s has been certified by a PassportClerk.\n", thisCashier->name, thisCashier->customer->getName());
            	if(thisCashier->customer->hasPaidForPassport == false){
		        	printf("%s has receieved the $100 from %s after ceritification.\n", thisCashier->name, thisCashier->customer->getName());
		        	thisCashier->customer->hasPaidForPassport = true;
	                thisCashier->moneyLock->Acquire();
	                thisCashier->money = thisCashier->money + 100;
	                thisCashier->moneyLock->Release();
            	}
            	printf("%s has provided %s their completed passport.\n", thisCashier->name, thisCashier->customer->getName());
            } else {
            	if(thisCashier->customer->hasPaidForPassport == false){
		        	printf("%s has receieved the $100 from %s before ceritification.\n", thisCashier->name, thisCashier->customer->getName());
		        	thisCashier->customer->hasPaidForPassport = true;
	                thisCashier->moneyLock->Acquire();
	                thisCashier->money = thisCashier->money + 100;
	                thisCashier->moneyLock->Release();
            	}
            }
            thisCashier->clerkCV->Signal(thisCashier->clerkLock);    // Give complete application back

            thisCashier->clerkCV->Wait(thisCashier->clerkLock);      // Wait for Customer to accept application
            thisCashier->customer = NULL;
            thisCashier->clerkLock->Release();
            thisCashier->state = FREE;
    	}
    	else if(thisCashier->bribeLineLength > 0) {
            thisCashier->bribeLineLock->Acquire();
            thisCashier->bribeLineCV->Signal(thisCashier->bribeLineLock);     // Signal Customer to exit line
            thisCashier->clerkLock->Acquire();
            thisCashier->bribeLineLock->Release();
            thisCashier->clerkCV->Wait(thisCashier->clerkLock);      // Wait for customer to pay
            printf("%s has received SSN %i from %s.\n", thisCashier->name, thisCashier->customer->SSN,thisCashier->customer->getName());
            if(thisCashier->customer->certifiedByPassportClerk) {
            	printf("%s has verified that %s has been certified by a PassportClerk.\n", thisCashier->name, thisCashier->customer->getName());
            	if(thisCashier->customer->hasPaidForPassport == false){
		        	printf("%s has receieved the $100 from %s after ceritification.\n", thisCashier->name, thisCashier->customer->getName());
		        	thisCashier->customer->hasPaidForPassport = true;
		        	thisCashier->moneyLock->Acquire();
	                thisCashier->money = thisCashier->money + 100;
	                thisCashier->moneyLock->Release();
            	}
            	printf("%s has provided %s their completed passport.\n", thisCashier->name, thisCashier->customer->getName());
            } else {
            	if(thisCashier->customer->hasPaidForPassport == false){
		        	printf("%s has receieved the $100 from %s before ceritification.\n", thisCashier->name, thisCashier->customer->getName());
		        	thisCashier->customer->hasPaidForPassport = true;
	                thisCashier->moneyLock->Acquire();
	                thisCashier->money = thisCashier->money + 100;
	                thisCashier->moneyLock->Release();
            	}
            }
            thisCashier->clerkCV->Signal(thisCashier->clerkLock);    // Give complete application back
            thisCashier->clerkCV->Wait(thisCashier->clerkLock);      // Wait for Customer to accept application
            thisCashier->customer = NULL;
            thisCashier->clerkLock->Release();
            thisCashier->state = FREE;
        }
        else if(thisCashier->lineLength > 0) {
            thisCashier->lineLock->Acquire();
            thisCashier->lineCV->Signal(thisCashier->lineLock);     // Signal Customer to exit line
            thisCashier->clerkLock->Acquire();
            thisCashier->lineLock->Release();
            thisCashier->clerkCV->Wait(thisCashier->clerkLock);      // Wait for customer to pay
            printf("%s has received SSN %i from %s.\n", thisCashier->name, thisCashier->customer->SSN,thisCashier->customer->getName());
            if(thisCashier->customer->certifiedByPassportClerk) {
            	printf("%s has verified that %s has been certified by a PassportClerk.\n", thisCashier->name, thisCashier->customer->getName());
            	if(thisCashier->customer->hasPaidForPassport == false){
		        	printf("%s has receieved the $100 from %s after ceritification.\n", thisCashier->name, thisCashier->customer->getName());
		        	thisCashier->customer->hasPaidForPassport = true;
	                thisCashier->moneyLock->Acquire();
	                thisCashier->money = thisCashier->money + 100;
	                thisCashier->moneyLock->Release();
            	}
            	printf("%s has provided %s their completed passport.\n", thisCashier->name, thisCashier->customer->getName());
            } else {
            	if(thisCashier->customer->hasPaidForPassport == false){
		        	printf("%s has receieved the $100 from %s before ceritification.\n", thisCashier->name, thisCashier->customer->getName());
		        	thisCashier->customer->hasPaidForPassport = true;
	                thisCashier->moneyLock->Acquire();
	                thisCashier->money = thisCashier->money + 100;
	                thisCashier->moneyLock->Release();
            	}
            }
            thisCashier->clerkCV->Signal(thisCashier->clerkLock);    // Give complete application back
            thisCashier->clerkCV->Wait(thisCashier->clerkLock);      // Wait for Customer to accept application
            thisCashier->customer = NULL;
            thisCashier->clerkLock->Release();
            thisCashier->state = FREE;
        } else {
            thisCashier->clerkLock->Acquire();
            thisCashier->state = BREAK;
            cout << thisCashier->getName() << " taking a break." << endl;
            thisCashier->breakCV->Wait(thisCashier->clerkLock);
            cout << thisCashier->getName() << " back from break." << endl;
            thisCashier->clerkLock->Release();
            thisCashier->state = FREE;
        }
    }
}

void runManager() {
	manager->managerMain();
}

//
// Passport Office code and menu
//

void PassportOffice() {
	//initial output
	cout<<"Number of Customers = " << numCustomers <<endl;
	cout<<"Number of ApplicationClerks = " << numApplicationClerks<<endl;
	cout<<"Number of PictureClerks = " << numPictureClerks <<endl;
	cout<<"Number of PassportClerks = " << numPassportClerks <<endl;
	cout<<"Number of Cashiers = " << numCashiers <<endl;
	cout<<"Number of Senators = " << numSenators <<endl;

	// Create locks and conditions
	senatorOutsideLineLock = new Lock("senatorOutsideLineLock");
	senatorInsideLock = new Lock("senatorInsideLock");
	customerOutsideLineLock = new Lock("customerOutsideLineLock");
	senatorOutsideLineCV = new Condition("senatorOutsideLineCV");
	customerOutsideLineCV = new Condition("customerOutsideLineCV");
	
    char* name;
    // Create Customers
    customers = new Customer*[numCustomers];
    for(int i = 0; i < numCustomers; i++) {
        name = new char[20];
        sprintf(name, "Customer %d", i);
        customers[i] = new Customer(name, i, false);
    }

    // Create Senators
    senators = new Customer*[numSenators];
    for(int i = 0; i < numSenators; i++) {
        name = new char[20];
        sprintf(name, "Senator %d", i);
        senators[i] = new Customer(name, i, true);
    }

    // Create ApplicationClerks
    applicationClerks = new Clerk*[numApplicationClerks];
    for(int i = 0; i < numApplicationClerks; i++) {
        name = new char[20];
        sprintf(name, "ApplicationClerk %d", i);
        applicationClerks[i] = new Clerk(name);
    }

    // Create PictureClerks
    pictureClerks = new Clerk*[numPictureClerks];
    for(int i = 0; i < numPictureClerks; i++) {
        name = new char[20];
        sprintf(name, "PictureClerk %d", i);
        pictureClerks[i] = new Clerk(name);
    }

    
    // Create PassportClerks
    passportClerks = new Clerk*[numPassportClerks];
    for(int i = 0; i < numPassportClerks; i++) {
        name = new char[20];
        sprintf(name, "PassportClerk%d", i);
        passportClerks[i] = new Clerk(name);
    }

    // Create Cashiers
    cashiers = new Clerk*[numCashiers];
    for(int i = 0; i < numCashiers; i++) {
        name = new char[20];
        sprintf(name, "Cashier%d", i);
        cashiers[i] = new Clerk(name);
    }
    

    // Run ApplicationClerks
    for(int i = 0; i < numApplicationClerks; i++)
        applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);
    // Run PictureClerks
    for(int i = 0; i < numPictureClerks; i++)
        pictureClerks[i]->Fork((VoidFunctionPtr)runPictureClerk, i);

        // Run PassportClerks
    for(int i = 0; i < numPassportClerks; i++)
        passportClerks[i]->Fork((VoidFunctionPtr)runPassportClerk, i);
    // Run Cashiers
    for(int i = 0; i < numCashiers; i++)
        cashiers[i]->Fork((VoidFunctionPtr)runCashier, i);

    // Run Customers
    for(int i = 0; i < numCustomers; i++)
        customers[i]->Fork((VoidFunctionPtr)runCustomer, i);

    // Run Senators
    for(int i = 0; i < numSenators; i++) {
    	senators[i]->Fork((VoidFunctionPtr)runSenator, i);
    }
    
    

    

    manager = new Manager("manager 1");
    manager->Fork((VoidFunctionPtr)runManager, 0);

}



void printMenu() {
    // Run menu for Part 2 of assignment
 //   while(true) {
        cout << "Please enter an option:" << endl;
        cout << " -a  (View/Edit default values)" << endl;
        //cout << " -b  (Run Passport Office)" << endl;
        cout << " -c  (Exit)" << endl;
        string input;
        getline(cin, input);
        if(input == "a") {     // Print/Edit default values
            cout << "Number of Customers: " <<  numCustomers << endl;
            cout << "Number of Appication Clerks: " << numApplicationClerks << endl;
            cout << "Number of Picture Clerks: " << numPictureClerks << endl;
            cout << "Number of Passport Clerks: " << numPassportClerks << endl;
            cout << "Number of Cashiers: " << numCashiers << endl << endl;
			cout << "Number of Senators: " << numSenators << endl << endl;
            cout << "Note: There must be 20 - 50 customers, 1 - 5 clerks per type, and 0-10 senators." << endl;
            cout << "Please enter new values: " << endl;
            int num;
            cout << "Number of Customers: ";
            if(cin >> num && num >= 20 && num <= 50 )
                numCustomers = num;
            else {
                cout << "Invalid input. Number of Customers unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
            cout << "Number of Application Clerks: ";
            if(cin >> num && num >= 1 && num <= 5 )
                numApplicationClerks = num;
            else {
                cout << "Invalid input. Number of Application Clerks unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
            cout << "Number of Picture Clerks: ";
            if(cin >> num && num >= 1 && num <= 5 )
                numPictureClerks = num;
            else {
                cout << "Invalid input. Number of Picture Clerks unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
            cout << "Number of Passport Clerks: ";
            if(cin >> num && num >= 1 && num <= 5 )
                numPassportClerks = num;
            else {
                cout << "Invalid input. Number of Passport Clerks unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
            cout << "Number of Cashiers: ";
            if(cin >> num && num >= 1 && num <= 5 )
                numCashiers = num;
            else {
                cout << "Invalid input. Number of Cashiers unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
			cout << "Number of Senators: ";
            if(cin >> num && num >= 0 && num <= 10 )
                numSenators = num;
            else {
                cout << "Invalid input. Number of Senators unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
            PassportOffice();
        } else if(input == "c") {  // Exit
            cout << "Exiting Passport Office." << endl;
            // break;
        } else {
            cout << "Invalid input. Please try again." << endl;
        }
        // cin.clear();
        // cin.ignore(10000, '\n');
 //   }
}

void Problem2() {
    // Default values for Customers and Clerks
    numCustomers = 20;
    numApplicationClerks = 1;
    numPictureClerks = 1;
    numPassportClerks = 1;
    numCashiers = 1;
    numSenators = 1;
    cout << "Welcome to the Passport Office." << endl;
    printMenu();
    //PassportOffice();
}


void Test1() {
	//test : Customers always take the shortest line, 
	//but no 2 customers ever choose the same shortest line at the same time
	runningTest1 = true;
	numCustomers = 6;
    numApplicationClerks = 2;
    numPictureClerks = 0;
    numPassportClerks = 0;
    numCashiers = 0;
    numSenators = 0;
    cout<<"Running Test 1" <<endl;
    cout<<"Number of Customers = " << numCustomers <<endl;
	cout<<"Number of ApplicationClerks = " << numApplicationClerks<<endl;
	cout<<"Number of PictureClerks = " << numPictureClerks <<endl;
	cout<<"Number of PassportClerks = " << numPassportClerks <<endl;
	cout<<"Number of Cashiers = " << numCashiers <<endl;
	cout<<"Number of Senators = " << numSenators <<endl;
    

	// Create locks and conditions
	senatorOutsideLineLock = new Lock("senatorOutsideLineLock");
	senatorInsideLock = new Lock("senatorInsideLock");
	customerOutsideLineLock = new Lock("customerOutsideLineLock");
	senatorOutsideLineCV = new Condition("senatorOutsideLineCV");
	customerOutsideLineCV = new Condition("customerOutsideLineCV");
	
    char* name;
    // Create Customers
    customers = new Customer*[numCustomers];
    for(int i = 0; i < numCustomers; i++) {
        name = new char[20];
        sprintf(name, "Customer %d", i);
        customers[i] = new Customer(name, i, false);
    }

    // Create ApplicationClerks
    applicationClerks = new Clerk*[numApplicationClerks];
    for(int i = 0; i < numApplicationClerks; i++) {
        name = new char[20];
        sprintf(name, "ApplicationClerk %d", i);
        applicationClerks[i] = new Clerk(name);
    }

    // Run ApplicationClerks
    for(int i = 0; i < numApplicationClerks; i++)
        applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);

     // Run Customers
    for(int i = 0; i < numCustomers; i++)
        customers[i]->Fork((VoidFunctionPtr)runCustomer, i);
}

void Test2() {
	//Managers only read one from one Clerk's total money received, at a time.
	runningTest2 = true;
	numCustomers = 0;
    numApplicationClerks = 5;
    numPictureClerks = 0;
    numPassportClerks = 0;
    numCashiers = 0;
    numSenators = 0;
    cout<<"Running Test 2" <<endl;
    cout<<"Number of Customers = " << numCustomers <<endl;
	cout<<"Number of ApplicationClerks = " << numApplicationClerks<<endl;
	cout<<"Number of PictureClerks = " << numPictureClerks <<endl;
	cout<<"Number of PassportClerks = " << numPassportClerks <<endl;
	cout<<"Number of Cashiers = " << numCashiers <<endl;
	cout<<"Number of Senators = " << numSenators <<endl;


	// Create locks and conditions
	senatorOutsideLineLock = new Lock("senatorOutsideLineLock");
	senatorInsideLock = new Lock("senatorInsideLock");
	customerOutsideLineLock = new Lock("customerOutsideLineLock");
	senatorOutsideLineCV = new Condition("senatorOutsideLineCV");
	customerOutsideLineCV = new Condition("customerOutsideLineCV");
	
    char* name;
	// Create ApplicationClerks
    applicationClerks = new Clerk*[numApplicationClerks];
    for(int i = 0; i < numApplicationClerks; i++) {
        name = new char[20];
        sprintf(name, "ApplicationClerk %d", i);
        applicationClerks[i] = new Clerk(name);
        applicationClerks[i]->money += (int)rand()% 100 + 1; //assign random amounts of money 1-100 for each clerk for testing purposes
    }

        // Run ApplicationClerks
    for(int i = 0; i < numApplicationClerks; i++)
        applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);

    manager = new Manager("manager 1");
    manager->Fork((VoidFunctionPtr)runManager, 0);
}

void Test3() {
	runningTest3 = true;
	numCustomers = 3;
    numApplicationClerks = 0;
    numPictureClerks = 0;
    numPassportClerks = 0;
    numCashiers = 1;
    numSenators = 0;
    cout<<"Running Test 3" <<endl;
    cout<<"Number of Customers = " << numCustomers <<endl;
	cout<<"Number of ApplicationClerks = " << numApplicationClerks<<endl;
	cout<<"Number of PictureClerks = " << numPictureClerks <<endl;
	cout<<"Number of PassportClerks = " << numPassportClerks <<endl;
	cout<<"Number of Cashiers = " << numCashiers <<endl;
	cout<<"Number of Senators = " << numSenators <<endl;


	// Create locks and conditions
	senatorOutsideLineLock = new Lock("senatorOutsideLineLock");
	senatorInsideLock = new Lock("senatorInsideLock");
	customerOutsideLineLock = new Lock("customerOutsideLineLock");
	senatorOutsideLineCV = new Condition("senatorOutsideLineCV");
	customerOutsideLineCV = new Condition("customerOutsideLineCV");
	
    char* name;
    // Create Cashiers
    cashiers = new Clerk*[numCashiers];
    for(int i = 0; i < numCashiers; i++) {
        name = new char[20];
        sprintf(name, "Cashier%d", i);
        cashiers[i] = new Clerk(name);
    }

    // Create Customers
    customers = new Customer*[numCustomers];
    for(int i = 0; i < numCustomers; i++) {
        name = new char[20];
        sprintf(name, "Customer %d", i);
        customers[i] = new Customer(name, i, false);
        customers[i]->hasApp = true; //assign each customer with app
        customers[i]->hasPic = true; //assign each customer with pic
        customers[i]->certifiedByPassportClerk = true; //assign each customer to be certified by passport clerk

    }

        // Run Customers
    for(int i = 0; i < numCustomers; i++)
        customers[i]->Fork((VoidFunctionPtr)runCustomer, i);

    // Run Cashiers
    for(int i = 0; i < numCashiers; i++)
        cashiers[i]->Fork((VoidFunctionPtr)runCashier, i);

    manager = new Manager("manager 1");
    manager->Fork((VoidFunctionPtr)runManager, 0);
}

void Test4() {
	//Clerks go on break when they have no one waiting in their line
	runningTest4 = true;
	numCustomers = 0;
    numApplicationClerks = 2;
    numPictureClerks = 2;
    numPassportClerks = 02;
    numCashiers = 2;
    numSenators = 0;
    cout<<"Running Test 4" <<endl;
    cout<<"Number of Customers = " << numCustomers <<endl;
	cout<<"Number of ApplicationClerks = " << numApplicationClerks<<endl;
	cout<<"Number of PictureClerks = " << numPictureClerks <<endl;
	cout<<"Number of PassportClerks = " << numPassportClerks <<endl;
	cout<<"Number of Cashiers = " << numCashiers <<endl;
	cout<<"Number of Senators = " << numSenators <<endl;


	// Create locks and conditions
	senatorOutsideLineLock = new Lock("senatorOutsideLineLock");
	senatorInsideLock = new Lock("senatorInsideLock");
	customerOutsideLineLock = new Lock("customerOutsideLineLock");
	senatorOutsideLineCV = new Condition("senatorOutsideLineCV");
	customerOutsideLineCV = new Condition("customerOutsideLineCV");
	
    char* name;

    // Create ApplicationClerks
    applicationClerks = new Clerk*[numApplicationClerks];
    for(int i = 0; i < numApplicationClerks; i++) {
        name = new char[20];
        sprintf(name, "ApplicationClerk %d", i);
        applicationClerks[i] = new Clerk(name);
    }

    // Create PictureClerks
    pictureClerks = new Clerk*[numPictureClerks];
    for(int i = 0; i < numPictureClerks; i++) {
        name = new char[20];
        sprintf(name, "PictureClerk %d", i);
        pictureClerks[i] = new Clerk(name);
    }

    
    // Create PassportClerks
    passportClerks = new Clerk*[numPassportClerks];
    for(int i = 0; i < numPassportClerks; i++) {
        name = new char[20];
        sprintf(name, "PassportClerk%d", i);
        passportClerks[i] = new Clerk(name);
    }

    // Create Cashiers
    cashiers = new Clerk*[numCashiers];
    for(int i = 0; i < numCashiers; i++) {
        name = new char[20];
        sprintf(name, "Cashier%d", i);
        cashiers[i] = new Clerk(name);
    }
    

    // Run ApplicationClerks
    for(int i = 0; i < numApplicationClerks; i++)
        applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);
    // Run PictureClerks
    for(int i = 0; i < numPictureClerks; i++)
        pictureClerks[i]->Fork((VoidFunctionPtr)runPictureClerk, i);
    
    
    // Run PassportClerks
    for(int i = 0; i < numPassportClerks; i++)
        passportClerks[i]->Fork((VoidFunctionPtr)runPassportClerk, i);
    // Run Cashiers
    for(int i = 0; i < numCashiers; i++)
        cashiers[i]->Fork((VoidFunctionPtr)runCashier, i);

    manager = new Manager("manager 1");
    manager->Fork((VoidFunctionPtr)runManager, 0);
}

void Test5() {
	runningTest5 = true;
	numCustomers = 3;
    numApplicationClerks = 0;
    numPictureClerks = 0;
    numPassportClerks = 0;
    numCashiers = 1;
    numSenators = 0;
    cout<<"Running Test 3" <<endl;
    cout<<"Number of Customers = " << numCustomers <<endl;
	cout<<"Number of ApplicationClerks = " << numApplicationClerks<<endl;
	cout<<"Number of PictureClerks = " << numPictureClerks <<endl;
	cout<<"Number of PassportClerks = " << numPassportClerks <<endl;
	cout<<"Number of Cashiers = " << numCashiers <<endl;
	cout<<"Number of Senators = " << numSenators <<endl;


	// Create locks and conditions
	senatorOutsideLineLock = new Lock("senatorOutsideLineLock");
	senatorInsideLock = new Lock("senatorInsideLock");
	customerOutsideLineLock = new Lock("customerOutsideLineLock");
	senatorOutsideLineCV = new Condition("senatorOutsideLineCV");
	customerOutsideLineCV = new Condition("customerOutsideLineCV");
	
    char* name;
    // Create Cashiers
    cashiers = new Clerk*[numCashiers];
    for(int i = 0; i < numCashiers; i++) {
        name = new char[20];
        sprintf(name, "Cashier%d", i);
        cashiers[i] = new Clerk(name);
    }

    // Create Customers
    customers = new Customer*[numCustomers];
    for(int i = 0; i < numCustomers; i++) {
        name = new char[20];
        sprintf(name, "Customer %d", i);
        customers[i] = new Customer(name, i, false);
        customers[i]->hasApp = true; //assign each customer with app
        customers[i]->hasPic = true; //assign each customer with pic
        customers[i]->certifiedByPassportClerk = true; //assign each customer to be certified by passport clerk

    }

        // Run Cashiers
    for(int i = 0; i < numCashiers; i++)
        cashiers[i]->Fork((VoidFunctionPtr)runCashier, i);
        // Run Customers
    for(int i = 0; i < numCustomers; i++)
        customers[i]->Fork((VoidFunctionPtr)runCustomer, i);



    manager = new Manager("manager 1");
    manager->Fork((VoidFunctionPtr)runManager, 0);
}

void Test6() {
	runningTest6 = true;
	numCustomers = 10;
    numApplicationClerks = 1;
    numPictureClerks = 1;
    numPassportClerks = 1;
    numCashiers = 1;
    numSenators = 0;
    cout<<"Running Test 6" <<endl;
    cout<<"Number of Customers = " << numCustomers <<endl;
	cout<<"Number of ApplicationClerks = " << numApplicationClerks<<endl;
	cout<<"Number of PictureClerks = " << numPictureClerks <<endl;
	cout<<"Number of PassportClerks = " << numPassportClerks <<endl;
	cout<<"Number of Cashiers = " << numCashiers <<endl;
	cout<<"Number of Senators = " << numSenators <<endl;


	// Create locks and conditions
	senatorOutsideLineLock = new Lock("senatorOutsideLineLock");
	senatorInsideLock = new Lock("senatorInsideLock");
	customerOutsideLineLock = new Lock("customerOutsideLineLock");
	senatorOutsideLineCV = new Condition("senatorOutsideLineCV");
	customerOutsideLineCV = new Condition("customerOutsideLineCV");
	
    char* name;
    // Create Customers
    customers = new Customer*[numCustomers];
    for(int i = 0; i < numCustomers; i++) {
        name = new char[20];
        sprintf(name, "Customer %d", i);
        customers[i] = new Customer(name, i, false);
    }

    // Create Senators
    senators = new Customer*[numSenators];
    for(int i = 0; i < numSenators; i++) {
        name = new char[20];
        sprintf(name, "Senator %d", i);
        senators[i] = new Customer(name, i, true);
    }

    // Create ApplicationClerks
    applicationClerks = new Clerk*[numApplicationClerks];
    for(int i = 0; i < numApplicationClerks; i++) {
        name = new char[20];
        sprintf(name, "ApplicationClerk %d", i);
        applicationClerks[i] = new Clerk(name);
    }

    // Create PictureClerks
    pictureClerks = new Clerk*[numPictureClerks];
    for(int i = 0; i < numPictureClerks; i++) {
        name = new char[20];
        sprintf(name, "PictureClerk %d", i);
        pictureClerks[i] = new Clerk(name);
    }

    
    // Create PassportClerks
    passportClerks = new Clerk*[numPassportClerks];
    for(int i = 0; i < numPassportClerks; i++) {
        name = new char[20];
        sprintf(name, "PassportClerk%d", i);
        passportClerks[i] = new Clerk(name);
    }

    // Create Cashiers
    cashiers = new Clerk*[numCashiers];
    for(int i = 0; i < numCashiers; i++) {
        name = new char[20];
        sprintf(name, "Cashier%d", i);
        cashiers[i] = new Clerk(name);
    }
    

    // Run ApplicationClerks
    for(int i = 0; i < numApplicationClerks; i++)
        applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);
    // Run PictureClerks
    for(int i = 0; i < numPictureClerks; i++)
        pictureClerks[i]->Fork((VoidFunctionPtr)runPictureClerk, i);

    // Run Customers
    for(int i = 0; i < numCustomers; i++)
        customers[i]->Fork((VoidFunctionPtr)runCustomer, i);

    // Run Senators
    for(int i = 0; i < numSenators; i++) {
    	senators[i]->Fork((VoidFunctionPtr)runSenator, i);
    }
    
    
    // Run PassportClerks
    for(int i = 0; i < numPassportClerks; i++)
        passportClerks[i]->Fork((VoidFunctionPtr)runPassportClerk, i);
    // Run Cashiers
    for(int i = 0; i < numCashiers; i++)
        cashiers[i]->Fork((VoidFunctionPtr)runCashier, i);
    

    manager = new Manager("manager 1");
    manager->Fork((VoidFunctionPtr)runManager, 0);
}

void Test7a() {
	runningTest7a = true;
	numCustomers = 1;
    numApplicationClerks = 1;
    numPictureClerks = 1;
    numPassportClerks = 1;
    numCashiers = 1;
    numSenators = 3;
    cout<<"Running Test 7a" <<endl;
    cout<<"Number of Customers = " << numCustomers <<endl;
	cout<<"Number of ApplicationClerks = " << numApplicationClerks<<endl;
	cout<<"Number of PictureClerks = " << numPictureClerks <<endl;
	cout<<"Number of PassportClerks = " << numPassportClerks <<endl;
	cout<<"Number of Cashiers = " << numCashiers <<endl;
	cout<<"Number of Senators = " << numSenators <<endl;


	// Create locks and conditions
	senatorOutsideLineLock = new Lock("senatorOutsideLineLock");
	senatorInsideLock = new Lock("senatorInsideLock");
	customerOutsideLineLock = new Lock("customerOutsideLineLock");
	senatorOutsideLineCV = new Condition("senatorOutsideLineCV");
	customerOutsideLineCV = new Condition("customerOutsideLineCV");
	
    char* name;
    // Create Customers
    customers = new Customer*[numCustomers];
    for(int i = 0; i < numCustomers; i++) {
        name = new char[20];
        sprintf(name, "Customer %d", i);
        customers[i] = new Customer(name, i, false);
    }

    // Create Senators
    senators = new Customer*[numSenators];
    for(int i = 0; i < numSenators; i++) {
        name = new char[20];
        sprintf(name, "Senator %d", i);
        senators[i] = new Customer(name, i, true);
    }

    // Create ApplicationClerks
    applicationClerks = new Clerk*[numApplicationClerks];
    for(int i = 0; i < numApplicationClerks; i++) {
        name = new char[20];
        sprintf(name, "ApplicationClerk %d", i);
        applicationClerks[i] = new Clerk(name);
    }

    // Create PictureClerks
    pictureClerks = new Clerk*[numPictureClerks];
    for(int i = 0; i < numPictureClerks; i++) {
        name = new char[20];
        sprintf(name, "PictureClerk %d", i);
        pictureClerks[i] = new Clerk(name);
    }

    
    // Create PassportClerks
    passportClerks = new Clerk*[numPassportClerks];
    for(int i = 0; i < numPassportClerks; i++) {
        name = new char[20];
        sprintf(name, "PassportClerk%d", i);
        passportClerks[i] = new Clerk(name);
    }

    // Create Cashiers
    cashiers = new Clerk*[numCashiers];
    for(int i = 0; i < numCashiers; i++) {
        name = new char[20];
        sprintf(name, "Cashier%d", i);
        cashiers[i] = new Clerk(name);
    }
    

    // Run ApplicationClerks
    for(int i = 0; i < numApplicationClerks; i++)
        applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);
    // Run PictureClerks
    for(int i = 0; i < numPictureClerks; i++)
        pictureClerks[i]->Fork((VoidFunctionPtr)runPictureClerk, i);

    // // Run Senator
	senators[0]->Fork((VoidFunctionPtr)runSenator, 0);
    // // Run Customer
	customers[0]->Fork((VoidFunctionPtr)runCustomer, 0);

    
    
    // Run PassportClerks
    for(int i = 0; i < numPassportClerks; i++)
        passportClerks[i]->Fork((VoidFunctionPtr)runPassportClerk, i);
    // Run Cashiers
    for(int i = 0; i < numCashiers; i++)
        cashiers[i]->Fork((VoidFunctionPtr)runCashier, i);
    

    manager = new Manager("manager 1");
    manager->Fork((VoidFunctionPtr)runManager, 0);




}

void Test7b() {
	runningTest7b = true;
	numCustomers = 1;
    numApplicationClerks = 1;
    numPictureClerks = 1;
    numPassportClerks = 1;
    numCashiers = 1;
    numSenators = 3;
    cout<<"Running Test 3" <<endl;
    cout<<"Number of Customers = " << numCustomers <<endl;
	cout<<"Number of ApplicationClerks = " << numApplicationClerks<<endl;
	cout<<"Number of PictureClerks = " << numPictureClerks <<endl;
	cout<<"Number of PassportClerks = " << numPassportClerks <<endl;
	cout<<"Number of Cashiers = " << numCashiers <<endl;
	cout<<"Number of Senators = " << numSenators <<endl;


	// Create locks and conditions
	senatorOutsideLineLock = new Lock("senatorOutsideLineLock");
	senatorInsideLock = new Lock("senatorInsideLock");
	customerOutsideLineLock = new Lock("customerOutsideLineLock");
	senatorOutsideLineCV = new Condition("senatorOutsideLineCV");
	customerOutsideLineCV = new Condition("customerOutsideLineCV");
	
    char* name;
    // Create Customers
    customers = new Customer*[numCustomers];
    for(int i = 0; i < numCustomers; i++) {
        name = new char[20];
        sprintf(name, "Customer %d", i);
        customers[i] = new Customer(name, i, false);
    }

    // Create Senators
    senators = new Customer*[numSenators];
    for(int i = 0; i < numSenators; i++) {
        name = new char[20];
        sprintf(name, "Senator %d", i);
        senators[i] = new Customer(name, i, true);
    }

    // Create ApplicationClerks
    applicationClerks = new Clerk*[numApplicationClerks];
    for(int i = 0; i < numApplicationClerks; i++) {
        name = new char[20];
        sprintf(name, "ApplicationClerk %d", i);
        applicationClerks[i] = new Clerk(name);
    }

    // Create PictureClerks
    pictureClerks = new Clerk*[numPictureClerks];
    for(int i = 0; i < numPictureClerks; i++) {
        name = new char[20];
        sprintf(name, "PictureClerk %d", i);
        pictureClerks[i] = new Clerk(name);
    }

    
    // Create PassportClerks
    passportClerks = new Clerk*[numPassportClerks];
    for(int i = 0; i < numPassportClerks; i++) {
        name = new char[20];
        sprintf(name, "PassportClerk%d", i);
        passportClerks[i] = new Clerk(name);
    }

    // Create Cashiers
    cashiers = new Clerk*[numCashiers];
    for(int i = 0; i < numCashiers; i++) {
        name = new char[20];
        sprintf(name, "Cashier%d", i);
        cashiers[i] = new Clerk(name);
    }
    

    // Run ApplicationClerks
    for(int i = 0; i < numApplicationClerks; i++)
        applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);
    // Run PictureClerks
    for(int i = 0; i < numPictureClerks; i++)
        pictureClerks[i]->Fork((VoidFunctionPtr)runPictureClerk, i);

    // // Run Customer
	customers[0]->Fork((VoidFunctionPtr)runCustomer, 0);

    // // Run Senator
	senators[0]->Fork((VoidFunctionPtr)runSenator, 0);


    
    
    // Run PassportClerks
    for(int i = 0; i < numPassportClerks; i++)
        passportClerks[i]->Fork((VoidFunctionPtr)runPassportClerk, i);
    // Run Cashiers
    for(int i = 0; i < numCashiers; i++)
        cashiers[i]->Fork((VoidFunctionPtr)runCashier, i);
    

    manager = new Manager("manager 1");
    manager->Fork((VoidFunctionPtr)runManager, 0);




}

void printTest7Menu() {
 cout << "Test 7" << endl;
    cout << "Please enter an option:" << endl;
    cout << " -T7a  (Run Test 7a: Senator comes in with no Customers inside)" << endl;
    cout << " -T7b  (Run Test 7b: Senator comes in with Customers inside)" << endl;
    string input;
    getline(cin, input);
    if(input == "T7a")
        Test7a();
    else if(input == "T7b")
        Test7b();
    else
        cout << "Invalid input." << endl;

}

 void printTestMenu() {
  cout << "Welcome to TestSuite" << endl;
    //while(true) {
        cout << "Please enter an option:" << endl;
        cout << " -T1  (Run Test 1)" << endl;
        cout << " -T2  (Run Test 2)" << endl;
        cout << " -T3  (Run Test 3)" << endl;
        cout << " -T4  (Run Test 4)" << endl;
        cout << " -T5  (Run Test 5)" << endl;
        cout << " -T6  (Run Test 6)" << endl;
        cout << " -T7  (Run Test 7)" << endl;
        cout << " -e  (Exit)" << endl;
        string input;
        getline(cin, input);
        if(input == "-T1")
            Test1();
        else if(input == "T2")
            Test2();
        else if(input == "T3")
            Test3();
        else if(input == "T4")
            Test4();
        else if(input == "T5")
            Test5();
        else if(input == "T6")
            Test6();
        else if(input == "T7")
            printTest7Menu();
        else if(input == "TC")
        	TestCode();
        else if(input == "e") {
        cout << "Exiting TestSuite" << endl;
        //break;
        } else
            cout << "Invalid input. Please try again." << endl;
        //cin.clear();
        //cin.ignore(10000, '\n');
    //}
 }


void TestSuite() {
	printTestMenu();
	//Test7a();
    //Test1();
    //Test2();
    //Test3();
    //Test4();
    //Test5();
    //Test6();
    //Test7a();
	//Test7b();
}










































// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
 
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
	    t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
	    t1_l1.getName());
    for (int i = 0; i < 10; i++)
	;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
	printf("%s: Trying to release Lock %s\n",currentThread->getName(),
	       t1_l1.getName());
	t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}
 
// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestCode() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
	t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t3_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
	t3_done.P();

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t4_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
	t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}










