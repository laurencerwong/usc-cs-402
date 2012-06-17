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
#ifdef CHANGED

int CreateLock_Syscall(int nameIndex, int length){
	char *name;
	copyin(nameIndex, length, name);
	if(lockArraySize == 0){
		lockTable = new LockEntry[50];
		lockMap = new BitMap(50);
		lockArraySize = 50;
	}
	int nextFreeIndex = lockMap->Find();
	if(nextFreeIndex == -1){
		lockMap->Resize();
		LockEntry *temp = new LockEntry[lockArraySize *2];
		for(int i = 0;i < lockArraySize; i++){
			temp[i] = lockTable[i];
		}
		lockArraySize *= 2;
		delete []lockTable;
		lockTable = temp;
		nextFreeIndex = lockMap->Find();
	}
	lockTable[nextFreeIndex].lock = new Lock (name);
	return nextFreeIndex;
}

int CreateCondition_Syscall(int nameIndex, int length){
	char *name;
	copyin(nameIndex, length, name);
	if(conditionArraySize == 0){
		conditionTable = new ConditionEntry[50];
		conditionMap = new BitMap(50);
		conditionArraySize = 50;
	}
	int nextFreeIndex = conditionMap->Find();
	if(nextFreeIndex == -1){
		conditionMap->Resize();
		ConditionEntry *temp = new ConditionEntry[conditionArraySize *2];
		for(int i = 0;i < conditionArraySize; i++){
			temp[i] = conditionTable[i];
		}
		conditionArraySize = conditionArraySize *2;
		delete []conditionTable;
		conditionTable = temp;
		nextFreeIndex = conditionMap->Find();
	}
	conditionTable[nextFreeIndex].condition = new Condition (name);
	return nextFreeIndex;
}

void Signal_Syscall(int conditionIndex, int lockIndex){
	if(conditionIndex < 0 || conditionIndex > conditionArraySize -1){
		printf("Thread %s called Signal with an invalid index %d\n", currentThread->getName(), conditionIndex);
	}
	if(!conditionMap->Test(conditionIndex)){
		printf("Thread %s called Signal on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
	}
	if(conditionTable[conditionIndex].conditionSpace != currentThread->space){
		printf("Thread %s called Signal on condition %d which does not belong to its address space\n", currentThread->getName(), conditionIndex);
	}
	if(lockIndex < 0 || lockIndex > lockArraySize -1){
		printf("Thread %s called Release in Signal with an invalid index %d\n", currentThread->getName(), lockIndex);
	}
	if(!lockMap->Test(lockIndex)){
		printf("Thread %s called Release in Signal on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){
		printf("Thread %s called Release in Signal on lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
	}
	conditionTable[conditionIndex].condition->Signal(lockTable[lockIndex].lock);
	if(conditionTable[conditionIndex].isToBeDeleted){
		if(!(conditionTable[conditionIndex].condition->hasWaiting())){
			delete conditionTable[conditionIndex].condition;
			conditionMap->Clear(conditionIndex);
		}
	}
}

void Broadcast_Syscall(int conditionIndex, int lockIndex){
	if(conditionIndex < 0 || conditionIndex > conditionArraySize -1){
		printf("Thread %s called Broadcast with an invalid index %d\n", currentThread->getName(), conditionIndex);
	}
	if(!conditionMap->Test(conditionIndex)){
		printf("Thread %s called Broadcast on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
	}
	if(conditionTable[conditionIndex].conditionSpace != currentThread->space){
		printf("Thread %s called Broadcast on condition %d which does not belong to its address space\n", currentThread->getName(), conditionIndex);
	}
	if(lockIndex < 0 || lockIndex > lockArraySize -1){
		printf("Thread %s called Release in Broadcast with an invalid index %d\n", currentThread->getName(), lockIndex);
	}
	if(!lockMap->Test(lockIndex)){
		printf("Thread %s called Release in Broadcast on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){
		printf("Thread %s called Release in Broadcast on lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
	}
	conditionTable[conditionIndex].condition->Broadcast(lockTable[lockIndex].lock);
	if(conditionTable[conditionIndex].isToBeDeleted){
		if(!(conditionTable[conditionIndex].condition->hasWaiting())){
			delete conditionTable[conditionIndex].condition;
			conditionMap->Clear(conditionIndex);
		}
	}
}

void Wait_Syscall(int conditionIndex, int lockIndex){
	if(conditionIndex < 0 || conditionIndex > conditionArraySize -1){
		printf("Thread %s called Wait with an invalid index %d\n", currentThread->getName(), conditionIndex);
	}
	if(!conditionMap->Test(conditionIndex)){
		printf("Thread %s called Wait on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
	}
	if(conditionTable[conditionIndex].conditionSpace != currentThread->space){
		printf("Thread %s called Wait on condition %d which does not belong to its address space\n", currentThread->getName(), conditionIndex);
	}
	if(lockIndex < 0 || lockIndex > lockArraySize -1){
		printf("Thread %s called Release in Wait with an invalid index %d\n", currentThread->getName(), lockIndex);
	}
	if(!lockMap->Test(lockIndex)){
		printf("Thread %s called Release in Wait on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){
		printf("Thread %s called Release in Wait on lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
	}
	conditionTable[conditionIndex].condition->Wait(lockTable[lockIndex].lock);
}

void DestroyLock_Syscall(int lockIndex){
	if(lockIndex < 0 || lockIndex > lockArraySize -1){
		printf("Thread %s tried to destroy a lock with an invalid index %d\n", currentThread->getName(), lockIndex);
	}
	if(!lockMap->Test(lockIndex)){
		printf("Thread %s tried to destroy a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){
		printf("Thread %s tried to destroy a lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
	}
	if(!(lockTable[lockIndex].lock->isBusy())){
		delete lockTable[lockIndex].lock;
		lockMap->Clear(lockIndex);
	}
	else{
		lockTable[lockIndex].isToBeDeleted = true;
	}
}

void DestroyCondition_Syscall(int conditionIndex){
	if(conditionIndex < 0 || conditionIndex > conditionArraySize -1){
		printf("Thread %s tried to delete a condition with an invalid index %d\n", currentThread->getName(), conditionIndex);
	}
	if(!conditionMap->Test(conditionIndex)){
		printf("Thread %s tried to delete a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
	}
	if(conditionTable[conditionIndex].conditionSpace != currentThread->space){
		printf("Thread %s tried to delete condition %d which does not belong to its address space\n", currentThread->getName(), conditionIndex);
	}
	if(!(conditionTable[conditionIndex].condition->hasWaiting())){
		delete conditionTable[conditionIndex].condition;
		conditionMap->Clear(conditionIndex);
	}
	else{
		conditionTable[conditionIndex].isToBeDeleted = true;
	}
}

void Yield_Syscall(){
	currentThread->Yield();
}


void Acquire_Syscall(int lockIndex){
	if(lockIndex < 0 || lockIndex > lockArraySize -1){
		printf("Thread %s called Acquire with an invalid index %d\n", currentThread->getName(), lockIndex);
	}
	if(!lockMap->Test(lockIndex)){
		printf("Thread %s called Acquire on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){
		printf("Thread %s called Acquire on lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
	}
	lockTable[lockIndex].lock->Acquire();
}

void Release_Syscall(int lockIndex){
	if(lockIndex < 0 || lockIndex > lockArraySize -1){
		printf("Thread %s called Release with an invalid index %d\n", currentThread->getName(), lockIndex);
	}
	if(!lockMap->Test(lockIndex)){
		printf("Thread %s called Release on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
	}
	if(lockTable[lockIndex].lockSpace != currentThread->space){
		printf("Thread %s called Release on lock %d which does not belong to its address space\n", currentThread->getName(), lockIndex);
	}
	lockTable[lockIndex].lock->Release();
	if(lockTable[lockIndex].isToBeDeleted){
		if(!(lockTable[lockIndex].lock->isBusy())){
			delete lockTable[lockIndex].lock;
			lockMap->Clear(lockIndex);
		}
	}
}



void Create_Kernel_Thread(unsigned int vaddr){
	currentThread->space->RestoreState();
	machine->WriteRegister(PCReg, vaddr);
	machine->WriteRegister(NextPCReg, vaddr + 4);
	int stackLoc = (12 /*numCodeDataPages*/ + ((currentThread->threadID /*offset*/ + 1) * 8)) * PageSize - 16;
	processTable[currentThread->space->processID].threadStacks[currentThread->threadID] =  stackLoc;
	machine->WriteRegister(StackReg, stackLoc );
	machine->Run();
}

void Fork_Syscall(unsigned int vaddr){
	//cout << "Forking a new thread..." << endl;
	Thread* t = new Thread("user thread");
	t->space = currentThread->space;
	//cout << "New thread's address space set to the current address space" << endl;
	processIDLock.Acquire();
	//cout << "Process table lock acquired" << endl;
	t->threadID = processTable[t->space->processID].nextThreadID;
	processTable[t->space->processID].nextThreadID++;
	processIDLock.Release();
	//cout << "Process table lock released, IDs set" << endl;

	t->Fork((VoidFunctionPtr)Create_Kernel_Thread, vaddr);
	//cout << "Thread Forked" << endl;
}

void Exec_Syscall(unsigned int fileName, int length){
	char buf[length + 1];
	copyin(fileName, length, buf);
	OpenFile *executable = fileSystem->Open(buf);
	AddrSpace* space = new AddrSpace(executable);

	processIDLock.Acquire();

	if(nextProcessID == MAX_PROCESSES){
		printf("Fatal error, system is out of memory.  Nachos terminating.\n");
		interrupt->Halt();
	}
	space->processID = nextProcessID;
	nextProcessID++;

	Thread* t = new Thread("main thread");
	t->space = space;
	t->threadID = processTable[space->processID].nextThreadID;
	processTable[space->processID].nextThreadID++;
	t->Fork((VoidFunctionPtr)Create_Kernel_Thread, 0);

	processIDLock.Release();
}

int NEncode2to1_Syscall(int v1, int v2) {
	int res = (v2 << 16) | (v1 & 0x0000ffff);
	//cout << "NEncode2to1: " << v1 << " and " << v2 << " encoded to: " << res << endl;
	return res;

//	return (v2 << 16) | (v1 & 0x0000ffff);
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

void decode2to1(int v, int target[2]) {
	target[0] = (v & 0x0000ffff);
	target[1] = (v & 0xffff0000) >> 16;
	//cout << "decode1to2: input : " << v << " value 1 = " << target[0] << " value 2 = " << target[1] << endl;
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
	printf(buf, t1[0], t1[1], t2[0], t2[1]);
	//cout << "In NPrint syscall... finished printing" << endl;
}


#endif

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
	    case SC_Broadcast:
		DEBUG('a', "Broadcast syscall.\n");
		Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
	    break;
	    case SC_CreateLock:
		DEBUG('a', "CreateLock syscall.\n");
		CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
	    break;
	    case SC_CreateCondition:
		DEBUG('a', "CreateCondition syscall.\n");
		CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
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
	    Fork_Syscall(machine->ReadRegister(4));
	    break;
	    case SC_Exec:
	    DEBUG('a', "Exec syscall.\n");
	    Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
	    break;
	    case SC_NPrint:
	    DEBUG('a', "NPrint syscall.\n");
	    NPrint_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6), machine->ReadRegister(7));
	    break;
	    case SC_NEncode2to1:
	    DEBUG('a', "NEncode2to1 syscall.\n");
	    rv = NEncode2to1_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
	    break;
	    case SC_NDecode1to2:
	    DEBUG('a', "NDecode1to2 syscall.\n");
	    NDecode1to2_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
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
