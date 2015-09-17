#include "copyright.h"
#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;
 

void PostOffice(int numCustomers, int numAppClerks, int numPicClerks, int numPassportClerks, int numCashiers) 
{
    cout << "Number of Customers: " <<  numCustomers << endl;
    cout << "Number of Appication Clerks: " << numAppClerks << endl;
    cout << "Number of Picture Clerks: " << numPicClerks << endl;
    cout << "Number of Passport Clerks: " << numPassportClerks << endl;
    cout << "Number of Cashiers: " << numCashiers << endl << endl;
}



// Run menu for Part 2 of assignment
void Problem2() 
{
    // Default values for Customers and Clerks
    int numCustomers = 20;
    int numAppClerks = 5;
    int numPicClerks = 5;
    int numPassportClerks = 5;
    int numCashiers = 5;

    cout << "Welcome to the Passport Office." << endl;
    while(true) {
        cout << "Please enter an option:" << endl;
        cout << " -a  (View/Edit default values)" << endl;
        cout << " -b  (Run a test)" << endl;
        cout << " -c  (Exit)" << endl;
        string input;
        getline(cin, input);
        if(input == "-a") {     // Print/Edit default values
            cout << "Number of Customers: " <<  numCustomers << endl;
            cout << "Number of Appication Clerks: " << numAppClerks << endl;
            cout << "Number of Picture Clerks: " << numPicClerks << endl;
            cout << "Number of Passport Clerks: " << numPassportClerks << endl;
            cout << "Number of Cashiers: " << numCashiers << endl << endl;
            cout << "Note: There can only be 20 - 50 Customers and 1 - 5 of each type of clerk." << endl;
            cout << "Please enter new values: " << endl;
            int num;
            cout << "Number of Customers: ";
            if(cin >> num && num >= 20 && num <= 50 )
                numCustomers = num;
            else {
                cout << "Invalid input. Number of Customers unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
            cout << "Number of Application Clerks: ";
            if(cin >> num && num >= 1 && num <= 5 )
                numAppClerks = num;
            else {
                cout << "Invalid input. Number of Application Clerks unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
            cout << "Number of Picture Clerks: ";
            if(cin >> num && num >= 1 && num <= 5 )
                numPicClerks = num;
            else {
                cout << "Invalid input. Number of Picture Clerks unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
            cout << "Number of Passport Clerks: ";
            if(cin >> num && num >= 1 && num <= 5 )
                numPassportClerks = num;
            else {
                cout << "Invalid input. Number of Passport Clerks unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
            cout << "Number of Cashiers: ";
            if(cin >> num && num >= 1 && num <= 5 )
                numCashiers = num;
            else {
                cout << "Invalid input. Number of Cashiers unchanged." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
        } else if(input == "-b") {  // Run a test
            PostOffice(numCustomers, numAppClerks, numPicClerks, numPassportClerks, numCashiers);
        } else if(input == "-c") {
            cout << "Exiting Passport Office." << endl;
            break;
        } else {
            cout << "Invalid input. Please try again." << endl;
        }
        cin.clear();
        cin.ignore(10000, '\n');
    }
}
