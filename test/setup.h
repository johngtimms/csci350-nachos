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

int numCustomers, numApplicationClerks, numPictureClerks, numPassportClerks, numCashiers, numSenators;

int nextAvailableCustomerIndex = 0; 
int nextAvailablePictureClerkIndex = 0; 
int nextAvailablePassportClerkIndex = 0;
int nextAvailableCashierIndex = 0;
int nextAvailableApplicationClerkIndex = 0;
int customerIndexLock, applicationClerkIndexLock, pictureClerkIndexLock, passportClerkIndexLock, cashierIndexLock, senatorIndexLock;

int amounts[] = {100, 600, 1100, 1600};
int totalMoneyMade;

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


void initClerk(ClerkType clerkType, int i) {
	switch(clerkType) {
		case APPLICATION_CLERK:
			applicationClerks[i].lineLength = 0;
			applicationClerks[i].bribeLineLength = 0;
			applicationClerks[i].senatorLineLength = 0;
			applicationClerks[i].money = 0;
			applicationClerks[i].state = BUSY;
			applicationClerks[i].lineLock = CreateLock();
			applicationClerks[i].bribeLineLock = CreateLock();
			applicationClerks[i].senatorLineLock = CreateLock();
			applicationClerks[i].clerkLock = CreateLock();
			applicationClerks[i].moneyLock = CreateLock();
			applicationClerks[i].lineCV = CreateCondition();
			applicationClerks[i].bribeLineCV = CreateCondition();
			applicationClerks[i].senatorLineCV = CreateCondition();
			applicationClerks[i].clerkCV = CreateCondition();
			applicationClerks[i].breakCV = CreateCondition();
			applicationClerks[i].clerkType = clerkType;
			break;
		case PICTURE_CLERK:
			pictureClerks[i].lineLength = 0;
			pictureClerks[i].bribeLineLength = 0;
			pictureClerks[i].senatorLineLength = 0;
			pictureClerks[i].money = 0;
			pictureClerks[i].state = BUSY;
			pictureClerks[i].lineLock = CreateLock();
			pictureClerks[i].bribeLineLock = CreateLock();
			pictureClerks[i].senatorLineLock = CreateLock();
			pictureClerks[i].clerkLock = CreateLock();
			pictureClerks[i].moneyLock = CreateLock();
			pictureClerks[i].lineCV = CreateCondition();
			pictureClerks[i].bribeLineCV = CreateCondition();
			pictureClerks[i].senatorLineCV = CreateCondition();
			pictureClerks[i].clerkCV = CreateCondition();
			pictureClerks[i].breakCV = CreateCondition();
			pictureClerks[i].clerkType = clerkType;
			break;
		case PASSPORT_CLERK:
			passportClerks[i].lineLength = 0;
			passportClerks[i].bribeLineLength = 0;
			passportClerks[i].senatorLineLength = 0;
			passportClerks[i].money = 0;
			passportClerks[i].state = BUSY;
			passportClerks[i].lineLock = CreateLock();
			passportClerks[i].bribeLineLock = CreateLock();
			passportClerks[i].senatorLineLock = CreateLock();
			passportClerks[i].clerkLock = CreateLock();
			passportClerks[i].moneyLock = CreateLock();
			passportClerks[i].lineCV = CreateCondition();
			passportClerks[i].bribeLineCV = CreateCondition();
			passportClerks[i].senatorLineCV = CreateCondition();
			passportClerks[i].clerkCV = CreateCondition();
			passportClerks[i].breakCV = CreateCondition();
			passportClerks[i].clerkType = clerkType;
			break;
		case CASHIER:
			cashiers[i].lineLength = 0;
			cashiers[i].bribeLineLength = 0;
			cashiers[i].senatorLineLength = 0;
			cashiers[i].money = 0;
			cashiers[i].state = BUSY;
			cashiers[i].lineLock = CreateLock();
			cashiers[i].bribeLineLock = CreateLock();
			cashiers[i].senatorLineLock = CreateLock();
			cashiers[i].clerkLock = CreateLock();
			cashiers[i].moneyLock = CreateLock();
			cashiers[i].lineCV = CreateCondition();
			cashiers[i].bribeLineCV = CreateCondition();
			cashiers[i].senatorLineCV = CreateCondition();
			cashiers[i].clerkCV = CreateCondition();
			cashiers[i].breakCV = CreateCondition();
			cashiers[i].clerkType = clerkType;
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
	senatorOutsideLineLock = CreateLock();
	senatorOutsideLineCV = CreateCondition();
	senatorInsideLock = CreateLock();
	customerOutsideLineLock = CreateLock();
	customerOutsideLineCV = CreateCondition();

	customerIndexLock = CreateLock();
	applicationClerkIndexLock = CreateLock();
	pictureClerkIndexLock = CreateLock();
	passportClerkIndexLock = CreateLock();
	cashierIndexLock = CreateLock();

	nextAvailableCustomerIndex = CreateMV();
	nextAvailableApplicationClerkIndex = CreateMV();
	nextAvailablePictureClerkIndex = CreateMV();
	nextAvailablePassportClerkIndex = CreateMV();
	nextAvailableCashierIndex = CreateMV();

}












