#include "syscall.h"
#include "setup.h"

int i;

void runApplicationClerk() {
	int myCustomer;
	Print("Running ApplicationClerk: %i\n",i);
	while(true) {
    	/*if(applicationClerks[i].senatorLineLength > 0) {*/
    	if(GetMV(applicationClerks[i].senatorLineLength) > 0) {
    		Acquire(applicationClerks[i].senatorLineLock);
            Signal(applicationClerks[i].senatorLineCV, applicationClerks[i].senatorLineLock);     /* Signal Customer to exit line */
            Print("ApplicationClerk %i has signalled a Senator to come to their counter\n", i);
            Acquire(applicationClerks[i].clerkLock); 
            Release(applicationClerks[i].senatorLineLock);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for customer to give application */
            /*myCustomer = applicationClerks[i].customerID;*/
            myCustomer = GetMV(applicationClerks[i].customerID);
            Print("ApplicationClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Senator %i\n", myCustomer);
            Signal(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);    /* Process application */
            Print("ApplicationClerk %i ", i);
            Print("has recorded a completed application from Senator %i\n", myCustomer);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for Customer to accept completed application */
            /*applicationClerks[i].customerID = -1;*/ 
            SetMV(applicationClerks[i].customerID, -1);
            Release(applicationClerks[i].clerkLock);
            /*applicationClerks[i].state = FREE;*/
            SetMV(applicationClerks[i].state, FREE);
    	/*} else if(applicationClerks[i].bribeLineLength > 0) {*/
    	} else if(GetMV(applicationClerks[i].bribeLineLength) > 0) {
            Acquire(applicationClerks[i].bribeLineLock);
            Signal(applicationClerks[i].bribeLineCV, applicationClerks[i].bribeLineLock);     /* Signal Customer to exit line */
            Print("ApplicationClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(applicationClerks[i].clerkLock); 
            Release(applicationClerks[i].bribeLineLock);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for customer to give application */
            /*myCustomer = applicationClerks[i].customerID;*/
            myCustomer = GetMV(applicationClerks[i].customerID);
            Print("ApplicationClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            Signal(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);    /* Process application */
            Print("ApplicationClerk %i ", i);
            Print("has accepted application from Customer %i\n", myCustomer);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for Customer to accept completed application */
            /*applicationClerks[i].customerID = -1;  */
            SetMV(applicationClerks[i].customerID, -1);  
            Release(applicationClerks[i].clerkLock);
            /*applicationClerks[i].state = FREE;*/
            SetMV(applicationClerks[i].state, FREE);
        /*} else if(applicationClerks[i].lineLength > 0) {*/
        } else if(GetMV(applicationClerks[i].lineLength) > 0) {
            Acquire(applicationClerks[i].lineLock);
            Signal(applicationClerks[i].lineCV, applicationClerks[i].lineLock);     /* Signal Customer to exit line */
            Print("ApplicationClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(applicationClerks[i].clerkLock); 
            Release(applicationClerks[i].lineLock);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);      /* Wait for customer to gve application */
            /*myCustomer = applicationClerks[i].customerID;*/
            myCustomer = GetMV(applicationClerks[i].customerID);
            Print("ApplicationClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            Signal(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);    /* Process application */
            Print("ApplicationClerk %i ", i);
            Print("has accepted application from Customer %i\n", myCustomer);
            Wait(applicationClerks[i].clerkCV, applicationClerks[i].clerkLock);		/* Wait for Customer to accept completed application */
            /*applicationClerks[i].customerID = -1;*/ 
            SetMV(applicationClerks[i].customerID, -1); 
            Release(applicationClerks[i].clerkLock);
            /*applicationClerks[i].state = FREE;*/
            SetMV(applicationClerks[i].state, FREE);
        } else {
            Acquire(applicationClerks[i].clerkLock);
            /*applicationClerks[i].state = BREAK;*/
            SetMV(applicationClerks[i].state, BREAK);
            Print("ApplicationClerk %i is going on break\n", i);
            Wait(applicationClerks[i].breakCV, applicationClerks[i].clerkLock);
            Print("ApplicationClerk %i is coming off break\n", i);
            Release(applicationClerks[i].clerkLock);
            /*applicationClerks[i].state = FREE;*/
            SetMV(applicationClerks[i].state, FREE);
        }
    }	
}

int main() {
    Print("applicationclerk.c execed\n",0);
    setup();
    Print("after setup in applicationclerk.c \n",0);
	Acquire(applicationClerkIndexLock);
	i = GetMV(nextAvailableApplicationClerkIndex);
    SetMV(nextAvailableApplicationClerkIndex, i + 1);
	Release(applicationClerkIndexLock);
	/*initClerk(APPLICATION_CLERK, i);*/
	runApplicationClerk();
	Exit(0);
}










