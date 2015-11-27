#include "syscall.h"
#include "setup.h"

int i;

void runPassportClerk() {
	int wait, k, myCustomer;
	
	Print("Running PassportClerk: %i\n", i);
	
	while(true) {
    	if(passportClerks[i].senatorLineLength > 0) {
    		Acquire(passportClerks[i].senatorLineLock);
            Signal(passportClerks[i].senatorLineCV, passportClerks[i].senatorLineLock);     /* Signal Senator to exit line */
            Print("PassportClerk %i has signalled a Senator to come to their counter\n", i);
            Acquire(passportClerks[i].clerkLock); 
            Release(passportClerks[i].senatorLineLock);
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Senator to hand over picture and application */
            myCustomer = passportClerks[i].customerID;
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i ", customers[myCustomer].SSN);
            Print("from Senator %i\n", customers[myCustomer].SSN);
            if(customers[myCustomer].hasApp && customers[myCustomer].hasPic) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i has both their application and picture completed\n", customers[myCustomer].SSN);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++)            /* Process application and picture */
                    Yield();
				Print("PassportClerk %i has recorded ", i);
            	Print("Senator %i's passport documentation\n", customers[myCustomer].SSN);
                customers[myCustomer].certifiedByPassportClerk = true;
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i does not have both their application and picture completed\n", customers[myCustomer].SSN);
            }
            Signal(passportClerks[i].clerkCV, passportClerks[i].clerkLock);    /* Give Senator a passport */
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Senator to accept passport */
            passportClerks[i].customerID -1;  
            Release(passportClerks[i].clerkLock);
            passportClerks[i].state = FREE;
    	} else if(passportClerks[i].bribeLineLength > 0) {
            Acquire(passportClerks[i].bribeLineLock);
            Signal(passportClerks[i].bribeLineCV, passportClerks[i].bribeLineLock);     /* Signal Customer to exit line */
            Print("PassportClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(passportClerks[i].clerkLock); 
            Release(passportClerks[i].bribeLineLock);
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);       /* Wait for Customer to hand over picture and application */
            passportClerks[i].customerID = -1;  
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i ", customers[myCustomer].SSN);
            Print("from Customer %i\n", customers[myCustomer].SSN);
            if(customers[myCustomer].hasApp && customers[myCustomer].hasPic) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i has both their application and picture completed\n", customers[myCustomer].SSN);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++)            /* Process application and picture */
                    Yield();
				Print("PassportClerk %i has recorded ", i);
            	Print("Customer %i's passport documentation\n", customers[myCustomer].SSN);
                customers[myCustomer].certifiedByPassportClerk = true;
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i does not have both their application and picture completed\n", customers[myCustomer].SSN);
            }
            Signal(passportClerks[i].clerkCV, passportClerks[i].clerkLock);    /* Give Customer a passport */
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Customer to accept passport */
            passportClerks[i].customerID -1; 
            Release(passportClerks[i].clerkLock);
            passportClerks[i].state = FREE;
        } else if(passportClerks[i].lineLength > 0) {
            Acquire(passportClerks[i].lineLock);
            Signal(passportClerks[i].lineCV, passportClerks[i].lineLock);     /* Signal Customer to exit line */
            Print("PassportClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(passportClerks[i].clerkLock); 
            Release(passportClerks[i].lineLock);
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Customer to hand over picture and application */
            myCustomer = passportClerks[i].customerID;
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i ", customers[myCustomer].SSN);
            Print("from Customer %i\n", customers[myCustomer].SSN);
            if(customers[myCustomer].hasApp && customers[myCustomer].hasPic) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i has both their application and picture completed\n", customers[myCustomer].SSN);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++)            /* Process application and picture */
                    Yield();
				Print("PassportClerk %i has recorded ", i);
            	Print("Customer %i's passport documentation\n", customers[myCustomer].SSN);
                customers[myCustomer].certifiedByPassportClerk = true;
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i does not have both their application and picture completed\n", customers[myCustomer].SSN);
            }
            Signal(passportClerks[i].clerkCV, passportClerks[i].clerkLock);    /* Give Customer a passport */
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Customer to accept passport */
            passportClerks[i].customerID -1; 
            Release(passportClerks[i].clerkLock);
            passportClerks[i].state = FREE;
        } else {
            Acquire(passportClerks[i].clerkLock);
            passportClerks[i].state = BREAK;
            Print("PassportClerk %i is going on break\n", i);
            Wait(passportClerks[i].breakCV, passportClerks[i].clerkLock);
            Print("PassportClerk %i is coming off break\n", i);
            Release(passportClerks[i].clerkLock);
            passportClerks[i].state = FREE;
        }
    }
}

int main() {
	Acquire(passportClerkIndexLock);
	i = nextAvailablePassportClerkIndex;
	nextAvailablePassportClerkIndex = nextAvailablePassportClerkIndex + 1; /*temporary keeping track of index*/
	Release(passportClerkIndexLock);

	runPassportClerk();
	Exit(0);
}