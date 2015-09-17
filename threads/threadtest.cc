#include "copyright.h"
#include "system.h"
//#include "stdio.h"

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

    printf("Welcome to the Passport Office.\n");
    while(true) {
        printf("Please enter an option:\n");
        printf("'a' - View/Edit default values\n");
        printf("'b' - Run a test\n");
        printf("'c' - Exit\n");
        char ch;    // User input
        scanf(" %c", &ch);
        if(ch == 'a') {     // Print default values
            printf("Number of Customers: %d\n", numCustomers);
            printf("Number of Appication Clerks: %d\n", numAppClerks);
            printf("Number of Picture Clerks: %d\n", numPicClerks);
            printf("Number of Passport Clerks: %d\n", numPassportClerks);
            printf("Number of Cashiers: %d\n", numCashiers);
            printf("Please enter new values:\n");
            int num;
            printf("Number of Customers: ");
            scanf("%d", &num);
            numCustomers = num;
            printf("Number of Application Clerks: ");
            scanf("%d", &num);
            numAppClerks = num;
            printf("Number of Picture Clerks: ");
            scanf("%d", &num);
            numPicClerks = num;
            printf("Number of Passport Clerks: ");
            scanf("%d", &num);
            numPassportClerks = num;
            printf("Number of Cashiers: ");
            scanf("%d", &num);
            numCashiers = num;

            // Edit default values
        } else if(ch == 'b') {
            // Run a test
        } else if(ch == 'c') {
            printf("Exiting Passport Office.")
            break;
        } else
            printf("Invalid input. Please try again.\n");
    }
    
    
}
