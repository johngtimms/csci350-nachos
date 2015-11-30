#include "syscall.h"
#include "setup.h"

int i;

void runCashier() {
	int myCustomer, money;
	Print("Running Cashier: %i\n", i);
	while(true) {
        /*if(cashiers[i].senatorLineLength > 0) {*/
    	if(GetMV(cashiers[i].senatorLineLength) > 0) {
    		Acquire(cashiers[i].senatorLineLock);
            Signal(cashiers[i].senatorLineCV, cashiers[i].senatorLineLock); 
            Print("Cashier %i has signalled a Senator to come to their counter\n", i);
            Acquire(cashiers[i].clerkLock); 
            Release(cashiers[i].senatorLineLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock); 
            /*myCustomer = cashiers[i].customerID;*/
            myCustomer = GetMV(cashiers[i].customerID);
            Print("Cashier %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Senator %i\n", myCustomer);
            /*if(customers[myCustomer].certifiedByPassportClerk) {*/
            if(GetMV(customers[myCustomer].certifiedByPassportClerk)) {
            	Print("Cashier %i has verified that ", i);
            	Print("Senator %i has been certified by a PassportClerk\n", myCustomer);
                /*if(customers[myCustomer].hasPaidForPassport == false) {*/
            	if(GetMV(customers[myCustomer].hasPaidForPassport) == false) {
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Senator %i after ceritification\n", myCustomer);
		        	/*customers[myCustomer].hasPaidForPassport = true;*/
                    SetMV(customers[myCustomer].hasPaidForPassport, true);
	                Acquire(cashiers[i].moneyLock);
	                /*cashiers[i].money = cashiers[i].money + 100;*/
                    money = GetMV(cashiers[i].money);
                    SetMV(cashiers[i].money, money + 100);
	                Release(cashiers[i].moneyLock);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Senator %i their completed passport\n", myCustomer);
            	Print("Cashier %i has recorded that ", i);
    			Print("Senator %i has been given their completed passport.\n", myCustomer);
            } else {
            	/*if(customers[myCustomer].hasPaidForPassport == false) {*/
                if(GetMV(customers[myCustomer].hasPaidForPassport) == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Senator %i before ceritification\n", myCustomer);
		        	/*customers[myCustomer].hasPaidForPassport = true;*/
                    SetMV(customers[myCustomer].hasPaidForPassport, true);
	                Acquire(cashiers[i].moneyLock);
	                /*cashiers[i].money = cashiers[i].money + 100;*/
                    money = GetMV(cashiers[i].money);
                    SetMV(cashiers[i].money, money + 100);
	                Release(cashiers[i].moneyLock);
            	}
            }
            Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);   
            /*cashiers[i].customerID = -1;*/
            SetMV(cashiers[i].customerID, -1);
            Release(cashiers[i].clerkLock);
            /*cashiers[i].state = FREE;*/
            SetMV(cashiers[i].state, FREE);
    	/*} else if(cashiers[i].bribeLineLength > 0) {*/
        } else if(GetMV(cashiers[i].bribeLineLength) > 0) {
            Acquire(cashiers[i].bribeLineLock);
            Signal(cashiers[i].bribeLineCV, cashiers[i].bribeLineLock); 
            Print("Cashier %i has signalled a Customer to come to their counter\n", i);
            Acquire(cashiers[i].clerkLock); 
            Release(cashiers[i].bribeLineLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);
            /*myCustomer = cashiers[i].customerID;*/
            myCustomer = GetMV(cashiers[i].customerID);
            Print("Cashier %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            /*if(customers[myCustomer].certifiedByPassportClerk) {*/
            if(GetMV(customers[myCustomer].certifiedByPassportClerk)) {
            	Print("Cashier %i has verified that ", i);
            	Print("Customer %i has been certified by a PassportClerk\n", myCustomer);
            	/*if(customers[myCustomer].hasPaidForPassport == false) {*/
                if(GetMV(customers[myCustomer].hasPaidForPassport) == false) {
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i after ceritification\n", myCustomer);
		        	/*customers[myCustomer].hasPaidForPassport = true;*/
                    SetMV(customers[myCustomer].hasPaidForPassport, true);
	                Acquire(cashiers[i].moneyLock);
	                /*cashiers[i].money = cashiers[i].money + 100;*/
                    money = GetMV(cashiers[i].money);
                    SetMV(cashiers[i].money, money + 100);
	                Release(cashiers[i].moneyLock);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Customer %i their completed passport\n", myCustomer);
            	Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);
            	Print("Cashier %i has recorded that ", i);
    			Print("Customer %i has been given their completed passport.\n", myCustomer);
            } else {
            	/*if(customers[myCustomer].hasPaidForPassport == false) {*/
                if(GetMV(customers[myCustomer].hasPaidForPassport) == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i before ceritification\n", myCustomer);
		        	/*customers[myCustomer].hasPaidForPassport = true;*/
                    SetMV(customers[myCustomer].hasPaidForPassport, true);
	                Acquire(cashiers[i].moneyLock);
	                /*cashiers[i].money = cashiers[i].money + 100;*/
                    money = GetMV(cashiers[i].money);
                    SetMV(cashiers[i].money, money + 100);
	                Release(cashiers[i].moneyLock);
            	}
            	Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);
            }
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);  
            /*cashiers[i].customerID = -1;*/
            SetMV(cashiers[i].customerID, -1);
            Release(cashiers[i].clerkLock);
            /*cashiers[i].state = FREE;*/
            SetMV(cashiers[i].state, FREE);
        /*} else if(cashiers[i].lineLength > 0) {*/
        } else if(GetMV(cashiers[i].lineLength) > 0) {
            Acquire(cashiers[i].lineLock);
            Signal(cashiers[i].lineCV, cashiers[i].lineLock);
            Print("Cashier %i has signalled a Customer to come to their counter\n", i);
            Acquire(cashiers[i].clerkLock); 
            Release(cashiers[i].lineLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);
            /*myCustomer = cashiers[i].customerID;*/
            myCustomer = GetMV(cashiers[i].customerID);
            Print("Cashier %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            /*if(customers[myCustomer].certifiedByPassportClerk) {*/
            if(GetMV(customers[myCustomer].certifiedByPassportClerk)) {
            	Print("Cashier %i has verified that ", i);
            	Print("Customer %i has been certified by a PassportClerk\n", myCustomer);
            	/*if(customers[myCustomer].hasPaidForPassport == false) {*/
                if(GetMV(customers[myCustomer].hasPaidForPassport) == false) {
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i after ceritification\n", myCustomer);
		        	/*customers[myCustomer].hasPaidForPassport = true;*/
                    SetMV(customers[myCustomer].hasPaidForPassport, true);
	                Acquire(cashiers[i].moneyLock);
	                /*cashiers[i].money = cashiers[i].money + 100;*/
                    money = GetMV(cashiers[i].money);
                    SetMV(cashiers[i].money, money + 100);
	                Release(cashiers[i].moneyLock);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Customer %i their completed passport\n", myCustomer);
            	Print("Cashier %i has recorded that ", i);
    			Print("Customer %i has been given their completed passport.\n", myCustomer);
            } else {
            	/*if(customers[myCustomer].hasPaidForPassport == false) {*/
                if(GetMV(customers[myCustomer].hasPaidForPassport) == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i before ceritification.\n", myCustomer);
		        	/*customers[myCustomer].hasPaidForPassport = true;*/
                    SetMV(customers[myCustomer].hasPaidForPassport, true);
	                Acquire(cashiers[i].moneyLock);
	                /*cashiers[i].money = cashiers[i].money + 100;*/
                    money = GetMV(cashiers[i].money);
                    SetMV(cashiers[i].money, money + 100);
	                Release(cashiers[i].moneyLock);
            	}
            }
            Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);
            /*cashiers[i].customerID = -1;*/
            SetMV(cashiers[i].customerID, -1);
            Release(cashiers[i].clerkLock);
            /*cashiers[i].state = FREE;*/
            SetMV(cashiers[i].state, FREE);
        } else {
            Acquire(cashiers[i].clerkLock);
            /*cashiers[i].state = BREAK;*/
            SetMV(cashiers[i].state, BREAK);
            Print("Cashier %i is going on break\n", i);
            Wait(cashiers[i].breakCV, cashiers[i].clerkLock);
            Print("Cashier %i is coming off break\n", i);
            Release(cashiers[i].clerkLock);
            /*cashiers[i].state = FREE;*/
            SetMV(cashiers[i].state, FREE);
            if(GetMV(timeToLeave)) {
                Print("Cashier %i is leaving the office.\n",i);
                Exit(0);
            }
        }
    }
	Exit(0);
}

int main() {
    setup();
	Acquire(cashierIndexLock);
	i = GetMV(nextAvailableCashierIndex);
    SetMV(nextAvailableCashierIndex, i + 1);
	Release(cashierIndexLock);
    /*initClerk(CASHIER, i);*/
	runCashier();
	
}