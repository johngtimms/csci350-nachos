#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
using namespace std;
#define NUM_CLERKS 4

enum ClerkState {FREE, BUSY, BREAK};
enum ClerkType {APPLICATION_CLERK, PICTURE_CLERK, PASSPORT_CLERK, CASHIER};

int numCustomers, numAppClerks, numPicClerks, numPassportClerks, numCashiers;
int* customerMoney;
int clerkMoney[] = {0, 0, 0, 0};
bool** doneWithClerk;

void PassportOffice();
void runApplicationClerk();
void runCustomer();
void printMenu();

class ApplicationClerk : public Thread {
public:
    int appClerkID, lineLength, bribeLineLength;
    ClerkState state;
    Lock *clerkLock, *lineLock, *bribeLineLock;
    Condition *clerkCV, *lineCV, *bribeLineCV, *breakCV;

    ApplicationClerk(int id, char* debugName) : Thread(debugName) {
        appClerkID = id;
        state = BUSY;
        lineLength = 0;
        bribeLineLength = 0;
        clerkLock = new Lock("appClerkLock");
        lineLock = new Lock("appClerkLineLock");
        bribeLineLock = new Lock("appClerkBribeLineLock");
        clerkCV = new Condition("appClerkCV");
        lineCV = new Condition("appClerkLineCV");
        bribeLineCV = new Condition("appClerkBribeLineCV");
        breakCV = new Condition("appClerkBreakCV");
    }
};

ApplicationClerk** applicationClerks;
//PictureClerk** pictureClerks;
//PassportClerk** passportClerks;
//Cashier** cashiers;

void runApplicationClerk(int id) {
    cout << currentThread->getName() << " currently running." << endl;
    int breakCounter = 0;
    while(true) {
        if(applicationClerks[id]->bribeLineLength > 0) {
            applicationClerks[id]->bribeLineLock->Acquire();
            applicationClerks[id]->bribeLineCV->Signal(applicationClerks[id]->bribeLineLock);     // Signal Customer to exit line
            applicationClerks[id]->clerkLock->Acquire(); //
            applicationClerks[id]->bribeLineLock->Release();
            applicationClerks[id]->clerkCV->Wait(applicationClerks[id]->clerkLock);      // Wait for customer to turn in application
            int wait = rand() % ((100 - 20) + 1) + 20;
            for(int i = 0; i < wait; i++)               // Process application
                currentThread->Yield();
            applicationClerks[id]->clerkCV->Signal(applicationClerks[id]->clerkLock);    // Give complete application back
            applicationClerks[id]->clerkCV->Wait(applicationClerks[id]->clerkLock);      // Wait for Customer to accept application
            applicationClerks[id]->clerkLock->Release();
            applicationClerks[id]->state = FREE;
        } else if(applicationClerks[id]->lineLength > 0) {
            applicationClerks[id]->lineLock->Acquire();
            applicationClerks[id]->lineCV->Signal(applicationClerks[id]->lineLock);     // Signal Customer to exit line
            applicationClerks[id]->clerkLock->Acquire(); //
            applicationClerks[id]->lineLock->Release();
            applicationClerks[id]->clerkCV->Wait(applicationClerks[id]->clerkLock);      // Wait for customer to turn in application
            int wait = rand() % ((100 - 20) + 1) + 20;
            for(int i = 0; i < wait; i++)               // Process application
                currentThread->Yield();
            applicationClerks[id]->clerkCV->Signal(applicationClerks[id]->clerkLock);    // Give complete application back
            applicationClerks[id]->clerkCV->Wait(applicationClerks[id]->clerkLock);      // Wait for Customer to accept application
            applicationClerks[id]->clerkLock->Release();
            applicationClerks[id]->state = FREE;
        } else {
            breakCounter++;
        }
        if(breakCounter == 1000) {
            applicationClerks[id]->clerkLock->Acquire();
            applicationClerks[id]->state = BREAK;
            cout << currentThread->getName() << " taking a break." << endl;
            applicationClerks[id]->breakCV->Wait(applicationClerks[id]->clerkLock);
            cout << currentThread->getName() << " back from break." << endl;
            applicationClerks[id]->clerkLock->Release();
            applicationClerks[id]->state = FREE;
        }
    }
}

void getApplicationFiled() {
    // Choose ApplictionClerk with shortest line
    int myClerk = -1;
    int minLength = 50;
    for(int i = 0; i < numAppClerks; i++) {
        if(applicationClerks[i]->lineLength < minLength && applicationClerks[i]->state != BREAK) {
            myClerk = i;
            minLength = applicationClerks[i]->lineLength; 
        }
    }
    

    if(applicationClerks[myClerk]->state == BUSY) {    // Stand in line if ApplicationClerk is busy
        if(applicationClerks[myClerk]->lineLength > 0 && customerMoney[atoi(currentThread->getName())] > 500) {
            applicationClerks[myClerk]->bribeLineLock->Acquire();
            customerMoney[atoi(currentThread->getName())] = customerMoney[atoi(currentThread->getName())] - 500;
            clerkMoney[APPLICATION_CLERK] += 500;
            applicationClerks[myClerk]->bribeLineLength++;
            cout << "Customer " << currentThread->getName() << " bribed " << applicationClerks[myClerk]->getName();
            cout << ", currently waiting in bribeLine" << endl;
            applicationClerks[myClerk]->bribeLineCV->Wait(applicationClerks[myClerk]->bribeLineLock);
            applicationClerks[myClerk]->state = BUSY;           // Called by Application Clerk
            applicationClerks[myClerk]->bribeLineLength--;           // Leaving line
            applicationClerks[myClerk]->bribeLineLock->Release();
        } else {
            applicationClerks[myClerk]->lineLock->Acquire();
            applicationClerks[myClerk]->lineLength++;
            cout << "Customer " << currentThread->getName() << " waiting in line for " << applicationClerks[myClerk]->getName() << endl;
            applicationClerks[myClerk]->lineCV->Wait(applicationClerks[myClerk]->lineLock);
            applicationClerks[myClerk]->state = BUSY;           // Called by Application Clerk
            applicationClerks[myClerk]->lineLength--;           // Leaving line
            applicationClerks[myClerk]->lineLock->Release();
        }
    }
    
    // Interaction with clerk
    applicationClerks[myClerk]->clerkLock->Acquire();
    cout << "Customer " << currentThread->getName() << " currently with " << applicationClerks[myClerk]->getName() << endl;
    applicationClerks[myClerk]->clerkCV->Signal(applicationClerks[myClerk]->clerkLock); // Give incomplete application to Application Clerk
    applicationClerks[myClerk]->clerkCV->Wait(applicationClerks[myClerk]->clerkLock);   // Wait for Application Clerk
    applicationClerks[myClerk]->clerkCV->Signal(applicationClerks[myClerk]->clerkLock); // Accept completed application
    cout << "Customer " << currentThread->getName() << " finished with " << applicationClerks[myClerk]->getName() << endl;
    doneWithClerk[atoi(currentThread->getName())][APPLICATION_CLERK] = true;
    applicationClerks[myClerk]->clerkLock->Release();
}

void runCustomer() {
    cout << "Customer" << currentThread->getName() << " currently running." << endl;
    getApplicationFiled();
    /*
    // Decide whether this Customer is a senator
    // Randomly decide whether to go to AppClerk or PicClerk first
    if(ApplicationClerk first) {
        getApplicationFiled();
        getPictureTaken();
    } else {    // PictureClerk first
        getPictureTaken();
        getApplicationFiled();
    }
    // Get verified by PassportClerk
    // Pay for Passport at Cashier
    */
}

void PassportOffice() {
    // Create dyanmic array containing Customer money amounts
    customerMoney = new int[numCustomers];
    int amounts[] = {100, 600, 1100, 1600};
    for(int i = 0; i < numCustomers; i++) 
        customerMoney[i] = amounts[(int)(rand() % NUM_CLERKS)];

    // Create dynamic 2D array containg information concering whether a Customer is done with a type of Clerk
    doneWithClerk = new bool*[numCustomers];
    for(int i = 0; i < numCustomers; i++)
        doneWithClerk[i] = new bool[NUM_CLERKS];
    for(int i = 0; i < numCustomers; i++)
        for(int j = 0; j < NUM_CLERKS; j++)
            doneWithClerk[i][j] = false;
    
    // Create ApplicationClerks
    Thread* appClerk;
    char* name; 
    applicationClerks = new ApplicationClerk*[numAppClerks];
    for(int i = 0; i < numAppClerks; i++) {
        name = new char[20];
        sprintf(name, "ApplicationClerk%d", i);
        applicationClerks[i] = new ApplicationClerk(i, name);
        //applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);
    }

    // Create PictureClerks
    // Create PassportClerks
    // Create Cashiers
    
    // Create Customers
    Thread* customer;
    char* ssn;
    for(int i = 0; i < numCustomers; i++) {
        ssn = new char[20];
        //sprintf(ssn, "Customer%d", i);
        sprintf(ssn, "%d", i);
        customer = new Thread(ssn);
        customer->Fork((VoidFunctionPtr)runCustomer, 0);
    }

    for(int i = 0; i < numAppClerks; i++)
        applicationClerks[i]->Fork((VoidFunctionPtr)runApplicationClerk, i);

}

void Problem2() {
    // Default values for Customers and Clerks
    numCustomers = 10;
    numAppClerks = 2;
    numPicClerks = 5;
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