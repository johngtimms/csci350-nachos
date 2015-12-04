#include "syscall.h"
#include "setup.h"


void runManager() {
	bool allClerksOnBreak, allCustomersLeftOffice;
	int applicationClerkMoneyTotal, pictureClerkMoneyTotal, passportClerkMoneyTotal, cashierMoneyTotal, totalMoneyMade, k;

	totalMoneyMade = 0;
	while(true) {
		allClerksOnBreak = true; 
		applicationClerkMoneyTotal = 0;
		pictureClerkMoneyTotal = 0;
		passportClerkMoneyTotal = 0;
		cashierMoneyTotal = 0;

		for(k = 0; k < numApplicationClerks; k++) { 
			Acquire(applicationClerk.moneyLock, k);
			applicationClerkMoneyTotal = applicationClerkMoneyTotal + GetMV(applicationClerk.money, k);
			Release(applicationClerk.moneyLock, k);
			if(GetMV(applicationClerk.state, k) != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(applicationClerk.clerkLock, k);
				Acquire(applicationClerk.bribeLineLock, k);
				Acquire(applicationClerk.lineLock, k);
				if((GetMV(applicationClerk.bribeLineLength, k) + GetMV(applicationClerk.lineLength, k)) >= 1 || GetMV(applicationClerk.senatorLineLength, k) > 0) {
					Signal(applicationClerk.breakCV, k, applicationClerk.clerkLock, k);
					Print("Manager has woken up an ApplicationClerk\n", 0);
					allClerksOnBreak = false;
				}
				Release(applicationClerk.clerkLock, k);
				Release(applicationClerk.lineLock, k);
				Release(applicationClerk.bribeLineLock, k);
				
			}
		}

		for(k = 0; k < numPictureClerks; k++) { 
			Acquire(pictureClerk.moneyLock, k);
			pictureClerkMoneyTotal = pictureClerkMoneyTotal + GetMV(pictureClerk.money, k);
			Release(pictureClerk.moneyLock, k);
			if(GetMV(pictureClerk.state, k) != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(pictureClerk.clerkLock, k);
				Acquire(pictureClerk.bribeLineLock, k);
				Acquire(pictureClerk.lineLock, k);
				if((GetMV(pictureClerk.bribeLineLength, k) + GetMV(pictureClerk.lineLength, k)) >= 1 || GetMV(pictureClerk.senatorLineLength, k) > 0) {
					Signal(pictureClerk.breakCV, k, pictureClerk.clerkLock, k);
					Print("Manager has woken up a PictureClerk\n", 0);
					allClerksOnBreak = false;
				}
				Release(pictureClerk.lineLock, k);
				Release(pictureClerk.bribeLineLock, k);
				Release(pictureClerk.clerkLock, k);
			}
		}
		
		for(k = 0; k < numPassportClerks; k++) { 
			Acquire(passportClerk.moneyLock, k);
			passportClerkMoneyTotal = passportClerkMoneyTotal + GetMV(passportClerk.money, k);
			Release(passportClerk.moneyLock, k);
			if(GetMV(passportClerk.state, k) != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(passportClerk.clerkLock, k);
				Acquire(passportClerk.bribeLineLock, k);
				Acquire(passportClerk.lineLock, k);
				if((GetMV(passportClerk.bribeLineLength, k) + GetMV(passportClerk.lineLength, k)) >= 1 || GetMV(passportClerk.senatorLineLength, k) > 0) {
					Signal(passportClerk.breakCV, k, passportClerk.clerkLock, k);
					Print("Manager has woken up a PassportClerk\n", 0);
					allClerksOnBreak = false;
				}
				Release(passportClerk.lineLock, k);
				Release(passportClerk.bribeLineLock, k);
				Release(passportClerk.clerkLock, k);
			}
		}
		
		for(k = 0; k < numCashiers; k++) { 
			Acquire(cashier.moneyLock, k);
			cashierMoneyTotal = cashierMoneyTotal + GetMV(cashier.money, k);
			Release(cashier.moneyLock, k);
			if(GetMV(cashier.state, k) != BREAK)
				allClerksOnBreak = false; 
			else {
				Acquire(cashier.clerkLock, k);
				Acquire(cashier.bribeLineLock, k);
				Acquire(cashier.lineLock, k);
				if((GetMV(cashier.bribeLineLength, k) + GetMV(cashier.lineLength, k)) >= 1 || GetMV(cashier.senatorLineLength, k) > 0) {
					Signal(cashier.breakCV, k, cashier.clerkLock, k);
					Print("Manager has woken up a Cashier\n", 0);
					allClerksOnBreak = false;
				}
				Release(cashier.lineLock, k);
				Release(cashier.bribeLineLock, k);
				Release(cashier.clerkLock, k);
			}
		}
		
		allCustomersLeftOffice = true;

		for(k = 0; k < numCustomers + numSenators; k++) {
			if(GetMV(customer.leftOffice, k) == false) {
				allCustomersLeftOffice = false;
				break;
			}
		}

		if(allCustomersLeftOffice == true) {
			Print("All Customers have left the office.\n",0);
			SetMV(timeToLeave, -1, true);
			/* tell all clerks to get off breaks and leave */
			for(k = 0; k < numApplicationClerks ; k++) {
				Acquire(applicationClerk.clerkLock, k);
				Signal(applicationClerk.breakCV, k, applicationClerk.clerkLock, k);
				Release(applicationClerk.clerkLock, k);
			}
			for(k = 0; k < numPictureClerks ; k++) {
				Acquire(pictureClerk.clerkLock, k);
				Signal(pictureClerk.breakCV, k, pictureClerk.clerkLock, k);
				Release(pictureClerk.clerkLock, k);
			}
			for(k = 0; k < numPassportClerks ; k++) {
				Acquire(passportClerk.clerkLock, k);
				Signal(passportClerk.breakCV, k, passportClerk.clerkLock, k);
				Release(passportClerk.clerkLock, k);
			}
			for(k = 0; k < numCashiers ; k++) {
				Acquire(cashier.clerkLock, k);
				Signal(cashier.breakCV, k, cashier.clerkLock, k);
				Release(cashier.clerkLock, k);
			}
			Print("Final totals: \n", 0);
			Print("Manager has counted a total of $%i for ApplicationClerks\n", applicationClerkMoneyTotal);
			Print("Manager has counted a total of $%i for PictureClerks\n", pictureClerkMoneyTotal);
			Print("Manager has counted a total of $%i for PassportClerks\n", passportClerkMoneyTotal);
			Print("Manager has counted a total of $%i for Cashiers\n", cashierMoneyTotal);
			totalMoneyMade = applicationClerkMoneyTotal + pictureClerkMoneyTotal + passportClerkMoneyTotal + cashierMoneyTotal;
			Print("Manager has counted a total of $%i for the Passport Office\n", totalMoneyMade);
			Exit(0);
		}


		if(allClerksOnBreak)
			continue;
		

		if(Rand() % 5 == 0) {
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
}

int main() {
    Setup();
	runManager();
	Exit(0);
}