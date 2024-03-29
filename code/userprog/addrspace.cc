// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "table.h"
#include "synch.h"
#include "PageTableEntry.h"

extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
	table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
	}
	lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

#ifdef CHANGED
AddrSpace::AddrSpace(OpenFile *executableIn) : fileTable(MaxOpenFiles) {
	executable = executableIn;
    unsigned int i, size;
    NoffHeader noffH;


    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);
    pageTableLock = new Lock("pageTable lock");

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
    numPages = divRoundUp(size, PageSize) + MAX_THREADS * divRoundUp(UserStackSize,PageSize); // i think we add divRoundUp(UserStackSize,PageSize) for the number of max_threads
                                                // we need to increase the size
						// to leave room for the stack
    size = numPages * PageSize;

   // ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
    // first, set up the translation

    //remember this so we can allocate stacks later taking this into account
    //also useful for an exiting process, so we can free the main memory physical pages we will allocate
    numExecutablePages = divRoundUp(noffH.code.size + noffH.initData.size, PageSize);
//    numExecutablePages = numPages;
    pageTable = new PageTableEntry[numPages];
    for (i = 0; i < numPages; i++) {
    	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
    	pageTable[i].valid = FALSE;
    	pageTable[i].use = FALSE;
    	pageTable[i].dirty = FALSE;
    	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
    	// a separate page, we could set its
    	// pages to be read-only
    	if(i < numExecutablePages){
    		pageTable[i].location = IN_EXECUTABLE;
    	}
    	else{
    		pageTable[i].location = UNINIT;
    	}
    	pageTable[i].offset = noffH.code.inFileAddr + i * PageSize;

    	//copy executable over
//    	if(i < (unsigned int)numExecutablePages) { // copy executable page by page (if we are still have any executable left to copy)
//    		int tempPhysAddr = pageTable[i].physicalPage * PageSize;
//    		executable->ReadAt(&(machine->mainMemory[tempPhysAddr]), PageSize, noffH.code.inFileAddr + i * PageSize);
//
//    	}]
    }
   numExecutablePages = divRoundUp(noffH.code.size + noffH.initData.size + noffH.uninitData.size, PageSize);

}
#endif




//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
    currentThread->Finish();
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
#ifdef CHANGED
	int stackLoc = (numExecutablePages /*numCodeDataPages*/ + ((0 + 1) * 8)) * PageSize - 16;
	processTable[currentThread->space->processID].threadStacks[currentThread->threadID] = numExecutablePages + 8;
	//cout << "num executable pages: " << numExecutablePages << endl;
	//cout << "initializing stack location to: " << stackLoc << " for thread " << currentThread->threadID << " in process " << currentThread->space->processID << endl;
	machine->WriteRegister(StackReg, stackLoc);
   // machine->WriteRegister(StackReg, numPages * PageSize - 16);
#endif
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
#ifdef CHANGED
	//IntStatus old = interrupt->SetLevel(IntOff);
	//invalidating tlb
	for(int i = 0; i < TLBSize; i++){

		if(machine->tlb[i].valid == TRUE)IPT[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;
		machine->tlb[i].valid = FALSE;
	}
	//(void)interrupt->SetLevel(old);
#endif
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
	//invalidating tlb
	//IntStatus old = interrupt->SetLevel(IntOff);
	for(int i = 0; i < TLBSize; i++){

		if(machine->tlb[i].valid == TRUE)IPT[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;
		machine->tlb[i].valid = false;
	}
	//(void)interrupt->SetLevel(old);
}
