#include "copyright.h"
#include "system.h"

typedef enum {AVAILABLE, BUSY, BREAK} clerkState;

struct ApplicationClerk 
{
    

};

void fileApplication()
{

}

void getPictureTaken() 
{

}

void PostOffice(int num) 
{
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
}

// Run menu for Part 2 of assignment
void Problem2() 
{
    
    
}
