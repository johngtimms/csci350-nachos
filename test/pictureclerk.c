#include "syscall.h"
#include "setup.h"

int i;

void runPictureClerk() {
	int wait, k, myCustomer;
	Print("Running PictureClerk: %i\n",i);
	while(true) {
    	/*if(pictureClerks[i].senatorLineLength > 0) {*/
        if(GetMV(pictureClerks[i].senatorLineLength) > 0) {
    		Acquire(pictureClerks[i].senatorLineLock);
            Signal(pictureClerks[i].senatorLineCV,pictureClerks[i].senatorLineLock);     /* Signal Senator to exit line */
            Print("PictureClerk %i has signalled a Senator to come to their counter\n", i);
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].senatorLineLock);
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for Senator to get ready for picture */
            /*myCustomer = pictureClerks[i].customerID;*/
            myCustomer = GetMV(pictureClerks[i].customerID);
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Senator %i\n", myCustomer);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Senator %i\n", myCustomer);
            Signal(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);    /* Give picture back */
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for Senator to accept picture */
            /*if(customers[myCustomer].likedPic) {*/
            if(GetMV(customers[myCustomer].likedPic)) {
            	Print("PictureClerk %i ",i);
            	Print("has been told that Senator %i does like their picture\n", myCustomer);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            } else {
            	Print("PictureClerk %i ",i);
            	Print("has been told that Senator %i does not like their picture\n", myCustomer);
            }
            /*pictureClerks[i].customerID = -1;*/
            SetMV(pictureClerks[i].customerID, -1);
            Release(pictureClerks[i].clerkLock);
            /*pictureClerks[i].state = FREE;*/
            SetMV(pictureClerks[i].state, FREE);
    	/*} else if(pictureClerks[i].bribeLineLength > 0) {*/
        } else if(GetMV(pictureClerks[i].bribeLineLength) > 0 && !GetMV(senatorInside) && !GetMV(senatorsOutside)) {
            Acquire(pictureClerks[i].bribeLineLock);
            Signal(pictureClerks[i].bribeLineCV, pictureClerks[i].bribeLineLock);     /* Signal Customer to exit line */
            Print("PictureClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].bribeLineLock);
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for customer to get ready for picture */
            /*myCustomer = pictureClerks[i].customerID;*/
            myCustomer = GetMV(pictureClerks[i].customerID);
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Customer %i\n", myCustomer);
            Signal(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);    /* Give picture back */
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for Customer to accept picture */
            /*if(customers[myCustomer].likedPic) {*/
            if(GetMV(customers[myCustomer].likedPic)) {
                Print("PictureClerk %i ",i);
            	Print("has been told that Customer %i does like their picture\n", myCustomer);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            } else {
            	Print("PictureClerk %i ", i);
            	Print("has been told that Customer %i does not like their picture\n", myCustomer);
            }
            /*pictureClerks[i].customerID = -1;*/
            SetMV(pictureClerks[i].customerID, -1);
            Release(pictureClerks[i].clerkLock);
            /*pictureClerks[i].state = FREE;*/
            SetMV(pictureClerks[i].state, FREE);
        /*} else if(pictureClerks[i].lineLength > 0) {*/
        } else if(GetMV(pictureClerks[i].lineLength) > 0 && !GetMV(senatorInside) && !GetMV(senatorsOutside)) {
            Acquire(pictureClerks[i].lineLock);
            Signal(pictureClerks[i].lineCV, pictureClerks[i].lineLock);     /* Signal Customer to exit line */
            Print("PictureClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].lineLock);
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for customer to get ready for picture */
            /*myCustomer = pictureClerks[i].customerID;*/
            myCustomer = GetMV(pictureClerks[i].customerID);
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Customer %i\n", myCustomer);
            Signal(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);    /* Give picture back */
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for Customer to accept picture */
            /*if(customers[myCustomer].likedPic) {*/
            if(GetMV(customers[myCustomer].likedPic)) {
                Print("PictureClerk %i ",i);
            	Print("has been told that Customer %i does like their picture\n", myCustomer);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            } else {
            	Print("PictureClerk %i ", i);
            	Print("has been told that Customer %i does not like their picture\n", myCustomer);
            }
            /*pictureClerks[i].customerID = -1;*/
            SetMV(pictureClerks[i].customerID, -1);
            Release(pictureClerks[i].clerkLock);
            /*pictureClerks[i].state = FREE;*/
            SetMV(pictureClerks[i].state, FREE);
        } else {
            Acquire(pictureClerks[i].clerkLock);
            /*pictureClerks[i].state = BREAK;*/
            SetMV(pictureClerks[i].state, BREAK);
            Print("PictureClerk %i is going on break\n", i);
            Wait(pictureClerks[i].breakCV, pictureClerks[i].clerkLock);
            Print("PictureClerk %i is coming off break\n",i);
            Release(pictureClerks[i].clerkLock);
            /*pictureClerks[i].state = FREE;*/
            SetMV(pictureClerks[i].state, FREE);
            if(GetMV(timeToLeave)) {
                Print("PictureClerk %i is leaving the office.\n",i);
                Exit(0);
            }
            
        }
    }
}

int main() {
	Setup();

	Acquire(pictureClerkIndexLock);
	i = GetMV(nextAvailablePictureClerkIndex);
    SetMV(nextAvailablePictureClerkIndex, i + 1);
	Release(pictureClerkIndexLock);

	runPictureClerk();
    
	Exit(0);
}