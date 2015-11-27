#include "syscall.h"
#include "setup.h"

int i;

void runCashier() {
	int myCustomer;
	
	Print("Running Cashier: %i\n", i);
	
	while(true) {
    	if(cashiers[i].senatorLineLength > 0) {
    		Acquire(cashiers[i].senatorLineLock);
            Signal(cashiers[i].senatorLineCV, cashiers[i].senatorLineLock); 
            Print("Cashier %i has signalled a Senator to come to their counter\n", i);
            Acquire(cashiers[i].clerkLock); 
            Release(cashiers[i].senatorLineLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock); 
            myCustomer = cashiers[i].customerID;
            Print("Cashier %i ", i);
            Print("has recieved SSN %i ", customers[myCustomer].SSN);
            Print("from Senator %i\n", customers[myCustomer].SSN);
            if(customers[myCustomer].certifiedByPassportClerk) {
            	Print("Cashier %i has verified that ", i);
            	Print("Senator %i has been certified by a PassportClerk\n", customers[myCustomer].SSN);
            	if(customers[myCustomer].hasPaidForPassport == false){
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Senator %i after ceritification\n", customers[myCustomer].SSN);
		        	customers[myCustomer].hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Senator %i their completed passport\n", customers[myCustomer].SSN);
            	Print("Cashier %i has recorded that ", i);
    			Print("Senator %i has been given their completed passport.\n", customers[myCustomer].SSN);
            } else {
            	if(customers[myCustomer].hasPaidForPassport == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Senator %i before ceritification\n", customers[myCustomer].SSN);
		        	customers[myCustomer].hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            }
            Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);   
            cashiers[i].customerID -1;
            Release(cashiers[i].clerkLock);
            cashiers[i].state = FREE;
    	} else if(cashiers[i].bribeLineLength > 0) {
            Acquire(cashiers[i].bribeLineLock);
            Signal(cashiers[i].bribeLineCV, cashiers[i].bribeLineLock); 
            Print("Cashier %i has signalled a Customer to come to their counter\n", i);
            Acquire(cashiers[i].clerkLock); 
            Release(cashiers[i].bribeLineLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);
            myCustomer = cashiers[i].customerID;
            Print("Cashier %i ", i);
            Print("has recieved SSN %i ", customers[myCustomer].SSN);
            Print("from Customer %i\n", customers[myCustomer].SSN);
            if(customers[myCustomer].certifiedByPassportClerk) {
            	Print("Cashier %i has verified that ", i);
            	Print("Customer %i has been certified by a PassportClerk\n", customers[myCustomer].SSN);
            	if(customers[myCustomer].hasPaidForPassport == false) {
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i after ceritification\n", customers[myCustomer].SSN);
		        	customers[myCustomer].hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Customer %i their completed passport\n", customers[myCustomer].SSN);
            	Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);
            	Print("Cashier %i has recorded that ", i);
    			Print("Customer %i has been given their completed passport.\n", customers[myCustomer].SSN);
            } else {
            	if(customers[myCustomer].hasPaidForPassport == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i before ceritification\n", customers[myCustomer].SSN);
		        	customers[myCustomer].hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            	Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);
            }
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);  
            cashiers[i].customerID -1;
            Release(cashiers[i].clerkLock);
            cashiers[i].state = FREE;
        } else if(cashiers[i].lineLength > 0) {
            Acquire(cashiers[i].lineLock);
            Signal(cashiers[i].lineCV, cashiers[i].lineLock);
            Print("Cashier %i has signalled a Customer to come to their counter\n", i);
            Acquire(cashiers[i].clerkLock); 
            Release(cashiers[i].lineLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);
            myCustomer = cashiers[i].customerID;
            Print("Cashier %i ", i);
            Print("has recieved SSN %i ", customers[myCustomer].SSN);
            Print("from Customer %i\n", customers[myCustomer].SSN);
            if(customers[myCustomer].certifiedByPassportClerk) {
            	Print("Cashier %i has verified that ", i);
            	Print("Customer %i has been certified by a PassportClerk\n", customers[myCustomer].SSN);
            	if(customers[myCustomer].hasPaidForPassport == false) {
		        	Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i after ceritification\n", customers[myCustomer].SSN);
		        	customers[myCustomer].hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            	Print("Cashier %i has provided ", i);
            	Print("Customer %i their completed passport\n", customers[myCustomer].SSN);
            	Print("Cashier %i has recorded that ", i);
    			Print("Customer %i has been given their completed passport.\n", customers[myCustomer].SSN);
            } else {
            	if(customers[myCustomer].hasPaidForPassport == false) {
            		Print("Cashier %i has receieved the $100 from ", i);
            		Print("Customer %i before ceritification.\n", customers[myCustomer].SSN);
		        	customers[myCustomer].hasPaidForPassport = true;
	                Acquire(cashiers[i].moneyLock);
	                cashiers[i].money = cashiers[i].money + 100;
	                Release(cashiers[i].moneyLock);
            	}
            }
            Signal(cashiers[i].clerkCV, cashiers[i].clerkLock);
            Wait(cashiers[i].clerkCV, cashiers[i].clerkLock);
            cashiers[i].customerID -1;
            Release(cashiers[i].clerkLock);
            cashiers[i].state = FREE;
        } else {
            Acquire(cashiers[i].clerkLock);
            cashiers[i].state = BREAK;
            Print("Cashier %i is going on break\n", i);
            Wait(cashiers[i].breakCV, cashiers[i].clerkLock);
            Print("Cashier %i is coming off break\n", i);
            Release(cashiers[i].clerkLock);
            cashiers[i].state = FREE;
        }
    }
	Exit(0);
}

int main() {
	Acquire(cashierIndexLock);
	i = nextAvailableCashierIndex;
	nextAvailableCashierIndex = nextAvailableCashierIndex + 1;
	Release(cashierIndexLock);

	runCashier();
	
}