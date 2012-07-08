/*
 * server.cc
 *
 *  Created on: Jul 7, 2012
 *      Author: Keith
 */

#include "system.h"
#include "post.h"
#include "network.h"
#include "synch.h"
#include "messagetypes.h"

#define MAX_SERVER_LOCKS 500
#define MAX_SERVER_CVS 500

typedef struct ServerLockEntry{
	ServerLock *lock;
	bool isToBeDeleted;
	int acquireCount;
} LockEntry;

typedef struct ServerConditionEntry{
	ServerCondition *condition;
	bool isToBeDeleted;
} ConditionEntry;

BitMap *serverLockMap;
BitMap *serverConditionMap;

int serverLockArraySize = 0;
int serverConditionArraySize = 0;
ServerLockEntry *severLockTable;
ServerConditionEntry *serverConditionTable;

int ServerCreateLock(int machineID, int mailboxID, char* name) {
	if(lockArraySize == 0){ //instantiate lockTable on first call to CreateLock_Syscall
		serverLockTable = new LockEntry[MAX_SERVER_LOCKS];
		serverLockMap = new BitMap(MAX_SERVER_LOCKS);
		serverLockArraySize = MAX_SERVER_LOCKS;
	}

	int nextFreeIndex = serverLockMap->Find(); //returns a free index in lockTable
	if(nextFreeIndex == -1){ //error, kernel is out of memory for locks and should terminate
		printf("Out of space for user program locks!\n");
		//interrupt->Halt();
		//on the server, don't halt, send a -1 as lock index response
	}
	else {
		printf("Creating %s lock of index of %d\n", name, nextFreeIndex);
		//initialize the lockTable entry we grabbed for this lock
		serverLockTable[nextFreeIndex].lock = new ServerLock (name);
		serverLockTable[nextFreeIndex].isToBeDeleted = false;
		serverLockTable[nextFreeIndex].acquireCount = 0;
	}
	return nextFreeIndex;
}

void ServerDestroyLock(int lockIndex) {
	if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
		printf("Thread %s tried to destroy a lock with an invalid index %d\n", currentThread->getName(), lockIndex);
		return;
	}
	if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s tried to destroy a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		return;
	}
	//validation done, ok to perform operation
	if(!(serverLockTable[lockIndex].lock->isBusy() && serverLockTable[lockIndex].acquireCount == 0)){ //delete now if no one has called an Acquire syscall without a Release syscall
		delete serverLockTable[lockIndex].lock;
		serverLockMap->Clear(lockIndex);
	}
	else{ //some thread is depending on the thread being there, so just defer deletion
		lockTable[lockIndex].isToBeDeleted = true;
	}
}



void Server() {

	while(true) {
		//get message
		PacketHeader *packetHeader = new PacketHeader;
		MailHeader *mailHeader = new MailHeader;
		char* messageData = new char[MaxMailSize];

		postOffice->Receive(0, packetHeader, mailHeader, messageData);	//server is always mailbox 0 for now

		//parse message
		char messageType = messageData[0];
		int machineID = packetHeader->from;
		int mailboxNumber = mailHeader->from;

		int replyMachineID = machineID;
		int replyMailboxNumber = mailboxNumber;


		//handle the message
		switch(messageType) {	//first byte is the message type
		case CREATE_LOCK:	//data[9] = nameLength, data[10:10+nameLength] = name
			char nameLength = messageData[9];
			char* name = new char[nameLength];
			strncpy(name, (messageData + 10), nameLength);
			reply = ServerCreateLock(machineID, mailboxNumber, name);
			break;
		case DESTROY_LOCK:
			reply = ServerDestroyLock();
			break;


		case ACQUIRE:
			ClientRequest* temp = ServerAcquire(request);
			if(temp->respond){
				replyMachine = temp->machineID;
				replyMailbox = temp->mailboxNumber;
			}
			break;
		case RELEASE:
			reply = ServerRelease();
			break;
		case CREATE_CV:
			reply = ServerCreateCV(request);
			break;
		case DESTROY_CV:
			reply = ServerDestroyCV(request);
			break;
		case SIGNAL:	//condition, then lock in message
			reply = ServerSignal(request);
			break;
		case WAIT:
			reply = ServerWait(request);
			break;
		case BROADCAST:
			reply = ServerBroadcast(request);
			break;
		case CREATE_MV:
			reply = ServerCreateMV(request);
			break;
		case DESTROY_MV:
			reply = ServerDestroyMV(request);
			break;
		case GET_MV:
			reply = ServerGetMV(request);
			break;
		case SET_MV:
			reply = ServerSetMV(request);
			break;
		default:
			//oops...
			break;

		}

		//send response message


	}
}

