#include "syscall.h"
#include "setup.h"

int i;

void runPassportClerk() {
	int wait, k, myCustomer;
	Print("Running PassportClerk: %i\n", i);
	while(true) {
        if(GetMV(passportClerk.senatorLineLength, i) > 0) {
    		Acquire(passportClerk.senatorLineLock, i);
            Signal(passportClerk.senatorLineCV, i, passportClerk.senatorLineLock, i);     /* Signal Senator to exit line */
            Print("PassportClerk %i has signalled a Senator to come to their counter\n", i);
            Acquire(passportClerk.clerkLock, i); 
            Release(passportClerk.senatorLineLock, i);
            Wait(passportClerk.clerkCV, i, passportClerk.clerkLock, i);      /* Wait for Senator to hand over picture and application */
            myCustomer = GetMV(passportClerk.customerID, i);
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Senator %i\n", myCustomer);
            if(GetMV(customer.hasApp, myCustomer) && GetMV(customer.hasPic, myCustomer)) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i has both their application and picture completed\n", myCustomer);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++)            /* Process application and picture */
                    Yield();
				Print("PassportClerk %i has recorded ", i);
            	Print("Senator %i's passport documentation\n", myCustomer);
                SetMV(customer.certifiedByPassportClerk, myCustomer, true);
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Senator %i does not have both their application and picture completed\n", myCustomer);
            }
            Signal(passportClerk.clerkCV, i, passportClerk.clerkLock, i);    /* Give Senator a passport */
            Wait(passportClerk.clerkCV, i, passportClerk.clerkLock, i);      /* Wait for Senator to accept passport */
            SetMV(passportClerk.customerID, i, -1);
            Release(passportClerk.clerkLock, i);
            SetMV(passportClerk.state, i, FREE);
    	} else if(GetMV(passportClerk.bribeLineLength, i) > 0 && !GetMV(senatorInside, -1) && !GetMV(senatorsOutside, -1)) {
            Acquire(passportClerk.bribeLineLock, i);
            Signal(passportClerk.bribeLineCV, i, passportClerk.bribeLineLock, i);     /* Signal Customer to exit line */
            Print("PassportClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(passportClerk.clerkLock, i); 
            Release(passportClerk.bribeLineLock, i);
            Wait(passportClerk.clerkCV, i, passportClerk.clerkLock, i);       /* Wait for Customer to hand over picture and application */
            myCustomer = GetMV(passportClerk.customerID, i);  
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            if(GetMV(customer.hasApp, myCustomer) && GetMV(customer.hasPic, myCustomer)) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i has both their application and picture completed\n", myCustomer);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++)            /* Process application and picture */
                    Yield();
				Print("PassportClerk %i has recorded ", i);
            	Print("Customer %i's passport documentation\n", myCustomer);
                SetMV(customer.certifiedByPassportClerk, myCustomer, true);
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i does not have both their application and picture completed\n", myCustomer);
            }
            Signal(passportClerk.clerkCV, i, passportClerk.clerkLock, i);    /* Give Customer a passport */
            Wait(passportClerk.clerkCV, i, passportClerk.clerkLock, i);      /* Wait for Customer to accept passport */
            SetMV(passportClerk.customerID, i, -1);
            Release(passportClerk.clerkLock, i);
            SetMV(passportClerk.state, i, FREE);
        } else if(GetMV(passportClerk.lineLength, i) > 0 && !GetMV(senatorInside, -1) && !GetMV(senatorsOutside, -1)) {
            Acquire(passportClerk.lineLock, i);
            Signal(passportClerk.lineCV, i, passportClerk.lineLock, i);     /* Signal Customer to exit line */
            Print("PassportClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(passportClerk.clerkLock, i); 
            Release(passportClerk.lineLock, i);
            Wait(passportClerk.clerkCV, i, passportClerk.clerkLock, i);      /* Wait for Customer to hand over picture and application */
            myCustomer = GetMV(passportClerk.customerID, i);  
            Print("PassportClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            if(GetMV(customer.hasApp, myCustomer) && GetMV(customer.hasPic, myCustomer)) {
                Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i has both their application and picture completed\n", myCustomer);
            	wait = Rand() % ((100 - 20) + 1) + 20; 
                for(k = 0; k < wait; k++)            /* Process application and picture */
                    Yield();
				Print("PassportClerk %i has recorded ", i);
            	Print("Customer %i's passport documentation\n", myCustomer);
                /*customers[myCustomer].certifiedByPassportClerk = true;*/
                SetMV(customer.certifiedByPassportClerk, myCustomer, true);
            } else {
            	Print("PassportClerk %i has determined that ", i);
            	Print("Customer %i does not have both their application and picture completed\n", myCustomer);
            }
            Signal(passportClerk.clerkCV, i, passportClerk.clerkLock, i);    /* Give Customer a passport */
            Wait(passportClerk.clerkCV, i, passportClerk.clerkLock, i);      /* Wait for Customer to accept passport */
            SetMV(passportClerk.customerID, i, -1);
            Release(passportClerk.clerkLock, i);
            SetMV(passportClerk.state, i, FREE);
        } else {
            Acquire(passportClerk.clerkLock, i);
            SetMV(passportClerk.state, i, BREAK);
            Print("PassportClerk %i is going on break\n", i);
            Wait(passportClerk.breakCV, i, passportClerk.clerkLock, i);
            Print("PassportClerk %i is coming off break\n", i);
            Release(passportClerk.clerkLock, i);
            SetMV(passportClerks.state, i, FREE);
            if(GetMV(timeToLeave, -1)) {
                Print("PassportClerk %i is leaving the office.\n",i);
                Exit(0);
            }
        }
    }
}

int main() {
	Setup();

	Acquire(passportClerk.indexLock, -1);
	i = GetMV(passportClerk.index, -1);
    SetMV(passportClerk.index, -1, i + 1);
	Release(passportClerk.indexLock, -1);
	runPassportClerk();
    
	Exit(0);
}