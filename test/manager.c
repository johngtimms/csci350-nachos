#include "syscall.h"
#include "setup.h"

void runManager() {
	bool allClerksOnBreak;
	int applicationClerkMoneyTotal, pictureClerkMoneyTotal, passportClerkMoneyTotal, cashierMoneyTotal, k;
	totalMoneyMade = 0;
	while(true) {
		allClerksOnBreak = true; 
		applicationClerkMoneyTotal = 0;
		pictureClerkMoneyTotal = 0;
		passportClerkMoneyTotal = 0;
		cashierMoneyTotal = 0;

		for(k = 0; k < NUM_APPLICATION_CLERKS; k++) { 
			Acquire(applicationClerks[k].moneyLock);
			/*applicationClerkMoneyTotal = applicationClerkMoneyTotal + applicationClerks[k].money;*/
			applicationClerkMoneyTotal = applicationClerkMoneyTotal + GetMV(applicationClerks[k].money);
			Release(applicationClerks[k].moneyLock);
			/*if(applicationClerks[k].state != BREAK)*/
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
				Release(applicationClerks[k].lineLock);
				Release(applicationClerks[k].bribeLineLock);
				Release(applicationClerks[k].clerkLock);
			}
		}
		for(k = 0; k < NUM_PICTURE_CLERKS; k++) { 
			Acquire(pictureClerks[k].moneyLock);
			/*pictureClerkMoneyTotal = pictureClerkMoneyTotal + pictureClerks[k].money;*/
			pictureClerkMoneyTotal = pictureClerkMoneyTotal + GetMV(pictureClerks[k].money);
			Release(pictureClerks[k].moneyLock);
			/*if(pictureClerks[k].state != BREAK)*/
			if(GetMV(pictureClerks[k].state) != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(pictureClerks[k].clerkLock);
				Acquire(pictureClerks[k].bribeLineLock);
				Acquire(pictureClerks[k].lineLock);
				/*if((pictureClerks[k].bribeLineLength + pictureClerks[k].lineLength) >= 3 || pictureClerks[k].senatorLineLength > 0) {*/
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
		for(k = 0; k < NUM_PASSPORT_CLERKS; k++) { 
			Acquire(passportClerks[k].moneyLock);
			/*passportClerkMoneyTotal = passportClerkMoneyTotal + passportClerks[k].money;*/
			passportClerkMoneyTotal = passportClerkMoneyTotal + GetMV(passportClerks[k].money);
			Release(passportClerks[k].moneyLock);
			/*if(passportClerks[k].state != BREAK)*/
			if(GetMV(passportClerks[k].state) != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(passportClerks[k].clerkLock);
				Acquire(passportClerks[k].bribeLineLock);
				Acquire(passportClerks[k].lineLock);
				/*if((passportClerks[k].bribeLineLength + passportClerks[k].lineLength) >= 3 || passportClerks[k].senatorLineLength > 0) {*/
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
		for(k = 0; k < NUM_CASHIERS; k++) { 
			Acquire(cashiers[k].moneyLock);
			/*cashierMoneyTotal = cashierMoneyTotal + cashiers[k].money;*/
			cashierMoneyTotal = cashierMoneyTotal + GetMV(cashiers[k].money);
			Release(cashiers[k].moneyLock);
			if(cashiers[k].state != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(cashiers[k].clerkLock);
				Acquire(cashiers[k].bribeLineLock);
				Acquire(cashiers[k].lineLock);
				/*if((cashiers[k].bribeLineLength + cashiers[k].lineLength) >= 3 || cashiers[k].senatorLineLength > 0) {*/
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

		Print("Manager has counted a total of $%i for ApplicationClerks\n", applicationClerkMoneyTotal);
		Print("Manager has counted a total of $%i for PictureClerks\n", pictureClerkMoneyTotal);
		Print("Manager has counted a total of $%i for PassportClerks\n", passportClerkMoneyTotal);
		Print("Manager has counted a total of $%i for Cashiers\n", cashierMoneyTotal);
		totalMoneyMade = applicationClerkMoneyTotal + pictureClerkMoneyTotal + passportClerkMoneyTotal + cashierMoneyTotal;
		Print("Manager has counted a total of $%i for the Passport Office\n", totalMoneyMade);
		
	}
}

int main() {
	runManager();
	Exit(0);
}