#include "syscall.h"
#include "setup.h"

int i;

void runApplicationClerk() {
	int myCustomer;
	Print("Running ApplicationClerk: %i\n",i);
	while(true) {
    	/*if(applicationClerk.senatorLineLength > 0) {*/
    	if(GetMV(applicationClerk.senatorLineLength, i) > 0) {
    		Acquire(applicationClerk.senatorLineLock, i);
            Signal(applicationClerk.senatorLineCV, i, applicationClerk.senatorLineLock, i);     /* Signal Customer to exit line */
            Print("ApplicationClerk %i has signalled a Senator to come to their counter\n", i);
            Acquire(applicationClerk.clerkLock, i); 
            Release(applicationClerk.senatorLineLock, i);
            Wait(applicationClerk.clerkCV, i, applicationClerk.clerkLock, i);      /* Wait for customer to give application */
            /*myCustomer = applicationClerk.customerID;*/
            myCustomer = GetMV(applicationClerk.customerID, i);
            Print("ApplicationClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Senator %i\n", myCustomer);
            Signal(applicationClerk.clerkCV, i, applicationClerk.clerkLock, i);    /* Process application */
            Print("ApplicationClerk %i ", i);
            Print("has recorded a completed application from Senator %i\n", myCustomer);
            Wait(applicationClerk.clerkCV, i, applicationClerk.clerkLock, i);      /* Wait for Customer to accept completed application */
            /*applicationClerk.customerID = -1;*/ 
            SetMV(applicationClerk.customerID, i, -1);
            Release(applicationClerk.clerkLock, i);
            /*applicationClerk.state = FREE;*/
            SetMV(applicationClerk.state, i, FREE);
    	/*} else if(applicationClerk.bribeLineLength > 0) {*/
    	} else if(GetMV(applicationClerk.bribeLineLength, i) > 0 && !GetMV(senatorInside, -1) && !GetMV(senatorsOutside, -1)) { /*never signal customers if a senator is inside*/
            Acquire(applicationClerk.bribeLineLock, i);
            Signal(applicationClerk.bribeLineCV, i, applicationClerk.bribeLineLock, i);     /* Signal Customer to exit line */
            Print("ApplicationClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(applicationClerk.clerkLock, i); 
            Release(applicationClerk.bribeLineLock, i);
            Wait(applicationClerk.clerkCV, i, applicationClerk.clerkLock, i);      /* Wait for customer to give application */
            /*myCustomer = applicationClerk.customerID;*/
            myCustomer = GetMV(applicationClerk.customerID, i);
            Print("ApplicationClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            Signal(applicationClerk.clerkCV, i, applicationClerk.clerkLock, i);    /* Process application */
            Print("ApplicationClerk %i ", i);
            Print("has accepted application from Customer %i\n", myCustomer);
            Wait(applicationClerk.clerkCV, i, applicationClerk.clerkLock, i);      /* Wait for Customer to accept completed application */
            /*applicationClerk.customerID = -1;  */
            SetMV(applicationClerk.customerID, i, -1);  
            Release(applicationClerk.clerkLock, i);
            /*applicationClerk.state = FREE;*/
            SetMV(applicationClerk.state, i, FREE);
        /*} else if(applicationClerk.lineLength > 0) {*/
        } else if(GetMV(applicationClerk.lineLength, i) > 0 && !GetMV(senatorInside, -1) && !GetMV(senatorsOutside, -1)) { /*never signal customers if a senator is inside*/
            Acquire(applicationClerk.lineLock, i);
            Signal(applicationClerk.lineCV, i, applicationClerk.lineLock, i);     /* Signal Customer to exit line */
            Print("ApplicationClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(applicationClerk.clerkLock, i); 
            Release(applicationClerk.lineLock, i);
            Wait(applicationClerk.clerkCV, i, applicationClerk.clerkLock, i);      /* Wait for customer to gve application */
            /*myCustomer = applicationClerk.customerID;*/
            myCustomer = GetMV(applicationClerk.customerID, i);
            Print("ApplicationClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            Signal(applicationClerk.clerkCV, i, applicationClerk.clerkLock, i);    /* Process application */
            Print("ApplicationClerk %i ", i);
            Print("has accepted application from Customer %i\n", myCustomer);
            Wait(applicationClerk.clerkCV, i, applicationClerk.clerkLock, i);		/* Wait for Customer to accept completed application */
            /*applicationClerk.customerID = -1;*/ 
            SetMV(applicationClerk.customerID, i, -1); 
            Release(applicationClerk.clerkLock, i);
            /*applicationClerk.state = FREE;*/
            SetMV(applicationClerk.state, i, FREE);
        } else {
            Acquire(applicationClerk.clerkLock, i);
            /*applicationClerk.state = BREAK;*/
            SetMV(applicationClerk.state, i, BREAK);
            Print("ApplicationClerk %i is going on break\n", i);
            Wait(applicationClerk.breakCV, i, applicationClerk.clerkLock, i);
            Print("ApplicationClerk %i is coming off break\n", i);
            Release(applicationClerk.clerkLock, i);
            /*applicationClerk.state = FREE;*/
            SetMV(applicationClerk.state, i, FREE);
            if(GetMV(timeToLeave, -1)) {
                Print("ApplicationClerk %i is leaving the office.\n",i);
                Exit(0);
            }
        }
    }	
}

int main() {
    Setup();

    Acquire(applicationClerkIndexLock, -1);
	i = GetMV(nextAvailableApplicationClerkIndex, -1);
    SetMV(nextAvailableApplicationClerkIndex, -1, i + 1);
	Release(applicationClerkIndexLock, -1);
	runApplicationClerk();
    
	Exit(0);
}










