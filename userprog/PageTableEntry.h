#ifndef PAGETABLEENTRY_H
#define PAGETABLEENTRY_H
#ifdef CHANGED

enum DiskLocation {IN_EXECUTABLE, IN_MEMORY, IN_SWAP, UNINIT};

class PageTableEntry: public TranslationEntry {

public:

	DiskLocation location;
	int offset;
};
#endif
#endif
