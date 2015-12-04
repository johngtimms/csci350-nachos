#include "syscall.h"
#include "setup.h"

int ssn;
bool didntGoInLineBecauseOfSenator = false;

bool enterApplicationLine(int clerkID) {
	int length;
	if(!GetMV(senatorInside, -1) || GetMV(customer.isSenator, ssn)) { /* don't go in line if there is a senator present, unless you are a senator */
		if(GetMV(customer.isSenator, ssn)) {	
			Acquire(applicationClerk.senatorLineLock, clerkID);
			/*applicationClerk.senatorLineLength++;*/
			length = GetMV(applicationClerk.senatorLineLength, clerkID);
			SetMV(applicationClerk.senatorLineLength, clerkID, length + 1);
			Print("Senator %i has gotten in senator line for ", ssn);
			Print("ApplicationClerk %i\n", clerkID);
			Wait(applicationClerk.senatorLineCV, clerkID, applicationClerk.senatorLineLock, clerkID);
			/*applicationClerk.senatorLineLength--;*/
			length = GetMV(applicationClerk.senatorLineLength, clerkID);
			SetMV(applicationClerk.senatorLineLength, clerkID, length - 1);
		/*} else if(customer.didBribe) {*/
		} else if(GetMV(customer.didBribe, ssn)) {
			Acquire(applicationClerk.bribeLineLock, clerkID);
			/*applicationClerk.bribeLineLength++;*/
			length = GetMV(applicationClerk.bribeLineLength, clerkID);
			SetMV(applicationClerk.bribeLineLength, clerkID, length + 1);
			Print("Customer %i has gotten in bribe line for ", ssn);
			Print("ApplicationClerk %i\n", clerkID);
			Wait(applicationClerk.bribeLineCV, clerkID, applicationClerk.bribeLineLock, clerkID);
			/*applicationClerk.bribeLineLength--;*/
			length = GetMV(applicationClerk.bribeLineLength, clerkID);
			SetMV(applicationClerk.bribeLineLength, clerkID, length - 1);
		} else {
			Acquire(applicationClerk.lineLock, clerkID);
			/*applicationClerk.lineLength++;*/
			length = GetMV(applicationClerk.lineLength, clerkID);
			SetMV(applicationClerk.lineLength, clerkID, length + 1);
			Print("Customer %i has gotten in regular line for ", ssn);
			Print("ApplicationClerk %i\n", clerkID);
			Wait(applicationClerk.lineCV, clerkID, applicationClerk.lineLock, clerkID);
			Print("Customer %i woken up from line\n",ssn);
			/*applicationClerk.lineLength--;*/
			length = GetMV(applicationClerk.lineLength, clerkID);
			SetMV(applicationClerk.lineLength, clerkID, length - 1);
		}
	} else {
		didntGoInLineBecauseOfSenator = true; /*set this so that we know not to release the line locks (never entered line)*/
	}
	
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock, -1);
	/*if(senatorInside && !customer.isSenator) {*/
	if(GetMV(senatorInside, -1) && !GetMV(customer.isSenator, ssn)) {
		Release(senatorInsideLock, -1);
		/*if(customer.didBribe)*/
		if(!didntGoInLineBecauseOfSenator) {
			if(GetMV(customer.didBribe, ssn))
				Release(applicationClerk.bribeLineLock, clerkID);
			else
				Release(applicationClerk.lineLock, clerkID);
		} else {
			didntGoInLineBecauseOfSenator = false; /*reset bool*/
		}
		Acquire(customerOutsideLineLock, -1);
		Print("Customer %i is going outside the Passport Office because there is a Senator present.\n", ssn);
		/*customersOutside += 1;*/
		length = GetMV(customersOutside, -1);
		SetMV(customersOutside, -1, length + 1);
		Wait(customerOutsideLineCV, -1, customerOutsideLineLock, -1);
		Print("Customer %i is notified that there are no more senators, and can come in now.\n", ssn);
		/*customersOutside -= 1;*/
		length = GetMV(customersOutside, -1);
		SetMV(customersOutside, -1, length - 1);
		Release(customerOutsideLineLock, -1);
		return true;
	} else {
		Release(senatorInsideLock, -1);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		/*applicationClerk.state = BUSY;*/
		SetMV(applicationClerk.state, clerkID, BUSY);
		/*if(customer.isSenator)*/
		if(GetMV(customer.isSenator, ssn))
			Release(applicationClerk.senatorLineLock, clerkID);
		/*else if(customer.didBribe)*/
		else if(GetMV(customer.didBribe, ssn))
			Release(applicationClerk.bribeLineLock, clerkID);
		else
			Release(applicationClerk.lineLock, clerkID);
		return false;
	}
}

bool enterPictureLine(int clerkID) { 
	int length;
	/* Stand in line */
	/*Print("Customer %i about to stand in picture clerk line\n", ssn); 
	Print("Picture clerk state %i\n",GetMV(pictureClerk.state));*/
	/*if(pictureClerk.state != FREE) {*/
	if(!GetMV(senatorInside, -1) || GetMV(customer.isSenator, ssn)) { /*don't go in line if there is a senator present, unless you are a senator /*
		/*if(customer.isSenator) {*/
		if(GetMV(customer.isSenator, ssn)) {
			Acquire(pictureClerk.senatorLineLock, clerkID);
			/*pictureClerk.senatorLineLength++;*/
			length = GetMV(pictureClerk.senatorLineLength, clerkID);
			SetMV(pictureClerk.senatorLineLength, clerkID, length + 1);
			Print("Senator %i has gotten in senator line for ", ssn);
			Print("PictureClerk %i\n", clerkID);
			Wait(pictureClerk.senatorLineCV, clerkID, pictureClerk.senatorLineLock, clerkID);
			/*pictureClerk.senatorLineLength--;*/
			length = GetMV(pictureClerk.senatorLineLength, clerkID);
			SetMV(pictureClerk.senatorLineLength, clerkID, length - 1);
		/*} else if (customer.didBribe) {*/
		} else if(GetMV(customer.didBribe, ssn)) {
			Acquire(pictureClerk.bribeLineLock, clerkID);
			/*pictureClerk.bribeLineLength++;*/
			length = GetMV(pictureClerk.bribeLineLength, clerkID);
			SetMV(pictureClerk.bribeLineLength, clerkID, length + 1);
			Print("Customer %i has gotten in bribe line for ", ssn);
			Print("PictureClerk %i\n", clerkID);
			Wait(pictureClerk.bribeLineCV, clerkID, pictureClerk.bribeLineLock, clerkID);
			/*pictureClerk.bribeLineLength--;*/
			length = GetMV(pictureClerk.bribeLineLength, clerkID);
			SetMV(pictureClerk.bribeLineLength, clerkID, length - 1);
		} else {
			Acquire(pictureClerk.lineLock, clerkID);
			/*pictureClerk.lineLength++;*/
			length = GetMV(pictureClerk.lineLength, clerkID);
			SetMV(pictureClerk.lineLength, clerkID, length + 1);
			Print("Customer %i has gotten in regular line for ", ssn);
			Print("PictureClerk %i\n", clerkID);
			Wait(pictureClerk.lineCV, clerkID, pictureClerk.lineLock, clerkID);
			/*pictureClerk.lineLength--;*/
			length = GetMV(pictureClerk.lineLength, clerkID);
			SetMV(pictureClerk.lineLength, clerkID, length - 1);
		}
	} else {
		didntGoInLineBecauseOfSenator = true; /*set this so that we know not to release the line locks (never entered line)*/
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock, -1);
	/*if(senatorInside && !customer.isSenator) {*/
	if(GetMV(senatorInside, -1) && !GetMV(customer.isSenator, ssn)) {
		Release(senatorInsideLock, -1);
		/*if(customer.didBribe)*/
		if(!didntGoInLineBecauseOfSenator) {
			if(GetMV(customer.didBribe, ssn))
				Release(pictureClerk.bribeLineLock, clerkID);
			else
				Release(pictureClerk.lineLock, clerkID);
		} else {
			didntGoInLineBecauseOfSenator = false; /*reset bool*/
		}
		Acquire(customerOutsideLineLock, -1);
		Print("Customer %i is going outside the Passport Office because there is a Senator present.\n", ssn);
		/*customersOutside += 1;*/
		length = GetMV(customersOutside, -1);
		SetMV(customersOutside, -1, length + 1);
		Wait(customerOutsideLineCV, -1, customerOutsideLineLock, -1);
		/*customersOutside -= 1;*/
		length = GetMV(customersOutside, -1);
		SetMV(customersOutside, -1, length - 1);
		Release(customerOutsideLineLock, -1);
		return true;
	} else {
		Release(senatorInsideLock, -1);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		/*pictureClerk.state = BUSY;*/
		SetMV(pictureClerk.state, clerkID, BUSY);
		/*if(customer.isSenator)*/
		if(GetMV(customer.isSenator, ssn))
			Release(pictureClerk.senatorLineLock, clerkID);
		/*else if(customer.didBribe)*/
		else if(GetMV(customer.didBribe, ssn))
			Release(pictureClerk.bribeLineLock, clerkID);
		else
			Release(pictureClerk.lineLock, clerkID);
		return false;
	}
}

bool enterPassportLine(int clerkID) { 
	int length;
	/* Stand in line */
	/*Print("Customer %i about to stand in passport clerk line\n", ssn);*/ 
	/*if(passportClerk.state != FREE) {*/
	if(!GetMV(senatorInside, -1) || GetMV(customer.isSenator, ssn)) { /*don't go in line if there is a senator present, unless you are a senator /*
		/*if(customer.isSenator) {*/
		if(GetMV(customer.isSenator, ssn)) {
			Acquire(passportClerk.senatorLineLock, clerkID);
			/*passportClerk.senatorLineLength++;*/
			length = GetMV(passportClerk.senatorLineLength, clerkID);
			SetMV(passportClerk.senatorLineLength, clerkID, length + 1);
			Print("Senator %i has gotten in senator line for ", ssn);
			Print("PassportClerk %i\n", clerkID);
			Wait(passportClerk.senatorLineCV, clerkID, passportClerk.senatorLineLock, clerkID);
			/*passportClerk.senatorLineLength--;*/
			length = GetMV(passportClerk.senatorLineLength, clerkID);
			SetMV(passportClerk.senatorLineLength, clerkID, length - 1);
		/*} else if(customer.didBribe) {*/
		} else if(GetMV(customer.didBribe, ssn)) {
			Acquire(passportClerk.bribeLineLock, clerkID);
			/*passportClerk.bribeLineLength++;*/
			length = GetMV(passportClerk.bribeLineLength, clerkID);
			SetMV(passportClerk.bribeLineLength, clerkID, length + 1);
			Print("Customer %i has gotten in bribe line for ", ssn);
			Print("PassportClerk %i\n", clerkID);
			Wait(passportClerk.bribeLineCV, clerkID, passportClerk.bribeLineLock, clerkID);
			/*passportClerk.bribeLineLength--;*/
			length = GetMV(passportClerk.bribeLineLength, clerkID);
			SetMV(passportClerk.bribeLineLength, clerkID, length - 1);
		} else {
			Acquire(passportClerk.lineLock, clerkID);
			/*passportClerk.lineLength++;*/
			length = GetMV(passportClerk.lineLength, clerkID);
			SetMV(passportClerk.lineLength, clerkID, length + 1);
			Print("Customer %i has gotten in regular line for ", ssn);
			Print("PassportClerk %i\n", clerkID);
			Wait(passportClerk.lineCV, clerkID, passportClerk.lineLock, clerkID);
			/*passportClerk.lineLength--;*/
			length = GetMV(passportClerk.lineLength, clerkID);
			SetMV(passportClerk.lineLength, clerkID, length - 1);
		}
	} else {
		didntGoInLineBecauseOfSenator = true; /*set this so that we know not to release the line locks (never entered line)*/
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock, -1);
	/*if(senatorInside && !customer.isSenator) {*/
	if(GetMV(senatorInside, -1) && !GetMV(customer.isSenator, ssn)) {
		Release(senatorInsideLock, -1);
		/*if(customer.didBribe)*/
		if(!didntGoInLineBecauseOfSenator) {
			if(GetMV(customer.didBribe, ssn))
				Release(passportClerk.bribeLineLock, clerkID);
			else
				Release(passportClerk.lineLock, clerkID);
		} else {
			didntGoInLineBecauseOfSenator = false; /*reset bool*/
		}
		Acquire(customerOutsideLineLock, -1);
		Print("Customer %i is going outside the Passport Office because there is a Senator present.\n", ssn);
		/*customersOutside += 1;*/
		length = GetMV(customersOutside, -1);
		SetMV(customersOutside, -1, length + 1);
		Wait(customerOutsideLineCV, -1, customerOutsideLineLock, -1);
		/*customersOutside -= 1;*/
		length = GetMV(customersOutside, -1);
		SetMV(customersOutside, -1, length - 1);
		Release(customerOutsideLineLock, -1);
		return true;
	} else {
		Release(senatorInsideLock, -1);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		/*passportClerk.state = BUSY;*/
		SetMV(passportClerk.state, clerkID, BUSY);
		/*if(customer.isSenator)*/
		if(GetMV(customer.isSenator, ssn))
			Release(passportClerk.senatorLineLock, clerkID);
		/*else if(customer.didBribe)*/
		else if(GetMV(customer.didBribe, ssn))
			Release(passportClerk.bribeLineLock, clerkID);
		else
			Release(passportClerk.lineLock, clerkID);
		return false;
	}
}

bool enterCashierLine(int clerkID) { 
	int length;
	/* Stand in line */
	/*Print("Customer %i about to stand in cashier clerk line\n", ssn); */
	/*if(cashier.state != FREE) {*/
	if(!GetMV(senatorInside, -1) || GetMV(customer.isSenator, ssn)) { /*don't go in line if there is a senator present, unless you are a senator /*
		/*if(customer.isSenator) {*/
		if(GetMV(customer.isSenator, ssn)) {
			Acquire(cashier.senatorLineLock, clerkID);
			/*cashier.senatorLineLength++;*/
			length = GetMV(cashier.senatorLineLength, clerkID);
			SetMV(cashier.senatorLineLength, clerkID, length + 1);
			Print("Senator %i has gotten in senator line for ", ssn);
			Print("Cashier %i\n", clerkID);
			Wait(cashier.senatorLineCV, clerkID, cashier.senatorLineLock, clerkID);
			/*cashier.senatorLineLength--;*/
			length = GetMV(cashier.senatorLineLength, clerkID);
			SetMV(cashier.senatorLineLength, clerkID, length - 1);
		/*} else if(customer.didBribe) {*/
		} else if(GetMV(customer.didBribe, ssn)) {
			Acquire(cashier.bribeLineLock, clerkID);
			/*cashier.bribeLineLength++;*/
			length = GetMV(cashier.bribeLineLength, clerkID);
			SetMV(cashier.bribeLineLength, clerkID, length + 1);
			Print("Customer %i has gotten in bribe line for ", ssn);
			Print("Cashier %i\n", clerkID);
			Wait(cashier.bribeLineCV, clerkID, cashier.bribeLineLock, clerkID);
			/*cashier.bribeLineLength--;*/
			length = GetMV(cashier.bribeLineLength, clerkID);
			SetMV(cashier.bribeLineLength, clerkID, length - 1);
		} else {
			Acquire(cashier.lineLock, clerkID);
			/*cashier.lineLength++;*/
			length = GetMV(cashier.lineLength, clerkID);
			SetMV(cashier.lineLength, clerkID, length + 1);
			Print("Customer %i has gotten in regular line for ", ssn);
			Print("Cashier %i\n", clerkID);
			Wait(cashier.lineCV, clerkID, cashier.lineLock, clerkID);
			/*cashier.lineLength--;*/
			length = GetMV(cashier.lineLength, clerkID);
			SetMV(cashier.lineLength, clerkID, length - 1);
		}
	} else {
		didntGoInLineBecauseOfSenator = true; /*set this so that we know not to release the line locks (never entered line)*/
	}
	/* Called out of line, make sure it wasn't because of a senator */
	Acquire(senatorInsideLock, -1);
	/*if(senatorInside && !customer.isSenator) {*/
	if(GetMV(senatorInside, -1) && !GetMV(customer.isSenator, ssn)) {
		Release(senatorInsideLock, -1);
		/*if(customer.didBribe)*/
		if(!didntGoInLineBecauseOfSenator) {
			if(GetMV(customer.didBribe, ssn))
				Release(cashier.bribeLineLock, clerkID);
			else
				Release(cashier.lineLock, clerkID);
		} else {
			didntGoInLineBecauseOfSenator = false; /*reset bool*/
		}
		Acquire(customerOutsideLineLock, -1);
		Print("Customer %i is going outside the Passport Office because there is a Senator present.\n", ssn);
		/*customersOutside += 1;*/
		length = GetMV(customersOutside, -1);
		SetMV(customersOutside, -1, length + 1);
		Wait(customerOutsideLineCV, -1, customerOutsideLineLock, -1);
		/*customersOutside -= 1;*/
		length = GetMV(customersOutside, -1);
		SetMV(customersOutside, -1, length - 1);
		Release(customerOutsideLineLock, -1);
		return true;
	} else {
		Release(senatorInsideLock, -1);
		/* Change the clerk to BUSY before releasing the clerk's line lock */
		/*cashier.state = BUSY;*/
		SetMV(cashier.state, clerkID, BUSY);
		/*if(customer.isSenator)*/
		if(GetMV(customer.isSenator, ssn))
			Release(cashier.senatorLineLock, clerkID);
		/*else if(customer.didBribe)*/
		else if(GetMV(customer.didBribe, ssn))
			Release(cashier.bribeLineLock, clerkID);
		else
			Release(cashier.lineLock, clerkID);
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

	/*if(customer.money > 500 && !customer.isSenator)*/
	if(GetMV(customer.money, ssn) > 500 && !GetMV(customer.isSenator, ssn))
		canBribe = true;
	/* Choose the shortest line possible */
	switch(clerkType) {
		case APPLICATION_CLERK:
			for(i = 0; i < numApplicationClerks; i++) {
				/*if(applicationClerk.lineLength < minLength) {*/
				if(GetMV(applicationClerk.lineLength, i) < minLength) {
					/*customer.clerkID = i;*/
					SetMV(customer.clerkID, ssn, i);
					/*minLength = applicationClerk.lineLength;*/
					minLength = GetMV(applicationClerk.lineLength, i);
					/*customer.didBribe = false;*/
					SetMV(customer.didBribe, ssn, false);
				}
				if(canBribe) {
					/*if(applicationClerk.bribeLineLength < minLength) {*/
					if(GetMV(applicationClerk.bribeLineLength, i) < minLength) {
						/*customer.clerkID = i;*/
						SetMV(customer.clerkID, ssn, i);
						/*minLength = applicationClerk.bribeLineLength;*/
						minLength = GetMV(applicationClerk.bribeLineLength, i);
						/*customer.didBribe = true;*/
						SetMV(customer.didBribe, ssn, true);
					}
				}
			}
			break;
		case PICTURE_CLERK:
			for(i = 0; i < numPictureClerks; i++) {
				/*if(pictureClerk.lineLength < minLength) {*/
				if(GetMV(pictureClerk.lineLength, i) < minLength) {
					/*customer.clerkID = i;*/
					SetMV(customer.clerkID, ssn, i);
					/*minLength = pictureClerk.lineLength;*/
					minLength = GetMV(pictureClerk.lineLength, i);
					/*customer.didBribe = false;*/
					SetMV(customer.didBribe, ssn, false);
				}
				if(canBribe) {
					/*if(pictureClerk.bribeLineLength < minLength) {*/
					if(GetMV(pictureClerk.bribeLineLength, i) < minLength) {
						/*customer.clerkID = i;*/
						SetMV(customer.clerkID, ssn, i);
						/*minLength = pictureClerk.bribeLineLength;*/
						minLength = GetMV(pictureClerk.bribeLineLength, i);
						/*customer.didBribe = true;*/
						SetMV(customer.didBribe, ssn, true);
					}
				}
			}
			break;
		case PASSPORT_CLERK:
			for(i = 0; i < numPassportClerks; i++) {
				/*if(passportClerk.lineLength < minLength) {*/
				if(GetMV(passportClerk.lineLength, i) < minLength) {
					/*customer.clerkID = i;*/
					SetMV(customer.clerkID, ssn, i);
					/*minLength = passportClerk.lineLength;*/
					minLength = GetMV(passportClerk.lineLength, i);
					/*customer.didBribe = false;*/
					SetMV(customer.didBribe, ssn, false);
				}
				if(canBribe) {
					/*if(passportClerk.bribeLineLength < minLength) {*/
					if(GetMV(passportClerk.bribeLineLength, i) < minLength) {
						/*customer.clerkID = i;*/
						SetMV(customer.clerkID, ssn, i);
						/*minLength = passportClerk.bribeLineLength;*/
						minLength = GetMV(passportClerk.bribeLineLength, i);
						/*customer.didBribe = true;*/
						SetMV(customer.didBribe, ssn, true);
					}
				}
			}
			break;
		case CASHIER:
			for(i = 0; i < numCashiers; i++) {
				/*if(cashier.lineLength < minLength) {*/
				if(GetMV(cashier.lineLength, i) < minLength) {
					/*customer.clerkID = i;*/
					SetMV(customer.clerkID, ssn, i);
					/*minLength = cashier.lineLength;*/
					minLength = GetMV(cashier.lineLength, i);
					/*customer.didBribe = false;*/
					SetMV(customer.didBribe, ssn, false);
				}
				if(canBribe) {
					/*if(cashier.bribeLineLength < minLength) {*/
					if(GetMV(cashier.bribeLineLength, i) < minLength) {
						/*customer.clerkID = i;*/
						SetMV(customer.clerkID, ssn, i);
						/*minLength = cashier.bribeLineLength;*/
						minLength = GetMV(cashier.bribeLineLength, i);
						/*customer.didBribe = true;*/
						SetMV(customer.didBribe, ssn, true);
					}
				}
			}
			break;
	}
}

void waitInLine(ClerkType clerkType) {
	bool senatord;
	int clerkID, money;
	senatord = false;
	chooseLine(clerkType);
	/*clerkID = customer.clerkID;*/
	clerkID = GetMV(customer.clerkID, ssn);
	senatord = enterLine(clerkType, clerkID);
	while(senatord) {
		clerkID = chooseLine(clerkType);
		senatord = enterLine(clerkType, clerkID);
	}
	/*if(customer.didBribe) {*/
	if(GetMV(customer.didBribe, ssn)) {
		/*customer.money = customer.money - 500;*/
		money = GetMV(customer.money, ssn);
		SetMV(customer.money, ssn, money - 500);
		switch(clerkType) {
			case APPLICATION_CLERK:
				Acquire(applicationClerk.moneyLock, clerkID);
				/*applicationClerk.money = applicationClerk.money + 500;*/
				money = GetMV(applicationClerk.money, clerkID);
				SetMV(applicationClerk.money, clerkID, money + 500);
				Release(applicationClerk.moneyLock, clerkID);
				break;
			case PICTURE_CLERK:
				Acquire(pictureClerk.moneyLock, clerkID);
				/*pictureClerk.money = pictureClerk.money + 500;*/
				money = GetMV(pictureClerk.money, clerkID);
				SetMV(pictureClerk.money, clerkID, money + 500);
				Release(pictureClerk.moneyLock, clerkID);
				break;
			case PASSPORT_CLERK:
				Acquire(passportClerk.moneyLock, clerkID);
				/*passportClerk.money = passportClerk.money + 500;*/
				money = GetMV(passportClerk.money, clerkID);
				SetMV(passportClerk.money, clerkID, money + 500);
				Release(passportClerk.moneyLock, clerkID);
				break;
			case CASHIER:
				Acquire(cashier.moneyLock, clerkID);
				/*cashier.money = cashier.money + 500;*/
				money = GetMV(cashier.money, clerkID);
				SetMV(cashier.money, clerkID, money + 500);
				Release(cashier.moneyLock, clerkID);
				break;
		}
	}
}

void doApplication() {
	int clerkID;
	waitInLine(APPLICATION_CLERK);
	/*clerkID = customer.clerkID;*/
	clerkID = GetMV(customer.clerkID, ssn);
	/* Interaction with clerk */
	Acquire(applicationClerk.clerkLock, clerkID);
	/*applicationClerk.customerID = ssn;*/
	SetMV(applicationClerk.customerID, clerkID, ssn);
    Signal(applicationClerk.clerkCV, clerkID, applicationClerk.clerkLock, clerkID); /* Give incomplete application to Application Clerk */
    /*if(customer.isSenator)*/
    if(GetMV(customer.isSenator, ssn))
		 Print("Senator %i has given SSN ", ssn);
	else
		 Print("Customer %i has given SSN ", ssn);
    Print("%i to ", ssn);
    Print("Application Clerk %i\n", clerkID);
    Wait(applicationClerk.clerkCV, clerkID, applicationClerk.clerkLock, clerkID);   /* Wait for Application Clerk */
    Signal(applicationClerk.clerkCV, clerkID, applicationClerk.clerkLock, clerkID); /* Accept completed application */
    /*customer.hasApp = true;*/
    SetMV(customer.hasApp, ssn, true);
    Release(applicationClerk.clerkLock, clerkID);
}

void doPicture() {
	int clerkID, randNumber;
	waitInLine(PICTURE_CLERK);
	/*clerkID = customer.clerkID;*/
	clerkID = GetMV(customer.clerkID, ssn);
    /* Interaction with clerk */
    /*customer.seenPic = false;*/
    SetMV(customer.seenPic, ssn, false);
    Acquire(pictureClerk.clerkLock, clerkID);
    /*pictureClerk.customerID = ssn;*/
    SetMV(pictureClerk.customerID, clerkID, ssn);
    Signal(pictureClerk.clerkCV, clerkID, pictureClerk.clerkLock, clerkID); /* Customer with Clerk */
    /*if(customer.isSenator)*/
    if(GetMV(customer.isSenator, ssn))
		 Print("Senator %i has given SSN ", ssn);
	else
		 Print("Customer %i has given SSN ", ssn);
    Print("%i to ", ssn);
    Print("Picture Clerk %i\n", clerkID);
    Wait(pictureClerk.clerkCV, clerkID, pictureClerk.clerkLock, clerkID);   /* Wait for Picture Clerk */
    /*customer.seenPic = true;*/
    SetMV(customer.seenPic, ssn, true);
    /*if(Rand() % 4 == 0 && !senatorInside) {*/ /* Customer decides whether they don't like picture */
    randNumber = Rand() % 4;
    if(randNumber == 0 && !GetMV(senatorInside, -1)) {
    	Print("randNumber was %i\n",randNumber);
    	/*customer.likedPic = false;*/
    	SetMV(customer.likedPic, ssn, false);
    	/*if(customer.isSenator)*/
    	if(GetMV(customer.isSenator, ssn))
        	Print("Senator %i does not like their picture from ", ssn);
    	else
    		Print("Customer %i does not like their picture from ", ssn);
    	Print("Picture Clerk %i\n", clerkID);
    	Signal(pictureClerk.clerkCV, clerkID, pictureClerk.clerkLock, clerkID);
        Release(pictureClerk.clerkLock, clerkID);
        doPicture();
    } else {
        /*customer.likedPic = true;*/
        SetMV(customer.likedPic, ssn, true);
        /*if(customer.isSenator)*/
        if(GetMV(customer.isSenator, ssn))
        	Print("Senator %i does like their picture from ", ssn);
        else	
        	Print("Customer %i does like their picture from ", ssn);
    	Print("Picture Clerk %i\n", clerkID);
        Signal(pictureClerk.clerkCV, clerkID, pictureClerk.clerkLock, clerkID);
        /*customer.hasPic = true;*/
        SetMV(customer.hasPic, ssn, true);
        Release(pictureClerk.clerkLock, clerkID);
    }
}

void doPassport() {
	int clerkID, wait, k;
	waitInLine(PASSPORT_CLERK);
	/*clerkID = customer.clerkID;*/
	clerkID = GetMV(customer.clerkID, ssn);
	/* Interaction with clerk */
	Acquire(passportClerk.clerkLock, clerkID);
    /*passportClerk.customerID = ssn;*/
    SetMV(passportClerk.customerID, clerkID, ssn);
    Signal(passportClerk.clerkCV, clerkID, passportClerk.clerkLock, clerkID); 
    Wait(passportClerk.clerkCV, clerkID, passportClerk.clerkLock, clerkID); 
    /*if(customer.hasApp && customer.hasPic) {*/
    if(GetMV(customer.hasApp, ssn) && GetMV(customer.hasPic, ssn)) {
        Signal(passportClerk.clerkCV, clerkID, passportClerk.clerkLock, clerkID);
        Release(passportClerk.clerkLock, clerkID);
    } else {
        Signal(passportClerk.clerkCV, clerkID, passportClerk.clerkLock, clerkID);
        /*if(customer.isSenator)*/
        if(GetMV(customer.isSenator, ssn))
        	Print("Senator %i has gone to PassportClerk ", ssn);
        else
        	Print("Customer %i has gone to PassportClerk ", ssn);
        Print("%i too soon\n", clerkID);
        Print("-They are going to the back of the line\n", 0);
        Release(passportClerk.clerkLock, clerkID);
        wait = Rand() % ((100 - 20) + 1) + 20;
        for(k = 0; k < wait; k++) 
            Yield();
    }
}

void doCashier() {
	int clerkID, wait, k, money;
	waitInLine(CASHIER);	
	/*clerkID = customer.clerkID;*/
	clerkID = GetMV(customer.clerkID, ssn);
	/* Interaction with cashier */
    Acquire(cashier.clerkLock, clerkID);
    /*cashier.customerID = ssn;*/
    SetMV(cashier.customerID, clerkID, ssn);
    Signal(cashier.clerkCV, clerkID, cashier.clerkLock, clerkID);
    Wait(cashier.clerkCV, clerkID, cashier.clerkLock, clerkID); 
    /*if(customer.certifiedByPassportClerk && customer.hasPaidForPassport) {*/
    if(GetMV(customer.certifiedByPassportClerk, ssn) && GetMV(customer.hasPaidForPassport, ssn)) {
        /*customer.money = customer.money - 100;*/
        money = GetMV(customer.money, ssn);
        SetMV(customer.money, ssn, money - 100);
        /*if(customer.isSenator)*/
        if(GetMV(customer.isSenator, ssn))
        	Print("Senator %i has given Cashier ", ssn);
        else
        	Print("Customer %i has given Cashier ", ssn);
        Print("%i $100\n", clerkID);
        /*customer.hasPassport = true;*/
        SetMV(customer.hasPassport, ssn, true);
 		Signal(cashier.clerkCV, clerkID, cashier.clerkLock, clerkID);
        Release(cashier.clerkLock, clerkID);
        /*if(customer.isSenator)*/
        if(GetMV(customer.isSenator, ssn)) {
        	Print("Senator %i is leaving the Passport Office\n", ssn);
        }
        else {
        	Print("Customer %i is leaving the Passport Office\n", ssn);
        }
    } else {
        Signal(cashier.clerkCV, clerkID, cashier.clerkLock, clerkID);
        /*if(customer.isSenator)*/
        if(GetMV(customer.isSenator, ssn))
        	Print("Senator %i has gone to Cashier ", ssn);
        else
        	Print("Customer %i has gone to Cashier ", ssn);
        Print("%i too soon\n", clerkID);
        Print("-They are going to the back of the line\n", 0);
        Release(cashier.clerkLock, clerkID);
        wait = Rand() % ((100 - 20) + 1) + 20;
        for(k = 0; k < wait; k++) 
            Yield();
    }
}

void runSenator() {
	int i, k;
	Print("Running Senator: %i\n", ssn);
	
	Acquire(senatorOutsideLineLock, -1);
	Acquire(senatorInsideLock, -1);

	/* Senators wait on other senators outside */
	/*if(senatorsOutside > 0 || senatorInside) {*/
	if(GetMV(senatorsOutside, -1) > 0 || GetMV(senatorInside, -1)) {
		/*senatorsOutside = senatorsOutside + 1;*/
		i = GetMV(senatorsOutside, -1);
		SetMV(senatorsOutside, -1, i + 1);
		Release(senatorInsideLock, -1);
		Print("Senator: %i is going to wait outside because there is a senator inside.\n", ssn);
		Wait(senatorOutsideLineCV, -1, senatorOutsideLineLock, -1);
		Acquire(senatorInsideLock, -1);
		/*senatorsOutside = senatorsOutside - 1;*/
		i = GetMV(senatorsOutside, -1);
		SetMV(senatorsOutside, -1, i - 1);
	}
		
	/* NOTICE: Don't Signal() senatorOutsideLineCV except for when a senator leaves. */
	/*senatorInside = true;*/
	SetMV(senatorInside, -1, true);
	Release(senatorInsideLock, -1);
	Release(senatorOutsideLineLock, -1);
		
	/* Senator entering, alert all lines to empty */
	for(k = 0; k < numApplicationClerks; k++) {
		Acquire(applicationClerk.lineLock, k);
		/*if(applicationClerk.lineLength > 0)*/
		if(GetMV(applicationClerk.lineLength, k) > 0) {
			Broadcast(applicationClerk.lineCV, k, applicationClerk.lineLock, k);
		}
		Release(applicationClerk.lineLock, k);
		Acquire(applicationClerk.bribeLineLock, k);
		/*if(applicationClerk.bribeLineLength > 0) */
		if(GetMV(applicationClerk.bribeLineLength, k) > 0)
			Broadcast(applicationClerk.bribeLineCV, k, applicationClerk.bribeLineLock, k);
		Release(applicationClerk.bribeLineLock, k);
	}
	for(k = 0; k < numPictureClerks; k++) {
		Acquire(pictureClerk.lineLock, k);
		/*if(pictureClerk.lineLength > 0)*/
		if(GetMV(pictureClerk.lineLength, k) > 0)
			Broadcast(pictureClerk.lineCV, k, pictureClerk.lineLock, k);
		Release(pictureClerk.lineLock, k);
		Acquire(pictureClerk.bribeLineLock, k);
		/*if(pictureClerk.bribeLineLength > 0) */
		if(GetMV(pictureClerk.bribeLineLength, k) > 0)
			Broadcast(pictureClerk.bribeLineCV, k, pictureClerk.bribeLineLock, k);
		Release(pictureClerk.bribeLineLock, k);
	}
	for(k = 0; k < numPassportClerks; k++) {
		Acquire(passportClerk.lineLock, k);
		/*if(passportClerk.lineLength > 0)*/
		if(GetMV(passportClerk.lineLength, k) > 0)
			Broadcast(passportClerk.lineCV, k, passportClerk.lineLock, k);
		Release(passportClerk.lineLock, k);
		Acquire(passportClerk.bribeLineLock, k);
		/*if(passportClerk.bribeLineLength > 0) */
		if(GetMV(passportClerk.bribeLineLength, k) > 0)
			Broadcast(passportClerk.bribeLineCV, k, passportClerk.bribeLineLock, k);
		Release(passportClerk.bribeLineLock, k);
	}
	for(k = 0; k < numCashiers; k++) {
		Acquire(cashier.lineLock, k);
		/*if(cashier.lineLength > 0)*/
		if(GetMV(cashier.lineLength, k) > 0)
			Broadcast(cashier.lineCV, k, cashier.lineLock, k);
		Release(cashier.lineLock, k);
		Acquire(cashier.bribeLineLock, k);
		/*if(cashier.bribeLineLength > 0) */
		if(GetMV(cashier.bribeLineLength, k) > 0)
			Broadcast(cashier.bribeLineCV, k, cashier.bribeLineLock, k);
		Release(cashier.bribeLineLock, k);
	}
	
    /* Randomly decide whether to go to AppClerk or PicClerk first */
    if(/*Rand() % 2 == 1*/true) {
        doApplication();
        doPicture();
    } else {
        doPicture();
        doApplication();
    }
	
    doPassport();
    doCashier();

    /* Add code here to Broadcast() when a senator leaves */
    Acquire(senatorOutsideLineLock, -1);
    Acquire(senatorInsideLock, -1);
    Acquire(customerOutsideLineLock, -1);
	/*senatorInside = false;*/
	SetMV(senatorInside, -1, false);
	/*if(senatorsOutside > 0)*/
	if(GetMV(senatorsOutside, -1) > 0)
		Broadcast(senatorOutsideLineCV, -1, senatorOutsideLineLock, -1);
	Release(senatorInsideLock, -1);
	Release(senatorOutsideLineLock, -1);
	/*if there are no other senators outside, then customers can come in */
	/*if(senatorsOutside == false && customersOutside > 0)*/
	if(GetMV(senatorsOutside, -1) == false && GetMV(customersOutside, -1) > 0) 
		Broadcast(customerOutsideLineCV, -1, customerOutsideLineLock, -1);
	Release(customerOutsideLineLock, -1);
}

void runCustomer() {
	int i;
	Print("Running Customer: %i\n", ssn);

	Acquire(senatorOutsideLineLock, -1);
	Acquire(senatorInsideLock, -1);

	if(GetMV(senatorsOutside, -1) > 0 || GetMV(senatorInside, -1)) {
		Release(senatorInsideLock, -1);
		Release(senatorOutsideLineLock, -1);
		Acquire(customerOutsideLineLock, -1);
		i = GetMV(customersOutside, -1);
		SetMV(customersOutside, -1, i + 1);
		Wait(customerOutsideLineCV, -1, customerOutsideLineLock, -1);
		Print("Customer %i is notified that there are no more senators, and can come in now.\n", ssn);
		i = GetMV(customersOutside, -1);
		SetMV(customersOutside, -1, i - 1);
		Release(customerOutsideLineLock, -1);
	}
	
	Release(senatorInsideLock, -1);
	Release(senatorOutsideLineLock, -1);

	/* Customer has 1 in 10 chance of going to Passport Clerk first*/
	if(Rand() % 10 == 9999) 
        doPassport();

    /* Randomly decide whether to go to AppClerk or PicClerk first */
   if(true) {
        doApplication();
        doPicture();
    } else {
        doPicture();
        doApplication();
    }
	/* Customer has 1 in 10 chance of going to Cashier before PassportClerk */
	if(Rand() % 10 == 9999) 
        doCashier();

    doPassport();
    doCashier();
}

int main() {
	Setup();
    
    Acquire(customer.indexLock, -1);
    ssn = GetMV(customer.index, -1);
    SetMV(customer.index, -1, ssn + 1);
    Release(customer.indexLock, -1);
    
    if(GetMV(customer.isSenator, ssn))
		runSenator();
	else
		runCustomer();

	SetMV(customer.leftOffice, ssn, true);
    
	Exit(0);
}









