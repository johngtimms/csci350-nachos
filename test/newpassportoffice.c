#include "syscall.h"
#include "setup.h"

int main() {
	int k;
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
		Exec("../test/cashier", sizeof("../test/cashier"));

	for(k = 0 ; k < NUM_PASSPORT_CLERKS; k++)
		Exec("../test/passportclerk", sizeof("../test/passportclerk"));

	for(k = 0 ; k < NUM_PICTURE_CLERKS; k++)
		Exec("../test/pictureclerk", sizeof("../test/pictureclerk"));

	for(k = 0 ; k < NUM_APPLICATION_CLERKS; k++)
		Exec("../test/applicationclerk", sizeof("../test/applicationclerk"));
	
	for(k = 0 ; k < NUM_CUSTOMERS; k++)
		Exec("../test/customer", sizeof("../test/customer"));

	for(k = 0 ; k < NUM_SENATORS; k++)
		Exec("../test/customer", sizeof("../test/customer"));
    /*
	Exec("../test/manager", sizeof("../test/manager"));
     */

	Exit(0);
}