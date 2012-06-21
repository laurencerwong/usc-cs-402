// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "exception.h"
#include <time.h>
#include <stdio.h>
#include <iostream>


using namespace std;


int copyin(unsigned int vaddr, int len, char *buf) {
	// Copy len bytes from the current thread's virtual address vaddr.
	// Return the number of bytes so read, or -1 if an error occors.
	// Errors can generally mean a bad virtual address was passed in.
	bool result;
	int n=0;			// The number of bytes copied in
	int *paddr = new int;

	while ( n >= 0 && n < len) {
		result = machine->ReadMem( vaddr, 1, paddr );
		while(!result) // FALL 09 CHANGES
		{
			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
		}

		buf[n++] = *paddr;

		if ( !result ) {
			//translation failed
			return -1;
		}

		vaddr++;
	}

	delete paddr;
	return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
	// Copy len bytes to the current thread's virtual address vaddr.
	// Return the number of bytes so written, or -1 if an error
	// occors.  Errors can generally mean a bad virtual address was
	// passed in.
	bool result;
	int n=0;			// The number of bytes copied in

	while ( n >= 0 && n < len) {
		// Note that we check every byte's address
		result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

		if ( !result ) {
			//translation failed
			return -1;
		}

		vaddr++;
	}

	return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
	// Create the file with the name in the user buffer pointed to by
	// vaddr.  The file name is at most MAXFILENAME chars long.  No
	// way to return errors, though...
	char *buf = new char[len+1];	// Kernel buffer to put the name in

	if (!buf) return;

	if( copyin(vaddr,len,buf) == -1 ) {
		printf("%s","Bad pointer passed to Create\n");
		delete buf;
		return;
	}

	buf[len]='\0';

	fileSystem->Create(buf,0);
	delete[] buf;
	return;
}

int Open_Syscall(unsigned int vaddr, int len) {
	// Open the file with the name in the user buffer pointed to by
	// vaddr.  The file name is at most MAXFILENAME chars long.  If
	// the file is opened successfully, it is put in the address
	// space's file table and an id returned that can find the file
	// later.  If there are any errors, -1 is returned.
	char *buf = new char[len+1];	// Kernel buffer to put the name in
	OpenFile *f;			// The new open file
	int id;				// The openfile id

	if (!buf) {
		printf("%s","Can't allocate kernel buffer in Open\n");
		return -1;
	}

	if( copyin(vaddr,len,buf) == -1 ) {
		printf("%s","Bad pointer passed to Open\n");
		delete[] buf;
		return -1;
	}

	buf[len]='\0';

	f = fileSystem->Open(buf);
	delete[] buf;

	if ( f ) {
		if ((id = currentThread->space->fileTable.Put(f)) == -1 )
			delete f;
		return id;
	}
	else
		return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
	// Write the buffer to the given disk file.  If ConsoleOutput is
	// the fileID, data goes to the synchronized console instead.  If
	// a Write arrives for the synchronized Console, and no such
	// console exists, create one. For disk files, the file is looked
	// up in the current address space's open file table and used as
	// the target of the write.

	char *buf;		// Kernel buffer for output
	OpenFile *f;	// Open file for output

	if ( id == ConsoleInput) return;

	if ( !(buf = new char[len]) ) {
		printf("%s","Error allocating kernel buffer for write!\n");
		return;
	} else {
		if ( copyin(vaddr,len,buf) == -1 ) {
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] buf;
			return;
		}
	}

	if ( id == ConsoleOutput) {
		for (int ii=0; ii<len; ii++) {
			printf("%c",buf[ii]);
		}

	} else {
		if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
			f->Write(buf, len);
		} else {
			printf("%s","Bad OpenFileId passed to Write\n");
			len = -1;
		}
	}

	delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
	// Write the buffer to the given disk file.  If ConsoleOutput is
	// the fileID, data goes to the synchronized console instead.  If
	// a Write arrives for the synchronized Console, and no such
	// console exists, create one.    We reuse len as the number of bytes
	// read, which is an unnessecary savings of space.
	char *buf;		// Kernel buffer for input
	OpenFile *f;	// Open file for output

	if ( id == ConsoleOutput) return -1;

	if ( !(buf = new char[len]) ) {
		printf("%s","Error allocating kernel buffer in Read\n");
		return -1;
	}

	if ( id == ConsoleInput) {
		//Reading from the keyboard
		scanf("%s", buf);

		if ( copyout(vaddr, len, buf) == -1 ) {
			printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	} else {
		if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
			len = f->Read(buf, len);
			if ( len > 0 ) {
				//Read something from the file. Put into user's address space
				if ( copyout(vaddr, len, buf) == -1 ) {
					printf("%s","Bad pointer passed to Read: data not copied\n");
				}
			}
		} else {
			printf("%s","Bad OpenFileId passed to Read\n");
			len = -1;
		}
	}

	delete[] buf;
	return len;
}

void Close_Syscall(int fd) {
	// Close the file associated with id fd.  No error reporting.
	OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

	if ( f ) {
		delete f;
	} else {
		printf("%s","Tried to close an unopen file\n");
	}
}

//#ifdef CHANGED

//Create a lock to be stored in kernel array lockTable.
//Can at most have MAX_LOCK_CONDITIONS number of locks, or abort
int CreateLock_Syscall(unsigned int nameIndex, int length){
	char* name = new char[length]; //allow user to pass in lock name for debug pruposes
	copyin(nameIndex, length, name);
	if(lockTableLock == NULL) lockTableLock = new Lock("Lock table lock"); // should execute only on the first call through.
								//this lock protects the BitMap of free locks
	lockTableLock->Acquire();
	if(lockArraySize == 0){ //instantiate lockTable on first call to CreateLock_Syscall
		lockTable = new LockEntry[MAX_LOCKS_CONDITIONS];
		lockMap = new BitMap(MAX_LOCKS_CONDITIONS);
		lockArraySize = MAX_LOCKS_CONDITIONS;
	}
	int nextFreeIndex = lockMap->Find(); //returns a free index in lockTable
	if(nextFreeIndex == -1){ //error, kernel is out of memory for locks and should terminate
		printf("We're all out of space for user program locks! Throwing away nachos!\n");
		interrupt->Halt();
	}
	lockTableLock->Release();
	ioLock->Acquire(); //make sure our print is done atomically
	printf("creating %s lock of index of %d\n", name, nextFreeIndex);
	ioLock->Release();
	//initialize the lockTable entry we grabbed for this lock
	lockTable[nextFreeIndex].lock = new Lock (name);
	lockTable[nextFreeIndex].lockSpace = currentThread->space;
	lockTable[nextFreeIndex].isToBeDeleted = false;
	lockTable[nextFreeIndex].acquireCount = 0;
	return nextFreeIndex;
}

//Create a condition to be stored in kernel array conditionTable.
//Can at most have MAX_LOCK_CONDITIONS number of condition or abort
int CreateCondition_Syscall(unsigned int nameIndex, int length){
	char* name = new char [length];
	copyin(nameIndex, length, name);
	if(conditionTableLock == NULL) conditionTableLock = new Lock("Condition table lock"); //should execute only on the first call in a run of nachos
	//this lock protects the BitMap and size for the conditionTable
	conditionTableLock->Acquire();
	if(conditionArraySize == 0){ //should only execute on first call to this function in a run of nachos
		conditionTable = new ConditionEntry[MAX_LOCKS_CONDITIONS];
		conditionMap = new BitMap(MAX_LOCKS_CONDITIONS);
		conditionArraySize = MAX_LOCKS_CONDITIONS;
	}
	int nextFreeIndex = conditionMap->Find();
	if(nextFreeIndex == -1){ //error, kernel is out of memory for locks and should terminate
		printf("Fatal system error, too many condition variables! Shut it down!\n");
		interrupt->Halt();
	}
	conditionTableLock->Release();
	ioLock->Acquire(); //make sure our print is atomic
	printf("creating %s CV of index of %d\n", name, nextFreeIndex);	
	ioLock->Release();
	//initialize the lockTable entry we grabbed for this lock
	conditionTable[nextFreeIndex].condition = new Condition (name);
	conditionTable[nextFreeIndex].conditionSpace = currentThread->space;
	conditionTable[nextFreeIndex].isToBeDeleted = false;
	return nextFreeIndex;
}

void Signal_Syscall(int conditionIndex, int lockIndex){
	IntStatus oldLevel = interrupt->SetLevel(IntOff); //turn off interrupts since we need our validations to be atomic
	if(conditionIndex < 0 || conditionIndex > conditionArraySize -1){ //array index is out of bounds
		printf("Thread %s called Signal with an invalid index %d\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(!conditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Signal on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(conditionTable[conditionIndex].conditionSpace != currentThread->space){ //the condition at this index belongs to another process
		printf("Thread %s called Signal on condition %d which does not belong to its address space\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(lockIndex < 0 || lockIndex > lockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Signal with an invalid lock index %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(!lockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Signal on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){ //the lock at this index belongs to another process
		printf("Thread %s called Signal on lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	//validation done, ok to perform operation
	conditionTable[conditionIndex].condition->Signal(lockTable[lockIndex].lock);
	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void Broadcast_Syscall(int conditionIndex, int lockIndex){
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	if(conditionIndex < 0 || conditionIndex > conditionArraySize -1){ //array index is out of bounds
		printf("Thread %s called Broadcast with an invalid index %d\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(!conditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Broadcast on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(conditionTable[conditionIndex].conditionSpace != currentThread->space){ //the condition at this index belongs to another process
		printf("Thread %s called Broadcast on condition %d which does not belong to its address space\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(lockIndex < 0 || lockIndex > lockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Broadcast with an invalid lock index %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(!lockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Broadcast with a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){ //the lock at this index belongs to another process
		printf("Thread %s called Broadcast with lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	//validation done, ok to perform operation
	conditionTable[conditionIndex].condition->Broadcast(lockTable[lockIndex].lock);
	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void Wait_Syscall(int conditionIndex, int lockIndex){
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	if(conditionIndex < 0 || conditionIndex > conditionArraySize -1){ //array index is out of bounds
		printf("Thread %s called Wait with an invalid index %d\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(!conditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Wait on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(conditionTable[conditionIndex].conditionSpace != currentThread->space){ //the condition at this index belongs to another process
		printf("Thread %s called Wait on condition %d which does not belong to its address space\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(lockIndex < 0 || lockIndex > lockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Wait with an invalid lock index %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(!lockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Wait with a lock index that does not exist: %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){ //the lock at this index belongs to another process
		printf("Thread %s called Wait with lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	//validation done, ok to perform operation
	conditionTable[conditionIndex].condition->Wait(lockTable[lockIndex].lock);
	//need to check if the condition has been marked for deletion.  if it has,
	//okay to delete if no one is currently in the condition's queue (this is an atomic operation, so no thread is in the middle of calling wait right now either)
	if(conditionTable[conditionIndex].isToBeDeleted){
		if(!(conditionTable[conditionIndex].condition->hasWaiting())){
			delete conditionTable[conditionIndex].condition;
			conditionMap->Clear(conditionIndex);
		}
	}
	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void DestroyLock_Syscall(int lockIndex){
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	if(lockIndex < 0 || lockIndex > lockArraySize -1){ //array index is out of bounds
		printf("Thread %s tried to destroy a lock with an invalid index %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(!lockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s tried to destroy a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){ //the lock at this index belongs to another process
		printf("Thread %s tried to destroy a lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	//validation done, ok to perform operation
	if(!(lockTable[lockIndex].lock->isBusy() && lockTable[lockIndex].acquireCount == 0)){ //delete now if no one has called an Acquire syscall without a Release syscall
		delete lockTable[lockIndex].lock;
		lockMap->Clear(lockIndex);
	}
	else{ //some thread is depending on the thread being there, so just defer deletion
		lockTable[lockIndex].isToBeDeleted = true;
	}
	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void DestroyCondition_Syscall(int conditionIndex){
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	if(conditionIndex < 0 || conditionIndex > conditionArraySize -1){ //array index is out of bounds
		printf("Thread %s tried to delete a condition with an invalid index %d\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(!conditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s tried to delete a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(conditionTable[conditionIndex].conditionSpace != currentThread->space){ //the condition at this index belongs to another process
		printf("Thread %s tried to delete condition %d which does not belong to its address space\n", currentThread->getName(), conditionIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	//validation done, ok to perform operation
	if(!(conditionTable[conditionIndex].condition->hasWaiting())){ //okay to delete if no threads in condition's queue
		delete conditionTable[conditionIndex].condition;
		conditionMap->Clear(conditionIndex);
	}
	else{ //if someone is in condition's queue, defer deletion
		conditionTable[conditionIndex].isToBeDeleted = true;
	}
	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void Yield_Syscall(){
	currentThread->Yield();
}


void Acquire_Syscall(int lockIndex){
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	if(lockIndex < 0 || lockIndex > lockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Acquire with an invalid index %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(!lockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Acquire on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){ //the lock at this index belongs to another process
		printf("Thread %s called Acquire on lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	//validation done, ok to perform operation
	lockTable[lockIndex].acquireCount++; //marks this lock as being held by a user thread, protects lock from deletion til same thread call Release syscall
	lockTable[lockIndex].lock->Acquire();
	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void Release_Syscall(int lockIndex){
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	if(lockIndex < 0 || lockIndex > lockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Release with an invalid index %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(!lockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Release on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){ //the lock at this index belongs to another process
		printf("Thread %s called Release on lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	//validation done, ok to perform operation
	lockTable[lockIndex].acquireCount--; //decrease count since currentThread no longer depends on lock
	lockTable[lockIndex].lock->Release();
	if(lockTable[lockIndex].isToBeDeleted && lockTable[lockIndex].acquireCount == 0){ //check if needs to be deleted, delete if no one is waiting
		if(!(lockTable[lockIndex].lock->isBusy())){
			delete lockTable[lockIndex].lock;
			lockMap->Clear(lockIndex);
		}
	}
	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}


//First function a new user thread executes.  Our Fork_Syscall function forks to this unconditionally.
//The argument vaddr is the logical address of the function the call user program intends to fork to
void Create_Kernel_Thread_Fork(unsigned int vaddr){
	machine->WriteRegister(PCReg, vaddr); //point the machine to the first instruction in the user program function
	machine->WriteRegister(NextPCReg, vaddr + 4);
	currentThread->space->RestoreState(); //make sure machine is working with the correct pageTable with our executable code
	//allocate the thread's stack location at the bottom of the 8 pages belonging to it.
	int stackLoc = (currentThread->space->numExecutablePages /*numCodeDataPages*/ + ((currentThread->threadID /*offset*/ + 1) * 8)) * PageSize - 16;
	//record where the thread's stack starts in processTable we can release next time the thread calls Exit
	processTable[currentThread->space->processID].threadStacks[currentThread->threadID] = (currentThread->space->numExecutablePages + ((currentThread->threadID + 1) * 8));
	machine->WriteRegister(StackReg, stackLoc ); //give machine the stack location
	machine->Run(); //begin execution of user program function
}

//first function a new user thread executes if it is the first in a new user process.  Our Exec_Syscall forks to this unconditionally
//differs from Create_Kernel_Thread_Fork because we always know what virtual address to execute (the main) and where our stack should be (first slot)
void Create_Kernel_Thread_Exec() {
	currentThread->space->InitRegisters(); //Set up PC/PCNext/StackRegisters
	currentThread->space->RestoreState(); //ensure machine has correct pageTable
	machine->Run(); //begin execution of new process
}

//syscall a user accesses when they fork.  vaddr is logical address of function to be forked, and other two arguments are so
//the new kernel Thread can be named by the user
void Fork_Syscall(unsigned int vaddr, unsigned int nameIndex, int length){
	char* name = new char[length + 1];
	copyin(nameIndex, length, name); //retrieve name for thread
	Thread* t = new Thread(name);
	t->space = currentThread->space;
	processIDLock.Acquire(); //threadIDs must be retrieved in mutually exclusive manner
	t->threadID = processTable[t->space->processID].nextThreadID;
	processTable[t->space->processID].nextThreadID++; //update so next thread doesn't grab this thread's ID
	processTable[t->space->processID].numThreadsAlive++; //allows Exit to tell there is another ReadyToRun thread for process
	processIDLock.Release();
	t->Fork((VoidFunctionPtr)Create_Kernel_Thread_Fork, vaddr);
}

//syscall a user accesses when they exec.  first two arguments are how the executable name is retrieved,
//second two are so the user can name the kernel level thread
void Exec_Syscall(unsigned int fileName, int filenameLength, unsigned int nameIndex, int nameLength){
	char* buf = new char[filenameLength + 1];
	copyin(fileName, filenameLength, buf);  //retrieve executable name
	OpenFile *executable = fileSystem->Open(buf);
	AddrSpace* space = new AddrSpace(executable); //first part of process, address space, instantiated

	processIDLock.Acquire(); //processTable must be accessed in mutually exclusive manner

	if(nextProcessID == MAX_PROCESSES){
		printf("Fatal error, system is out of memory.  Nachos terminating.\n");
		interrupt->Halt();
	}

	space->processID = nextProcessID;
	numLivingProcesses++; //information for Exit
	nextProcessID++; //make sure no one else can get our id
	buf = new char[nameLength];
	copyin(nameIndex, nameLength, buf); //retrieve user's desired thread name
	Thread* t = new Thread(buf);
	t->space = space;
	t->threadID = processTable[space->processID].nextThreadID; //have to also set thread id
	processTable[space->processID].processID = space->processID;
	processTable[space->processID].nextThreadID++;
	processTable[space->processID].numThreadsAlive++;
	t->Fork((VoidFunctionPtr)Create_Kernel_Thread_Exec, 0);
	processIDLock.Release();
}

//encodes 2 ints into a single value. usually to allow user to pass more values to NPrint
int NEncode2to1_Syscall(int v1, int v2) {
	if( ((v1 & 0xffff0000) != 0) || ((v2 & 0xffff0000) != 0)) { //decode algorithm zero extends numbers, so we shouldn't get passed negative numbers
		cout << "WARNING: values passed to NEncode2to1 should be limited to 16 bits.  "
				<< v1 << " and " << v2 << " were passed" << endl;
	}

	int res = (v2 << 16) | (v1 & 0x0000ffff);
	return res;
}

void NDecode1to2_Syscall(int v, int targetV1, int targetV2) {
	/*
	int target[2];
	char targetChars[8];
	target[0] = (v & 0x0000ffff);
	target[1] = (v & 0xffff0000) >> 16;

	for(int i = 0; i < 4; i++) {
		targetChars[i] = target[0] >> (i * 4);
		targetChars[i + 4] = target[1] >> (i * 4);
	}

	copyout(targetV1, 4, targetChars);
	copyout(targetV2, 4, targetChars + 4);
	 */

	cout << "WARNING: NDecode doesn't do anything" << endl;

	//	cout << "NDecode1to2: value 1 = " << target[0] << " value 2 = " << target[1] << endl;
}

//takes  an int and takes its upper 2 bytes and translates them into a 0 extended int,
// same with the lower 2.  allows user to pass in encoded values to print up to 4 ints with an NPrint syscall
void decode2to1(int v, int target[2]) {
	target[0] = (v & 0x0000ffff);
	target[1] = (v & 0xffff0000) >> 16;
}

void NPrint_Syscall(int outputString, int length, int encodedVal1, int encodedVal2){
	//cout << "In NPrint syscall... starting" << endl;
	char *buf = new char[length + 1];
	buf[length] = NULL;
	copyin(outputString, length, buf);
	//cout << "In NPrint syscall... allocated buffer" << endl;

	//cout << "NPrint encoded values: " << encodedVal1 << " " << encodedVal2 << endl;

	int t1[2];
	int t2[2];
	decode2to1(encodedVal1, t1);
	decode2to1(encodedVal2, t2);
	//cout << "In NPrint syscall... finished decoding" << endl;
	//cout << (char*)buf << endl;
	ioLock->Acquire();
	printf(buf, t1[0], t1[1], t2[0], t2[1]);
	ioLock->Release();
	//cout << "In NPrint syscall... finished printing" << endl;
}


void Exit_Syscall() {

	processIDLock.Acquire();

	if(numLivingProcesses == 1 && processTable[currentThread->space->processID].numThreadsAlive == 1) {
		//I am the last thread of the last process
		//cout << "Exiting: Last thread of last process" << endl;
		interrupt->Halt();
	}
	else if(numLivingProcesses > 1 && processTable[currentThread->space->processID].numThreadsAlive == 1) {
		//I am the last thread in a process, but there are other processes
		//cout << "Exiting: Last thread of a process, more processes exist" << endl;
		AddrSpace *currentSpace = currentThread->space;

		for(int i = 0; i < lockArraySize; i++) {
			if(lockTable[i].lockSpace == currentSpace) {
				lockTable[i].lockSpace = NULL;
				lockTable[i].isToBeDeleted = false;
				delete lockTable[i].lock;
			}
		}

		for(int i = 0; i < conditionArraySize; i++) {
			if(conditionTable[i].conditionSpace == currentSpace) {
				conditionTable[i].conditionSpace = NULL;
				conditionTable[i].isToBeDeleted = false;
				delete conditionTable[i].condition;
			}
		}

		delete currentThread->space;
		numLivingProcesses--;

		processIDLock.Release();
		currentThread->Finish();
	}
	else if(processTable[currentThread->space->processID].numThreadsAlive > 1) {
		//I am just a thread (not the last) in a process (also not the last)
		//cout << "Exiting: a thread of a process, more processes and threads exist: " << endl;
		//cout << processTable[currentThread->space->processID].numThreadsAlive << " threads remain in this process ("
		//	 << currentThread->space->processID << ") and " << numLivingProcesses
		//	 << " processes are still alive" << endl;

		processTable[currentThread->space->processID].numThreadsAlive--;

		int lastStackPage = processTable[currentThread->space->processID].threadStacks[currentThread->threadID];
		//cout << "last stack page" << lastStackPage << endl;

		for(int i = 0; i < 7; i++) {
			//cout << "clearing out stack... virtpage: " << lastStackPage - i
			//		<<  "  phys page: " << machine->pageTable[lastStackPage - i].physicalPage << endl;
			mainMemoryBitmap->Clear(machine->pageTable[lastStackPage - i].physicalPage);
		}

		processIDLock.Release();
		currentThread->Finish();
	}
	else {
		cout << "ERROR: Unknown Exit status!!" << endl;
		cout << "Number of living processes: " << numLivingProcesses << endl;
		cout << "Number of Threads for each process:" << endl;
		for(int i = 0; i < numLivingProcesses; i++) {
			cout << "Process: " << processTable[i].processID << "  Threads: " << processTable[i].numThreadsAlive << endl;
		}
		cout << "Terminating Nachos..." << endl;
		interrupt->Halt();
	}

	//currentThread->Finish();
}

int CreateQueue_Syscall(){
	if(queueArraySize == 0){
		queueTable = new queue<int>[5];
		queueMap = new BitMap(5);
		queueArraySize = 5;
	}
	int nextFreeIndex = queueMap->Find();
	if(nextFreeIndex == -1){
		queueMap->Resize();
		queue<int> *tempQueueTable = new queue<int>[queueArraySize *2];
		for(int i = 0; i < queueArraySize; i++){
			tempQueueTable[i] = queueTable[i];
		}
		queueArraySize *= 2;
		delete []queueTable;
		queueTable = tempQueueTable;
		nextFreeIndex = queueMap->Find();
	}
	return nextFreeIndex;
}

void QueuePush_Syscall(int queueNum, int whatToPush){
	if(queueNum < 0 || queueNum > queueArraySize){
		printf("Push on out of bounds queue %d was attempted\n", queueNum);
	}
	if(!queueMap->Test(queueNum)){
		printf("Push was called on queue that does not exist: %d\n", queueNum);
	}
	queueTable[queueNum].push(whatToPush);
}

int QueueFront_Syscall(int queueNum){
	if(queueNum < 0 || queueNum > queueArraySize){
		printf("Front on out of bounds queue %d was attempted\n", queueNum);
	}
	if(!queueMap->Test(queueNum)){
		printf("Front was called on queue that does not exist: %d\n", queueNum);
	}
	return queueTable[queueNum].front();
}

void QueuePop_Syscall(int queueNum){
	if(queueNum < 0 || queueNum > queueArraySize){
		printf("Pop on out of bounds queue %d was attempted\n", queueNum);
	}
	if(!queueMap->Test(queueNum)){
		printf("Pop was called on queue that does not exist: %d\n", queueNum);
	}
	queueTable[queueNum].pop();
}

int QueueEmpty_Syscall(int queueNum){
	if(queueNum < 0 || queueNum > queueArraySize){
		printf("Empty on out of bounds queue %d was attempted\n", queueNum);
	}
	if(!queueMap->Test(queueNum)){
		printf("Empty was called on queue that does not exist: %d\n", queueNum);
	}
	return  queueTable[queueNum].empty()?1:0;
}


int QueueSize_Syscall(int queueNum){
	if(queueNum < 0 || queueNum > queueArraySize){
		printf("Size on out of bounds queue %d was attempted\n", queueNum);
	}
	if(!queueMap->Test(queueNum)){
		printf("Size was called on queue that does not exist: %d\n", queueNum);
	}
	return   queueTable[queueNum].size();
}

int NRand_Syscall(){
	return rand();
}

time_t NTime_Syscall(){
	return time(NULL);
}

void NSrand_Syscall(unsigned int seed){
	srand(seed);
}

int ReadInt_Syscall(unsigned int vaddr, int size){
	char* buf = new char[size];
	copyin(vaddr, size, buf);
	ioLock->Acquire();
	printf("%s", buf);
	int choice;
	while(true){
		cin >> choice;
		if(cin.fail()){
			cin.clear();
			cin.ignore(100, '\n');
			cout << "Not a valid integer option. Please try again: ";
			continue;
		}
		else break;
	}
	ioLock->Release();
	return choice;
}

int RandInt_Syscall() {
	return rand();
}

//#endif

void ExceptionHandler(ExceptionType which) {
	int type = machine->ReadRegister(2); // Which syscall?
	int rv=0; 	// the return value from a syscall

	if ( which == SyscallException ) {
		switch (type) {
		default:
			DEBUG('a', "Unknown syscall - shutting down.\n");
		case SC_Halt:
			DEBUG('a', "Shutdown, initiated by user program.\n");
			interrupt->Halt();
			break;
		case SC_Create:
			DEBUG('a', "Create syscall.\n");
			Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_Open:
			DEBUG('a', "Open syscall.\n");
			rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_Write:
			DEBUG('a', "Write syscall.\n");
			Write_Syscall(machine->ReadRegister(4),
					machine->ReadRegister(5),
					machine->ReadRegister(6));
			break;
		case SC_Read:
			DEBUG('a', "Read syscall.\n");
			rv = Read_Syscall(machine->ReadRegister(4),
					machine->ReadRegister(5),
					machine->ReadRegister(6));
			break;
		case SC_Close:
			DEBUG('a', "Close syscall.\n");
			Close_Syscall(machine->ReadRegister(4));
			break;
		case SC_Acquire:
			DEBUG('a', "Acquire syscall.\n");
			Acquire_Syscall(machine->ReadRegister(4));
			break;
		case SC_Release:
			DEBUG('a', "Release syscall.\n");
			Release_Syscall(machine->ReadRegister(4));
			break;
		case SC_Signal:
			DEBUG('a', "Signal syscall.\n");
			Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_Wait:
			DEBUG('a', "Wait syscall.\n");
			Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_Broadcast:
			DEBUG('a', "Broadcast syscall.\n");
			Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_CreateLock:
			DEBUG('a', "CreateLock syscall.\n");
			rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_CreateCondition:
			DEBUG('a', "CreateCondition syscall.\n");
			rv = CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_Yield:
			DEBUG('a', "Yield syscall.\n");
			Yield_Syscall();
			break;
		case SC_DestroyLock:
			DEBUG('a', "DestroyLock syscall.\n");
			DestroyLock_Syscall(machine->ReadRegister(4));
			break;
		case SC_DestroyCondition:
			DEBUG('a', "DestroyCondition syscall.\n");
			DestroyCondition_Syscall(machine->ReadRegister(4));
			break;
		case SC_Fork:
			DEBUG('a', "Fork syscall.\n");
			Fork_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
			break;
		case SC_Exec:
			DEBUG('a', "Exec syscall.\n");
			Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6), machine->ReadRegister(7));
			break;
		case SC_NPrint:
			DEBUG('a', "NPrint syscall.\n");
			NPrint_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6), machine->ReadRegister(7));
			break;
		case SC_NEncode2to1:
			DEBUG('a', "NEncode2to1 syscall.\n");
			rv = NEncode2to1_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_Exit:
			DEBUG('a', "Exit syscall.\n");
			rv = machine->ReadRegister(4);
			Exit_Syscall();
			break;
		case SC_ReadInt:
			DEBUG('a', "ReadInt syscall.\n");
			rv = ReadInt_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_RandInt:
			DEBUG('a', "RandINt syscall.\n");
			rv = RandInt_Syscall();
			break;
		case SC_CreateQueue:
			rv = CreateQueue_Syscall();
			DEBUG('a', "CreateQueue syscall.\n");
			break;
		case SC_QueuePush:
			QueuePush_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			DEBUG('a', "QueuePush syscall.\n");
			break;
		case SC_QueueFront:
			rv = QueueFront_Syscall(machine->ReadRegister(4));
			DEBUG('a', "QueueFront syscall.\n");
			break;
		case SC_QueuePop:
			QueuePop_Syscall(machine->ReadRegister(4));
			DEBUG('a', "QueuePop syscall.\n");
			break;
		case SC_QueueEmpty:
			rv = QueueEmpty_Syscall(machine->ReadRegister(4));
			DEBUG('a', "QueueEmpty syscall.\n");
			break;
		case SC_QueueSize:
			rv = QueueSize_Syscall(machine->ReadRegister(4));
			DEBUG('a', "QueueSize syscall.\n");
			break;
		case SC_NRand:
			rv = NRand_Syscall();
			DEBUG('a', "Rand syscall.\n");
			break;
		case SC_NTime:
			rv = NTime_Syscall();
			DEBUG('a', "Time syscall.\n");
			break;
		case SC_NSrand:
			NSrand_Syscall(machine->ReadRegister(4));
			DEBUG('a', "Srand syscall.\n");
			break;
		}

		// Put in the return value and increment the PC
		machine->WriteRegister(2,rv);
		machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
		return;
	} else {
		cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
		interrupt->Halt();
	}
}
