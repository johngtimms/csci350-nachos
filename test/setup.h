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

struct Customer;
struct Clerk;

int nextAvailableCustomerIndex = 0; 
int nextAvailablePictureClerkIndex = 0; 
int nextAvailablePassportClerkIndex = 0;
int nextAvailableCashierIndex = 0;
int nextAvailableApplicationClerkIndex = 0;
int customerIndexLock, applicationClerkIndexLock, pictureClerkIndexLock, passportClerkIndexLock, cashierIndexLock, senatorIndexLock;

int amounts[] = {100, 600, 1100, 1600};
int totalMoneyMade;

int globalDataLock;
int senatorOutsideLineLock;
int senatorOutsideLineCV;
int senatorsOutside = 0;
int senatorInsideLock;
bool senatorInside = false;
int customerOutsideLineLock;
int customerOutsideLineCV;
int customersOutside = 0;

bool timeToLeave;

int dummyMV;

typedef enum {BUSY, FREE, BREAK} ClerkState;
typedef enum {APPLICATION_CLERK, PICTURE_CLERK, PASSPORT_CLERK, CASHIER} ClerkType;

typedef struct Customer {
	int money;
	int clerkID;
	int SSN;
	bool didBribe;		
	bool isSenator;
	bool hasApp;
	bool hasPic;
	bool certifiedByPassportClerk;
	bool hasPassport;
	bool hasPaidForPassport;
	bool seenApp;
	bool seenPic;
	bool likedPic;
	bool leftOffice;
} Customer;

typedef struct Clerk {
	int lineLength;
	int bribeLineLength;
	int senatorLineLength;
	int money;
	int customerID;

	ClerkState state;
	ClerkType clerkType;

	int lineLock, bribeLineLock, senatorLineLock, clerkLock, moneyLock;
	int lineCV, bribeLineCV, senatorLineCV, clerkCV, breakCV;
} Clerk;

Customer customers[50];
Clerk cashiers[10];
Clerk passportClerks[10];
Clerk pictureClerks[10];
Clerk applicationClerks[10];

int numCustomers, numApplicationClerks, numPassportClerks, numCashiers, numPictureClerks, numSenators;


void initClerk(ClerkType clerkType, int i) {
	switch(clerkType) {
		case APPLICATION_CLERK:
			applicationClerks[i].lineLength = CreateMV("appLineLength", sizeof("appLineLength"), i);
			applicationClerks[i].bribeLineLength = CreateMV("appBribeLength", sizeof("appBribeLength"), i);
			applicationClerks[i].senatorLineLength = CreateMV("appSenatorLength", sizeof("appSenatorLength"), i);
			applicationClerks[i].money = CreateMV("appMoneyMV", sizeof("appMoneyMV"), i);
			applicationClerks[i].customerID = CreateMV("appCustomerID", sizeof("appCustomerID"), i);
			applicationClerks[i].state = CreateMV("appClerkState", sizeof("appClerkState"), i);
			/*
			applicationClerks[i].lineLength = 0;
			applicationClerks[i].bribeLineLength = 0;
			applicationClerks[i].senatorLineLength = 0;
			applicationClerks[i].money = 0;
			applicationClerks[i].customerID = -1;
			applicationClerks[i].state = BUSY;
			*/
			applicationClerks[i].lineLock = CreateLock("appLine", sizeof("appLine"), i);
			applicationClerks[i].bribeLineLock = CreateLock("appBribe", sizeof("appBribe"), i);
			applicationClerks[i].senatorLineLock = CreateLock("appSenator", sizeof("appSenator"), i);
			applicationClerks[i].clerkLock = CreateLock("appkClerk", sizeof("appkClerk"), i);
			applicationClerks[i].moneyLock = CreateLock("appMoney", sizeof("appMoney"), i);
			applicationClerks[i].lineCV = CreateCondition("appLineCV", sizeof("appLineCV"), i);
			applicationClerks[i].bribeLineCV = CreateCondition("appBribeCV", sizeof("appBribeCV"), i);
			applicationClerks[i].senatorLineCV = CreateCondition("appSenatorCV", sizeof("appSenatorCV"), i);
			applicationClerks[i].clerkCV = CreateCondition("appClerkCV", sizeof("appClerkCV"), i);
			applicationClerks[i].breakCV = CreateCondition("appBreakCV", sizeof("appBreakCV"), i);
			/*SetMV(applicationClerks[i].customerID, -1);*/
			/*SetMV(applicationClerks[i].state, BUSY);*/
			break;
		case PICTURE_CLERK:
			pictureClerks[i].lineLength = CreateMV("picClerkLineLength", sizeof("picClerkLineLength"), i);
			pictureClerks[i].bribeLineLength = CreateMV("picClerkBribeLineLength", sizeof("picClerkBribeLineLength"), i);
			pictureClerks[i].senatorLineLength = CreateMV("picClerkSenatorLineLength", sizeof("picClerkSenatorLineLength"), i);
			pictureClerks[i].money = CreateMV("picClerkMoney", sizeof("picClerkMoney"), i);
			pictureClerks[i].customerID = CreateMV("picClerkCustomerID", sizeof("picClerkCustomerID"), i);
			pictureClerks[i].state = CreateMV("picClerkState", sizeof("picClerkState"), i);
			/*
			pictureClerks[i].lineLength = 0;
			pictureClerks[i].bribeLineLength = 0;
			pictureClerks[i].senatorLineLength = 0;
			pictureClerks[i].money = 0;
			pictureClerks[i].customerID = -1;
			pictureClerks[i].state = BUSY;
			*/
			pictureClerks[i].lineLock = CreateLock("picLine", sizeof("picLine"), i);
			pictureClerks[i].bribeLineLock = CreateLock("picBribe", sizeof("picBribe"), i);
			pictureClerks[i].senatorLineLock = CreateLock("picSenator", sizeof("picSenator"), i);
			pictureClerks[i].clerkLock = CreateLock("picClerk", sizeof("picClerk"), i);
			pictureClerks[i].moneyLock = CreateLock("picMoney", sizeof("picMoney"), i);
			pictureClerks[i].lineCV = CreateCondition("picLineCV", sizeof("picLineCV"), i);
			pictureClerks[i].bribeLineCV = CreateCondition("picBribeCV", sizeof("picBribeCV"), i);
			pictureClerks[i].senatorLineCV = CreateCondition("picSenatorCV", sizeof("picSenatorCV"), i);
			pictureClerks[i].clerkCV = CreateCondition("picClerkCV", sizeof("picClerkCV"), i);
			pictureClerks[i].breakCV = CreateCondition("picBreakCV", sizeof("picBreakCV"), i);
			/*
			SetMV(pictureClerks[i].customerID, -1);
			SetMV(pictureClerks[i].state, BUSY);
			*/
			break;
		case PASSPORT_CLERK:
			passportClerks[i].lineLength = CreateMV("passClerkLineLength", sizeof("passClerkLineLength"), i);
			passportClerks[i].bribeLineLength = CreateMV("passClerkBribeLineLength", sizeof("passClerkBribeLineLength"), i);
			passportClerks[i].senatorLineLength = CreateMV("passClerkSenatorLineLength", sizeof("passClerkSenatorLineLength"), i);
			passportClerks[i].money = CreateMV("passClerkMoney", sizeof("passClerkMoney"), i);
			passportClerks[i].customerID = CreateMV("passClerkCustomerID", sizeof("passClerkCustomerID"), i);
			passportClerks[i].state = CreateMV("passClerkState", sizeof("passClerkState"), i);
			/*
			passportClerks[i].lineLength = 0;
			passportClerks[i].bribeLineLength = 0;
			passportClerks[i].senatorLineLength = 0;
			passportClerks[i].money = 0;
			passportClerks[i].customerID = -1;
			passportClerks[i].state = BUSY;
			*/
			passportClerks[i].lineLock = CreateLock("passLine", sizeof("passLine"), i);
			passportClerks[i].bribeLineLock = CreateLock("passBribe", sizeof("passBribe"), i);
			passportClerks[i].senatorLineLock = CreateLock("passSenator", sizeof("passSenator"), i);
			passportClerks[i].clerkLock = CreateLock("passClerk", sizeof("passClerk"), i);
			passportClerks[i].moneyLock = CreateLock("passMoney", sizeof("passMoney"), i);
			passportClerks[i].lineCV = CreateCondition("passLineCV", sizeof("passLineCV"), i);
			passportClerks[i].bribeLineCV = CreateCondition("passBribeCV", sizeof("passBribeCV"), i);
			passportClerks[i].senatorLineCV = CreateCondition("passSenatorCV", sizeof("passSenatorCV"), i);
			passportClerks[i].clerkCV = CreateCondition("passClerkCV", sizeof("passClerkCV"), i);
			passportClerks[i].breakCV = CreateCondition("passBreakCV", sizeof("passBreakCV"), i);
			/*
			SetMV(passportClerks[i].customerID, -1);
			SetMV(passportClerks[i].state, BUSY);
			*/
			break;
		case CASHIER:
			cashiers[i].lineLength = CreateMV("cashLineLength", sizeof("cashLineLength"), i);
			cashiers[i].bribeLineLength = CreateMV("cashBribeLength", sizeof("cashBribeLength"), i);
			cashiers[i].senatorLineLength = CreateMV("cashSenatorLength", sizeof("cashSenatorLength"), i);
			cashiers[i].money = CreateMV("cashMoney", sizeof("cashMoney"), i);
			cashiers[i].customerID = CreateMV("cashCustomerID", sizeof("cashCustomerID"), i);
			cashiers[i].state = CreateMV("cashState", sizeof("cashState"), i);
			/*
			cashiers[i].lineLength = 0;
			cashiers[i].bribeLineLength = 0;
			cashiers[i].senatorLineLength = 0;
			cashiers[i].money = 0;
			cashiers[i].customerID = -1;
			cashiers[i].state = BUSY;
			*/
			cashiers[i].lineLock = CreateLock("cashLineLock", sizeof("cashLineLock"), i);
			cashiers[i].bribeLineLock = CreateLock("cashBribeLine", sizeof("cashBribeLine"), i);
			cashiers[i].senatorLineLock = CreateLock("cashSenatorLine", sizeof("cashSenatorLine"), i);
			cashiers[i].clerkLock = CreateLock("cashClerkLock", sizeof("cashClerkLock"), i);
			cashiers[i].moneyLock = CreateLock("cashMoneyLock", sizeof("cashMoneyLock"), i);
			cashiers[i].lineCV = CreateCondition("cashLineCV", sizeof("cashLineCV"), i);
			cashiers[i].bribeLineCV = CreateCondition("cashBribeCV", sizeof("cashBribeCV"), i);
			cashiers[i].senatorLineCV = CreateCondition("cashSenatorCV", sizeof("cashSenatorCV"), i);
			cashiers[i].clerkCV = CreateCondition("cashClerkCV", sizeof("cashClerkCV"), i);
			cashiers[i].breakCV = CreateCondition("cashBreakCV", sizeof("cashBreakCV"), i);
			/*
			SetMV(cashiers[i].customerID, -1);
			SetMV(cashiers[i].state, BUSY);
			*/
			break;
	}
}


void initCustomer(int ssn, bool _isSenator) {
	/*
	customers[ssn].isSenator = _isSenator;
    customers[ssn].clerkID = -1;
    customers[ssn].SSN = ssn;
    customers[ssn].money = amounts[(int)(Rand() % 4)];
    customers[ssn].hasApp = false;
    customers[ssn].hasPic = false;
    customers[ssn].certifiedByPassportClerk = false;
    customers[ssn].hasPassport = false;
    customers[ssn].seenApp = false;
    customers[ssn].seenPic = false;
    customers[ssn].likedPic = false;
    customers[ssn].hasPaidForPassport = false;
	
	*/
	customers[ssn].isSenator = CreateMV("custIsSenator", sizeof("custIsSenator"), ssn);
	customers[ssn].clerkID = CreateMV("custClerkID", sizeof("custClerkID"), ssn);
    customers[ssn].SSN = CreateMV("custSSN", sizeof("custSSN"), ssn);
    customers[ssn].money = CreateMV("custMoney", sizeof("custMoney"), ssn);
    customers[ssn].hasApp = CreateMV("custHasApp", sizeof("custHasApp"), ssn);
    customers[ssn].hasPic = CreateMV("custHasPic", sizeof("custHasPic"), ssn);
    customers[ssn].didBribe = CreateMV("custDidBribe", sizeof("custDidBribe"), ssn);
    customers[ssn].certifiedByPassportClerk = CreateMV("custCertified", sizeof("custCertified"), ssn);
    customers[ssn].hasPassport = CreateMV("custHasPassport", sizeof("custHasPassport"), ssn);
    customers[ssn].seenApp = CreateMV("custSeenApp", sizeof("custSeenApp"), ssn);
    customers[ssn].seenPic = CreateMV("custSeenPic", sizeof("custSeenPic"), ssn);
    customers[ssn].likedPic = CreateMV("custLikedPic", sizeof("custLikedPic"), ssn);
    customers[ssn].hasPaidForPassport = CreateMV("custHasPaid", sizeof("custHasPaid"), ssn);
    customers[ssn].leftOffice = CreateMV("leftOffice", sizeof("leftOffice"), ssn);

    SetMV(customers[ssn].isSenator, _isSenator);
    SetMV(customers[ssn].clerkID, -1);
    SetMV(customers[ssn].SSN, ssn);
    SetMV(customers[ssn].money, amounts[(int)(Rand() % 4)]);
    
    
}

void setup() {
	int k;

	numCustomers = NUM_CUSTOMERS;
	numApplicationClerks = NUM_APPLICATION_CLERKS;
	numPictureClerks = NUM_PICTURE_CLERKS;
	numPassportClerks = NUM_PASSPORT_CLERKS;
	numCashiers = NUM_CASHIERS;
	numSenators = NUM_SENATORS;
	
	dummyMV = CreateMV("dummyMV", sizeof("dummyMV"));
	globalDataLock = CreateLock("globalDataLock", sizeof("globalDataLock"));
	timeToLeave = CreateMV("timeToLeave", sizeof("timeToLeave"));

	
	senatorOutsideLineLock = CreateLock("senOutsideLineLock", sizeof("senOutsideLineLock"));
	senatorOutsideLineCV = CreateCondition("senOutsideLineCV", sizeof("senOutsideLineCV"));
	senatorInside = CreateMV("senInside", sizeof("senInside"));
	senatorInsideLock = CreateLock("senInsideLock", sizeof("senInsideLock"));
	senatorsOutside = CreateMV("sensOutside", sizeof("sensOutside"));
	customerOutsideLineLock = CreateLock("custOutsideLineLock", sizeof("custOutsideLineLock"));
	customerOutsideLineCV = CreateCondition("custOutsideLineCV", sizeof("custOutsideLineCV"));
	customersOutside = CreateMV("custsOutside", sizeof("custsOutside"));
	
	customerIndexLock = CreateLock("custIndexLock", sizeof("custIndexLock"));
	applicationClerkIndexLock = CreateLock("appClerkIndexLock", sizeof("appClerkIndexLock"));
	pictureClerkIndexLock = CreateLock("picClerkIndexLock", sizeof("picClerkIndexLock"));
	passportClerkIndexLock = CreateLock("passClerkIndexLock", sizeof("passClerkIndexLock"));
	cashierIndexLock = CreateLock("cashierIndexLock", sizeof("cashierIndexLock"));
	
	nextAvailableCustomerIndex = CreateMV("custIndex", sizeof("custIndex"));
	nextAvailablePictureClerkIndex = CreateMV("nextPicClerkIndex", sizeof("nextPicClerkIndex"));
	nextAvailablePassportClerkIndex = CreateMV("nextPassClerkIndex", sizeof("nextPassClerkIndex"));
	nextAvailableCashierIndex = CreateMV("nextCashIndex", sizeof("nextCashIndex"));
	nextAvailableApplicationClerkIndex = CreateMV("nextAppClerkIndex", sizeof("nextAppClerkIndex"));
	
	SetMV(senatorInside, 0);

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













