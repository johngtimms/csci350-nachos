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
			applicationClerks[i].lineLength = 0;
			applicationClerks[i].bribeLineLength = 0;
			applicationClerks[i].senatorLineLength = 0;
			applicationClerks[i].money = 0;
			applicationClerks[i].state = BUSY;
			applicationClerks[i].lineLock = CreateLock(concatenate("applicationClerkLineLock", i));
			applicationClerks[i].bribeLineLock = CreateLock(concatenate("applicationClerkBribeLineLock", i));
			applicationClerks[i].senatorLineLock = CreateLock(concatenate("applicationClerkSenatorLineLock", i));
			applicationClerks[i].clerkLock = CreateLock(concatenate("applicationClerkClerkLock", i));
			applicationClerks[i].moneyLock = CreateLock(concatenate("applicationClerkMoneyLock", i));
			applicationClerks[i].lineCV = CreateCondition(concatenate("applicationClerkLineCV", i));
			applicationClerks[i].bribeLineCV = CreateCondition(concatenate("applicationClerkBribeLineCV", i));
			applicationClerks[i].senatorLineCV = CreateCondition(concatenate("applicationClerkSenatorLineCV", i));
			applicationClerks[i].clerkCV = CreateCondition(concatenate("applicationClerkClerkCV", i));
			applicationClerks[i].breakCV = CreateCondition(concatenate("applicationClerkBreakCV", i));
			break;
		case PICTURE_CLERK:
			pictureClerks[i].lineLength = 0;
			pictureClerks[i].bribeLineLength = 0;
			pictureClerks[i].senatorLineLength = 0;
			pictureClerks[i].money = 0;
			pictureClerks[i].state = BUSY;
			pictureClerks[i].lineLock = CreateLock(concatenate("pictureClerkLineLock", i));
			pictureClerks[i].bribeLineLock = CreateLock(concatenate("pictureClerkBribeLineLock", i));
			pictureClerks[i].senatorLineLock = CreateLock(concatenate("pictureClerkSenatorLineLock", i));
			pictureClerks[i].clerkLock = CreateLock(concatenate("pictureClerkClerkLock", i));
			pictureClerks[i].moneyLock = CreateLock(concatenate("pictureClerkMoneyLock", i));
			pictureClerks[i].lineCV = CreateCondition(concatenate("pictureClerkLineCV", i));
			pictureClerks[i].bribeLineCV = CreateCondition(concatenate("pictureClerkBribeLineCV", i));
			pictureClerks[i].senatorLineCV = CreateCondition(concatenate("pictureClerkSenatorLineCV", i));
			pictureClerks[i].clerkCV = CreateCondition(concatenate("pictureClerkClerkCV", i));
			pictureClerks[i].breakCV = CreateCondition(concatenate("pictureClerkBreakCV", i));
			break;
		case PASSPORT_CLERK:
			passportClerks[i].lineLength = 0;
			passportClerks[i].bribeLineLength = 0;
			passportClerks[i].senatorLineLength = 0;
			passportClerks[i].money = 0;
			passportClerks[i].state = BUSY;
			passportClerks[i].lineLock = CreateLock(concatenate("passportClerkLineLock", i));
			passportClerks[i].bribeLineLock = CreateLock(concatenate("passportClerkBribeLineLock", i));
			passportClerks[i].senatorLineLock = CreateLock(concatenate("passportClerkSenatorLineLock", i));
			passportClerks[i].clerkLock = CreateLock(concatenate("passportClerkClerkLock", i));
			passportClerks[i].moneyLock = CreateLock(concatenate("passportClerkMoneyLock", i));
			passportClerks[i].lineCV = CreateCondition(concatenate("passportClerkLineCV", i));
			passportClerks[i].bribeLineCV = CreateCondition(concatenate("passportClerkBribeLineCV", i));
			passportClerks[i].senatorLineCV = CreateCondition(concatenate("passportClerkSenatorLineCV", i));
			passportClerks[i].clerkCV = CreateCondition(concatenate("passportClerkClerkCV", i));
			passportClerks[i].breakCV = CreateCondition(concatenate("passportClerkBreakCV", i));
			break;
		case CASHIER:
			cashiers[i].lineLength = 0;
			cashiers[i].bribeLineLength = 0;
			cashiers[i].senatorLineLength = 0;
			cashiers[i].money = 0;
			cashiers[i].state = BUSY;
			cashiers[i].lineLock = CreateLock(concatenate("cashierClerkLineLock", i));
			cashiers[i].bribeLineLock = CreateLock(concatenate("cashierClerkBribeLineLock", i));
			cashiers[i].senatorLineLock = CreateLock(concatenate("cashierClerkSenatorLineLock", i));
			cashiers[i].clerkLock = CreateLock(concatenate("cashierClerkClerkLock", i));
			cashiers[i].moneyLock = CreateLock(concatenate("cashierClerkMoneyLock", i));
			cashiers[i].lineCV = CreateCondition(concatenate("cashierClerkLineCV", i));
			cashiers[i].bribeLineCV = CreateCondition(concatenate("cashierClerkBribeLineCV", i));
			cashiers[i].senatorLineCV = CreateCondition(concatenate("cashierClerkSenatorLineCV", i));
			cashiers[i].clerkCV = CreateCondition(concatenate("cashierClerkClerkCV", i));
			cashiers[i].breakCV = CreateCondition(concatenate("cashierClerkBreakCV", i));
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
	globalDataLock = CreateLock("globalDataLock", 14);
	senatorOutsideLineLock = CreateLock("senatorOutsideLineLock", 22);
	senatorOutsideLineCV = CreateCondition("senatorOutsideLineCV", 20);
	senatorInsideLock = CreateLock("senatorInsideLock", 17);
	senatorsOutside = CreateMV("senatorsOutside", 15);
	customerOutsideLineLock = CreateLock("customerOutsideLineLock",23);
	customerOutsideLineCV = CreateCondition("customerOutsideLineCV", 21);
	customersOutside = CreateMV("customersOutside", 16);
	*/
	customerIndexLock = CreateLock("customerIndexLock",17);
	/*
	applicationClerkIndexLock = CreateLock("applicationClerkIndexLock", 25);
	pictureClerkIndexLock = CreateLock("pictureClerkIndexLock", 21);
	passportClerkIndexLock = CreateLock("passportClerkIndexLock", 22);
	cashierIndexLock = CreateLock("cashierIndexLock", 16);
	*/
	nextAvailableCustomerIndex = CreateMV("customerIndex",13);
	/*
	nextAvailablePictureClerkIndex = CreateMV("nextAvailablePictureClerkIndex", 30);
	nextAvailablePassportClerkIndex = CreateMV("nextAvailablePassportClerkIndex", 31);
	nextAvailableCashierIndex = CreateMV("nextAvailableCashierIndex", 25);
	nextAvailableApplicationClerkIndex = CreateMV("nextAvailableApplicationClerkIndex", 34);
	
	SetMV(nextAvailableCustomerIndex, 0);
	SetMV(nextAvailablePictureClerkIndex, 0);
	SetMV(nextAvailablePassportClerkIndex, 0);
	SetMV(nextAvailableCashierIndex, 0);
	SetMV(nextAvailableApplicationClerkIndex, 0);

	Print("All locks initialized\n", 0);

	for(k = 0; k < NUM_APPLICATION_CLERKS; k++)
		initClerk(APPLICATION_CLERK,k);
	
	for(k = 0; k < NUM_PICTURE_CLERKS; k++)
		initClerk(PICTURE_CLERK,k);
	
	for(k = 0; k < NUM_PASSPORT_CLERKS; k++)
		initClerk(PASSPORT_CLERK,k);
	
	for(k = 0; k < NUM_CASHIERS; k++)
		initClerk(CASHIER,k);
	*/
	for(k = 0; k < NUM_CUSTOMERS; k++)
		initCustomer(k, false);
	/*
	for(k = 0; k < NUM_SENATORS; k++)
		initCustomer(k, true);
	*/
}













