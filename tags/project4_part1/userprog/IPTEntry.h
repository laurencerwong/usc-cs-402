
#ifndef IPTENTRY_H
#define IPTENTRY_H
#ifdef CHANGED
#include "addrspace.h"

class IPTEntry: public TranslationEntry {
public:
	AddrSpace *space;
};
#endif
#endif
