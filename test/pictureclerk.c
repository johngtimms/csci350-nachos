#include "syscall.h"
#include "setup.h"

int i;

void runPictureClerk() {
	int wait, k, myCustomer;
	Print("Running PictureClerk: %i\n",i);
	while(true) {
        if(GetMV(pictureClerk.senatorLineLength, i) > 0) {
    		Acquire(pictureClerk.senatorLineLock, i);
            Signal(pictureClerk.senatorLineCV, i, pictureClerk.senatorLineLock, i);     /* Signal Senator to exit line */
            Print("PictureClerk %i has signalled a Senator to come to their counter\n", i);
            Acquire(pictureClerk.clerkLock, i); 
            Release(pictureClerk.senatorLineLock, i);
            Wait(pictureClerk.clerkCV, i, pictureClerk.clerkLock, i);      /* Wait for Senator to get ready for picture */
            myCustomer = GetMV(pictureClerk.customerID, i);
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Senator %i\n", myCustomer);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Senator %i\n", myCustomer);
            Signal(pictureClerk.clerkCV, i, pictureClerk.clerkLock, i);    /* Give picture back */
            Wait(pictureClerk.clerkCV, i, pictureClerk.clerkLock, i);      /* Wait for Senator to accept picture */
            if(GetMV(customer.likedPic, myCustomer)) {
            	Print("PictureClerk %i ",i);
            	Print("has been told that Senator %i does like their picture\n", myCustomer);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            } else {
            	Print("PictureClerk %i ",i);
            	Print("has been told that Senator %i does not like their picture\n", myCustomer);
            }
            SetMV(pictureClerk.customerID, i, -1);
            Release(pictureClerk.clerkLock, i);
            SetMV(pictureClerk.state, i, FREE);
        } else if(GetMV(pictureClerk.bribeLineLength, i) > 0 && !GetMV(senatorInside, -1) && !GetMV(senatorsOutside, -1)) {
            Acquire(pictureClerk.bribeLineLock, i);
            Signal(pictureClerk.bribeLineCV, i, pictureClerk.bribeLineLock, i);     /* Signal Customer to exit line */
            Print("PictureClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(pictureClerk.clerkLock, i); 
            Release(pictureClerk.bribeLineLock, i);
            Wait(pictureClerk.clerkCV, i, pictureClerk.clerkLock, i);      /* Wait for customer to get ready for picture */
            myCustomer = GetMV(pictureClerk.customerID, i);
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Customer %i\n", myCustomer);
            Signal(pictureClerk.clerkCV, i, pictureClerk.clerkLock, i);    /* Give picture back */
            Wait(pictureClerk.clerkCV, i, pictureClerk.clerkLock, i);      /* Wait for Customer to accept picture */
            if(GetMV(customer.likedPic, myCustomer)) {
                Print("PictureClerk %i ",i);
            	Print("has been told that Customer %i does like their picture\n", myCustomer);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            } else {
            	Print("PictureClerk %i ", i);
            	Print("has been told that Customer %i does not like their picture\n", myCustomer);
            }
            SetMV(pictureClerk.customerID, i, -1);
            Release(pictureClerk.clerkLock, i);
            SetMV(pictureClerk.state, i, FREE);
        } else if(GetMV(pictureClerk.lineLength, i) > 0 && !GetMV(senatorInside, -1) && !GetMV(senatorsOutside, -1)) {
            Acquire(pictureClerk.lineLock, i);
            Signal(pictureClerk.lineCV, i, pictureClerk.lineLock, i);     /* Signal Customer to exit line */
            Print("PictureClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(pictureClerk.clerkLock, i); 
            Release(pictureClerk.lineLock, i);
            Wait(pictureClerk.clerkCV, i, pictureClerk.clerkLock, i);      /* Wait for customer to get ready for picture */
            myCustomer = GetMV(pictureClerk.customerID, i);
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Customer %i\n", myCustomer);
            Signal(pictureClerk.clerkCV, i, pictureClerk.clerkLock, i);    /* Give picture back */
            Wait(pictureClerk.clerkCV, i, pictureClerk.clerkLock, i);      /* Wait for Customer to accept picture */
            if(GetMV(customer.likedPic, myCustomer)) {
                Print("PictureClerk %i ",i);
            	Print("has been told that Customer %i does like their picture\n", myCustomer);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            } else {
            	Print("PictureClerk %i ", i);
            	Print("has been told that Customer %i does not like their picture\n", myCustomer);
            }
            SetMV(pictureClerk.customerID, i, -1);
            Release(pictureClerk.clerkLock, i);
            SetMV(pictureClerk.state, i, FREE);
        } else {
            Acquire(pictureClerk.clerkLock, i);
            SetMV(pictureClerk.state, i, BREAK);
            Print("PictureClerk %i is going on break\n", i);
            Wait(pictureClerk.breakCV, i, pictureClerk.clerkLock, i);
            Print("PictureClerk %i is coming off break\n", i);
            Release(pictureClerk.clerk, i);
            SetMV(pictureClerk.state, i, FREE);
            if(GetMV(timeToLeave, -1)) {
                Print("PictureClerk %i is leaving the office.\n",i);
                Exit(0);
            }
            
        }
    }
}

int main() {
	Setup();
	Acquire(pictureClerk.indexLock, -1);
	i = GetMV(pictureClerk.index, -1);
    SetMV(pictureClerk.index, -1, i + 1);
	Release(pictureClerk.indexLock, -1);
	runPictureClerk();
	Exit(0);
}