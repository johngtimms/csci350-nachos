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
	
	Print("Number of Customers: %i\n", NUM_CUSTOMERS);
	Print("Number of ApplicationClerks: %i\n", NUM_APPLICATION_CLERKS);
	Print("Number of PictureClerks: %i\n", NUM_PICTURE_CLERKS);
	Print("Number of PassportClerks: %i\n", NUM_PICTURE_CLERKS);
	Print("Number of Cashiers: %i\n", NUM_CASHIERS);
	Print("Number of Senators: %i\n", NUM_SENATORS);

	/*
	for(k = 0; k < NUM_APPLICATION_CLERKS; k++)
		initClerk(APPLICATION_CLERK,k);
	
	for(k = 0; k < NUM_PICTURE_CLERKS; k++)
		initClerk(PICTURE_CLERK, k);
	
	for(k = 0; k < NUM_PASSPORT_CLERKS; k++)
		initClerk(PASSPORT_CLERK, k);
	
	for(k = 0; k < NUM_CASHIERS; k++)
		initClerk(CASHIER, k);
	
	for(k = 0; k < NUM_CUSTOMERS; k++)
		initCustomer(k, false);

	for(k = 0; k < NUM_SENATORS; k++)
		initCustomer(k, true);
	*/

	for(k = 0 ; k < NUM_CASHIERS; k++)
		Exec("../test/cashier", 15);

	for(k = 0 ; k < NUM_PASSPORT_CLERKS; k++)
		Exec("../test/passportclerk", 21);

	for(k = 0 ; k < NUM_PICTURE_CLERKS; k++)
		Exec("../test/pictureclerk", 20);

	for(k = 0 ; k < NUM_APPLICATION_CLERKS; k++)
		Exec("../test/applicationclerk", 23);
	
	for(k = 0 ; k < NUM_CUSTOMERS; k++)
		Exec("../test/customer", 16);

	for(k = 0 ; k < NUM_SENATORS; k++)
		Exec("../test/customer", 15);
    /*
	Exec("../test/manager", 15);
     */

	Exit(0);
}