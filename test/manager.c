#include "syscall.h"
#include "setup.h"

int customerIndexLock, applicationClerkIndexLock, pictureClerkIndexLock, passportClerkIndexLock, cashierIndexLock, senatorIndexLock;

void runManager() {
	bool allClerksOnBreak;
	int applicationClerkMoneyTotal, pictureClerkMoneyTotal, passportClerkMoneyTotal, cashierMoneyTotal, k;
	numCustomers = NUM_CUSTOMERS;
	numApplicationClerks = NUM_APPLICATION_CLERKS;
	numPictureClerks = NUM_PICTURE_CLERKS;
	numPassportClerks = NUM_PASSPORT_CLERKS;
	numCashiers = NUM_CASHIERS;

	totalMoneyMade = 0;
	while(true) {
		allClerksOnBreak = true; 
		applicationClerkMoneyTotal = 0;
		pictureClerkMoneyTotal = 0;
		passportClerkMoneyTotal = 0;
		cashierMoneyTotal = 0;

		for(k = 0; k < numApplicationClerks; k++) { 
			Acquire(applicationClerks[k].moneyLock);
			applicationClerkMoneyTotal = applicationClerkMoneyTotal + GetMV(applicationClerks[k].money);
			Release(applicationClerks[k].moneyLock);
			if(GetMV(applicationClerks[k].state) != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(applicationClerks[k].clerkLock);
				Acquire(applicationClerks[k].bribeLineLock);
				Acquire(applicationClerks[k].lineLock);
				/*if((applicationClerks[k].bribeLineLength + applicationClerks[k].lineLength) >= 3 || applicationClerks[k].senatorLineLength > 0) {*/
				if((GetMV(applicationClerks[k].bribeLineLength) + GetMV(applicationClerks[k].lineLength)) >= 3 || GetMV(applicationClerks[k].senatorLineLength) > 0) {
					Signal(applicationClerks[k].breakCV, applicationClerks[k].clerkLock);
					Print("Manager has woken up an ApplicationClerk\n", 0);
					allClerksOnBreak = false;
				}
				Release(applicationClerks[k].clerkLock);
				Release(applicationClerks[k].lineLock);
				Release(applicationClerks[k].bribeLineLock);
				
			}
		}


		for(k = 0; k < numPictureClerks; k++) { 
			Acquire(pictureClerks[k].moneyLock);
			pictureClerkMoneyTotal = pictureClerkMoneyTotal + GetMV(pictureClerks[k].money);
			Release(pictureClerks[k].moneyLock);
			if(GetMV(pictureClerks[k].state) != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(pictureClerks[k].clerkLock);
				Acquire(pictureClerks[k].bribeLineLock);
				Acquire(pictureClerks[k].lineLock);
				if((GetMV(pictureClerks[k].bribeLineLength) + GetMV(pictureClerks[k].lineLength)) >= 3 || GetMV(pictureClerks[k].senatorLineLength) > 0) {
					Signal(pictureClerks[k].breakCV, pictureClerks[k].clerkLock);
					Print("Manager has woken up a PictureClerk\n", 0);
					allClerksOnBreak = false;
				}
				Release(pictureClerks[k].lineLock);
				Release(pictureClerks[k].bribeLineLock);
				Release(pictureClerks[k].clerkLock);
			}
		}
		
		for(k = 0; k < numPassportClerks; k++) { 
			Acquire(passportClerks[k].moneyLock);
			passportClerkMoneyTotal = passportClerkMoneyTotal + GetMV(passportClerks[k].money);
			Release(passportClerks[k].moneyLock);
			if(GetMV(passportClerks[k].state) != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(passportClerks[k].clerkLock);
				Acquire(passportClerks[k].bribeLineLock);
				Acquire(passportClerks[k].lineLock);
				if((GetMV(passportClerks[k].bribeLineLength) + GetMV(passportClerks[k].lineLength)) >= 3 || GetMV(passportClerks[k].senatorLineLength) > 0) {
					Signal(passportClerks[k].breakCV, passportClerks[k].clerkLock);
					Print("Manager has woken up a PassportClerk\n", 0);
					allClerksOnBreak = false;
				}
				Release(passportClerks[k].lineLock);
				Release(passportClerks[k].bribeLineLock);
				Release(passportClerks[k].clerkLock);
			}
		}
		
		for(k = 0; k < numCashiers; k++) { 
			Acquire(cashiers[k].moneyLock);
			cashierMoneyTotal = cashierMoneyTotal + GetMV(cashiers[k].money);
			Release(cashiers[k].moneyLock);
			if(GetMV(cashiers[k].state) != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(cashiers[k].clerkLock);
				Acquire(cashiers[k].bribeLineLock);
				Acquire(cashiers[k].lineLock);
				if((GetMV(cashiers[k].bribeLineLength) + GetMV(cashiers[k].lineLength)) >= 3 || GetMV(cashiers[k].senatorLineLength) > 0) {
					Signal(cashiers[k].breakCV, cashiers[k].clerkLock);
					Print("Manager has woken up a Cashier\n", 0);
					allClerksOnBreak = false;
				}
				Release(cashiers[k].lineLock);
				Release(cashiers[k].bribeLineLock);
				Release(cashiers[k].clerkLock);
			}
		}
		
		
		if(allClerksOnBreak)
			continue;
		for(k = 0; k < 100; k++)
			Yield();

/*
		Print("Manager has counted a total of $%i for ApplicationClerks\n", applicationClerkMoneyTotal);
		Print("Manager has counted a total of $%i for PictureClerks\n", pictureClerkMoneyTotal);
		Print("Manager has counted a total of $%i for PassportClerks\n", passportClerkMoneyTotal);
		Print("Manager has counted a total of $%i for Cashiers\n", cashierMoneyTotal);
		totalMoneyMade = applicationClerkMoneyTotal + pictureClerkMoneyTotal + passportClerkMoneyTotal + cashierMoneyTotal;
		Print("Manager has counted a total of $%i for the Passport Office\n", totalMoneyMade);
		*/
		
	}
}

int main() {
    setup();
	runManager();
	Exit(0);
}