#ifndef IPT_H
#define IPT_H

#include "../machine/translate.h"
#include "addrspace.h"

class IPTEntry : public TranslationEntry {

public:
	AddrSpace *owner;

	IPTEntry(){
		owner = NULL;
	}

};

#endif