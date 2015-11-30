#include "syscall.h"

typedef int bool;
#define true 1
#define false 0

#define NUM_CUSTOMERS			3
#define NUM_APPLICATION_CLERKS	1
#define NUM_PICTURE_CLERKS		0
#define NUM_PASSPORT_CLERKS		0
#define NUM_CASHIERS			0
#define NUM_SENATORS			0

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

typedef enum {FREE, BUSY, BREAK} ClerkState;
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

int numCustomers, numApplicationClerks, numPassportClerks, numCashiers, numPictureClerks;


void initClerk(ClerkType clerkType, int i) {
	switch(clerkType) {
		case APPLICATION_CLERK:
			applicationClerks[i].lineLength = CreateMV("appClerkLineLength", sizeof("appClerkLineLength"), i);
			applicationClerks[i].bribeLineLength = CreateMV("appClerkBribeLineLength", sizeof("appClerkBribeLineLength"), i);
			applicationClerks[i].senatorLineLength = CreateMV("appClerkSenatorLineLength", sizeof("appClerkSenatorLineLength"), i);
			applicationClerks[i].money = CreateMV("appClerkMoney", sizeof("appClerkMoney"), i);
			applicationClerks[i].customerID = CreateMV("appClerkCustomerID", sizeof("appClerkCustomerID"), i);
			applicationClerks[i].state = CreateMV("appClerkState", sizeof("appClerkState"), i);
			/*
			applicationClerks[i].lineLength = 0;
			applicationClerks[i].bribeLineLength = 0;
			applicationClerks[i].senatorLineLength = 0;
			applicationClerks[i].money = 0;
			applicationClerks[i].customerID = -1;
			applicationClerks[i].state = BUSY;
			*/
			applicationClerks[i].lineLock = CreateLock("appClerkLineLock", sizeof("appClerkLineLock"), i);
			applicationClerks[i].bribeLineLock = CreateLock("appClerkBribeLine", sizeof("appClerkBribeLine"), i);
			applicationClerks[i].senatorLineLock = CreateLock("appClerkSenatorLineLock", sizeof("appClerkSenatorLineLock"), i);
			applicationClerks[i].clerkLock = CreateLock("appClerkClerkLock", sizeof("appClerkClerkLock"), i);
			applicationClerks[i].moneyLock = CreateLock("appClerkMoneyLock", sizeof("appClerkMoneyLock"), i);
			applicationClerks[i].lineCV = CreateCondition("appClerkLineCV", sizeof("appClerkLineCV"), i);
			applicationClerks[i].bribeLineCV = CreateCondition("appClerkBribeLineCV", sizeof("appClerkBribeLineCV"), i);
			applicationClerks[i].senatorLineCV = CreateCondition("appClerkSenatorLineCV", sizeof("appClerkSenatorLineCV"), i);
			applicationClerks[i].clerkCV = CreateCondition("appClerkClerkCV", sizeof("appClerkClerkCV"), i);
			applicationClerks[i].breakCV = CreateCondition("appClerkBreakCV", sizeof("appClerkBreakCV"), i);
			SetMV(applicationClerks[i].customerID, -1);
			SetMV(applicationClerks[i].state, BUSY);
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
			pictureClerks[i].lineLock = CreateLock("picClerkLineLock", sizeof("picClerkLineLock"), i);
			pictureClerks[i].bribeLineLock = CreateLock("picClerkBribeLine", sizeof("picClerkBribeLine"), i);
			pictureClerks[i].senatorLineLock = CreateLock("picClerkSenatorLineLock", sizeof("picClerkSenatorLineLock"), i);
			pictureClerks[i].clerkLock = CreateLock("picClerkClerkLock", sizeof("picClerkClerkLock"), i);
			pictureClerks[i].moneyLock = CreateLock("picClerkMoneyLock", sizeof("picClerkMoneyLock"), i);
			pictureClerks[i].lineCV = CreateCondition("picClerkLineCV", sizeof("picClerkLineCV"), i);
			pictureClerks[i].bribeLineCV = CreateCondition("picClerkBribeLineCV", sizeof("picClerkBribeLineCV"), i);
			pictureClerks[i].senatorLineCV = CreateCondition("picClerkSenatorLineCV", sizeof("picClerkSenatorLineCV"), i);
			pictureClerks[i].clerkCV = CreateCondition("picClerkClerkCV", sizeof("picClerkClerkCV"), i);
			pictureClerks[i].breakCV = CreateCondition("picClerkBreakCV", sizeof("picClerkBreakCV"), i);
			SetMV(pictureClerks[i].customerID, -1);
			SetMV(pictureClerks[i].state, BUSY);
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
			passportClerks[i].lineLock = CreateLock("passClerkLineLock", sizeof("passClerkLineLock"), i);
			passportClerks[i].bribeLineLock = CreateLock("passClerkBribeLine", sizeof("passClerkBribeLine"), i);
			passportClerks[i].senatorLineLock = CreateLock("passClerkSenatorLineLock", sizeof("passClerkSenatorLineLock"), i);
			passportClerks[i].clerkLock = CreateLock("passClerkClerkLock", sizeof("passClerkClerkLock"), i);
			passportClerks[i].moneyLock = CreateLock("passClerkMoneyLock", sizeof("passClerkMoneyLock"), i);
			passportClerks[i].lineCV = CreateCondition("passClerkLineCV", sizeof("passClerkLineCV"), i);
			passportClerks[i].bribeLineCV = CreateCondition("passClerkBribeLineCV", sizeof("passClerkBribeLineCV"), i);
			passportClerks[i].senatorLineCV = CreateCondition("passClerkSenatorLineCV", sizeof("passClerkSenatorLineCV"), i);
			passportClerks[i].clerkCV = CreateCondition("passClerkClerkCV", sizeof("passClerkClerkCV"), i);
			passportClerks[i].breakCV = CreateCondition("passClerkBreakCV", sizeof("passClerkBreakCV"), i);
			SetMV(passportClerks[i].customerID, -1);
			SetMV(passportClerks[i].state, BUSY);
			break;
		case CASHIER:
			cashiers[i].lineLength = CreateMV("cashLineLength", sizeof("cashLineLength"), i);
			cashiers[i].bribeLineLength = CreateMV("cashBribeLineLength", sizeof("cashBribeLineLength"), i);
			cashiers[i].senatorLineLength = CreateMV("cashSenatorLineLength", sizeof("cashSenatorLineLength"), i);
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
			cashiers[i].bribeLineLock = CreateLock("cashBribeLineLock", sizeof("cashBribeLineLock"), i);
			cashiers[i].senatorLineLock = CreateLock("cashSenatorLineLock", sizeof("cashSenatorLineLock"), i);
			cashiers[i].clerkLock = CreateLock("cashClerkLock", sizeof("cashClerkLock"), i);
			cashiers[i].moneyLock = CreateLock("cashMoneyLock", sizeof("cashMoneyLock"), i);
			cashiers[i].lineCV = CreateCondition("cashLineCV", sizeof("cashLineCV"), i);
			cashiers[i].bribeLineCV = CreateCondition("cashBribeLineCV", sizeof("cashBribeLineCV"), i);
			cashiers[i].senatorLineCV = CreateCondition("cashSenatorLineCV", sizeof("cashSenatorLineCV"), i);
			cashiers[i].clerkCV = CreateCondition("cashClerkCV", sizeof("cashClerkCV"), i);
			cashiers[i].breakCV = CreateCondition("cashBreakCV", sizeof("cashBreakCV"), i);
			SetMV(cashiers[i].customerID, -1);
			SetMV(cashiers[i].state, BUSY);
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
    customers[ssn].certifiedByPassportClerk = CreateMV("custCertified", sizeof("custCertified"), ssn);
    customers[ssn].hasPassport = CreateMV("custHasPassport", sizeof("custHasPassport"), ssn);
    customers[ssn].seenApp = CreateMV("custSeenApp", sizeof("custSeenApp"), ssn);
    customers[ssn].seenPic = CreateMV("custSeenPic", sizeof("custSeenPic"), ssn);
    customers[ssn].likedPic = CreateMV("custLikedPic", sizeof("custLikedPic"), ssn);
    customers[ssn].hasPaidForPassport = CreateMV("custHasPaid", sizeof("custHasPaid"), ssn);
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
	
	globalDataLock = CreateLock("globalDataLock", sizeof("globalDataLock"));

	
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
	nextAvailableApplicationClerkIndex = CreateMV("nextAppClerkIndex", sizeof("nextppClerkIndex"));
	
	SetMV(nextAvailableCustomerIndex, 0);
	
	SetMV(nextAvailablePictureClerkIndex, 0);
	SetMV(nextAvailablePassportClerkIndex, 0);
	SetMV(nextAvailableCashierIndex, 0);
	SetMV(nextAvailableApplicationClerkIndex, 0);


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
	/*
	for(k = 0; k < NUM_SENATORS; k++)
		initCustomer(k, true);
		*/

	
}













