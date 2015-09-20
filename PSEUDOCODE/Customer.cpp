Lock senatorOutsideLineLock;
int senatorsOutside = 0;
Lock senatorInsideLock;
book senatorInside = false;
Lock customersOutsideLineLock;
int customersOutside = 0;

class Customer {

	private:
		char* ssn;
		int cash;
		Lock access;
		bool isSenator = false;
		
		bool hasApp;
		bool hasPic;
		bool hasPassport;
		bool seenAppClerk;
		bool seenPicClerk;
		bool seenPassportClerk;
		
		main() {
			access.Acquire();
			
			// Determine money and bribes
			
			// Senator stuff
			chooseIfImASenator();
			senatorOutsideLineLock.Acquire();
			senatorInsideLock.Acquire();
			if (isSenator) {
				if (senatorsOutside > 0 || senatorInside) {
					senatorsOutside += 1;
					senatorInsideLock.Release();
					Wait(SenatorsOutsideLineLock);
					senatorInsideLock.Acquire();
					senatorsOutside -= 1;
				}

				senatorInside = true;
				senatorOutsideLineLock.Release();
				senatorInsideLock.Release();
				
				appLineLock.Acquire();
				Broadcast(appLineLock);
				appLineLock.Release();
				
				picLineLock.Acquire();
				Broadcast(picLineLock);
				picLineLock.Release();
				
				// And so on, for each line INCLUDING PAID LINES
				
			} else {
				if (senatorsOutside > 0 || senatorInside) {
					senatorOutsideLineLock.Release();
					senatorInsideLock.Release();
					customersOutsideLineLock.Acquire();
					customersOutside += 1;
					Wait(customersOutsideLineLock);
					customersOutside -= 1;
				}
				senatorOutsideLineLock.Release();
				senatorInsideLock.Release();
			}
			
			// Ready to enter, choose a line
			int appLineLength;
			int picLineLength;
			
			if (willBribeApp) {
				appBribeLineLock.Acquire();
				appLineLength = appBribeLineLength;
			} else {
				appPlebLineLock.Acquire();
				appLineLength = appPlebLineLength;
			}
			if (willBribePic) {
				picBribeLineLock.Acquire();
				picLineLength = picBribeLineLength;
			} else {
				picPlebLineLock.Acquire();
				picLineLength = picPlebLineLength;
			}
			
			appBribeLineLock.Release();
			appPlebLineLock.Release();
			picBribeLineLock.Release();
			picPlebLineLock.Release();
			
			access.Release();
			
			// Enter the first line chosen
			if (appLineLength <= picLineLength) {
				doApp();
				enterPicLine();
			} else {
				enterPicLine();
				doApp();
			}
			
			// Enter the passport line
			
			// Enter the cashier line
			
			// Leave this godforsaken place

		}
		
		doApp() {
			bool senatord = false;
			senatord = enterAppLine();
			while (senatored) {
				enterAppLine();
			}
			interactWithAppClerk();
		}
		
		bool enterAppLine() {
			int senatord = false;
			
			if (willBribeApp) {
				appBribeLineLock.Acquire();
				appBribeLineLength += 1;
				Wait(AppBribeLineLock);
				appBribeLineLength -= 1;
				appBribeLineLock.Release();
			} else {
				appPlebLineLock.Acquire();
				appPlebLineLength += 1;
				Wait(AppPlebLineLock);
				appPlebLineLength -= 1;
				appPlebLineLock.Release();
			}
			
			senatorInside.Acquire();
			if (senatorInside) {
				customersOutsideLineLock.Acquire();
				customersOutside += 1;
				Wait(customersOutsideLineLock);
				customersOutside -= 1;
				customerOutsideLineLock.Release();
				senatord = true;
			}
			senatorInside.Release();
			
			return senatord;
		}
		
		interactWithAppClerk() {
			access.acquire();
			access.release();
		}
		
		enterPicLine() {
			// remember to include stuff about liking / not liking pic here!
		}
		
		interactWithPicClerk() {}
		
		enterPassportLine() {}
		
		interactWithPassportClerk() {}
		
		enterCashierLine() {}
		
		interactWithCashier() {}
		
		chooseIfImASenator() {}
		
	public:
	
		giveApp();
		givePic();
		givePassport();
		
		appClerkReady();
		picClerkReady();
		passportClerkReady();
		cashierReady();
		
		serviceFinished();
		youAreBad();
		
		
	

}