#include "syscall.h"
#include "setup.h"


int main() {
	int k, numCustomers, numApplicationClerks, numPassportClerks, numCashiers, numPictureClerks, numSenators;

	numCustomers = NUM_CUSTOMERS;
	numApplicationClerks = NUM_APPLICATION_CLERKS;
	numPictureClerks = NUM_PICTURE_CLERKS;
	numPassportClerks = NUM_PASSPORT_CLERKS;
	numCashiers = NUM_CASHIERS;
	numSenators = NUM_SENATORS;

	Print("=================================================\n",0);
	Print("=========Starting Customers And Senators=========\n",0);
	Print("=================================================\n",0);
	Print("Number of Customers: %i\n", numCustomers);
	Print("Number of Senators: %i\n", numSenators);
	
	for(k = 0 ; k < numCustomers; k++)
		Exec("../test/customer", sizeof("../test/customer"));

	for(k = 0 ; k < numSenators; k++)
		Exec("../test/customer", sizeof("../test/customer"));
    
  	Print("Setting up locks/cvs/mvs...\n",0);
     

	Exit(0);
}