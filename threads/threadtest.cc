#include "copyright.h"
#include "system.h"

//typedef enum {AVAILABLE, BUSY, BREAK} clerkState;

struct ApplicationClerk 
{
    // wallet
    // queue
    // bribeQueue
    // state   

};

void fileApplication()
{

}

void getPictureTaken() 
{

}

void PostOffice(int numCustomers, int numAppClerks, int numPicClerks, int numPassportClerks, int numCashiers) 
{
    // Create Clerks first

    /*
    Thread *customer;
    char *ssn;
    for(int i = 0; i < 5; i++) {
        ssn = new char[20];
        sprintf(ssn,"%d", i);
        customer = new Thread(ssn);
        // Customer randomly decides whether to file application or get picture taken
        customer->Fork((VoidFunctionPtr)fileApplication, 0);
        // or
        customer->Fork((VoidFunctionPtr)getPictureTaken, 0);
    }
    */
}

// Run menu for Part 2 of assignment
void Problem2() 
{
    // Default values for Customers and Clerks
    int numCustomers = 20;
    int numAppClerks = 5;
    int numPicClerks = 5;
    int numPassportClerks = 5;
    int numCashiers = 5;

    printf("Welcome to the Post Office\n");
    printf("Please enter an option:\n");
    printf("'a' - View default values\n");
    printf("'b' - Change default values\n");
    printf("'c' - View default values\n");


    
    
}
