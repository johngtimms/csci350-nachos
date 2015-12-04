#include "syscall.h"

typedef int bool;
#define true 1
#define false 0

#define NUM_CUSTOMERS			5
#define NUM_APPLICATION_CLERKS	2
#define NUM_PICTURE_CLERKS		2
#define NUM_PASSPORT_CLERKS		2
#define NUM_CASHIERS			2
#define NUM_SENATORS			2

int amounts[] = {100, 600, 1100, 1600};
typedef enum {BUSY, FREE, BREAK} ClerkState;
typedef enum {APPLICATION_CLERK, PICTURE_CLERK, PASSPORT_CLERK, CASHIER} ClerkType;

int numCustomers, numApplicationClerks, numPassportClerks, numCashiers, numPictureClerks, numSenators;

void initClerk(ClerkType clerkType, int i) {
	switch(clerkType) {
		case APPLICATION_CLERK:
			CreateMV("appClerkLineLength", i);
			CreateMV("appClerkBribeLineLength", i);
			CreateMV("appClerkSenatorLineLength", i);
			CreateMV("appClerkMoney", i);
			CreateMV("appClerkCustomerID", i);
			CreateMV("appClerkState", i);
			CreateLock("appClerkLineLock", i);
			CreateLock("appClerkBribeLineLock", i);
			CreateLock("appClerkSenatorLineLock", i);
			CreateLock("appClerkLock", i);
			CreateLock("appClerkMoneyLock", i);
			CreateCondition("appClerkLineCV", i);
			CreateCondition("appClerkBribeLineCV", i);
			CreateCondition("appClerkSenatorLineCV", i);
			CreateCondition("appClerkCV", i);
			CreateCondition("appClerkBreakCV", i);
			break;
		case PICTURE_CLERK:
			CreateMV("picClerkLineLength", i);
			CreateMV("picClerkBribeLineLength", i);
			CreateMV("picClerkSenatorLineLength", i);
			CreateMV("picClerkMoney", i);
			CreateMV("picClerkCustomerID", i);
			CreateMV("picClerkState", i);
			CreateLock("picClerkLineLock", i);
			CreateLock("picClerkBribeLineLock", i);
			CreateLock("picClerkSenatorLineLock", i);
			CreateLock("picClerkLock", i);
			CreateLock("picClerkMoneyLock", i);
			CreateCondition("picClerkLineCV", i);
			CreateCondition("picClerkBribeCV", i);
			CreateCondition("picClerkSenatorCV", i);
			CreateCondition("picClerkCV", i);
			CreateCondition("picClerkBreakCV", i);
			break;
		case PASSPORT_CLERK:
			CreateMV("passClerkLineLength", i);
			CreateMV("passClerkBribeLineLength", i);
			CreateMV("passClerkSenatorLineLength", i);
			CreateMV("passClerkMoney", i);
			CreateMV("passClerkCustomerID", i);
			CreateMV("passClerkState", i);
			CreateLock("passClerkLineLock", i);
			CreateLock("passClerkBribeLineLock", i);
			CreateLock("passClerkSenatorLineLock", i);
			CreateLock("passClerkLock", i);
			CreateLock("passClerkMoneyLock", i);
			CreateCondition("passClerkLineCV", i);
			CreateCondition("passClerkBribeLineCV", i);
			CreateCondition("passClerkSenatorLineCV", i);
			CreateCondition("passClerkCV", i);
			CreateCondition("passClerkBreakCV", i);
			break;
		case CASHIER:
			CreateMV("cashierLineLength", i);
			CreateMV("cashierBribeLineLength", i);
			CreateMV("cashierSenatorLineLength", i);
			CreateMV("cashierMoney", i);
			CreateMV("cashierCustomerID", i);
			CreateMV("cashierState", i);
			CreateLock("cashierLineLock", i);
			CreateLock("cashierBribeLineLock", i);
			CreateLock("cashierSenatorLineLock", i);
			CreateLock("cashierLock", i);
			CreateLock("cashierMoneyLock", i);
			CreateCondition("cashierLineCV", i);
			CreateCondition("cashierBribeLineCV", i);
			CreateCondition("cashierSenatorLineCV", i);
			CreateCondition("cashierCV", i);
			CreateCondition("cashierBreakCV", i);
			break;
	}
}


void initCustomer(int ssn, bool _isSenator) {
    CreateMV("customerIsSenator", ssn);
	CreateMV("customerClerkID", ssn);
    CreateMV("customerMoney", ssn);
    CreateMV("customerHasApp", ssn);
    CreateMV("customerHasPic", ssn);
    CreateMV("customerDidBribe", ssn);
    CreateMV("customerCertified", ssn);
    CreateMV("customerHasPassport", ssn);
    CreateMV("customerSeenApp", ssn);
    CreateMV("customerSeenPic", ssn);
    CreateMV("customerLikedPic", ssn);
    CreateMV("customerHasPaid", ssn);
    CreateMV("customerLeftOffice", ssn);
    
    SetMV("customerIsSenator", ssn, _isSenator);
    SetMV("customerMoney", ssn, amounts[(int)(Rand() % 4)]);
}

void Setup() {
	int k;

	numCustomers = NUM_CUSTOMERS;
	numApplicationClerks = NUM_APPLICATION_CLERKS;
	numPictureClerks = NUM_PICTURE_CLERKS;
	numPassportClerks = NUM_PASSPORT_CLERKS;
	numCashiers = NUM_CASHIERS;
	numSenators = NUM_SENATORS;
	
	CreateMV("senatorInside", -1);
	CreateLock("senatorInsideLock", -1);
	CreateLock("senatorOutsideLineLock", -1);
	CreateCondition("senenatorOutsideLineCV", -1);
	CreateMV("numSenatorsOutside", -1);
	CreateLock("customerOutsideLineLock", -1);
	CreateCondition("customerOutsideLineCV", -1);
	CreateMV("numCustomersOutside", -1);

	CreateMV("customerIndex", -1);
	CreateMV("applicationClerkIndex", -1);
	CreateMV("pictureClerkIndex", -1);
	CreateMV("passportClerkIndex", -1);
	CreateMV("cashierIndex", -1);
	
	CreateLock("customerIndexLock", -1);
	CreateLock("applicationClerkIndexLock", -1);
	CreateLock("pictureClerkIndexLock", -1);
	CreateLock("passportClerkIndexLock", -1;
	CreateLock("cashierIndexLock", -1);

	CreateMV("timeToLeave", -1);
	
	for(k = 0; k < numApplicationClerks; k++)
		initClerk(APPLICATION_CLERK, k);
	
	for(k = 0; k < numPictureClerks; k++)
		initClerk(PICTURE_CLERK, k);
	
	for(k = 0; k < numPassportClerks; k++)
		initClerk(PASSPORT_CLERK, k);
	
	for(k = 0; k < numCashiers; k++)
		initClerk(CASHIER, k);
	
	for(k = 0; k < numCustomers; k++)
		initCustomer(k, false);
	
	for(k = numCustomers; k < numCustomers + numSenators; k++)
		initCustomer(k, true);
}













