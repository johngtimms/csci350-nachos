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

#define NUM_CLERKS 4

//
// Forward declarations
//

class Customer;
class Clerk;
class ApplicationClerk;
class PictureClerk;
class PassportClerk;
class Cashier;
class Manager;

//
// Variables
//

enum ClerkState {FREE, BUSY, BREAK};
enum ClerkType {APPLICATION_CLERK, PICTURE_CLERK, PASSPORT_CLERK, CASHIER};
int amounts[] = {100, 600, 1100, 1600};
int numCustomers, numApplicationClerks, numPictureClerks, numPassportClerks, numCashiers, numSenators;
Customer** customers;
ApplicationClerk** applicationClerks;
PictureClerk** pictureClerks;
PassportClerk** passportClerks;
Cashier** cashiers;
Manager *theManager;
Lock *senatorOutsideLineLock;
Condition *senatorOutsideLineCV;
int senatorsOutside = 0;
Lock *senatorInsideLock;
bool senatorInside = false;
Lock *customerOutsideLineLock;
Condition *customerOutsideLineCV;
int customersOutside = 0;

//
// Function declarations
//

void PassportOffice();
void runApplicationClerk(int id);
void runCustomer(int ssn);
void printMenu();

//
// Customer class header
//

class Customer : public Thread {
	
	public:
		int ssn;
		int money;
		int myClerk;

		bool didBribe;
		int clerkID; // intended as a clearer reference to myClerk
		
		bool isSenator;
		bool hasApp;
		bool hasPic;
		bool hasPassport;
		bool seenApp;
		bool seenPic;
		bool likedPic;

		Customer(int _ssn, char* debugName, bool _isSenator = false);
		void doApplication();
		void doPicture();
		void waitInLine(Clerk** clerks, int numClerks);
		void chooseLine(Clerk** clerks, int numClerks);
		bool enterLine(Clerk* clerk);
};

//
// Clerk class header (only for inheritance)
//

class Clerk : public Thread { 

	public:
		int lineLength;
		int bribeLineLength;
		int money;

		ClerkState state;

		Lock *lineLock, *bribeLineLock;
		Condition *lineCV, *bribeLineCV;

		Clerk(int id, char* debugName) : Thread(debugName) { }
};

//
// ApplicationClerk class header
//

class ApplicationClerk : public Clerk {
	
	public:
		int appClerkID;
		
		Lock *clerkLock, *moneyLock;
		Condition *clerkCV, *breakCV;

		ApplicationClerk(int id, char* debugName);
};

//
// PictureClerk class header
//

class PictureClerk : public Clerk {
	
	public:
		int picClerkID;

		Customer *myCustomer;
		
		Lock *clerkLock, *moneyLock;
		Condition *clerkCV, *breakCV;

		PictureClerk(int id, char* debugName);
};

//
// PassportClerk class header
//



//
// Cashier class header
//



//
// Cashier class source
//



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

Customer::Customer(int _ssn, char* debugName, bool _isSenator) : Thread(debugName) {
    ssn = _ssn;
	isSenator = _isSenator;
    myClerk = -1;
    money = amounts[(int)(rand() % NUM_CLERKS)];
    hasApp = false;
    hasPic = false;
    hasPassport = false;
    seenApp = false;
    seenPic = false;
    likedPic = false;
}

void Customer::doApplication() {
	this->waitInLine(applicationClerks, numApplicationClerks);
	ApplicationClerk* clerk = applicationClerks[clerkID];

    // Interaction with clerk
    clerk->clerkLock->Acquire();
    cout << "Customer " << currentThread->getName() << " currently with " << clerk->getName() << endl;
    clerk->clerkCV->Signal(clerk->clerkLock); // Give incomplete application to Application Clerk
    clerk->clerkCV->Wait(clerk->clerkLock);   // Wait for Application Clerk
    clerk->clerkCV->Signal(clerk->clerkLock); // Accept completed application
    cout << "Customer " << currentThread->getName() << " finished with " << clerk->getName() << endl;
    hasApp = true;
    clerk->clerkLock->Release();
}

void Customer::doPicture() {
	this->waitInLine(pictureClerks, numPictureClerks);
	PictureClerk* clerk = pictureClerks[clerkID];

    // Interaction with clerk
    // JGT- didn't make any changes here, but recursivly calling doPicture() isn't a good solution
    seenPic = false;
    clerk->clerkLock->Acquire();
    clerk->myCustomer = this;
    cout << currentThread->getName() << " currently with " << clerk->getName() << endl;
    clerk->clerkCV->Signal(clerk->clerkLock); // Customer with Clerk
    clerk->clerkCV->Wait(clerk->clerkLock);   // Wait for Picture Clerk
    seenPic = true;
    if(((double) rand() / RAND_MAX) < .25) {// Customer decides whether they don't like picture
        likedPic = false;
        cout << currentThread->getName() <<  " didn't like picture." << endl;
        clerk->clerkCV->Signal(clerk->clerkLock);
        clerk->clerkLock->Release();
        doPicture();
    } else {
        likedPic = true;
        cout << currentThread->getName() << " liked picture." << endl;
        clerk->clerkCV->Signal(clerk->clerkLock);
        cout << currentThread->getName() << " finished with " << clerk->getName() << endl;
        hasPic = true;
        clerk->clerkLock->Release();
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
		clerks[clerkID]->money += 500;
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
		if (didBribe) {
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
	if (senatorInside) {
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

		return false;
	}
}

//
// ApplicationClerk class source
//

ApplicationClerk::ApplicationClerk(int id, char* debugName) : Clerk(id, debugName) {
	appClerkID = id;
	money = 0;
	state = BUSY;
	lineLength = 0;
	bribeLineLength = 0;
	clerkLock = new Lock("appClerkLock");
	lineLock = new Lock("appClerkLineLock");
	bribeLineLock = new Lock("appClerkBribeLineLock");
	moneyLock = new Lock("appmoneyLock");
	clerkCV = new Condition("appClerkCV");
	lineCV = new Condition("appClerkLineCV");
	bribeLineCV = new Condition("appClerkBribeLineCV");
	breakCV = new Condition("appClerkBreakCV");
}

//
// PictureClerk class source
//

PictureClerk::PictureClerk(int id, char* debugName) : Clerk(id, debugName) {
	picClerkID = id;
	money = 0;
	state = BUSY;
	lineLength = 0;
	bribeLineLength = 0;
	clerkLock = new Lock("appClerkLock");
	lineLock = new Lock("appClerkLineLock");
	bribeLineLock = new Lock("appClerkBribeLineLock");
	moneyLock = new Lock("picmoneyLock");
	clerkCV = new Condition("appClerkCV");
	lineCV = new Condition("appClerkLineCV");
	bribeLineCV = new Condition("appClerkBribeLineCV");
	breakCV = new Condition("appClerkBreakCV");
}

//
// PassportClerk class source
//



//
// Cashier class source
//



//
// Manager class source
//

Manager::Manager(char* debugName) : Thread(debugName) {
	totalMoneyMade = 0;
}

void Manager::managerMain() {
	while(true) {
		//delay
		bool allClerksOnBreak = true; 
		int appClerkMoneyTotal = 0;
		int picClerkMoneyTotal = 0;
		int passportClerkMoneyTotal = 0;

		for(int k = 0; k < numApplicationClerks; k ++) { //loop through all app clerks
			ApplicationClerk *thisClerk = applicationClerks[k];
			thisClerk->moneyLock->Acquire();
			appClerkMoneyTotal += thisClerk->money;  //get this clerks money total
			thisClerk->moneyLock->Release();
			if(thisClerk->state != BREAK) { //check if not on break 
				allClerksOnBreak = false; 
			} else { //if on break
				thisClerk->bribeLineLock->Acquire();
				thisClerk->lineLock->Acquire();
				if((thisClerk->bribeLineLength + thisClerk->lineLength) >= 3) { //check if >=3 customers are in line
					thisClerk->breakCV->Signal(thisClerk->clerkLock); //if so, then wake up clerk 
					allClerksOnBreak = false; //all clerks are no longer are on break
				}
				thisClerk->bribeLineLock->Release();
				thisClerk->lineLock->Release();
			}

		}
		for(int k = 0; k < numPictureClerks; k ++) { //loop through all pic clerks
			PictureClerk *thisClerk = pictureClerks[k];
			thisClerk->moneyLock->Acquire();
			picClerkMoneyTotal += thisClerk->money;  //get this clerks money total
			thisClerk->moneyLock->Release();
			if(thisClerk->state != BREAK) { //check if not on break 
				allClerksOnBreak = false; 
			} else { //if on break
				thisClerk->bribeLineLock->Acquire();
				thisClerk->lineLock->Acquire();
				if((thisClerk->bribeLineLength + thisClerk->lineLength) >= 3) { //check if >=3 customers are in line
					thisClerk->breakCV->Signal(thisClerk->clerkLock); //if so, then wake up clerk 
					allClerksOnBreak = false; //all clerks are no longer are on break
				}
				thisClerk->bribeLineLock->Release();
				thisClerk->lineLock->Release();
			}

		}

		if(allClerksOnBreak) //if all clerks are on break
			continue;

		//output money statements

		for(int k = 0 ; k < 100 ; k ++ ) {
			currentThread->Yield();
		}
		

		cout << "Manager: App clerks have made a total of $" << appClerkMoneyTotal << endl;
		cout << "Manager: Pic clerks have made a total of $" << picClerkMoneyTotal << endl;
		
		totalMoneyMade = appClerkMoneyTotal + picClerkMoneyTotal + passportClerkMoneyTotal;
		cout << "Manager: Passport Office has made a total of $" << totalMoneyMade << endl;
	}
}

//
// Thread behavior runners
// 

void runCustomer(int ssn) {
	// Setup
    Customer *thisCustomer = customers[ssn];
    cout << thisCustomer->getName() << " currently running." << endl;  

	// Determine money and bribes
	
	// Senator stuff
	senatorOutsideLineLock->Acquire();
	senatorInsideLock->Acquire();
	if (thisCustomer->isSenator) {
		// Senators wait on other senators outside
		if (senatorsOutside > 0 || senatorInside) {
			senatorsOutside += 1;
			senatorInsideLock->Release();
			senatorOutsideLineCV->Wait(senatorOutsideLineLock);
			senatorInsideLock->Acquire();
			senatorsOutside -= 1;
		}
		
		// Don't Signal() senatorOutsideLineCV except for when a senator leaves.
		senatorInside = true;
		senatorInsideLock->Release();
		senatorOutsideLineLock->Release();
		
		// Senator entering, alert all lines to empty
		for(int i = 0; i < numApplicationClerks; i++) {
			ApplicationClerk *clerk = applicationClerks[i];
			clerk->lineLock->Acquire();
			clerk->lineCV->Broadcast(clerk->lineLock);
			clerk->lineLock->Release();
			clerk->bribeLineLock->Acquire();
			clerk->bribeLineCV->Broadcast(clerk->bribeLineLock);
			clerk->bribeLineLock->Release();
		}
		
		for(int i = 0; i < numPictureClerks; i++) {
			PictureClerk *clerk = pictureClerks[i];
			clerk->lineLock->Acquire();
			clerk->lineCV->Broadcast(clerk->lineLock);
			clerk->lineLock->Release();
			clerk->bribeLineLock->Acquire();
			clerk->bribeLineCV->Broadcast(clerk->bribeLineLock);
			clerk->bribeLineLock->Release();
		}
		
		/* for(int i = 0; i < numPassportClerks; i++) {
			PassportClerk *clerk = passportClerks[i];
			clerk->lineLock->Acquire();
			clerk->lineCV->Broadcast(clerk->lineLock);
			clerk->lineLock->Release();
			clerk->bribeLineLock->Acquire();
			clerk->bribeLineCV->Broadcast(clerk->bribeLineLock);
			clerk->bribeLineLock->Release();
		}
		
		for(int i = 0; i < numCashiers; i++) {
			Cashier *clerk = cashiers[i];
			clerk->lineLock->Acquire();
			clerk->lineCV->Broadcast(clerk->lineLock);
			clerk->lineLock->Release();
			clerk->bribeLineLock->Acquire();
			clerk->bribeLineCV->Broadcast(clerk->bribeLineLock);
			clerk->bribeLineLock->Release();
		} */
	} else {
		if (senatorsOutside > 0 || senatorInside) {
			senatorOutsideLineLock->Release();
			senatorInsideLock->Release();
			customerOutsideLineLock->Acquire();
			customersOutside += 1;
			customerOutsideLineCV->Wait(customerOutsideLineLock);
			customersOutside -= 1;
			customerOutsideLineLock->Release();
		}
		senatorInsideLock->Release();
		senatorOutsideLineLock->Release();
	}

    // Randomly decide whether to go to AppClerk or PicClerk first
    if(rand() % 2 == 1) {				// ApplicationClerk first
        cout << thisCustomer->getName() << " going to complete application before taking picture." << endl;
        thisCustomer->doApplication();
        cout << thisCustomer->getName() << " going to take picture." << endl;
        thisCustomer->doPicture();

    } else {							// PictureClerk first
        cout << thisCustomer->getName() << " going to take picture before completing application." << endl;
        customers[ssn]->doPicture();
        cout << thisCustomer->getName() << " going to complete application." << endl;
        customers[ssn]->doApplication();
    }
	
    /*
    // Get verified by PassportClerk
    // Pay for Passport at Cashier
    */
}

void runApplicationClerk(int id) {
    ApplicationClerk *thisClerk = applicationClerks[id];
    cout << thisClerk->getName() << " currently running." << endl;
    while(true) {
        if(thisClerk->bribeLineLength > 0) {
            thisClerk->bribeLineLock->Acquire();
            thisClerk->bribeLineCV->Signal(thisClerk->bribeLineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->bribeLineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to turn in application
            int wait = rand() % ((100 - 20) + 1) + 20;
            for(int i = 0; i < wait; i++)               // Process application
                currentThread->Yield();
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give complete application back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept application
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        } else if(thisClerk->lineLength > 0) {
            thisClerk->lineLock->Acquire();
            thisClerk->lineCV->Signal(thisClerk->lineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->lineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to turn in application
            int wait = rand() % ((100 - 20) + 1) + 20;
            for(int i = 0; i < wait; i++)               // Process application
                thisClerk->Yield();
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give complete application back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept application
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        } else {
            thisClerk->clerkLock->Acquire();
            thisClerk->state = BREAK;
            cout << thisClerk->getName() << " taking a break." << endl;
            thisClerk->breakCV->Wait(applicationClerks[id]->clerkLock);
            cout << thisClerk->getName() << " back from break." << endl;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        }
    }
}

void runPictureClerk(int id) {
    PictureClerk *thisClerk = pictureClerks[id];
    cout << thisClerk->getName() << " currently running." << endl;
    while(true) {
        if(thisClerk->bribeLineLength > 0) {
            thisClerk->bribeLineLock->Acquire();
            thisClerk->bribeLineCV->Signal(thisClerk->bribeLineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->bribeLineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to get ready for picture
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give picture back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept picture
            if(thisClerk->myCustomer->likedPic) {
                cout << thisClerk->getName() << " filing " << thisClerk->myCustomer->getName() << "'s picture." << endl;
                int wait = rand() % ((100 - 20) + 1) + 20;
                for(int i = 0; i < wait; i++)               // Process picture
                    currentThread->Yield();
                cout << thisClerk->getName() << " finished filing " << thisClerk->myCustomer->getName() << "'s picture." << endl;
            }
            thisClerk->myCustomer = NULL;
            thisClerk->clerkLock->Release();
            thisClerk->state = FREE;
        } else if(thisClerk->lineLength > 0) {
            thisClerk->lineLock->Acquire();
            thisClerk->lineCV->Signal(thisClerk->lineLock);     // Signal Customer to exit line
            thisClerk->clerkLock->Acquire(); //
            thisClerk->lineLock->Release();
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for customer to get ready for picture
            thisClerk->clerkCV->Signal(thisClerk->clerkLock);    // Give picture back
            thisClerk->clerkCV->Wait(thisClerk->clerkLock);      // Wait for Customer to accept picture
            cout << thisClerk->getName() << " - " << thisClerk->myCustomer->getName() << endl;
            if(thisClerk->myCustomer->likedPic) {
                cout << thisClerk->getName() << " filing " << thisClerk->myCustomer->getName() << "'s picture." << endl;
                int wait = rand() % ((100 - 20) + 1) + 20;
                for(int i = 0; i < wait; i++)               // Process picture
                    currentThread->Yield();
                cout << thisClerk->getName() << " finished filing " << thisClerk->myCustomer->getName() << "'s picture." << endl;
            }
            thisClerk->myCustomer = NULL;
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

void runManager() {
	theManager->managerMain();
}

//
// Passport Office code and menu
//

void PassportOffice() {
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
        sprintf(name, "Customer%d", i);
        customers[i] = new Customer(i, name);
    }

    // Create ApplicationClerks
    applicationClerks = new ApplicationClerk*[numApplicationClerks];
    for(int i = 0; i < numApplicationClerks; i++) {
        name = new char[20];
        sprintf(name, "ApplicationClerk%d", i);
        applicationClerks[i] = new ApplicationClerk(i, name);
    }

    // Create PictureClerks
    pictureClerks = new PictureClerk*[numPictureClerks];
    for(int i = 0; i < numPictureClerks; i++) {
        name = new char[20];
        sprintf(name, "PictureClerk%d", i);
        pictureClerks[i] = new PictureClerk(i, name);
    }

    /*
    // Create PassportClerks
    passportClerks = new PassportClerk*[numPassportClerks];
    for(int i = 0; i < numPassportClerks; i++) {
        name = new char[20];
        sprintf(name, "PassportClerk%d", i);
        passportClerks[i] = new PassportClerk(i, name);
    }

    // Create Cashiers
    cashiers = new Cashier*[numCashiers];
    for(int i = 0; i < numCashiers; i++) {
        name = new char[20];
        sprintf(name, "Cashier%d", i);
        cashiers[i] = new Cashiers(i, name);
    }
    */

    // Run ApplicationClerks
    for(int i = 0; i < numApplicationClerks; i++)
        applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);
    // Run PictureClerks
    for(int i = 0; i < numPictureClerks; i++)
        pictureClerks[i]->Fork((VoidFunctionPtr)runPictureClerk, i);

    // Run Customers
    for(int i = 0; i < numCustomers; i++)
        customers[i]->Fork((VoidFunctionPtr)runCustomer, i);
    
    /*
    // Run PassportClerks
    for(int i = 0; i < numPassportClerks; i++)
        passportClerks[i]->Fork((VoidFunctionPtr)runPassportClerk, i);
    // Run Cashiers
    for(int i = 0; i < numCashiers; i++)
        cashiers[i]->Fork((VoidFunctionPtr)runCashier, i);
    */

    theManager = new Manager("manager 1");
    theManager->Fork((VoidFunctionPtr)runManager, 0);

}

void Problem2() {
    // Default values for Customers and Clerks
    numCustomers = 5;
    numApplicationClerks = 1;
    numPictureClerks = 1;
    numPassportClerks = 5;
    numCashiers = 5;
    cout << "Welcome to the Passport Office." << endl;
    //printMenu();
    PassportOffice();
}

void printMenu() {
    // Run menu for Part 2 of assignment
    while(true) {
        cout << "Please enter an option:" << endl;
        cout << " -a  (View/Edit default values)" << endl;
        cout << " -b  (Run a test)" << endl;
        cout << " -c  (Exit)" << endl;
        string input;
        getline(cin, input);
        if(input == "-a") {     // Print/Edit default values
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
        } else if(input == "-b") {  // Run a test
            PassportOffice();
        } else if(input == "-c") {  // Exit
            cout << "Exiting Passport Office." << endl;
            break;
        } else {
            cout << "Invalid input. Please try again." << endl;
        }
        cin.clear();
        cin.ignore(10000, '\n');
    }
}

int getRandomNumber() {
	return 4; // chosen by fair dice roll, guaranteed to be random
}
