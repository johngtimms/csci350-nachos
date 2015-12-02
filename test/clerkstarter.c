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
	Print("================Starting Clerks==================\n",0);
	Print("=================================================\n",0);
	Print("Number of ApplicationClerks: %i\n", numApplicationClerks);
	Print("Number of PictureClerks: %i\n", numPictureClerks);
	Print("Number of PassportClerks: %i\n", numPassportClerks);
	Print("Number of Cashiers: %i\n", numCashiers);
	Print("Number of Managers: 1\n", 0);
	
	for(k = 0 ; k < numCashiers; k++)
		Exec("../test/cashier", sizeof("../test/cashier"));

	for(k = 0 ; k < numPassportClerks; k++)
		Exec("../test/passportclerk", sizeof("../test/passportclerk"));

	for(k = 0 ; k < numPictureClerks; k++)
		Exec("../test/pictureclerk", sizeof("../test/pictureclerk"));

	for(k = 0 ; k < numApplicationClerks; k++)
		Exec("../test/applicationclerk", sizeof("../test/applicationclerk")); 
    
	Exec("../test/manager", sizeof("../test/manager"));
	
	Print("Setting up locks/cvs/mvs...\n",0);
     

	Exit(0);
}