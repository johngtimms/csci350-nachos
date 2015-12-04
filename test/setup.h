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

int numCustomers, numSenators, numApplicationClerks, numPictureClerks, numPassportClerks, numCashiers;

char *senatorInside = "SenInside";
char *senatorInsideLock = "SenInsideLock";
char *senatorsOutside = "NumSenOutside";
char *senatorOutsideLineLock = "SenOutsideLineLock";
char *senatorOutsideLineCV = "SenOutsideLineCV";
char *customersOutside = "NumCustOutside";
char *customerOutsideLineLock = "CustOutsideLineLock";
char *customerOutsideLineCV = "CustOutsideLineCV";
char *timeToLeave = "TimeToLeave";

int amounts[] = {100, 600, 1100, 1600};
typedef enum {FREE, BUSY, BREAK} ClerkState;
typedef enum {APPLICATION_CLERK, PICTURE_CLERK, PASSPORT_CLERK, CASHIER} ClerkType;

typedef struct Customer {
	char *index, *indexLock, *money, *clerkID, *didBribe, *isSenator, *seenApp, *hasApp, *seenPic, *likedPic, *hasPic, *hasPaidForPassport, 
			*certifiedByPassportClerk, *hasPassport, *leftOffice;
} Customer;


typedef struct ApplicationClerk {
	char *index, *indexLock, *customerID, *state, *clerkLock, *clerkCV, *breakCV, *money, *moneyLock, *lineLength, *lineLock, *lineCV, *bribeLineLength,
			*bribeLineLock, *bribeLineCV, *senatorLineLength, *senatorLineLock, *senatorLineCV;
} ApplicationClerk;

typedef struct PictureClerk {
	char *index, *indexLock, *customerID, *state, *clerkLock, *clerkCV, *breakCV, *money, *moneyLock, *lineLength, *lineLock, *lineCV, *bribeLineLength,
			*bribeLineLock, *bribeLineCV, *senatorLineLength, *senatorLineLock, *senatorLineCV;
} PictureClerk;

typedef struct PassportClerk {
	char *index, *indexLock, *customerID, *state, *clerkLock, *clerkCV, *breakCV, *money, *moneyLock, *lineLength, *lineLock, *lineCV, *bribeLineLength,
			*bribeLineLock, *bribeLineCV, *senatorLineLength, *senatorLineLock, *senatorLineCV;
} PassportClerk;

typedef struct Cashier {
	char *index, *indexLock, *customerID, *state, *clerkLock, *clerkCV, *breakCV, *money, *moneyLock, *lineLength, *lineLock, *lineCV, *bribeLineLength,
			*bribeLineLock, *bribeLineCV, *senatorLineLength, *senatorLineLock, *senatorLineCV;
} Cashier;

Customer customer;
ApplicationClerk applicationClerk;
PictureClerk pictureClerk;
PassportClerk passportClerk;
Cashier cashier;

void initCustomer(int ssn, bool isSen) {
	customer.index = "CustIndex";
	customer.indexLock = "CustIndexLock";
	customer.money = "CustMoney";
	customer.clerkID = "CustClerkID";
	customer.didBribe = "CustDidBribe";		
	customer.isSenator = "CustIsSenator";
	customer.seenApp = "CustSeenApp";
	customer.hasApp = "CustrHasApp";
	customer.seenPic = "CustSeenPic";
	customer.likedPic = "CustLikedPic";
	customer.hasPic = "CustHasPic";
	customer.hasPaidForPassport = "CustHasPaid";
	customer.certifiedByPassportClerk = "CustCertified";
	customer.hasPassport = "CustHasPassport";
	customer.leftOffice = "CustLeftOffice";

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
    CreateMV(customer.hasPaidForPassport, ssn);
    CreateMV(customer.leftOffice, ssn);
    
    SetMV(customer.isSenator, ssn, isSen);
    SetMV(customer.money, ssn, amounts[(int)(Rand() % 4)]);
}


void initClerk(ClerkType clerkType, int i) {
	switch(clerkType) {
		case APPLICATION_CLERK:
			applicationClerk.index = "AppIndex";
			applicationClerk.indexLock = "AppIndexLock";
			applicationClerk.customerID = "AppCustID";
			applicationClerk.state = "AppState";
			applicationClerk.clerkLock = "AppLock";
			applicationClerk.clerkCV = "AppCV";
			applicationClerk.breakCV = "AppBreakCV";
			applicationClerk.money = "AppMoney";
			applicationClerk.moneyLock = "AppMoneyLock";
			applicationClerk.lineLength = "AppLineLength";
			applicationClerk.lineLock = "AppLineLock";
			applicationClerk.lineCV = "AppLineCV";
			applicationClerk.bribeLineLength = "AppBribeLineLength";
			applicationClerk.bribeLineLock = "AppBribeLineLock";
			applicationClerk.bribeLineCV = "AppBribeLineCV";
			applicationClerk.senatorLineLength = "AppSenatorLineLength";
			applicationClerk.senatorLineLock = "AppSenLineLock";
			applicationClerk.senatorLineCV = "AppSenatorLineCV";	
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
			pictureClerk.index = "PicIndex";
			pictureClerk.indexLock = "PicIndexLock";
			pictureClerk.customerID = "PicCustID";
			pictureClerk.state = "PicState";
			pictureClerk.clerkLock = "PicLock";
			pictureClerk.clerkCV = "PicCV";
			pictureClerk.breakCV = "PicBreakCV";
			pictureClerk.money = "PicMoney";
			pictureClerk.moneyLock = "PicMoneyLock";
			pictureClerk.lineLength = "PicLineLength";
			pictureClerk.lineLock = "PicLineLock";
			pictureClerk.lineCV = "PicLineCV";
			pictureClerk.bribeLineLength = "PicBribeLineLength";
			pictureClerk.bribeLineLock = "PicBribeLineLock";
			pictureClerk.bribeLineCV = "PicBribeLineCV";
			pictureClerk.senatorLineLength = "PicSenLineLength";
			pictureClerk.senatorLineLock = "PicSenLineLock";
			pictureClerk.senatorLineCV = "PicSenatorLineCV";	
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
			passportClerk.index = "PassIndex";
			passportClerk.indexLock = "PassIndexLock";
			passportClerk.customerID = "PassCustID";
			passportClerk.state = "PassState";
			passportClerk.clerkLock = "PassLock";
			passportClerk.clerkCV = "PassCV";
			passportClerk.breakCV = "PassBreakCV";
			passportClerk.money = "PassMoney";
			passportClerk.moneyLock = "PassMoneyLock";
			passportClerk.lineLength = "PassLineLength";
			passportClerk.lineLock = "PassLineLock";
			passportClerk.lineCV = "PassLineCV";
			passportClerk.bribeLineLength = "PassBribeLineLength";
			passportClerk.bribeLineLock = "PassBribeLineLock";
			passportClerk.bribeLineCV = "PassBribeLineCV";
			passportClerk.senatorLineLength = "PassSenLineLength";
			passportClerk.senatorLineLock = "PassSenLineLock";
			passportClerk.senatorLineCV = "PassSenLineCV";	
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
			cashier.index = "CashIndex";
			cashier.indexLock = "CashIndexLock";
			cashier.customerID = "CashCustomerID";
			cashier.state = "CashState";
			cashier.clerkLock = "CashLock";
			cashier.clerkCV = "CashCV";
			cashier.breakCV = "CashBreakCV";
			cashier.money = "CashMoney";
			cashier.moneyLock = "CashMoneyLock";
			cashier.lineLength = "CashLineLength";
			cashier.lineLock = "CashLineLock";
			cashier.lineCV = "CashLineCV";
			cashier.bribeLineLength = "CashBribeLineLength";
			cashier.bribeLineLock = "CashBribeLineLock";
			cashier.bribeLineCV = "CashBribeLineCV";
			cashier.senatorLineLength = "CashSenLineLength";
			cashier.senatorLineLock = "CashSenLineLock";
			cashier.senatorLineCV = "CashSenLineCV";	
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
	CreateLock(passportClerk.indexLock, -1);
	CreateLock(cashier.indexLock, -1);
}













