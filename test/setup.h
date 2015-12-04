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

char *senatorInside = "SenatorInside";
char *senatorInsideLock = "SenatorInsideLock";
char *senatorsOutside = "NumSenatorsOutside";
char *senatorOutsideLineLock = "SenatorOutsideLineLock";
char *senatorOutsideLineCV = "SenatorOutsideLineCV";
char *customersOutside = "NumCustomersOutside";
char *customerOutsideLineLock = "CustomerOutsideLineLock";
char *customerOutsideLineCV = "CustomerOutsideLineCV";
char *timeToLeave = "TimeToLeave";

int amounts[] = {100, 600, 1100, 1600};
typedef enum {FREE, BUSY, BREAK} ClerkState;
typedef enum {APPLICATION_CLERK, PICTURE_CLERK, PASSPORT_CLERK, CASHIER} ClerkType;

typedef struct Customer {
	char *index = "CustomerIndex";
	char *indexLock = "CustomerIndexLock";
	char *money = "CustomerMoney";
	char *clerkID = "CustomerClerkID";
	char *didBribe = "CustomerDidBribe";		
	char *isSenator = "CustomerIsSenator";
	char *seenApp = "CustomerSeenApp";
	char *hasApp = "CustomerHasApp";
	char *seenPic = "CustomerSeenPic";
	char *likedPic = "CustomerLikedPic";
	char *hasPic = "CustomerHasPic";
	char *hasPaidForPassport = "CustomerHasPaid";
	char *certifiedByPassportClerk = "CustomerCertified";
	char *hasPassport = "CustomerHasPassport";
	char *leftOffice = "CustomerLeftOffice";
} Customer;

typedef struct ApplicationClerk {
	char *index = "ApplicationClerkIndex";
	char *indexLock = "ApplicationClerkIndexLock";
	char *customerID = "ApplicationClerkCustomerID";
	char *state = "ApplicationClerkState";
	char *clerkLock = "ApplicationClerkLock";
	char *clerkCV = "ApplicationClerkCV";
	char *breakCV = "ApplicationClerkBreakCV";
	char *money = "ApplicationClerkMoney";
	char *moneyLock = "ApplicationClerkMoneyLock";
	char *lineLength = "ApplicationClerkLineLength";
	char *lineLock = "ApplicationClerkLineLength";
	char *lineCV = "ApplicationClerkLineCV";
	char *bribeLineLength = "ApplicationClerkBribeLineLength";
	char *bribeLineLock = "ApplicationClerkBribeLineLock";
	char *bribeLineCV = "ApplicationClerkBribeLineCV";
	char *senatorLineLength = "ApplicationClerkSenatorLineLength";
	char *senatorLineLock = "ApplicationClerkSenatorLineLock";
	char *senatorLineCV = "ApplicationClerkSenatorLineCV";	
} ApplicationClerk;

typedef struct PictureClerk {
	char *index = "PictureClerkIndex";
	char *indexLock = "PictureClerkIndexLock";
	char *customerID = "PictureClerkCustomerID";
	char *state = "PictureClerkState";
	char *clerkLock = "PictureClerkLock";
	char *clerkCV = "PictureClerkCV";
	char *breakCV = "PictureClerkBreakCV";
	char *money = "PictureClerkMoney";
	char *moneyLock = "PictureClerkMoneyLock";
	char *lineLength = "PictureClerkLineLength";
	char *lineLock = "PictureClerkLineLength";
	char *lineCV = "PictureClerkLineCV";
	char *bribeLineLength = "PictureClerkBribeLineLength";
	char *bribeLineLock = "PictureClerkBribeLineLock";
	char *bribeLineCV = "PictureClerkBribeLineCV";
	char *senatorLineLength = "PictureClerkSenatorLineLength";
	char *senatorLineLock = "PictureClerkSenatorLineLock";
	char *senatorLineCV = "PictureClerkSenatorLineCV";	
} PictureClerk;

typedef struct PassportClerk {
	char *index = "PassportClerkIndex";
	char *indexLock = "PassportClerkIndexLock";
	char *customerID = "PassportClerkCustomerID";
	char *state = "PassportClerkState";
	char *clerkLock = "PassportClerkLock";
	char *clerkCV = "PassportClerkCV";
	char *breakCV = "PassportClerkBreakCV";
	char *money = "PassportClerkMoney";
	char *moneyLock = "PassportClerkMoneyLock";
	char *lineLength = "PassportClerkLineLength";
	char *lineLock = "PassportClerkLineLength";
	char *lineCV = "PassportClerkLineCV";
	char *bribeLineLength = "PassportClerkBribeLineLength";
	char *bribeLineLock = "PassportClerkBribeLineLock";
	char *bribeLineCV = "PassportClerkBribeLineCV";
	char *senatorLineLength = "PassportClerkSenatorLineLength";
	char *senatorLineLock = "PassportClerkSenatorLineLock";
	char *senatorLineCV = "PassportClerkSenatorLineCV";	
} PassportClerk;

typedef struct Cashier {
	char *index = "CashierIndex";
	char *indexLock = "CashierIndexLock";
	char *customerID = "CashierCustomerID";
	char *state = "CashierState";
	char *clerkLock = "CashierLock";
	char *clerkCV = "CashierCV";
	char *breakCV = "CashierBreakCV";
	char *money = "CashierMoney";
	char *moneyLock = "CashierMoneyLock";
	char *lineLength = "CashierLineLength";
	char *lineLock = "CashierLineLength";
	char *lineCV = "CashierLineCV";
	char *bribeLineLength = "CashierBribeLineLength";
	char *bribeLineLock = "CashierBribeLineLock";
	char *bribeLineCV = "CashierBribeLineCV";
	char *senatorLineLength = "CashierSenatorLineLength";
	char *senatorLineLock = "CashierSenatorLineLock";
	char *senatorLineCV = "CashierSenatorLineCV";	
} Cashier;

Customer customer;
ApplicationClerk applicationClerk;
PictureClerk pictureClerk;
PassportClerk passportClerk;
Cashier cashier;

void initCustomer(int ssn, bool isSen) {
    CreateMV(customer.isSenator, ssn);
	CreateMV(customer.clerkID, ssn);
    CreateMV(customer.money, ssn);
    CreateMV(customer.hasApp, ssn);
    CreateMV(customer.hasPic, ssn);
    CreateMV(customer.didBribe, ssn);
    CreateMV(customer.certifiedByPassportClerk, ssn);
    CreateMV(customer.hasPassport, ssn);
    CreateMV(customer.seenApp, ssn);
    CreateMV(customer.seenPic, ssn);
    CreateMV(customer.likedPic, ssn);
    CreateMV(customer.hasPaid, ssn);
    CreateMV(customer.leftOffice, ssn);
    
    SetMV(customer.isSenator, ssn, isSen);
    SetMV(customer.money, ssn, amounts[(int)(Rand() % 4)]);
}

void initClerk(ClerkType clerkType, int i) {
	switch(clerkType) {
		case APPLICATION_CLERK:
			CreateMV(applicationClerk.lineLength, i);
			CreateMV(applicationClerk.bribeLineLength, i);
			CreateMV(applicationClerk.senatorLineLength, i);
			CreateMV(applicationClerk.money, i);
			CreateMV(applicationClerk.customerID, i);
			CreateMV(applicationClerk.state, i);
			CreateLock(applicationClerk.lineLock, i);
			CreateLock(applicationClerk.bribeLineLock, i);
			CreateLock(applicationClerk.senatorLineLock, i);
			CreateLock(applicationClerk.clerkLock, i);
			CreateLock(applicationClerk.moneyLock, i);
			CreateCondition(applicationClerk.lineCV, i);
			CreateCondition(applicationClerk.bribeLineCV, i);
			CreateCondition(applicationClerk.senatorLineCV, i);
			CreateCondition(applicationClerk.clerkCV, i);
			CreateCondition(applicationClerk.breakCV, i);
			break;
		case PICTURE_CLERK:
			CreateMV(pictureClerk.lineLength, i);
			CreateMV(pictureClerk.bribeLineLength, i);
			CreateMV(pictureClerk.senatorLineLength, i);
			CreateMV(pictureClerk.money, i);
			CreateMV(pictureClerk.customerID, i);
			CreateMV(pictureClerk.state, i);
			CreateLock(pictureClerk.lineLock, i);
			CreateLock(pictureClerk.bribeLineLock, i);
			CreateLock(pictureClerk.senatorLineLock, i);
			CreateLock(pictureClerk.clerkLock, i);
			CreateLock(pictureClerk.moneyLock, i);
			CreateCondition(pictureClerk.lineCV, i);
			CreateCondition(pictureClerk.bribeLineCV, i);
			CreateCondition(pictureClerk.senatorLineCV, i);
			CreateCondition(pictureClerk.clerkCV, i);
			CreateCondition(pictureClerk.breakCV, i);
			break;
		case PASSPORT_CLERK:
			CreateMV(passportClerk.lineLength, i);
			CreateMV(passportClerk.bribeLineLength, i);
			CreateMV(passportClerk.senatorLineLength, i);
			CreateMV(passportClerk.money, i);
			CreateMV(passportClerk.customerID, i);
			CreateMV(passportClerk.state, i);
			CreateLock(passportClerk.lineLock, i);
			CreateLock(passportClerk.bribeLineLock, i);
			CreateLock(passportClerk.senatorLineLock, i);
			CreateLock(passportClerk.clerkLock, i);
			CreateLock(passportClerk.moneyLock, i);
			CreateCondition(passportClerk.lineCV, i);
			CreateCondition(passportClerk.bribeLineCV, i);
			CreateCondition(passportClerk.senatorLineCV, i);
			CreateCondition(passportClerk.clerkCV, i);
			CreateCondition(passportClerk.breakCV, i);
			break;
		case CASHIER:
			CreateMV(cashier.lineLength, i);
			CreateMV(cashier.bribeLineLength, i);
			CreateMV(cashier.senatorLineLength, i);
			CreateMV(cashier.money, i);
			CreateMV(cashier.customerID, i);
			CreateMV(cashier.state, i);
			CreateLock(cashier.lineLock, i);
			CreateLock(cashier.bribeLineLock, i);
			CreateLock(cashier.senatorLineLock, i);
			CreateLock(cashier.clerkLock, i);
			CreateLock(cashier.moneyLock, i);
			CreateCondition(cashier.lineCV, i);
			CreateCondition(cashier.bribeLineCV, i);
			CreateCondition(cashier.senatorLineCV, i);
			CreateCondition(cashier.clerkCV, i);
			CreateCondition(cashier.breakCV, i);
			break;
	}
}

void Setup() {
	int k;

	numCustomers = NUM_CUSTOMERS;
	numApplicationClerks = NUM_APPLICATION_CLERKS;
	numPictureClerks = NUM_PICTURE_CLERKS;
	numPassportClerks = NUM_PASSPORT_CLERKS;
	numCashiers = NUM_CASHIERS;
	numSenators = NUM_SENATORS;
	
	CreateMV(senatorInside, -1);
	CreateLock(senatorInsideLock, -1);
	CreateMV(senatorsOutside, -1);
	CreateLock(senatorOutsideLineLock, -1);
	CreateCondition(senatorOutsideLineCV, -1);
	CreateMV(customersOutside, -1);
	CreateLock(customerOutsideLineLock, -1);
	CreateCondition(customerOutsideLineCV, -1);
	CreateMV(timeToLeave, -1);
	
	CreateMV(customer.index, -1);
	CreateMV(applicationClerk.index, -1);
	CreateMV(pictureClerk.index, -1);
	CreateMV(passportClerk.index, -1);
	CreateMV(cashier.index, -1);
	
	CreateLock(customer.indexLock, -1);
	CreateLock(applicationClerk.indexLock, -1);
	CreateLock(pictureClerk.indexLock, -1);
	CreateLock(passportClerk.indexLock, -1;
	CreateLock(cashier.indexLock, -1);

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













