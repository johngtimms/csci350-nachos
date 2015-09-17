// part2.cc 

#include "copyright.h"
#include "system.h"

typedef enum {AVAILABLE, BUSY, BREAK} clerkState;

struct ApplicationClerk {
    

};



void Part2() {
    Thread *t;
    char *ssn;
    printf("Creating 5 customers\n");
    for(int i = 0; i < 5; i++) {
        ssn = new char[20];
        sprintf(name,"%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)t4_waiter, 0);
    }
    
}
