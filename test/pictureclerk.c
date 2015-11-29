#include "syscall.h"
#include "setup.h"

int i;

void runPictureClerk() {
	int wait, k, myCustomer;
	
	Print("Running PictureClerk: %i\n",i);
	
	while(true) {
    	if(pictureClerks[i].senatorLineLength > 0) {
    		Acquire(pictureClerks[i].senatorLineLock);
            Signal(pictureClerks[i].senatorLineCV,pictureClerks[i].senatorLineLock);     /* Signal Senator to exit line */
            Print("PictureClerk %i has signalled a Senator to come to their counter\n", i);
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].senatorLineLock);
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for Senator to get ready for picture */
            myCustomer = pictureClerks[i].customerID;
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i ", pictureClerks[i].customerID);
            Print("from Senator %i\n", pictureClerks[i].customerID);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Senator %i\n", pictureClerks[i].customerID);
            Signal(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);    /* Give picture back */
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for Senator to accept picture */
            if(customers[myCustomer].likedPic) {
            	Print("PictureClerk %i ",i);
            	Print("has been told that Senator %i does like their picture\n", pictureClerks[i].customerID);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            }else{
            	Print("PictureClerk %i ",i);
            	Print("has been told that Senator %i does not like their picture\n", pictureClerks[i].customerID);
            }
            pictureClerks[i].customerID = -1; 
            Release(pictureClerks[i].clerkLock);
            pictureClerks[i].state = FREE;
    	} else if(pictureClerks[i].bribeLineLength > 0) {
            Acquire(pictureClerks[i].bribeLineLock);
            Signal(pictureClerks[i].bribeLineCV, pictureClerks[i].bribeLineLock);     /* Signal Customer to exit line */
            Print("PictureClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].bribeLineLock);
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for customer to get ready for picture */
            myCustomer = pictureClerks[i].customerID;
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i ", pictureClerks[i].customerID);
            Print("from Customer %i\n", pictureClerks[i].customerID);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Customer %i\n", pictureClerks[i].customerID);
            Signal(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);    /* Give picture back */
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for Customer to accept picture */
            if(customers[myCustomer].likedPic) {
                Print("PictureClerk %i ",i);
            	Print("has been told that Customer %i does like their picture\n", pictureClerks[i].customerID);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            }else{
            	Print("PictureClerk %i ", i);
            	Print("has been told that Customer %i does not like their picture\n", pictureClerks[i].customerID);
                }
            pictureClerks[i].customerID = -1; 
            Release(pictureClerks[i].clerkLock);
            pictureClerks[i].state = FREE;
        } else if(pictureClerks[i].lineLength > 0) {
            Acquire(pictureClerks[i].lineLock);
            Signal(pictureClerks[i].lineCV, pictureClerks[i].lineLock);     /* Signal Customer to exit line */
            Print("PictureClerk %i has signalled a Customer to come to their counter\n", i);
            Acquire(pictureClerks[i].clerkLock); 
            Release(pictureClerks[i].lineLock);
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for customer to get ready for picture */
            myCustomer = pictureClerks[i].customerID;
            Print("PictureClerk %i ", i);
            Print("has recieved SSN %i ", pictureClerks[i].customerID);
            Print("from Customer %i\n", pictureClerks[i].customerID);
            Print("PictureClerk %i ", i);
            Print("has taken a picture of Customer %i\n", pictureClerks[i].customerID);
            Signal(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);    /* Give picture back */
            Wait(pictureClerks[i].clerkCV, pictureClerks[i].clerkLock);      /* Wait for Customer to accept picture */
            if(customers[myCustomer].likedPic) {
                Print("PictureClerk %i ",i);
            	Print("has been told that Customer %i does like their picture\n", pictureClerks[i].customerID);
                wait = Rand() % ((100 - 20) + 1) + 20;
                for(k = 0; k < wait; k++)               /* Process picture */
                    Yield();
            }else{
            	Print("PictureClerk %i ", i);
            	Print("has been told that Customer %i does not like their picture\n", pictureClerks[i].customerID);
                }
            pictureClerks[i].customerID = -1; 
            Release(pictureClerks[i].clerkLock);
            pictureClerks[i].state = FREE;
        } else {
            Acquire(pictureClerks[i].clerkLock);
            pictureClerks[i].state = BREAK;
            Print("PictureClerk %i is going on break\n", i);
            Wait(pictureClerks[i].breakCV, pictureClerks[i].clerkLock);
            Print("PictureClerk %i is coming off break\n",i);
            Release(pictureClerks[i].clerkLock);
            pictureClerks[i].state = FREE;
        }
    }
}

int main() {
	setup();
	Acquire(pictureClerkIndexLock);
	i = GetMV(nextAvailablePictureClerkIndex);
    SetMV(nextAvailablePictureClerkIndex, i + 1);
	Release(pictureClerkIndexLock);

	runPictureClerk();
	Exit(0);
}