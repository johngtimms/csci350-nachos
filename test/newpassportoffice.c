#include "syscall.h"
#include "setup.h"

int main() {
	int k;
    /*
	senatorOutsideLineLock = CreateLock();
	senatorOutsideLineCV = CreateCondition();
	senatorInsideLock = CreateLock();
	customerOutsideLineLock = CreateLock();
	customerOutsideLineCV = CreateCondition();
	customerIndexLock = CreateLock();
	applicationClerkIndexLock = CreateLock();
	pictureClerkIndexLock = CreateLock();
	passportClerkIndexLock = CreateLock();
	cashierIndexLock = CreateLock();
     */
	
	numCustomers = 2;
	numApplicationClerks = 0;
    numPictureClerks = 0;
	numPassportClerks = 0;
	numCashiers = 0;
	numSenators = 0;

	Print("Number of Customers: %i\n", numCustomers);
	Print("Number of ApplicationClerks: %i\n", numApplicationClerks);
	Print("Number of PictureClerks: %i\n", numPictureClerks);
	Print("Number of PassportClerks: %i\n", numPassportClerks);
	Print("Number of Cashiers: %i\n", numCashiers);
	Print("Number of Senators: %i\n", numSenators);
    
/*
	for(k = 0; k < numApplicationClerks; k++)
		initClerk(APPLICATION_CLERK,k);
	
	for(k = 0; k < numPictureClerks; k++)
		initClerk(PICTURE_CLERK,k);
	
	for(k = 0; k < numPassportClerks; k++)
		initClerk(PASSPORT_CLERK,k);
	
	for(k = 0; k < numCashiers; k++)
		initClerk(CASHIER,k);
	
	for(k = 0; k < numCustomers; k++)
		initCustomer(k, false);

	for(k = 0; k < numSenators; k++)
		initCustomer(k, true);



	for(k = 0 ; k < numCashiers; k++)
		Exec("../test/cashier", 15);

	for(k = 0 ; k < numPassportClerks; k++)
		Exec("../test/passportclerk", 21);

	for(k = 0 ; k < numPictureClerks; k++)
		Exec("../test/pictureclerk", 20);
	
	for(k = 0 ; k < numApplicationClerks; k++)
		Exec("../test/applicationclerk", 24);
	*/
	for(k = 0 ; k < numCustomers; k++)
		Exec("../test/customer", 16);

	for(k = 0 ; k < numSenators; k++)
		Exec("../test/customer", 16);
    /*
	Exec("../test/manager", 15);
     */


	Exit(0);
}