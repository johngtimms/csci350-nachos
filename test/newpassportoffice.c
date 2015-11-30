#include "syscall.h"
#include "setup.h"


int main() {
	int k, numCustomers, numApplicationClerks, numPassportClerks, numCashiers, numPictureClerks;

	numCustomers = NUM_CUSTOMERS;
	numApplicationClerks = NUM_APPLICATION_CLERKS;
	numPictureClerks = NUM_PICTURE_CLERKS;
	numPassportClerks = NUM_PASSPORT_CLERKS;
	numCashiers = NUM_CASHIERS;

	Print("Number of Customers: %i\n", numCustomers);
	Print("Number of ApplicationClerks: %i\n", numApplicationClerks);
	Print("Number of PictureClerks: %i\n", numPictureClerks);
	Print("Number of PassportClerks: %i\n", numPassportClerks);
	Print("Number of Cashiers: %i\n", numCashiers);
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
	
	for(k = 0 ; k < numCashiers; k++)
		Exec("../test/cashier", sizeof("../test/cashier"));

	for(k = 0 ; k < numPassportClerks; k++)
		Exec("../test/passportclerk", sizeof("../test/passportclerk"));

	for(k = 0 ; k < numPictureClerks; k++)
		Exec("../test/pictureclerk", sizeof("../test/pictureclerk"));

	for(k = 0 ; k < numApplicationClerks; k++)
		Exec("../test/applicationclerk", sizeof("../test/applicationclerk"));
	
	for(k = 0 ; k < numCustomers; k++)
		Exec("../test/customer", sizeof("../test/customer"));
/*
	for(k = 0 ; k < NUM_SENATORS; k++)
		Exec("../test/customer", sizeof("../test/customer"));
    
    */
	Exec("../test/manager", sizeof("../test/manager"));
	
	
     

	Exit(0);
}