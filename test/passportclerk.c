#include "syscall.h"
#include "setup.h"

int i;

void runPassportClerk() {
	int wait, k, myCustomer;
	
	Print("Running PassportClerk: %i\n", i);
	
	while(true) {
    	/*if(passportClerks[i].senatorLineLength > 0) {*/
        if(GetMV(passportClerks[i].senatorLineLength) > 0) {
    		Acquire(passportClerks[i].senatorLineLock);
            Signal(passportClerks[i].senatorLineCV, passportClerks[i].senatorLineLock);     /* Signal Senator to exit line */
            Print("PassportClerk %i has signalled a Senator to come to their counter\n", i);
            Acquire(passportClerks[i].clerkLock); 
            Release(passportClerks[i].senatorLineLock);
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Senator to hand over picture and application */
            /*myCustomer = passportClerks[i].customerID;*/
            myCustomer = GetMV(passportClerks[i].customerID);
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Senator %i\n", myCustomer);
            /*if(customers[myCustomer].hasApp && customers[myCustomer].hasPic) {*/
            if(GetMV(customers[myCustomer].hasApp) && GetMV(customers[myCustomer].hasPic)) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i has both their application and picture completed\n", myCustomer);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++)            /* Process application and picture */
                    Yield();
				Print("PassportClerk %i has recorded ", i);
            	Print("Senator %i's passport documentation\n", myCustomer);
                /*customers[myCustomer].certifiedByPassportClerk = true;*/
                SetMV(customers[myCustomer].certifiedByPassportClerk, true);
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i does not have both their application and picture completed\n", myCustomer);
            }
            Signal(passportClerks[i].clerkCV, passportClerks[i].clerkLock);    /* Give Senator a passport */
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Senator to accept passport */
            /*passportClerks[i].customerID = -1;*/
            SetMV(passportClerks[i].customerID, -1);
            Release(passportClerks[i].clerkLock);
            /*passportClerks[i].state = FREE;*/
            SetMV(passportClerks[i].state, FREE);
        /*} else if(passportClerks[i].bribeLineLength > 0) {*/
    	} else if(GetMV(passportClerks[i].bribeLineLength) > 0) {
            Acquire(passportClerks[i].bribeLineLock);
            Signal(passportClerks[i].bribeLineCV, passportClerks[i].bribeLineLock);     /* Signal Customer to exit line */
            Print("PassportClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(passportClerks[i].clerkLock); 
            Release(passportClerks[i].bribeLineLock);
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);       /* Wait for Customer to hand over picture and application */
            /*myCustomer = passportClerks[i].customerID;*/  
            myCustomer = GetMV(passportClerks[i].customerID);  
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            /*if(customers[myCustomer].hasApp && customers[myCustomer].hasPic) {*/
            if(GetMV(customers[myCustomer].hasApp) && GetMV(customers[myCustomer].hasPic)) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i has both their application and picture completed\n", myCustomer);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++)            /* Process application and picture */
                    Yield();
				Print("PassportClerk %i has recorded ", i);
            	Print("Customer %i's passport documentation\n", myCustomer);
                /*customers[myCustomer].certifiedByPassportClerk = true;*/
                SetMV(customers[myCustomer].certifiedByPassportClerk, true);
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i does not have both their application and picture completed\n", myCustomer);
            }
            Signal(passportClerks[i].clerkCV, passportClerks[i].clerkLock);    /* Give Customer a passport */
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Customer to accept passport */
            /*passportClerks[i].customerID = -1;*/ 
            SetMV(passportClerks[i].customerID, -1);
            Release(passportClerks[i].clerkLock);
            /*passportClerks[i].state = FREE;*/
            SetMV(passportClerks[i].state, FREE);
        /*} else if(passportClerks[i].lineLength > 0) {*/
        } else if(GetMV(passportClerks[i].lineLength) > 0) {
            Acquire(passportClerks[i].lineLock);
            Signal(passportClerks[i].lineCV, passportClerks[i].lineLock);     /* Signal Customer to exit line */
            Print("PassportClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(passportClerks[i].clerkLock); 
            Release(passportClerks[i].lineLock);
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Customer to hand over picture and application */
            /*myCustomer = passportClerks[i].customerID;*/
            myCustomer = GetMV(passportClerks[i].customerID);  
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            /*if(customers[myCustomer].hasApp && customers[myCustomer].hasPic) {*/
            if(GetMV(customers[myCustomer].hasApp) && GetMV(customers[myCustomer].hasPic)) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i has both their application and picture completed\n", myCustomer);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++)            /* Process application and picture */
                    Yield();
				Print("PassportClerk %i has recorded ", i);
            	Print("Customer %i's passport documentation\n", myCustomer);
                /*customers[myCustomer].certifiedByPassportClerk = true;*/
                SetMV(customers[myCustomer].certifiedByPassportClerk, true);
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i does not have both their application and picture completed\n", myCustomer);
            }
            Signal(passportClerks[i].clerkCV, passportClerks[i].clerkLock);    /* Give Customer a passport */
            Wait(passportClerks[i].clerkCV, passportClerks[i].clerkLock);      /* Wait for Customer to accept passport */
            /*passportClerks[i].customerID = -1;*/
            SetMV(passportClerks[i].customerID, -1);
            Release(passportClerks[i].clerkLock);
            /*passportClerks[i].state = FREE;*/
            SetMV(passportClerks[i].state, FREE);
        } else {
            Acquire(passportClerks[i].clerkLock);
            /*passportClerks[i].state = BREAK;*/
            SetMV(passportClerks[i].state, BREAK);
            Print("PassportClerk %i is going on break\n", i);
            Wait(passportClerks[i].breakCV, passportClerks[i].clerkLock);
            Print("PassportClerk %i is coming off break\n", i);
            Release(passportClerks[i].clerkLock);
            /*passportClerks[i].state = FREE;*/
            SetMV(passportClerks[i].state, FREE);
            if(GetMV(timeToLeave)) {
                Print("PassportClerk %i is leaving the office.\n",i);
                Exit(0);
            }
        }
    }
}

int main() {
	setup();

	Acquire(passportClerkIndexLock);
	i = GetMV(nextAvailablePassportClerkIndex);
    SetMV(nextAvailablePassportClerkIndex, i + 1);
	Release(passportClerkIndexLock);
    /*initClerk(PASSPORT_CLERK, i);*/
	runPassportClerk();
    
	Exit(0);
}