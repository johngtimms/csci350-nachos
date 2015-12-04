#include "syscall.h"
#include "setup.h"

int i;

void runCashier() {
	int myCustomer, money;
	Print("Running Cashier: %i\n", i);
	while(true) {
    	if(GetMV(cashier.senatorLineLength, i) > 0) {
    		Acquire(cashier.senatorLineLock, i);
            Signal(cashier.senatorLineCV, i, cashier.senatorLineLock, i); 
            Print("Cashier %i has signalled a Senator to come to their counter\n", i);
            Acquire(cashier.clerkLock, i); 
            Release(cashier.senatorLineLock, i);
            Wait(cashier.clerkCV, i, cashier.clerkLock, i); 
            myCustomer = GetMV(cashier.customerID, i);
            Print("Cashier %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Senator %i\n", myCustomer);
            if(GetMV(customer.certifiedByPassportClerk, myCustomer)) {
            	Print("Cashier %i has verified that ", i);
            	Print("Senator %i has been certified by a PassportClerk\n", myCustomer);
            	if(GetMV(customer.hasPaidForPassport, myCustomer) == false) {
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Senator %i after ceritification\n", myCustomer);
                    SetMV(customer.hasPaidForPassport, myCustomer, true);
	                Acquire(cashier.moneyLock, i);
                    money = GetMV(cashier.money, i);
                    SetMV(cashier.money, i, money + 100);
	                Release(cashier.moneyLock, i);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Senator %i their completed passport\n", myCustomer);
            	Print("Cashier %i has recorded that ", i);
    			Print("Senator %i has been given their completed passport.\n", myCustomer);
            } else {
                if(GetMV(customer.hasPaidForPassport, myCustomer) == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Senator %i before ceritification\n", myCustomer);
                    SetMV(customer.hasPaidForPassport, myCustomer, true);
	                Acquire(cashier.moneyLock, i);
                    money = GetMV(cashier.money, i);
                    SetMV(cashier.money, i, money + 100);
	                Release(cashier.moneyLock, i);
            	}
            }
            Signal(cashier.clerkCV, i, cashier.clerkLock, i);
            Wait(cashier.clerkCV, i, cashier.clerkLock, i);   
            SetMV(cashier.customerID, i, -1);
            Release(cashier.clerkLock, i);
            SetMV(cashier.state, i, FREE);
        } else if(GetMV(cashier.bribeLineLength, i) > 0 && !GetMV(senatorInside, -1) && !GetMV(senatorsOutside, -1)) {
            Acquire(cashier.bribeLineLock, i);
            Signal(cashier.bribeLineCV, i, cashier.bribeLineLock, i); 
            Print("Cashier %i has signalled a Customer to come to their counter\n", i);
            Acquire(cashier.clerkLock, i); 
            Release(cashier.bribeLineLock, i);
            Wait(cashier.clerkCV, i, cashier.clerkLock, i);
            myCustomer = GetMV(cashier.customerID, i);
            Print("Cashier %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            if(GetMV(customer.certifiedByPassportClerk, myCustomer)) {
            	Print("Cashier %i has verified that ", i);
            	Print("Customer %i has been certified by a PassportClerk\n", myCustomer);
                if(GetMV(customer.hasPaidForPassport, myCustomer) == false) {
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i after ceritification\n", myCustomer);
                    SetMV(customer.hasPaidForPassport, myCustomer, true);
	                Acquire(cashier.moneyLock, i);
                    money = GetMV(cashier.money, i);
                    SetMV(cashier.money, i, money + 100);
	                Release(cashier.moneyLock, i);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Customer %i their completed passport\n", myCustomer);
            	Signal(cashier.clerkCV, i, cashier.clerkLock, i);
            	Print("Cashier %i has recorded that ", i);
    			Print("Customer %i has been given their completed passport.\n", myCustomer);
            } else {
                if(GetMV(customer.hasPaidForPassport, myCustomer) == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i before ceritification\n", myCustomer);
                    SetMV(customer.hasPaidForPassport, myCustomer, true);
	                Acquire(cashier.moneyLock, i);
                    money = GetMV(cashier.money, i);
                    SetMV(cashier.money, i, money + 100);
	                Release(cashier.moneyLock, i);
            	}
            	Signal(cashier.clerkCV, i, cashier.clerkLock, i);
            }
            Wait(cashier.clerkCV, i, cashier.clerkLock, i);  
            SetMV(cashier.customerID, i, -1);
            Release(cashier.clerkLock, i);
            SetMV(cashier.state, i, FREE);
        } else if(GetMV(cashier.lineLength, i) > 0 && !GetMV(senatorInside, -1) && !GetMV(senatorsOutside, -1)) {
            Acquire(cashier.lineLock, i);
            Signal(cashier.lineCV, i, cashier.lineLock, i);
            Print("Cashier %i has signalled a Customer to come to their counter\n", i);
            Acquire(cashier.clerkLock, i); 
            Release(cashier.lineLock, i);
            Wait(cashier.clerkCV, i, cashier.clerkLock, i);
            myCustomer = GetMV(cashier.customerID, i);
            Print("Cashier %i ", i);
            Print("has recieved SSN %i ", myCustomer);
            Print("from Customer %i\n", myCustomer);
            if(GetMV(customer.certifiedByPassportClerk, myCustomer)) {
            	Print("Cashier %i has verified that ", i);
            	Print("Customer %i has been certified by a PassportClerk\n", myCustomer);
                if(GetMV(customer.hasPaidForPassport, myCustomer) == false) {
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i after ceritification\n", myCustomer);
                    SetMV(customer.hasPaidForPassport, myCustomer, true);
	                Acquire(cashier.moneyLock, i);
                    money = GetMV(cashier.money, i);
                    SetMV(cashier.money, i, money + 100);
	                Release(cashier.moneyLock, i);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Customer %i their completed passport\n", myCustomer);
            	Print("Cashier %i has recorded that ", i);
    			Print("Customer %i has been given their completed passport.\n", myCustomer);
            } else {
                if(GetMV(customer.hasPaidForPassport, myCustomer) == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i before ceritification.\n", myCustomer);
                    SetMV(customer.hasPaidForPassport, myCustomer, true);
	                Acquire(cashier.moneyLock, i);
                    money = GetMV(cashier.money, i);
                    SetMV(cashier.money, i, money + 100);
	                Release(cashier.moneyLock, i);
            	}
            }
            Signal(cashier.clerkCV, i, cashier.clerkLock, i);
            Wait(cashier.clerkCV, i, cashier.clerkLock, i);
            SetMV(cashier.customerID, i, -1);
            Release(cashier.clerkLock, i);
            SetMV(cashier.state, i, FREE);
        } else {
            Acquire(cashier.clerkLock, i);
            SetMV(cashier.state, i, BREAK);
            Print("Cashier %i is going on break\n", i);
            Wait(cashier.breakCV, i, cashier.clerkLock, i);
            Print("Cashier %i is coming off break\n", i);
            Release(cashier.clerkLock, i);
            SetMV(cashier.state, i, FREE);
            if(GetMV(timeToLeave, -1)) {
                Print("Cashier %i is leaving the office.\n",i);
                Exit(0);
            }
        }
    }
	Exit(0);
}

int main() {
    Setup();

	Acquire(cashier.indexLock, -1);
	i = GetMV(cashier.index, -1);
    SetMV(cashier.index, -1, i + 1);
	Release(cashier.indexLock, -1);
    
	runCashier();

    Exit(0);
}