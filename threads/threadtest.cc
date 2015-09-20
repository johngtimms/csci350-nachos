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
int numCustomers, numAppClerks, numPicClerks, numPassportClerks, numCashiers;
Customer** customers;
ApplicationClerk** applicationClerks;
PictureClerk** pictureClerks;
PassportClerk** passportClerks;
Cashier** cashiers;
Manager *theManager;

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
		
		bool hasApp;
		bool hasPic;
		bool hasPassport;
		bool seenApp;
		bool seenPic;
		bool likedPic;

		Customer(int _ssn, char* debugName);
		void getApplicationFiled();
		void getPictureTaken();
};

//
// ApplicationClerk class header
//

class ApplicationClerk : public Thread {
	
	public:
		int appClerkID;
		int money;
		int lineLength;
		int bribeLineLength;
		
		ClerkState state;
		
		Lock *clerkLock, *lineLock, *bribeLineLock, *moneyLock;
		Condition *clerkCV, *lineCV, *bribeLineCV, *breakCV;

		ApplicationClerk(int id, char* debugName);
};

//
// PictureClerk class header
//

class PictureClerk : public Thread {
	
	public:
		int picClerkID;
		int money; 
		int lineLength; 
		int bribeLineLength;
		
		ClerkState state;
		Customer *myCustomer;
		
		Lock *clerkLock, *lineLock, *bribeLineLock, *moneyLock;
		Condition *clerkCV, *lineCV, *bribeLineCV, *breakCV;

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

Customer::Customer(int _ssn, char* debugName) : Thread(debugName) {
    ssn = _ssn;
    myClerk = -1;
    money = amounts[(int)(rand() % NUM_CLERKS)];
    hasApp = false;
    hasPic = false;
    hasPassport = false;
    seenApp = false;
    seenPic = false;
    likedPic = false;
}

void Customer::getApplicationFiled() {
    // Choose ApplictionClerk with shortest line
    //int myClerk;
    int minLength = 50;
    for(int i = 0; i < numAppClerks; i++) {
        if(applicationClerks[i]->lineLength < minLength) {
            myClerk = i;
            minLength = applicationClerks[i]->lineLength; 
        }
    }
    ApplicationClerk* appClerk = applicationClerks[myClerk];
    if(appClerk->state != FREE) {    // Stand in line if ApplicationClerk is busy
        if(appClerk->lineLength > 0 && money > 500) {
            appClerk->bribeLineLock->Acquire();
            money = money - 500;
            //clerkMoney[APPLICATION_CLERK] += 500;
            //appClerk->moneyLock->Acquire();
            appClerk->money = appClerk->money + 500;
            //appClerk->moneyLock->Release();
            appClerk->bribeLineLength++;
            cout << "Customer " << currentThread->getName() << " bribed " << appClerk->getName();
            cout << ", currently waiting in bribeLine" << endl;
            appClerk->bribeLineCV->Wait(appClerk->bribeLineLock);
            appClerk->state = BUSY;           // Called by Application Clerk
            appClerk->bribeLineLength--;           // Leaving line
            appClerk->bribeLineLock->Release();
        } else {
            appClerk->lineLock->Acquire();
            appClerk->lineLength++;
            cout << "Customer " << currentThread->getName() << " waiting in line for " << appClerk->getName() << endl;
            appClerk->lineCV->Wait(appClerk->lineLock);
            appClerk->state = BUSY;           // Called by Application Clerk
            appClerk->lineLength--;           // Leaving line
            appClerk->lineLock->Release();
        }
    }
    // Interaction with clerk
    appClerk->clerkLock->Acquire();
    cout << "Customer " << currentThread->getName() << " currently with " << appClerk->getName() << endl;
    appClerk->clerkCV->Signal(appClerk->clerkLock); // Give incomplete application to Application Clerk
    appClerk->clerkCV->Wait(appClerk->clerkLock);   // Wait for Application Clerk
    appClerk->clerkCV->Signal(appClerk->clerkLock); // Accept completed application
    cout << "Customer " << currentThread->getName() << " finished with " << appClerk->getName() << endl;
    hasApp = true;
    appClerk->clerkLock->Release();
}

void Customer::getPictureTaken() {
    int minLength = 50;
    for(int i = 0; i < numPicClerks; i++) {
        if(pictureClerks[i]->lineLength < minLength) {
            myClerk = i;
            minLength = pictureClerks[i]->lineLength; 
        }
    }
    seenPic = false;
    PictureClerk* picClerk = pictureClerks[myClerk];
    
    if(picClerk->state != FREE) {    // Stand in line if ApplicationClerk is busy
        if(picClerk->lineLength > 0 && money > 500) {
            picClerk->bribeLineLock->Acquire();
            money = money - 500;
            //clerkMoney[APPLICATION_CLERK] += 500;
            //picClerk->moneyLock->Acquire();
            picClerk->money = picClerk->money + 500;
            //picClerk->moneyLock->Release();
            picClerk->bribeLineLength++;
            cout << getName() << " bribed " << picClerk->getName();
            cout << ", currently waiting in bribeLine" << endl;
            picClerk->bribeLineCV->Wait(picClerk->bribeLineLock);
            picClerk->state = BUSY;           // Called by Application Clerk
            picClerk->bribeLineLength--;           // Leaving line
            picClerk->bribeLineLock->Release();
        } else {
            picClerk->lineLock->Acquire();
            picClerk->lineLength++;
            cout << currentThread->getName() << " waiting in line for " << picClerk->getName() << endl;
            picClerk->lineCV->Wait(picClerk->lineLock);
            picClerk->state = BUSY;           // Called by Application Clerk
            picClerk->lineLength--;           // Leaving line
            picClerk->lineLock->Release();
        }
    }
    // Interaction with clerk
    picClerk->clerkLock->Acquire();
    picClerk->myCustomer = this;
    cout << currentThread->getName() << " currently with " << picClerk->getName() << endl;
    picClerk->clerkCV->Signal(picClerk->clerkLock); // Customer with Clerk
    picClerk->clerkCV->Wait(picClerk->clerkLock);   // Wait for Picture Clerk
    seenPic = true;
    if(((double) rand() / RAND_MAX) < .25) {// Customer decides whether they don't like picture
        likedPic = false;
        cout << currentThread->getName() <<  " didn't like picture." << endl;
        picClerk->clerkCV->Signal(picClerk->clerkLock);
        picClerk->clerkLock->Release();
        getPictureTaken();
    } else {
        likedPic = true;
        cout << currentThread->getName() << " liked picture." << endl;
        picClerk->clerkCV->Signal(picClerk->clerkLock);
        cout << currentThread->getName() << " finished with " << picClerk->getName() << endl;
        hasPic = true;
        picClerk->clerkLock->Release();
    }
}

//
// ApplicationClerk class source
//

ApplicationClerk::ApplicationClerk(int id, char* debugName) : Thread(debugName) {
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

PictureClerk::PictureClerk(int id, char* debugName) : Thread(debugName) {
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

		for(int k = 0; k < numAppClerks; k ++) { //loop through all app clerks
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
		for(int k = 0; k < numPicClerks; k ++) { //loop through all pic clerks
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
    Customer* thisCustomer = customers[ssn];
    cout << thisCustomer->getName() << " currently running." << endl;    
    // Decide whether this Customer is a senator
    // Randomly decide whether to go to AppClerk or PicClerk first
    if(rand() % 2 == 1) {   // ApplicationClerk first
        cout << thisCustomer->getName() << " going to complete application before taking picture." << endl;
        thisCustomer->getApplicationFiled();
        cout << thisCustomer->getName() << " going to take picture." << endl;
        thisCustomer->getPictureTaken();

    } else {    // PictureClerk first
        cout << thisCustomer->getName() << " going to take picture before completing application." << endl;
        customers[ssn]->getPictureTaken();
        cout << thisCustomer->getName() << " going to complete application." << endl;
        customers[ssn]->getApplicationFiled();
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
    char* name;
    // Create Customers
    customers = new Customer*[numCustomers];
    for(int i = 0; i < numCustomers; i++) {
        name = new char[20];
        sprintf(name, "Customer%d", i);
        customers[i] = new Customer(i, name);
    }

    // Create ApplicationClerks
    applicationClerks = new ApplicationClerk*[numAppClerks];
    for(int i = 0; i < numAppClerks; i++) {
        name = new char[20];
        sprintf(name, "ApplicationClerk%d", i);
        applicationClerks[i] = new ApplicationClerk(i, name);
    }

    // Create PictureClerks
    pictureClerks = new PictureClerk*[numPicClerks];
    for(int i = 0; i < numPicClerks; i++) {
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
    for(int i = 0; i < numAppClerks; i++)
        applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);
    // Run PictureClerks
    for(int i = 0; i < numPicClerks; i++)
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
    numAppClerks = 1;
    numPicClerks = 1;
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
            cout << "Number of Appication Clerks: " << numAppClerks << endl;
            cout << "Number of Picture Clerks: " << numPicClerks << endl;
            cout << "Number of Passport Clerks: " << numPassportClerks << endl;
            cout << "Number of Cashiers: " << numCashiers << endl << endl;
            cout << "Note: There can only be 20 - 50 Customers and 1 - 5 of each type of clerk." << endl;
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
                numAppClerks = num;
            else {
                cout << "Invalid input. Number of Application Clerks unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
            cout << "Number of Picture Clerks: ";
            if(cin >> num && num >= 1 && num <= 5 )
                numPicClerks = num;
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
