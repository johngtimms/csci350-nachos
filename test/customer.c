#include "syscall.h"
#include "setup.h"

int ssn;

bool enterApplicationLine(int clerkID) { 
	/* Stand in line */
	/* Print("Customer %i about to stand in application clerk line\n", ssn); */
	if(applicationClerks[clerkID].state != FREE) {
		if(customers[ssn].isSenator) {
			Acquire(applicationClerks[clerkID].senatorLineLock);
			applicationClerks[clerkID].senatorLineLength++;
			Print("Senator %i has gotten in senator line for ", ssn);
			Print("ApplicationClerk %i\n", clerkID);
			Wait(applicationClerks[clerkID].senatorLineCV, applicationClerks[clerkID].senatorLineLock);
			applicationClerks[clerkID].senatorLineLength--;
		} else if(customers[ssn].didBribe) {
			Acquire(applicationClerks[clerkID].bribeLineLock);
			applicationClerks[clerkID].bribeLineLength++;
			Print("Customer %i has gotten in bribe line for ", ssn);
			Print("ApplicationClerk %i\n", clerkID);
			Wait(applicationClerks[clerkID].bribeLineCV, applicationClerks[clerkID].bribeLineLock);
			applicationClerks[clerkID].bribeLineLength--;
		} else {
			Acquire(applicationClerks[clerkID].lineLock);
			applicationClerks[clerkID].lineLength++;
			Print("Customer %i has gotten in regular line for ", ssn);
			Print("ApplicationClerk %i\n", clerkID);
			Wait(applicationClerks[clerkID].lineCV, applicationClerks[clerkID].lineLock);
			applicationClerks[clerkID].lineLength--;
		}
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock);
	if(senatorInside && !customers[ssn].isSenator) {
		Release(senatorInsideLock);
		if(customers[ssn].didBribe)
			Release(applicationClerks[clerkID].bribeLineLock);
		else
			Release(applicationClerks[clerkID].lineLock);
		Acquire(customerOutsideLineLock);
		Print("Customer %i is going outside the Passport Office because there is a Senator present.\n", ssn);
		customersOutside += 1;
		Wait(customerOutsideLineCV, customerOutsideLineLock);
		customersOutside -= 1;
		Release(customerOutsideLineLock);
		return true;
	} else {
		Release(senatorInsideLock);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		applicationClerks[clerkID].state = BUSY;
		if(customers[ssn].isSenator)
			Release(applicationClerks[clerkID].senatorLineLock);
		else if(customers[ssn].didBribe)
			Release(applicationClerks[clerkID].bribeLineLock);
		else
			Release(applicationClerks[clerkID].lineLock);
		return false;
	}
}

bool enterPictureLine(int clerkID) { 
	/* Stand in line */
	if(pictureClerks[clerkID].state != FREE) {
		if(customers[ssn].isSenator) {
			Acquire(pictureClerks[clerkID].senatorLineLock);
			pictureClerks[clerkID].senatorLineLength++;
			Print("Senator %i has gotten in senator line for ", ssn);
			Print("PictureClerk %i\n", clerkID);
			Wait(pictureClerks[clerkID].senatorLineCV, pictureClerks[clerkID].senatorLineLock);
			pictureClerks[clerkID].senatorLineLength--;
		} else if (customers[ssn].didBribe) {
			Acquire(pictureClerks[clerkID].bribeLineLock);
			pictureClerks[clerkID].bribeLineLength++;
			Print("Customer %i has gotten in bribe line for ", ssn);
			Print("PictureClerk %i\n", clerkID);
			Wait(pictureClerks[clerkID].bribeLineCV, pictureClerks[clerkID].bribeLineLock);
			pictureClerks[clerkID].bribeLineLength--;
		} else {
			Acquire(pictureClerks[clerkID].lineLock);
			pictureClerks[clerkID].lineLength++;
			Print("Customer %i has gotten in regular line for ", ssn);
			Print("PictureClerk %i\n", clerkID);
			Wait(pictureClerks[clerkID].lineCV, pictureClerks[clerkID].lineLock);
			pictureClerks[clerkID].lineLength--;
		}
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock);
	if(senatorInside && !customers[ssn].isSenator) {
		Release(senatorInsideLock);
		if(customers[ssn].didBribe)
			Release(pictureClerks[clerkID].bribeLineLock);
		else
			Release(pictureClerks[clerkID].lineLock);
		Acquire(customerOutsideLineLock);
		Print("Customer %i is going outside the Passport Office because there is a Senator present.\n", ssn);
		customersOutside += 1;
		Wait(customerOutsideLineCV, customerOutsideLineLock);
		customersOutside -= 1;
		Release(customerOutsideLineLock);
		return true;
	} else {
		Release(senatorInsideLock);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		pictureClerks[clerkID].state = BUSY;
		if(customers[ssn].isSenator)
			Release(pictureClerks[clerkID].senatorLineLock);
		else if(customers[ssn].didBribe)
			Release(pictureClerks[clerkID].bribeLineLock);
		else
			Release(pictureClerks[clerkID].lineLock);
		return false;
	}
}

bool enterPassportLine(int clerkID) { 
	/* Stand in line */
	if(passportClerks[clerkID].state != FREE) {
		if(customers[ssn].isSenator) {
			Acquire(passportClerks[clerkID].senatorLineLock);
			passportClerks[clerkID].senatorLineLength++;
			Print("Senator %i has gotten in senator line for ", ssn);
			Print("PassportClerk %i\n", clerkID);
			Wait(passportClerks[clerkID].senatorLineCV, passportClerks[clerkID].senatorLineLock);
			passportClerks[clerkID].senatorLineLength--;
		} else if(customers[ssn].didBribe) {
			Acquire(passportClerks[clerkID].bribeLineLock);
			passportClerks[clerkID].bribeLineLength++;
			Print("Customer %i has gotten in bribe line for ", ssn);
			Print("PassportClerk %i\n", clerkID);
			Wait(passportClerks[clerkID].bribeLineCV, passportClerks[clerkID].bribeLineLock);
			passportClerks[clerkID].bribeLineLength--;
		} else {
			Acquire(passportClerks[clerkID].lineLock);
			passportClerks[clerkID].lineLength++;
			Print("Customer %i has gotten in regular line for ", ssn);
			Print("PassportClerk %i\n", clerkID);
			Wait(passportClerks[clerkID].lineCV, passportClerks[clerkID].lineLock);
			passportClerks[clerkID].lineLength--;
		}
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock);
	if(senatorInside && !customers[ssn].isSenator) {
		Release(senatorInsideLock);
		if(customers[ssn].didBribe)
			Release(passportClerks[clerkID].bribeLineLock);
		else
			Release(passportClerks[clerkID].lineLock);
		Acquire(customerOutsideLineLock);
		Print("Customer %i is going outside the Passport Office because there is a Senator present.\n", ssn);
		customersOutside += 1;
		Wait(customerOutsideLineCV, customerOutsideLineLock);
		customersOutside -= 1;
		Release(customerOutsideLineLock);
		return true;
	} else {
		Release(senatorInsideLock);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		passportClerks[clerkID].state = BUSY;
		if(customers[ssn].isSenator)
			Release(passportClerks[clerkID].senatorLineLock);
		else if(customers[ssn].didBribe)
			Release(passportClerks[clerkID].bribeLineLock);
		else
			Release(passportClerks[clerkID].lineLock);
		return false;
	}
}

bool enterCashierLine(int clerkID) { 
	/* Stand in line */
	if(cashiers[clerkID].state != FREE) {
		if(customers[ssn].isSenator) {
			Acquire(cashiers[clerkID].senatorLineLock);
			cashiers[clerkID].senatorLineLength++;
			Print("Senator %i has gotten in senator line for ", ssn);
			Print("Cashier %i\n", clerkID);
			Wait(cashiers[clerkID].senatorLineCV, cashiers[clerkID].senatorLineLock);
			cashiers[clerkID].senatorLineLength--;
		} else if(customers[ssn].didBribe) {
			Acquire(cashiers[clerkID].bribeLineLock);
			cashiers[clerkID].bribeLineLength++;
			Print("Customer %i has gotten in bribe line for ", ssn);
			Print("Cashier %i\n", clerkID);
			Wait(cashiers[clerkID].bribeLineCV, cashiers[clerkID].bribeLineLock);
			cashiers[clerkID].bribeLineLength--;
		} else {
			Acquire(cashiers[clerkID].lineLock);
			cashiers[clerkID].lineLength++;
			Print("Customer %i has gotten in regular line for ", ssn);
			Print("Cashier %i\n", clerkID);
			Wait(cashiers[clerkID].lineCV, cashiers[clerkID].lineLock);
			cashiers[clerkID].lineLength--;
		}
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock);
	if(senatorInside && !customers[ssn].isSenator) {
		Release(senatorInsideLock);
		if(customers[ssn].didBribe)
			Release(cashiers[clerkID].bribeLineLock);
		else
			Release(cashiers[clerkID].lineLock);
		Acquire(customerOutsideLineLock);
		Print("Customer %i is going outside the Passport Office because there is a Senator present.\n", ssn);
		customersOutside += 1;
		Wait(customerOutsideLineCV, customerOutsideLineLock);
		customersOutside -= 1;
		Release(customerOutsideLineLock);
		return true;
	} else {
		Release(senatorInsideLock);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		cashiers[clerkID].state = BUSY;
		if(customers[ssn].isSenator)
			Release(cashiers[clerkID].senatorLineLock);
		else if(customers[ssn].didBribe)
			Release(cashiers[clerkID].bribeLineLock);
		else
			Release(cashiers[clerkID].lineLock);
		return false;
	}
}

bool enterLine(ClerkType clerkType, int clerkID) { 
	switch(clerkType) {
		case APPLICATION_CLERK:
			return enterApplicationLine(clerkID);
			break;
		case PICTURE_CLERK:
			return enterPictureLine(clerkID);
			break;
		case PASSPORT_CLERK:
			return enterPassportLine(clerkID);
			break;
		case CASHIER:
			return enterCashierLine(clerkID);
			break;
		}
}

int chooseLine(ClerkType clerkType) {
	int i, minLength, clerkID;
	bool canBribe;
	minLength = 51;
	canBribe = false;

	if(customers[ssn].money > 500 && !customers[ssn].isSenator)
		canBribe = true;
	/* Choose the shortest line possible */
	switch(clerkType) {
		case APPLICATION_CLERK:
			for(i = 0; i < NUM_APPLICATION_CLERKS; i++) {
				if(applicationClerks[i].lineLength < minLength) {
					customers[ssn].clerkID = i;
					minLength = applicationClerks[i].lineLength;
					customers[ssn].didBribe = false;
				}
				if(canBribe) {
					if(applicationClerks[i].bribeLineLength < minLength) {
						customers[ssn].clerkID = i;
						minLength = applicationClerks[i].bribeLineLength;
						customers[ssn].didBribe = true;
					}
				}
			}
			break;
		case PICTURE_CLERK:
			for(i = 0; i < NUM_PICTURE_CLERKS; i++) {
				if(pictureClerks[i].lineLength < minLength) {
					customers[ssn].clerkID = i;
					minLength = pictureClerks[i].lineLength;
					customers[ssn].didBribe = false;
				}
				if(canBribe) {
					if(pictureClerks[i].bribeLineLength < minLength) {
						customers[ssn].clerkID = i;
						minLength = pictureClerks[i].bribeLineLength;
						customers[ssn].didBribe = true;
					}
				}
			}
			break;
		case PASSPORT_CLERK:
			for(i = 0; i < NUM_PASSPORT_CLERKS; i++) {
				if(passportClerks[i].lineLength < minLength) {
					customers[ssn].clerkID = i;
					minLength = passportClerks[i].lineLength;
					customers[ssn].didBribe = false;
				}
				if(canBribe) {
					if(passportClerks[i].bribeLineLength < minLength) {
						customers[ssn].clerkID = i;
						minLength = passportClerks[i].bribeLineLength;
						customers[ssn].didBribe = true;
					}
				}
			}
			break;
		case CASHIER:
			for(i = 0; i < NUM_CASHIERS; i++) {
				if(cashiers[i].lineLength < minLength) {
					customers[ssn].clerkID = i;
					minLength = cashiers[i].lineLength;
					customers[ssn].didBribe = false;
				}
				if(canBribe) {
					if(cashiers[i].bribeLineLength < minLength) {
						customers[ssn].clerkID = i;
						minLength = cashiers[i].bribeLineLength;
						customers[ssn].didBribe = true;
					}
				}
			}
			break;
	}
}

void waitInLine(ClerkType clerkType) {
	bool senatord;
	int clerkID;
	senatord = false;
	chooseLine(clerkType);
	clerkID = customers[ssn].clerkID;
	senatord = enterLine(clerkType, clerkID);
	while(senatord) {
		clerkID = chooseLine(clerkType);
		senatord = enterLine(clerkType, clerkID);
	}
	if(customers[ssn].didBribe) {
		customers[ssn].money = customers[ssn].money - 500;
		switch(clerkType) {
			case APPLICATION_CLERK:
				Acquire(applicationClerks[clerkID].moneyLock);
				applicationClerks[clerkID].money = applicationClerks[clerkID].money + 500;
				Release(applicationClerks[clerkID].moneyLock);
				break;
			case PICTURE_CLERK:
				Acquire(pictureClerks[clerkID].moneyLock);
				pictureClerks[clerkID].money = pictureClerks[clerkID].money + 500;
				Release(pictureClerks[clerkID].moneyLock);
				break;
			case PASSPORT_CLERK:
				Acquire(passportClerks[clerkID].moneyLock);
				passportClerks[clerkID].money = passportClerks[clerkID].money + 500;
				Release(passportClerks[clerkID].moneyLock);
				break;
			case CASHIER:
				Acquire(cashiers[clerkID].moneyLock);
				cashiers[clerkID].money = cashiers[clerkID].money + 500;
				Release(cashiers[clerkID].moneyLock);
				break;
		}
	}
}

void doApplication() {
	int clerkID;
	waitInLine(APPLICATION_CLERK);
	clerkID = customers[ssn].clerkID;
	applicationClerks[clerkID].customerID = ssn;
	/* Interaction with clerk */
	Acquire(applicationClerks[clerkID].clerkLock);
    Signal(applicationClerks[clerkID].clerkCV, applicationClerks[clerkID].clerkLock); /* Give incomplete application to Application Clerk */
    if(customers[ssn].isSenator)
		 Print("Senator %i has given SSN ", ssn);
	else
		 Print("Customer %i has given SSN ", ssn);
    Print("%i to ", ssn);
    Print("Application Clerk %i\n", clerkID);
    Wait(applicationClerks[clerkID].clerkCV, applicationClerks[clerkID].clerkLock);   /* Wait for Application Clerk */
    Signal(applicationClerks[clerkID].clerkCV, applicationClerks[clerkID].clerkLock); /* Accept completed application */
    customers[ssn].hasApp = true;
    Release(applicationClerks[clerkID].clerkLock);
}

void doPicture() {
	int clerkID;
	waitInLine(PICTURE_CLERK);
	clerkID = customers[ssn].clerkID;
    /* Interaction with clerk */
    customers[ssn].seenPic = false;
    Acquire(pictureClerks[clerkID].clerkLock);
    pictureClerks[clerkID].customerID = ssn;
    Signal(pictureClerks[clerkID].clerkCV,pictureClerks[clerkID].clerkLock); /* Customer with Clerk */
    if(customers[ssn].isSenator)
		 Print("Senator %i has given SSN ", ssn);
	else
		 Print("Customer %i has given SSN ", ssn);
    Print("%i to ", ssn);
    Print("Picture Clerk %i\n", clerkID);
    Wait(pictureClerks[clerkID].clerkCV, pictureClerks[clerkID].clerkLock);   /* Wait for Picture Clerk */
    customers[ssn].seenPic = true;
    if(Rand() % 4 == 0 && !senatorInside) { /* Customer decides whether they don't like picture */
    	customers[ssn].likedPic = false;
    	if(customers[ssn].isSenator)
        	Print("Senator %i does not like their picture from ", ssn);
    	else
    		Print("Customer %i does not like their picture from ", ssn);
    	Print("Picture Clerk %i\n", clerkID);
    	Signal(pictureClerks[clerkID].clerkCV, pictureClerks[clerkID].clerkLock);
        Release(pictureClerks[clerkID].clerkLock);
        doPicture(ssn);
    } else {
        customers[ssn].likedPic = true;
        if(customers[ssn].isSenator)
        	Print("Senator %i does like their picture from ", ssn);
        else	
        	Print("Customer %i does like their picture from ", ssn);
    	Print("Picture Clerk %i\n", clerkID);
        Signal(pictureClerks[clerkID].clerkCV, pictureClerks[clerkID].clerkLock);
        customers[ssn].hasPic = true;
        Release(pictureClerks[clerkID].clerkLock);
    }
}

void doPassport() {
	int clerkID, wait, k;
	waitInLine(PASSPORT_CLERK);
	clerkID = customers[ssn].clerkID;
	Acquire(passportClerks[clerkID].clerkLock);
    passportClerks[clerkID].customerID = ssn;
    Signal(passportClerks[clerkID].clerkCV,passportClerks[clerkID].clerkLock); 
    Wait(passportClerks[clerkID].clerkCV,passportClerks[clerkID].clerkLock); 
    if(customers[ssn].hasApp && customers[ssn].hasPic) {
        Signal(passportClerks[clerkID].clerkCV,passportClerks[clerkID].clerkLock);
        Release(passportClerks[clerkID].clerkLock);
    } else {
        Signal(passportClerks[clerkID].clerkCV,passportClerks[clerkID].clerkLock);
        if(customers[ssn].isSenator)
        	Print("Senator %i has gone to PassportClerk ", ssn);
        else
        	Print("Customer %i has gone to PassportClerk ", ssn);
        Print("%i too soon\n", clerkID);
        Print("-They are going to the back of the line\n", 0);
        Release(passportClerks[clerkID].clerkLock);
        wait = Rand() % ((100 - 20) + 1) + 20;
        for(k = 0; k < wait; k++) 
            Yield();
    }
}

void doCashier() {
	int clerkID, wait, k;
	waitInLine(CASHIER);	
	clerkID = customers[ssn].clerkID;
	  /* Interaction with cashier */
    Acquire(cashiers[clerkID].clerkLock);
    cashiers[clerkID].customerID = ssn;
    Signal(cashiers[clerkID].clerkCV,cashiers[clerkID].clerkLock);
    Wait(cashiers[clerkID].clerkCV,cashiers[clerkID].clerkLock); 
    if(customers[ssn].certifiedByPassportClerk && customers[ssn].hasPaidForPassport) {
        customers[ssn].money = customers[ssn].money - 100;
        if(customers[ssn].isSenator)
        	Print("Senator %i has given Cashier ", ssn);
        else
        	Print("Customer %i has given Cashier ", ssn);
        Print("%i $100\n", clerkID);
        customers[ssn].hasPassport = true;
 		Signal(cashiers[clerkID].clerkCV,cashiers[clerkID].clerkLock);
        Release(cashiers[clerkID].clerkLock);
        if(customers[ssn].isSenator)
        	Print("Senator %i is leaving the Passport Office\n", ssn);
        else
        	Print("Customer %i is leaving the Passport Office\n", ssn);
    } else {
        Signal(cashiers[clerkID].clerkCV,cashiers[clerkID].clerkLock);
        if(customers[ssn].isSenator)
        	Print("Senator %i has gone to Cashier ", ssn);
        else
        	Print("Customer %i has gone to Cashier ", ssn);
        Print("%i too soon\n", clerkID);
        Print("-They are going to the back of the line\n", 0);
        Release(cashiers[clerkID].clerkLock);
        wait = Rand() % ((100 - 20) + 1) + 20;
        for(k = 0; k < wait; k++) 
            Yield();
    }
}

void runSenator() {
	int k;
	/*
	Acquire(customerIndexLock);
	i = nextAvailableCustomerIndex;
	nextAvailableCustomerIndex = nextAvailableCustomerIndex + 1;
	Release(customerIndexLock);

	initCustomer(i, true);
	*/
	Print("Running Senator: %i\n", ssn);
	
	Acquire(senatorOutsideLineLock);
	Acquire(senatorInsideLock);

	/* Senators wait on other senators outside */
	if (senatorsOutside > 0 || senatorInside) {
		senatorsOutside = senatorsOutside + 1;
		Release(senatorInsideLock);
		Wait(senatorOutsideLineCV, senatorOutsideLineLock);
		Acquire(senatorInsideLock);
		senatorsOutside = senatorsOutside - 1;
	}
		
	/* NOTICE: Don't Signal() senatorOutsideLineCV except for when a senator leaves. */
	senatorInside = true;
	Release(senatorInsideLock);
	Release(senatorOutsideLineLock);
		
	/* Senator entering, alert all lines to empty */
	for(k = 0; k < NUM_APPLICATION_CLERKS; k++) {
		Acquire(applicationClerks[k].lineLock);
		if(applicationClerks[k].lineLength > 0)
			Broadcast(applicationClerks[k].lineCV, applicationClerks[k].lineLock);
		Release(applicationClerks[k].lineLock);
		Acquire(applicationClerks[k].bribeLineLock);
		if(applicationClerks[k].bribeLineLength > 0) 
			Broadcast(applicationClerks[k].bribeLineCV, applicationClerks[k].bribeLineLock);
		Release(applicationClerks[k].bribeLineLock);
	}
	for(k = 0; k < NUM_PICTURE_CLERKS; k++) {
		Acquire(pictureClerks[k].lineLock);
		if(pictureClerks[k].lineLength > 0)
			Broadcast(pictureClerks[k].lineCV, pictureClerks[k].lineLock);
		Release(pictureClerks[k].lineLock);
		Acquire(pictureClerks[k].bribeLineLock);
		if(pictureClerks[k].bribeLineLength > 0)
			Broadcast(pictureClerks[k].bribeLineCV, pictureClerks[k].bribeLineLock);
		Release(pictureClerks[k].bribeLineLock);
	}
	for(k = 0; k < NUM_PASSPORT_CLERKS; k++) {
		Acquire(passportClerks[k].lineLock);
		if(passportClerks[k].lineLength > 0)
			Broadcast(passportClerks[k].lineCV, passportClerks[k].lineLock);
		Release(passportClerks[k].lineLock);
		Acquire(passportClerks[k].bribeLineLock);
		if(passportClerks[k].bribeLineLength > 0)
			Broadcast(passportClerks[k].bribeLineCV, passportClerks[k].bribeLineLock);
		Release(passportClerks[k].bribeLineLock);
	}
	for(k = 0; k < NUM_CASHIERS; k++) {
		Acquire(cashiers[k].lineLock);
		if(cashiers[k].lineLength > 0)
			Broadcast(cashiers[k].lineCV, cashiers[k].lineLock);
		Release(cashiers[k].lineLock);
		Acquire(cashiers[k].bribeLineLock);
		if(cashiers[k].bribeLineLength > 0)
			Broadcast(cashiers[k].bribeLineCV, cashiers[k].bribeLineLock);
		Release(cashiers[k].bribeLineLock);
	}
	
    /* Randomly decide whether to go to AppClerk or PicClerk first */
    if(Rand() % 2 == 1) {
        doApplication();
        doPicture();
    } else {
        doPicture();
        doApplication();
    }
	
    doPassport();
    doCashier();

    /* Add code here to Broadcast() when a senator leaves */
    Acquire(senatorOutsideLineLock);
    Acquire(senatorInsideLock);
    Acquire(customerOutsideLineLock);
	senatorInside = false;
	if(senatorsOutside > 0)
		Broadcast(senatorOutsideLineCV, senatorOutsideLineLock);
	Release(senatorInsideLock);
	Release(senatorOutsideLineLock);
	/*if there are no other senators outside, then customers can come in */
	if(senatorsOutside == false && customersOutside > 0) 
		Broadcast(customerOutsideLineCV, customerOutsideLineLock);
	Release(customerOutsideLineLock);
}

void runCustomer() {
	int i;
	/*
	Acquire(customerIndexLock);
	i = nextAvailableCustomerIndex;
	nextAvailableCustomerIndex = nextAvailableCustomerIndex + 1;
	Release(customerIndexLock);

	initCustomer(i,false); 
	*/
	Print("Running Customer: %i\n", ssn);

	Acquire(senatorOutsideLineLock);
	Acquire(senatorInsideLock);

	if(senatorsOutside > 0 || senatorInside) {
		Release(senatorInsideLock);
		Release(senatorOutsideLineLock);
		Acquire(customerOutsideLineLock);
		customersOutside += 1;
		Wait(customerOutsideLineCV,customerOutsideLineLock);
		customersOutside -= 1;
		Release(customerOutsideLineLock);
	}
	
	Release(senatorInsideLock);
	Release(senatorOutsideLineLock);

	/* Customer has 1 in 10 chance of going to Passport Clerk first*/
	if(Rand() % 10 == 0) 
        doPassport();

    /* Randomly decide whether to go to AppClerk or PicClerk first */
   if(Rand() % 2 == 0) {
        doApplication();
        doPicture();
    } else {
        doPicture();
        doApplication();
    }
	/* Customer has 1 in 10 chance of going to Cashier before PassportClerk */
	if(Rand() % 10 == 0) 
        doCashier();

    doPassport();
    doCashier();
}

int main() {
    setup();
    
    Acquire(customerIndexLock);
    ssn = GetMV(nextAvailableCustomerIndex);
    SetMV(nextAvailableCustomerIndex, ssn + 1);
    Release(customerIndexLock);
    Print("My ssn: %i\n",ssn);
    /*
    
	if(customers[ssn].isSenator)
		runSenator();
	else
		runCustomer();
     */
    
	Exit(0);
}








