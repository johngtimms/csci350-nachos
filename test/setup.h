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

int senatorOutsideLineLock;
int senatorOutsideLineCV;
int senatorsOutside = 0;
int senatorInsideLock;
bool senatorInside = false;
int customerOutsideLineLock;
int customerOutsideLineCV;
int customersOutside = 0;
int numCustomers, numApplicationClerks, numPictureClerks, numPassportClerks, numCashiers, numSenators;
int nextAvailableCustomerIndex = 0;
int nextAvailablePictureClerkIndex = 0;
int nextAvailablePassportClerkIndex = 0;
int nextAvailableCashierIndex = 0;
int nextAvailableApplicationClerkIndex = 0;

int customerIndexLock, applicationClerkIndexLock, pictureClerkIndexLock, passportClerkIndexLock, cashierIndexLock, senatorIndexLock;
int totalMoneyMade;

bool runningTest1 = false;
bool runningTest2 = false;
bool runningTest3 = false;
bool runningTest4 = false;
bool runningTest5 = false;
bool runningTest6 = false;
bool runningTest7a = false;
bool runningTest7b = false;

typedef enum {FREE, BUSY, BREAK} ClerkState;
typedef enum {APPLICATION_CLERK, PICTURE_CLERK, PASSPORT_CLERK, CASHIER} ClerkType;
int amounts[] = {100, 600, 1100, 1600}; /*random amounts of money customer can start with */


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



void initGlobalVariables() {
	int k;
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
	
	numCustomers = CreateMV();
	numApplicationClerks = CreateMV();
	numPictureClerks = CreateMV();
	numPassportClerks = CreateMV();
	numCashiers = CreateMV();
	numSenators = CreateMV();

	SetMV(numCustomers, NUM_CUSTOMERS);
	SetMV(numApplicationClerks, NUM_APPLICATION_CLERKS);
	SetMV(numPictureClerks, NUM_PICTURE_CLERKS);
	SetMV(numPassportClerks, NUM_PASSPORT_CLERKS);
	SetMV(numCashiers, NUM_CASHIERS);
	SetMV(numSenators, NUM_SENATORS);

}














