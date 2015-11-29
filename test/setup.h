#include "syscall.h"

typedef int bool;
#define true 1
#define false 0

#define NUM_CUSTOMERS			10
#define NUM_APPLICATION_CLERKS	1
#define NUM_PICTURE_CLERKS		1
#define NUM_PASSPORT_CLERKS		1
#define NUM_CASHIERS			1
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

char* concatenate(char* str, int num) {
	char numstr = (char)(((int) '0') + num);
	int len = sizeof(str);
	str[len] = numstr;
	str[len + 1] = '\0';
	return str;
}

void initClerk(ClerkType clerkType, int i) {
	switch(clerkType) {
		case APPLICATION_CLERK:
			applicationClerks[i].lineLength = CreateMV(concatenate("applicationClerkLineLength", i), sizeof("applicationClerkLineLength") + 1);
			applicationClerks[i].bribeLineLength = CreateMV(concatenate("applicationClerkBribeLineLength", i), sizeof("applicationClerkBribeLineLength") + 1);
			applicationClerks[i].senatorLineLength = CreateMV(concatenate("applicationClerkSenatorLineLength", i), sizeof("applicationClerkSenatorLineLength") + 1);
			applicationClerks[i].money = CreateMV(concatenate("applicationClerkMoney", i), sizeof("applicationClerkMoney") + 1);
			applicationClerks[i].customerID = CreateMV(concatenate("applicationClerkCustomerID", i), sizeof("applicationClerkCustomerID") + 1);
			applicationClerks[i].state = CreateMV(concatenate("applicationClerkState", i), sizeof("applicationClerkState") + 1);
			/*
			applicationClerks[i].lineLength = 0;
			applicationClerks[i].bribeLineLength = 0;
			applicationClerks[i].senatorLineLength = 0;
			applicationClerks[i].money = 0;
			applicationClerks[i].customerID = -1;
			applicationClerks[i].state = BUSY;
			*/
			applicationClerks[i].lineLock = CreateLock(concatenate("applicationClerkLineLock", i), sizeof("applicationClerkLineLock") + 1);
			applicationClerks[i].bribeLineLock = CreateLock(concatenate("applicationClerkBribeLineLock", i), sizeof("applicationClerkBribeLineLock") + 1);
			applicationClerks[i].senatorLineLock = CreateLock(concatenate("applicationClerkSenatorLineLock", i), sizeof("applicationClerkSenatorLineLock") + 1);
			applicationClerks[i].clerkLock = CreateLock(concatenate("applicationClerkClerkLock", i), sizeof("applicationClerkClerkLock") + 1);
			applicationClerks[i].moneyLock = CreateLock(concatenate("applicationClerkMoneyLock", i), sizeof("applicationClerkMoneyLock") + 1);
			applicationClerks[i].lineCV = CreateCondition(concatenate("applicationClerkLineCV", i), sizeof("applicationClerkLineCV") + 1);
			applicationClerks[i].bribeLineCV = CreateCondition(concatenate("applicationClerkBribeLineCV", i), sizeof("applicationClerkBribeLineCV") + 1);
			applicationClerks[i].senatorLineCV = CreateCondition(concatenate("applicationClerkSenatorLineCV", i), sizeof("applicationClerkSenatorLineCV") + 1);
			applicationClerks[i].clerkCV = CreateCondition(concatenate("applicationClerkClerkCV", i), sizeof("applicationClerkClerkCV") + 1);
			applicationClerks[i].breakCV = CreateCondition(concatenate("applicationClerkBreakCV", i), sizeof("applicationClerkBreakCV") + 1);
			SetMV(applicationClerks[i].customerID, -1);
			SetMV(applicationClerks[i].state, BUSY);
			break;
		case PICTURE_CLERK:
			pictureClerks[i].lineLength = CreateMV(concatenate("pictureClerkLineLength", i), sizeof("pictureClerkLineLength") + 1);
			pictureClerks[i].bribeLineLength = CreateMV(concatenate("pictureClerkBribeLineLength", i), sizeof("pictureClerkBribeLineLength") + 1);
			pictureClerks[i].senatorLineLength = CreateMV(concatenate("pictureClerkSenatorLineLength", i), sizeof("pictureClerkSenatorLineLength") + 1);
			pictureClerks[i].money = CreateMV(concatenate("pictureClerkMoney", i), sizeof("pictureClerkMoney") + 1);
			pictureClerks[i].customerID = CreateMV(concatenate("pictureClerkCustomerID", i), sizeof("pictureClerkCustomerID") + 1);
			pictureClerks[i].state = CreateMV(concatenate("pictureClerkState", i), sizeof("pictureClerkState") + 1);
			/*
			pictureClerks[i].lineLength = 0;
			pictureClerks[i].bribeLineLength = 0;
			pictureClerks[i].senatorLineLength = 0;
			pictureClerks[i].money = 0;
			pictureClerks[i].customerID = -1;
			pictureClerks[i].state = BUSY;
			*/
			pictureClerks[i].lineLock = CreateLock(concatenate("pictureClerkLineLock", i), sizeof("pictureClerkLineLock") + 1);
			pictureClerks[i].bribeLineLock = CreateLock(concatenate("pictureClerkBribeLineLock", i), sizeof("pictureClerkBribeLineLock") + 1);
			pictureClerks[i].senatorLineLock = CreateLock(concatenate("pictureClerkSenatorLineLock", i), sizeof("pictureClerkSenatorLineLock") + 1);
			pictureClerks[i].clerkLock = CreateLock(concatenate("pictureClerkClerkLock", i), sizeof("pictureClerkClerkLock") + 1);
			pictureClerks[i].moneyLock = CreateLock(concatenate("pictureClerkMoneyLock", i), sizeof("pictureClerkMoneyLock") + 1);
			pictureClerks[i].lineCV = CreateCondition(concatenate("pictureClerkLineCV", i), sizeof("pictureClerkLineCV") + 1);
			pictureClerks[i].bribeLineCV = CreateCondition(concatenate("pictureClerkBribeLineCV", i), sizeof("pictureClerkBribeLineCV") + 1);
			pictureClerks[i].senatorLineCV = CreateCondition(concatenate("pictureClerkSenatorLineCV", i), sizeof("pictureClerkSenatorLineCV") + 1);
			pictureClerks[i].clerkCV = CreateCondition(concatenate("pictureClerkClerkCV", i), sizeof("pictureClerkClerkCV") + 1);
			pictureClerks[i].breakCV = CreateCondition(concatenate("pictureClerkBreakCV", i), sizeof("pictureClerkBreakCV") + 1);
			SetMV(pictureClerks[i].customerID, -1);
			SetMV(pictureClerks[i].state, BUSY);
			break;
		case PASSPORT_CLERK:
			passportClerks[i].lineLength = CreateMV(concatenate("passportClerkLineLength", i), sizeof("passportClerkLineLength") + 1);
			passportClerks[i].bribeLineLength = CreateMV(concatenate("passportClerkBribeLineLength", i), sizeof("passportClerkBribeLineLength") + 1);
			passportClerks[i].senatorLineLength = CreateMV(concatenate("passportClerkSenatorLineLength", i), sizeof("passportClerkSenatorLineLength") + 1);
			passportClerks[i].money = CreateMV(concatenate("passportClerkMoney", i), sizeof("passportClerkMoney") + 1);
			passportClerks[i].customerID = CreateMV(concatenate("passportClerkCustomerID", i), sizeof("passportClerkCustomerID") + 1);
			passportClerks[i].state = CreateMV(concatenate("passportClerkState", i), sizeof("passportClerkState") + 1);
			/*
			passportClerks[i].lineLength = 0;
			passportClerks[i].bribeLineLength = 0;
			passportClerks[i].senatorLineLength = 0;
			passportClerks[i].money = 0;
			passportClerks[i].customerID = -1;
			passportClerks[i].state = BUSY;
			*/
			passportClerks[i].lineLock = CreateLock(concatenate("passportClerkLineLock", i), sizeof("passportClerkLineLock") + 1);
			passportClerks[i].bribeLineLock = CreateLock(concatenate("passportClerkBribeLineLock", i), sizeof("passportClerkBribeLineLock") + 1);
			passportClerks[i].senatorLineLock = CreateLock(concatenate("passportClerkSenatorLineLock", i), sizeof("passportClerkSenatorLineLock") + 1);
			passportClerks[i].clerkLock = CreateLock(concatenate("passportClerkClerkLock", i), sizeof("passportClerkClerkLock") + 1);
			passportClerks[i].moneyLock = CreateLock(concatenate("passportClerkMoneyLock", i), sizeof("passportClerkMoneyLock") + 1);
			passportClerks[i].lineCV = CreateCondition(concatenate("passportClerkLineCV", i), sizeof("passportClerkLineCV") + 1);
			passportClerks[i].bribeLineCV = CreateCondition(concatenate("passportClerkBribeLineCV", i), sizeof("passportClerkBribeLineCV") + 1);
			passportClerks[i].senatorLineCV = CreateCondition(concatenate("passportClerkSenatorLineCV", i), sizeof("passportClerkSenatorLineCV") + 1);
			passportClerks[i].clerkCV = CreateCondition(concatenate("passportClerkClerkCV", i), sizeof("passportClerkClerkCV") + 1);
			passportClerks[i].breakCV = CreateCondition(concatenate("passportClerkBreakCV", i), sizeof("passportClerkBreakCV") + 1);
			SetMV(passportClerks[i].customerID, -1);
			SetMV(passportClerks[i].state, BUSY);
			break;
		case CASHIER:
			cashiers[i].lineLength = CreateMV(concatenate("cashiersClerkLineLength", i), sizeof("cashiersClerkLineLength") + 1);
			cashiers[i].bribeLineLength = CreateMV(concatenate("cashiersClerkBribeLineLength", i), sizeof("cashiersClerkBribeLineLength") + 1);
			cashiers[i].senatorLineLength = CreateMV(concatenate("cashiersClerkSenatorLineLength", i), sizeof("cashiersClerkSenatorLineLength") + 1);
			cashiers[i].money = CreateMV(concatenate("cashiersClerkMoney", i), sizeof("cashiersClerkMoney") + 1);
			cashiers[i].customerID = CreateMV(concatenate("cashiersClerkCustomerID", i), sizeof("cashiersClerkCustomerID") + 1);
			cashiers[i].state = CreateMV(concatenate("cashiersClerkState", i), sizeof("cashiersClerkState") + 1);
			/*
			cashiers[i].lineLength = 0;
			cashiers[i].bribeLineLength = 0;
			cashiers[i].senatorLineLength = 0;
			cashiers[i].money = 0;
			cashiers[i].customerID = -1;
			cashiers[i].state = BUSY;
			*/
			cashiers[i].lineLock = CreateLock(concatenate("cashierClerkLineLock", i), sizeof("cashierClerkLineLock") + 1);
			cashiers[i].bribeLineLock = CreateLock(concatenate("cashierClerkBribeLineLock", i), sizeof("cashierClerkBribeLineLock") + 1);
			cashiers[i].senatorLineLock = CreateLock(concatenate("cashierClerkSenatorLineLock", i), sizeof("cashierClerkSenatorLineLock") + 1);
			cashiers[i].clerkLock = CreateLock(concatenate("cashierClerkClerkLock", i), sizeof("cashierClerkClerkLock") + 1);
			cashiers[i].moneyLock = CreateLock(concatenate("cashierClerkMoneyLock", i), sizeof("cashierClerkMoneyLock") + 1);
			cashiers[i].lineCV = CreateCondition(concatenate("cashierClerkLineCV", i), sizeof("cashierClerkLineCV") + 1);
			cashiers[i].bribeLineCV = CreateCondition(concatenate("cashierClerkBribeLineCV", i), sizeof("cashierClerkBribeLineCV") + 1);
			cashiers[i].senatorLineCV = CreateCondition(concatenate("cashierClerkSenatorLineCV", i), sizeof("cashierClerkSenatorLineCV") + 1);
			cashiers[i].clerkCV = CreateCondition(concatenate("cashierClerkClerkCV", i), sizeof("cashierClerkClerkCV") + 1);
			cashiers[i].breakCV = CreateCondition(concatenate("cashierClerkBreakCV", i), sizeof("cashierClerkBreakCV") + 1);
			SetMV(cashiers[i].customerID, -1);
			SetMV(cashiers[i].state, BUSY);
			break;
	}
}

void initCustomer(int ssn, bool _isSenator){
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
}

void setup() {
	int k;
	
	/*
	globalDataLock = CreateLock("globalDataLock", sizeof("globalDataLock"));
	senatorOutsideLineLock = CreateLock("senatorOutsideLineLock", sizeof("senatorOutsideLineLock"));
	senatorOutsideLineCV = CreateCondition("senatorOutsideLineCV", sizeof("senatorOutsideLineCV"));
	senatorInside = CreateMV("senatorInside", sizeof("senatorInside"));
	senatorInsideLock = CreateLock("senatorInsideLock", sizeof("senatorInsideLock"));
	senatorsOutside = CreateMV("senatorsOutside", sizeof("senatorsOutside"));
	customerOutsideLineLock = CreateLock("customerOutsideLineLock", sizeof("customerOutsideLineLock"));
	customerOutsideLineCV = CreateCondition("customerOutsideLineCV", sizeof("customerOutsideLineCV"));
	customersOutside = CreateMV("customersOutside", sizeof("customersOutside"));
	*/
	customerIndexLock = CreateLock("customerIndexLock", sizeof("customerIndexLock"));
	/*
	applicationClerkIndexLock = CreateLock("applicationClerkIndexLock", sizeof("applicationClerkIndexLock"));
	pictureClerkIndexLock = CreateLock("pictureClerkIndexLock", sizeof("pictureClerkIndexLock"));
	passportClerkIndexLock = CreateLock("passportClerkIndexLock", sizeof("passportClerkIndexLock"));
	cashierIndexLock = CreateLock("cashierIndexLock", sizeof("cashierIndexLock"));
	*/
	nextAvailableCustomerIndex = CreateMV("customerIndex", sizeof("customerIndex"));
	/*
	nextAvailablePictureClerkIndex = CreateMV("nextAvailablePictureClerkIndex", sizeof("nextAvailablePictureClerkIndex"));
	nextAvailablePassportClerkIndex = CreateMV("nextAvailablePassportClerkIndex", sizeof("nextAvailablePassportClerkIndex"));
	nextAvailableCashierIndex = CreateMV("nextAvailableCashierIndex", sizeof("nextAvailableCashierIndex"));
	nextAvailableApplicationClerkIndex = CreateMV("nextAvailableApplicationClerkIndex", sizeof("nextAvailableApplicationClerkIndex"));
	
	SetMV(nextAvailableCustomerIndex, 0);
	SetMV(nextAvailablePictureClerkIndex, 0);
	SetMV(nextAvailablePassportClerkIndex, 0);
	SetMV(nextAvailableCashierIndex, 0);
	SetMV(nextAvailableApplicationClerkIndex, 0);

	Print("All locks initialized\n", 0);

	for(k = 0; k < NUM_APPLICATION_CLERKS; k++)
		initClerk(APPLICATION_CLERK, k);
	
	for(k = 0; k < NUM_PICTURE_CLERKS; k++)
		initClerk(PICTURE_CLERK, k);
	
	for(k = 0; k < NUM_PASSPORT_CLERKS; k++)
		initClerk(PASSPORT_CLERK, k);
	
	for(k = 0; k < NUM_CASHIERS; k++)
		initClerk(CASHIER, k);
	*/
	for(k = 0; k < NUM_CUSTOMERS; k++)
		initCustomer(k, false);
	/*
	for(k = 0; k < NUM_SENATORS; k++)
		initCustomer(k, true);
	*/
}













