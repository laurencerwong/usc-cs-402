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
#include <queue>

#define MAX_SERVER_LOCKS 500
#define MAX_SERVER_CVS 500
#define MAX_SERVER_MV_ARRAYS 500
#define MAX_SERVER_ENTRIES_PER_MV_ARRAY 500

using namespace std;

struct ServerLockEntry{
	ServerLock *lock;
	bool isToBeDeleted;
	int acquireCount;
};

struct ServerConditionEntry{
	ServerCondition *condition;
	bool isToBeDeleted;
};

struct ServerMVEntry {
	int *mvEntries;
	int numEntries;
	char *name;
};

BitMap *serverLockMap;
BitMap *serverConditionMap;
BitMap *serverMVMap;

int serverLockArraySize = 0;
int serverConditionArraySize = 0;
int serverMVArraySize = 0;
ServerLockEntry *serverLockTable;
ServerConditionEntry *serverConditionTable;
ServerMVEntry *serverMVTable;

struct ServerResponse {
	int toMachine;
	int toMailbox;
	int data;
	//int operationSuccessful;
};

queue<ServerResponse> necessaryResponses;

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
		serverLockTable[lockIndex].isToBeDeleted = true;
	}
}

ClientRequest* ServerAcquire(int machineID, int mailbox, int lockIndex) {
	if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Acquire with an invalid index %d\n", currentThread->getName(), lockIndex);
		return NULL;
	}
	if(!lockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Acquire on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		return NULL;
	}

	//validation done, ok to perform operation
	serverLockTable[lockIndex].acquireCount++; //marks this lock as being held by a user thread, protects lock from deletion til same thread call Release syscall
	return serverLockTable[lockIndex].lock->Acquire(new ClientRequest(machineID, mailbox));
}

ClientRequest* ServerRelease(int machineID, int mailbox, int lockIndex){

	if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Release with an invalid index %d\n", currentThread->getName(), lockIndex);
		return NULL;
	}
	if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Release on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		return NULL;
	}

	//validation done, ok to perform operation
	serverLockTable[lockIndex].acquireCount--; //decrease count since currentThread no longer depends on lock
	ClientRequest* rv =  serverLockTable[lockIndex].lock->Release(new ClientRequest(machineID, mailbox));
	if(serverLockTable[lockIndex].isToBeDeleted && serverLockTable[lockIndex].acquireCount == 0){ //check if needs to be deleted, delete if no one is waiting
		if(!(serverLockTable[lockIndex].lock->isBusy())){
			delete serverLockTable[lockIndex].lock;
			serverLockMap->Clear(lockIndex);
		}
	}
	return rv;
}

int ServerCreateCV(int machineID, int mailbox, char* name){

	if(serverConditionArraySize == 0){ //should only execute on first call to this function in a run of nachos
		serverConditionTable = new ServerConditionEntry[MAX_SERVER_CONDITIONS];
		serverConditionMap = new BitMap(MAX_SERVER_CONDITIONS);
		serverConditionArraySize = MAX_SERVER_CONDITIONS;
	}
	int nextFreeIndex = serverConditionMap->Find();
	if(nextFreeIndex == -1){ //error, kernel is out of memory for CVs and should terminate
		printf("Fatal system error, too many condition variables! Shut it down!\n");
		//interrupt->Halt();
		//on the server, don't halt, send a -1 as CV index response
	}
	else {
		printf("Creating %s CV of index of %d\n", name, nextFreeIndex);
		//initialize the conditionTable entry we grabbed for this CV
		serverConditionTable[nextFreeIndex].condition = new ServerCondition (name);
		serverConditionTable[nextFreeIndex].isToBeDeleted = false;
	}
	return nextFreeIndex;
}

void ServerDestroyCV(int conditionIndex) {
	if(conditionIndex < 0 || conditionIndex > serverConditionArraySize -1){ //array index is out of bounds
		printf("Thread %s tried to delete a condition with an invalid index %d\n", currentThread->getName(), conditionIndex);
		return;
	}
	if(!serverConditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s tried to delete a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		return;
	}

	//validation done, ok to perform operation
	if(!(serverConditionTable[conditionIndex].condition->hasWaiting())){ //okay to delete if no threads in condition's queue
		delete serverConditionTable[conditionIndex].condition;
		serverConditionMap->Clear(conditionIndex);
	}
	else{ //if someone is in condition's queue, defer deletion
		serverConditionTable[conditionIndex].isToBeDeleted = true;
	}
}

ClientRequest* ServerSignal(int machineID, int mailbox, int conditionIndex, int lockIndex){
	if(conditionIndex < 0 || conditionIndex > serverConditionArraySize -1){ //array index is out of bounds
		printf("Thread %s called Signal with an invalid index %d\n", currentThread->getName(), conditionIndex);
		return NULL;
	}
	if(!serverConditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Signal on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		return NULL;
	}
	if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Signal with an invalid lock index %d\n", currentThread->getName(), lockIndex);
		return NULL;
	}
	if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Signal on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		return NULL;
	}

	//validation done, ok to perform operation
	return serverConditionTable[conditionIndex].condition->Signal(serverLockTable[lockIndex].lock, new ClientRequest(machineID, mailbox));
}

void ServerWait(int machineID, int mailbox, int conditionIndex, int lockIndex){

	if(conditionIndex < 0 || conditionIndex > serverConditionArraySize -1){ //array index is out of bounds
		printf("Thread %s called Wait with an invalid index %d\n", currentThread->getName(), conditionIndex);
		return;
	}
	if(!serverConditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Wait on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		return;
	}
	if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Wait with an invalid lock index %d\n", currentThread->getName(), lockIndex);
		return;
	}
	if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Wait with a lock index that does not exist: %d\n", currentThread->getName(), lockIndex);
		return;
	}

	//validation done, ok to perform operation
	serverConditionTable[conditionIndex].condition->Wait(serverLockTable[lockIndex].lock, new ClientRequest(machineID, mailbox));
	//need to check if the condition has been marked for deletion.  if it has,
	//okay to delete if no one is currently in the condition's queue (this is an atomic operation, so no thread is in the middle of calling wait right now either)
	if(serverConditionTable[conditionIndex].isToBeDeleted){
		if(!(serverConditionTable[conditionIndex].condition->hasWaiting())){
			delete serverConditionTable[conditionIndex].condition;
			serverConditionMap->Clear(conditionIndex);
		}
	}
}

void ServerBroadcast(int machineID, int mailbox, int conditionIndex, int lockIndex){

	if(conditionIndex < 0 || conditionIndex > conditionArraySize -1){ //array index is out of bounds
		printf("Thread %s called Broadcast with an invalid index %d\n", currentThread->getName(), conditionIndex);
		return;
	}
	if(!conditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Broadcast on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		return;
	}
	if(lockIndex < 0 || lockIndex > lockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Broadcast with an invalid lock index %d\n", currentThread->getName(), lockIndex);
		return;
	}
	if(!lockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Broadcast with a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		return;
	}

	//validation done, ok to perform operation
	//serverConditionTable[conditionIndex].condition->Broadcast(lockTable[lockIndex].lock);
	respond = false;	//broadcast is special, it doesn't send 1 response, it sends many, so it handles that itself.


	while(serverConditionTable[conditionIndex].condition->) {

	}

	necessaryResponses.push()
	//also, construct and return a blank clientrequest with response false
}

int ServerCreateMV(int machineID, int mailboxID, char* name, int numEntries, int initialValue) {
	if(serverMVArraySize == 0){ //instantiate serverMVTable on first call to CreateMV_Syscall
		serverMVTable = new ServerMVEntry[MAX_SERVER_MV_ARRAYS];
		serverMVMap = new BitMap(MAX_SERVER_MV_ARRAYS);
		serverMVArraySize = MAX_SERVER_MV_ARRAYS;
	}

	if(numEntries > MAX_SERVER_ENTRIES_PER_MV_ARRAY) {
		printf("User program requested too many MVs!  Max is %d entries per MV array\n", MAX_SERVER_ENTRIES_PER_MV_ARRAY);
		return -1;
	}

	int nextFreeIndex = serverMVMap->Find(); //returns a free index in lockTable
	if(nextFreeIndex == -1){ //error, kernel is out of memory for locks and should terminate
		printf("Out of space for user program MVs!\n");
		//interrupt->Halt();
		//on the server, don't halt, send a -1 as index response
	}
	else {
		printf("Creating MV set %s with %d entries at index %d\n", name, numEntries, nextFreeIndex);
		//initialize the MVtable entry
		serverMVTable[nextFreeIndex].mvEntries = new int[numEntries];
		serverMVTable[nextFreeIndex].numEntries = numEntries;
		serverMVTable[nextFreeIndex].name = name;
		for(int i = 0; i < numEntries; i++) {
			serverMVTable[nextFreeIndex].mvEntries[i] = initialValue;
		}
	}
	return nextFreeIndex;
}

void ServerDestroyMV(int mvIndex) {
	if(mvIndex < 0 || mvIndex > serverMVArraySize - 1){ //array index is out of bounds
		printf("Thread %s tried to delete a MV with an invalid index %d\n", currentThread->getName(), mvIndex);
		return;
	}
	if(!serverMVMap->Test(mvIndex)){ //MV has not been instantiated at this index
		printf("Thread %s tried to delete a MV that does not exist: %d\n", currentThread->getName(), mvIndex);
		return;
	}

	//validation done, ok to perform operation
	delete serverMVTable[mvIndex].mvEntries;
	serverMVMap->Clear(mvIndex);
}

int ServerGetMV(int mvIndex, int entryIndex) {
	if(mvIndex < 0 || mvIndex > serverMVArraySize - 1) {
		printf("Thread %s tried to get a MV with an invalid index %d\n", currentThread->getName(), mvIndex);
		return -1;
	}
	if(!serverMVMap->Test(mvIndex)){ //MV has not been instantiated at this index
		printf("Thread %s tried to get a MV that does not exist: %d\n", currentThread->getName(), mvIndex);
		return -1;
	}
	if(entryIndex < 0 || entryIndex > serverMVTable[mvIndex].numEntries - 1) {
		printf("Thread %s tried to get a MV at invalid entry index %d in MV array %d\n", currentThread->getName(), entryIndex, mvIndex);
		return -1;
	}

	return serverMVTable[mvIndex].mvEntries[entryIndex];
}

void ServerSetMV(int mvIndex, int entryIndex, int value) {
	if(mvIndex < 0 || mvIndex > serverMVArraySize - 1) {
		printf("Thread %s tried to set a MV with an invalid index %d\n", currentThread->getName(), mvIndex);
		return;
	}
	if(!serverMVMap->Test(mvIndex)){ //MV has not been instantiated at this index
		printf("Thread %s tried to set a MV that does not exist: %d\n", currentThread->getName(), mvIndex);
		return;
	}
	if(entryIndex < 0 || entryIndex > serverMVTable[mvIndex].numEntries - 1) {
		printf("Thread %s tried to set a MV at invalid entry index %d in MV array %d\n", currentThread->getName(), entryIndex, mvIndex);
		return;
	}

	serverMVTable[mvIndex].mvEntries[entryIndex] = value;
}

//takes buf[0:3] and makes an int, where 0 is the MSByte of the int
int extractInt(char *buf) {
	return (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
}

void compressInt(int x, char dest[4]) {
	dest[0] = (x >> 24) & 0x000000ff;
	dest[1] = (x >> 16) & 0x000000ff;
	dest[2] = (x >> 8) & 0x000000ff;
	dest[3] = (x >> 0) & 0x000000ff;
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
		int mailbox = mailHeader->from;

		int replyMachineID = machineID;
		int replyMailbox = mailbox;
		bool respond = false;
		int replyData = 0;

		ServerResponse response;
		response.toMachine = machineID;
		response.toMailbox = mailbox;
		response.data = -1;

		//handle the message
		switch(messageType) {	//first byte is the message type
		case CREATE_LOCK:	//data[1] = nameLength, data[2:2+nameLength] = name
			char nameLength = messageData[1];
			char* name = new char[nameLength];
			strncpy(name, (messageData + 2), nameLength);
			response.data = ServerCreateLock(machineID, mailbox, name);
			necessaryResponses.push(response);
			respond = true;
			break;

		case DESTROY_LOCK:
			int lockIndex = extractInt(messageData + 1);
			ServerDestroyLock(lockIndex);
			break;

		case ACQUIRE:
			int lockIndex = extractInt(messageData + 1);
			ClientRequest* temp = ServerAcquire(machineID, mailbox, lockIndex);
			if(temp->respond){
				response.toMachine = temp->machineID;
				response.toMailbox = temp->mailboxNumber;
				necessaryResponses.push(response);
				respond = true;
			}
			break;

		case RELEASE:
			int lockIndex = extractInt(messageData + 1);
			ClientRequest* temp = ServerRelease(machineID, mailbox, lockIndex);
			if(temp->respond){
				response.toMachine = temp->machineID;
				response.toMailbox = temp->mailboxNumber;
				necessaryResponses.push(response);
				respond = true;
			}
			delete temp;
			break;

		case CREATE_CV:
			char nameLength = messageData[1];
			char* name = new char[nameLength];
			strncpy(name, (messageData + 2), nameLength);
			response.data = ServerCreateCV(machineID, mailbox, name);
			necessaryResponses.push(response);
			respond = true;
			break;

		case DESTROY_CV:
			int cvToDestroy = extractInt(messageData + 1);
			ServerDestroyCV(cvToDestroy);
			break;

		case SIGNAL:	//condition, then lock in message
			int cvIndex = extractInt(messageData + 1);
			int lockIndex = extractInt(messageData + 5);
			ClientRequest *temp = ServerSignal(machineID, mailbox, cvIndex, lockIndex);
			if(temp->respond){
				response.toMachine = temp->machineID;
				response.toMailbox = temp->mailboxNumber;
				necessaryResponses.push(response);
				respond = true;
			}
			delete temp;
			break;

		case WAIT:
			int cvIndex = extractInt(messageData + 1);
			int lockIndex = extractInt(messageData + 5);
			ServerWait(machineID, mailbox, cvIndex, lockIndex);
			break;

		case BROADCAST:
			int cvIndex = extractInt(messageData + 1);
			int lockIndex = extractInt(messageData + 5);
			ServerBroadcast(machineID, mailbox, cvIndex, lockIndex);
			necessaryResponses.push(response);
			respond = true;
			break;

		case CREATE_MV:
			int numEntries = extractInt(messageData + 1);
			int initialValue = extractInt(messageData + 5)
			char nameLength = messageData[9];
			char* name = new char[nameLength];
			strncpy(name, (messageData + 10), nameLength);
			response.data = ServerCreateMV(machineID, mailbox, name, numEntries, initialValue);
			necessaryResponses.push(response);

			respond = true;
			break;

		case DESTROY_MV:
			int mvToDestroy = extractInt(messageData + 1);
			ServerDestroyMV(mvToDestroy);
			break;

		case GET_MV:
			int mvIndex = extractInt(messageData + 1);
			int entryIndex = extractInt(messageData + 5);
			response.data = ServerGetMV(mvIndex, entryIndex);
			necessaryResponses.push(response);
			respond = true;
			break;

		case SET_MV:
			int mvIndex = extractInt(messageData + 1);
			int entryIndex = extractInt(messageData + 5);
			int value = extractInt(messageData + 9);
			ServerSetMV(mvIndex, entryIndex, value);
			break;

		default:
			//oops...
			break;

		}

		if(respond) {
			PacketHeader *responsePacketHeader = new PacketHeader;
			MailHeader *responseMailHeader = new MailHeader;
			char* responseData = new char[MaxMailSize];

			responsePacketHeader->to = response.toMachine; //machine i'm responding to
			responsePacketHeader->from = myMachineID; //this machine's number
			mailHeader->to = response.toMailbox;	//mailbox i'm responding to
			mailHeader->from = 0; //server mailbox?  TODO

			//pack message
			data[0] = compressInt(replyData, (responseData + 0));

			//send
			bool success = postOffice->Send(*packetHeader, *mailHeader, data);
		}
	}
}

