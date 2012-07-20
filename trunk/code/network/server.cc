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
#include <string>

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

enum ServerActions{ Query_All_Servers, Respond_Once_To_Server, Respond_Once_To_Client
};
class PendingRequest{
public:
	char* name;
	StructureType type;
	bool responseTracker[totalNumServers];
	PendingRequest(char* name, StructureType type){
		this->name = name;
		this->type = type;
		responseTracker[myMachineID] = true;
	}
	bool isAMatch(char* otherName, StructureType structType){
		return !strcmp(name, otherName) && structType == type;
	}
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
deque<QueryStatus> queryQueue;
int nextQueryID = 0;
struct ServerResponse {
	int toMachine;
	int toMailbox;
	int data;
	//int operationSuccessful;
};
deque<QueryStatus> queryQueue;
enum StructureType {Lock, CV, MV};

struct QueryStatus {
	PacketHeader *packetHeader;
	MailHeader *mailHeader;
	char* data;
	int numResponsesReceived;
	int id;
	bool isCreateOperation;
	StructureType type;
};

queue<ServerResponse> necessaryResponses;

int encodeIndex(int internalIndex){
	return (myMachineID * MAX_SERVER_LOCKS) + internalIndex;
}

int decodeIndex(int encodedIndex){
	return encodedIndex - (myMachineID * MAX_SERVER_LOCKS);
}

bool checkIfLockExists(char* name){
	//check if there already is a lock with this name
	if(serverLockArraySize == 0) return false;
	for(int i = 0; i < MAX_SERVER_LOCKS; i++) {
		if(serverLockMap->Test(i) == 1) {
			if(strcmp(serverLockTable[i].lock->getName(), name) == 0) {	//if they have the same name
				printf("A Lock with name %s has already been created at index %d\n", name, i);
				return true;
			}
		}
	}
	return false;
}

bool checkIfCVExists(char* name){
	//check if there already is a CV with this name
	if(serverConditionArraySize == 0) return false;
	for(int i = 0; i < MAX_SERVER_CVS; i++) {
		if(serverConditionMap->Test(i) == 1) {
			if(strcmp(serverConditionTable[i].condition->getName(), name) == 0) {	//if they have the same name
				printf("A CV with name %s has already been created at index %d\n", name, i);
				return true;
			}
		}
	}
	return false;
}

bool checkIfMVExists(char* name){
	//check if there already is a lock with this name
	if(serverMVArraySize == 0) return false;
	for(int i = 0; i < MAX_SERVER_MV_ARRAYS; i++) {
		if(serverMVMap->Test(i) == 1) {
			if(strcmp(serverMVTable[i].name, name) == 0) {	//if they have the same name
				printf("An MV with name %s has already been created at index %d\n", name, i);
				return true;
			}
		}
	}
	return false;
}

int ServerCreateLock(char* name) {
	if(serverLockArraySize == 0){ //instantiate lockTable on first call to CreateLock_Syscall
		serverLockTable = new ServerLockEntry[MAX_SERVER_LOCKS];
		serverLockMap = new BitMap(MAX_SERVER_LOCKS);
		serverLockArraySize = MAX_SERVER_LOCKS;
	}

	//check if there already is a lock with this name
	for(int i = 0; i < MAX_SERVER_LOCKS; i++) {
		if(serverLockMap->Test(i) == 1) {
			if(strcmp(serverLockTable[i].lock->getName(), name) == 0) {	//if they have the same name
				printf("A Lock with name %s has already been created at index %d\n", name, i);
				return i;
			}
		}
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
	if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Acquire on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		return NULL;
	}

	//validation done, ok to perform operation
	serverLockTable[lockIndex].acquireCount++; //marks this lock as being held by a user thread, protects lock from deletion til same thread call Release syscall
	return serverLockTable[lockIndex].lock->Acquire(new ClientRequest(machineID, mailbox));
}

ClientRequest* ServerRelease(int machineID, int mailbox, int lockIndex) {

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

int ServerCreateCV(char* name){

	if(serverConditionArraySize == 0){ //should only execute on first call to this function in a run of nachos
		serverConditionTable = new ServerConditionEntry[MAX_SERVER_CVS];
		serverConditionMap = new BitMap(MAX_SERVER_CVS);
		serverConditionArraySize = MAX_SERVER_CVS;
	}

	//check if there already is a CV with this name
	for(int i = 0; i < MAX_SERVER_CVS; i++) {
		if(serverConditionMap->Test(i) == 1) {
			if(strcmp(serverConditionTable[i].condition->getName(), name) == 0) {	//if they have the same name
				printf("A CV with name %s has already been created at index %d\n", name, i);
				return i;
			}
		}
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

	if(conditionIndex < 0 || conditionIndex > serverConditionArraySize -1){ //array index is out of bounds
		printf("Thread %s called Broadcast with an invalid index %d\n", currentThread->getName(), conditionIndex);
		return;
	}
	if(!serverConditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Broadcast on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		return;
	}
	if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
		printf("Thread %s called Broadcast with an invalid lock index %d\n", currentThread->getName(), lockIndex);
		return;
	}
	if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Broadcast with a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		return;
	}

	//validation done, ok to perform operation
	//signal everyone in the CV and remember who to respond to
	while(serverConditionTable[conditionIndex].condition->hasWaiting()) {
		ClientRequest *cr = serverConditionTable[conditionIndex].condition->Signal(serverLockTable[lockIndex].lock, new ClientRequest(machineID, mailbox));
		ServerResponse r;
		r.toMachine = cr->machineID;
		r.toMailbox = cr->mailboxNumber;
		r.data = -1;
		delete cr;
		necessaryResponses.push(r);
	}
}

int ServerCreateMV(char* name, int numEntries, int initialValue) {
	if(serverMVArraySize == 0){ //instantiate serverMVTable on first call to CreateMV_Syscall
		serverMVTable = new ServerMVEntry[MAX_SERVER_MV_ARRAYS];
		serverMVMap = new BitMap(MAX_SERVER_MV_ARRAYS);
		serverMVArraySize = MAX_SERVER_MV_ARRAYS;
	}

	if(numEntries > MAX_SERVER_ENTRIES_PER_MV_ARRAY) {
		printf("User program requested too many MVs!  Max is %d entries per MV array\n", MAX_SERVER_ENTRIES_PER_MV_ARRAY);
		return -1;
	}

	//check if there already is a lock with this name
	for(int i = 0; i < MAX_SERVER_MV_ARRAYS; i++) {
		if(serverMVMap->Test(i) == 1) {
			if(strcmp(serverMVTable[i].name, name) == 0) {	//if they have the same name
				printf("An MV with name %s has already been created at index %d\n", name, i);
				return i;
			}
		}
	}

	int nextFreeIndex = serverMVMap->Find(); //returns a free index in lockTable
	if(nextFreeIndex == -1){ //error, kernel is out of memory for locks and should terminate
		printf("Out of space for user program MVs!\n");
		//interrupt->Halt();
		//on the server, don't halt, send a -1 as index response
	}
	else {
		printf("Creating MV set %s with %d entries at index %d, initialized to %d\n", name, numEntries, nextFreeIndex, initialValue);
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

int ServerGetMV(int mvIndex, int entryIndex) {	//TODO this call could use a fail bit instead of returning -1, or else we won't know when it doesn't work client side
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
	int value = serverMVTable[mvIndex].mvEntries[entryIndex];
	printf("Getting MV %d entry %d, value %d\n", mvIndex, entryIndex, value);
	return value;
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

	printf("Setting MV %d entry %d to value %d\n", mvIndex, entryIndex, value);
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

string getMessageTypeName(char messageType){
	string messageTypeName;
	switch(messageType) {	//first byte is the message type
	case CREATE_LOCK:	//data[1] = nameLength, data[2:2+nameLength] = name
		messageTypeName = "Create Lock";
		break;
	case DESTROY_LOCK:
		messageTypeName = "Destroy Lock";
		break;
	case ACQUIRE:
		messageTypeName = "Acquire";
		break;
	case RELEASE:
		messageTypeName = "Release";
		break;
	case CREATE_CV:
		messageTypeName = "Create CV";
		break;
	case DESTROY_CV:
		messageTypeName = "Destroy CV";
		break;
	case SIGNAL:
		messageTypeName = "Signal";
		break;
	case WAIT:
		messageTypeName = "Wait";
		break;
	case BROADCAST:
		messageTypeName = "Broadcast";
		break;
	case CREATE_MV:
		messageTypeName = "Create MV";
		break;
	case DESTROY_MV:
		messageTypeName = "Destroy MV";
		break;
	case GET_MV:
		messageTypeName = "Get MV";
		break;
	case SET_MV:
		messageTypeName = "Set MV";
		break;
	case DO_YOU_HAVE_LOCK:
		messageTypeName = "Do you have Lock";
		break;
	case HAVE_LOCK:
		messageTypeName = "Have lock";
		break;
	case DO_NOT_HAVE_LOCK:
		messageTypeName = "Do not have lock";
		break;
	case DO_YOU_HAVE_CV:
		messageTypeName = "Do you have CV";
		break;
	case HAVE_CV:
		messageTypeName = "Have CV";
		break;
	case DO_NOT_HAVE_CV:
		messageTypeName = "Do not have CV";
		break;
	case DO_YOU_HAVE_MV:
		messageTypeName = "Do you have MV";
		break;
	case HAVE_MV:
		messageTypeName = "Have MV";
		break;
	case DO_NOT_HAVE_MV:
		messageTypeName = "Do not have MV";
		break;
	default:
		messageTypeName = "UNKNOWN";
		cout << "Oops, no message type?" << endl;
		break;
	}
	return messageTypeName;

}

StructureType getType(char messageType){
	switch(messageType){
	case CREATE_LOCK:
	case DESTROY_LOCK:
	case ACQUIRE:
	case RELEASE:
		return Lock;
		break;
	case CREATE_CV:
	case DESTROY_CV:
	case SIGNAL:
	case BROADCAST:
	case WAIT:
		return CV;
		break;
	case CREATE_MV:
	case DESTROY_MV:
	case SET_MV:
	case GET_MV:
		return MV;
		break;
	default:
		cout << "incorrect messageType passed to getType() function" << endl;
		break;
	}
	return Lock;
}
bool isCreation(char messageType){
	switch(messageType){
	case CREATE_LOCK:
		return true;
		break;
	case CREATE_CV:
		return true;
		break;
	case CREATE_CV:
		return true;
		break;
	}
	return false;
}
void ServerToServerMessageHandler(){
	while(true){ //infinitely check for messages to handle
		PacketHeader *packetHeader = new PacketHeader;
		MailHeader *mailHeader = new MailHeader;
		char* inData = new char[MaxMailSize];

		postOffice->Receive(1, packetHeader, mailHeader, inData);	//server is always mailbox 1
		//parse message
				char messageType = inData[0];
				int machineID = packetHeader->from;
				int mailbox = mailHeader->from;

				/*cout << "myMachineID " << myMachineID << "  myMailbox " << 0 << endl;
				for(int i = 0; i < 10; i++) {
					cout << inData[i] << endl;
				}*/

				string messageTypeName = getMessageTypeName(messageType);
				cout << "\nServer received message of type: " << messageTypeName << " from machine "
					 << machineID << " mailbox " << mailbox << endl;

				PacketHeader* outPacketHeader = new PacketHeader;
				MailHeader* outMailHeader = new MailHeader;
				char* outData = new char[MAX_MAIL_SIZE];


				//handle the message
				switch(messageType) {	//first byte is the message type
				/**These cases would be received from another ServerToServerHandler.
				 * Thus, they will have messageIDs in inData[1:4].
				 */
				case DO_YOU_HAVE_LOCK:
					int index = extractInt(inData + 5);
					outData[0] = (decodeIndex(serverLockMap->Test(index))) ? HAVE_LOCK : DO_NOT_HAVE_LOCK;
					action = Respond_Once_To_Server;
					break;
				case DO_YOU_HAVE_LOCK_NAME:
					int nameLength = inData[5];
					char* name = new char[nameLength];
					strncpy(name, inData + 6, nameLength);
					outData[0] = (checkIfLockExists(name)) ? HAVE_LOCK : DO_NOT_HAVE_LOCK;
					action = Respond_Once_To_Server;
					break;
				case DO_YOU_HAVE_CV:
					int index = extractInt(inData + 5);
					outData[0] = (serverCVMap->Test(decodeIndex(index))) ? HAVE_CV : DO_NOT_HAVE_CV;
					action = Respond_Once_To_Server;
					break;
				case DO_YOU_HAVE_CV_NAME:
					int nameLength = inData[5];
					char* name = new char[nameLength];
					strncpy(name, inData + 6, nameLength);
					outData[0] = (checkIfCVExists(name)) ? HAVE_CV : DO_NOT_HAVE_CV;
					action = Respond_Once_To_Server;
					break;
				case DO_YOU_HAVE_MV:
					int index = extractInt(inData + 5);
					outData[0] = (decodeIndex(serverMVMap->Test(index))) ? HAVE_MV : DO_NOT_HAVE_MV;
					action = Respond_Once_To_Server;
					break;
				case DO_YOU_HAVE_MV_NAME:
					int nameLength = inData[5];
					char* name = new char[nameLength];
					strncpy(name, inData + 6, nameLength);
					outData[0] = (checkIfMVExists(name)) ? HAVE_MV : DO_NOT_HAVE_MV;
					action = Respond_Once_To_Server;
					break;

				/**These cases are passed on from the Server function on our machine.
				 * Thus, there will be no messageID, and we must put one in if we are to send
				 * off a message to another ServerToServerHandler.
				 */
				case CREATE_LOCK:
					int nameLength = inData[1];
					char* name = new char[nameLength];
					strncpy(name, inData + 2, nameLength);
					outData[0] = DO_YOU_HAVE_LOCK_NAME;
					outData[5] = nameLength;
					strncpy(outData + 6, inData + 2, nameLength);
					action = Query_All_Servers;
					break;//inData[1] = nameLength, inData[2:2+nameLength] = name
				case ACQUIRE:
				case RELEASE:
				case DESTROY_LOCK:
					outData[0] = DO_YOU_HAVE_LOCK;
					strncpy(outData + 5, inData + 1, 4);
					action = Query_All_Servers;
					break;


				case CREATE_CV:
					int nameLength = inData[1];
					char* name = new char[nameLength];
					strncpy(name, inData + 2, nameLength);
					outData[0] = DO_YOU_HAVE_CV_NAME;
					outData[5] = nameLength;
					strncpy(outData + 6, inData + 2, nameLength);
					action = Query_All_Servers;
					break;
				case DESTROY_CV:
				case SIGNAL:	//condition, then lock in message
				case WAIT:
				case BROADCAST:
					outData[0] = DO_YOU_HAVE_CV;
					strncpy(outData + 5, inData + 1, 4);
					action = Query_All_Servers;
					break;

				case CREATE_MV:
					int nameLength = inData[1];
					char* name = new char[nameLength];
					strncpy(name, inData + 2, nameLength);
					outData[0] = DO_YOU_HAVE_MV_NAME;
					outData[5] = nameLength;
					strncpy(outData + 6, inData + 2, nameLength);
					action = Query_All_Servers;
					break;
				case DESTROY_MV:
				case GET_MV:
				case SET_MV:
					outData[0] = DO_YOU_HAVE_MV;
					strncpy(outData + 5, inData + 1, 4);
					action = Query_All_Servers;
					break;

				case DO_HAVE_LOCK:
				case DO_HAVE_CV:
				case DO_HAVE_MV:
					action = Do_Bookkeeping;
					int id = extractInt(inData + 1);
					for(int i = 0; i < queryQueue.size(); i++){
						QueryStatus* temp = queryQueue.at(queryQueue.begin() + i);
						if(i->id == id){
							temp->packetHeader->to = packetHeader->from;
							temp->mailHeader->to = 0;
							postOffice->Send(*temp->packetHeader, *temp->mailHeader, temp->inData);
							queryQueue.erase(queryQueue.begin() + i);
							/*
							 * delete temp;
							 */
							break;
						}
					}
					break;
				case DO_NOT_HAVE_LOCK:
				case DO_NOT_HAVE_CV:
				case DO_NOT_HAVE_MV:
					action = Do_Bookkeeping;

					int id = (int) inData[1] << 24  + (int)inData[2] << 16 + (int)inData[3] << 8 + (int) inData[4];
					for(int i = 0; i < queryQueue.size(); i++){
						QueryStatus* temp = queryQueue.at(queryQueue.begin() + i);
						if(i->id == id){
							temp->numResponsesReceived++; //separate variable for No/yes count? if so that means we would wait for everyone to message back before sending message off
							if(temp->numResponsesReceived == totalNumServer - 1){
								//do creates

								if(temp->isCreateOperation){
									action = Respond_Once_To_Client;
									delete mailHeader;
									delete packetHeader;
									delete inData;
									mailHeader = temp->mailHeader;
									packetHeader = temp->packetHeader;
									outData = temp->data;
									switch(type){
									case Lock:
										char* name = new char[temp->data[1]];
										strncpy(name, temp->data + 2, temp->data[1]);
										compressInt(encodeIndex(ServerCreateLock(name)), inData);
										break;
									case CV:
										char* name = new char[temp->data[1]];
										strncpy(name, temp->data + 2, temp->data[1]);
										compressInt(encodeIndex(ServerCreateCV(name)), inData);
										break;
									case MV:
										char* name = new char[temp->data[9]];
										strncpy(name, temp->data + 10, temp->data[9]);
										int numEntries = (int)(temp->data[1]) << 24 + (int)(temp->data[2]) << 16 + (int)(temp->data[3]) << 8 + (int) temp->data[4];
										int val = (int)(temp->data[6]) << 24 + (int)(temp->data[6]) << 16 + (int)(temp->data[7]) << 8 + (int) temp->data[8];
										compressInt(encodeIndex(ServerCreateMV(name, numEntries, val)), inData);
										break;
									default:
										break;
									}
								}
								else switch(type){
								case Lock:
									cout << "Lock did not exist." << endl;
									break;
								case CV:
									cout << "Condition did not exist." << endl;
									break;
								case MV:
									cout << "Monitor variable did not exist." << endl;
									break;
								default:
									break;
								}
							}
						}
						break;
					}
					break;
					break;
				default:
				{
					//oops...
					cout << "Opps, no message type?" << endl;
					break;
				}

				}

				switch(action){
				case Respond_Once_To_Client:
						postOffice->Send(*packetHeader, *mailHeader, outData);
						break;
				case Respond_Once_To_Server:
						PacketHeader* outPacketHeader = new PacketHeader;
						MailHeader* outMailHeader = new MailHeader;
						outPacketHeader->from = myMachineID;
						outPacketHeader->to = packetHeader->from;
						outMailHeader->from = mailHeader->to;
						outMailHeader->to = mailHeader->from;
						outMailHeader->size = 1;
						strncpy(outData + 1, inData + 1, 4); //copy message ID into response
						bool success = postOffice->Send(*outPacketHeader, *outMailHeader, outData);

						break;
				case Query_All_Servers:
						QueryStatus* qs = new QueryStatus;
						for(int i = 0; i < totalNumServers; i++){
							if(i == myMachineID) continue;
							qs->packetHeader = packetHeader;
							qs->mailHeader = mailHeader;
							qs->data = inData;
							qs->id = nextQueryID;
							qs->isCreateOperation = isCreation(messageType);
							qs->structureType = getType(messageType);
							compressInt(nextQueryID, outData + 1);
							nextQueryID++;
							PacketHeader* outPacketHeader = new PacketHeader;
							MailHeader* outMailHeader = new MailHeader;
							outPacketHeader->from = myMachineID;
							outPacketHeader->to = i;
							outMailHeader->from = 1;
							outMailHeader->to = mailHeader->from;
							outMailHeader->size = 1;
							bool success = postOffice->Send(*outPacketHeader, *outMailHeader, outData);
							queryQueue.push(qs);
						}
						break;

				case Do_Bookkeeping:

					break;
				}


	}
}

void Server() {
	cout << "Nachos server starting up with machine ID " << myMachineID << endl;
	cout << " -note, all clients expect a server with machineID 0" << endl;

	while(true) {
		//get message
		PacketHeader *packetHeader = new PacketHeader;
		MailHeader *mailHeader = new MailHeader;
		char* messageData = new char[MaxMailSize];

		postOffice->Receive(0, packetHeader, mailHeader, messageData);	//server is always mailbox 0 for now?

		//parse message
		char messageType = messageData[0];
		int machineID = packetHeader->from;
		int mailbox = mailHeader->from;

		/*cout << "myMachineID " << myMachineID << "  myMailbox " << 0 << endl;
		for(int i = 0; i < 10; i++) {
			cout << messageData[i] << endl;
		}*/

		string messageTypeName;

		switch(messageType) {	//first byte is the message type
		case CREATE_LOCK:	//data[1] = nameLength, data[2:2+nameLength] = name
			messageTypeName = "Create Lock";
			break;
		case DESTROY_LOCK:
			messageTypeName = "Destroy Lock";
			break;
		case ACQUIRE:
			messageTypeName = "Acquire";
			break;
		case RELEASE:
			messageTypeName = "Release";
			break;
		case CREATE_CV:
			messageTypeName = "Create CV";
			break;
		case DESTROY_CV:
			messageTypeName = "Destroy CV";
			break;
		case SIGNAL:
			messageTypeName = "Signal";
			break;
		case WAIT:
			messageTypeName = "Wait";
			break;
		case BROADCAST:
			messageTypeName = "Broadcast";
			break;
		case CREATE_MV:
			messageTypeName = "Create MV";
			break;
		case DESTROY_MV:
			messageTypeName = "Destroy MV";
			break;
		case GET_MV:
			messageTypeName = "Get MV";
			break;
		case SET_MV:
			messageTypeName = "Set MV";
			break;
		default:
			messageTypeName = "UNKNOWN";
			cout << "Oops, no message type?" << endl;
			break;
		}


		cout << "\nServer received message of type: " << messageTypeName << " from machine "
			 << machineID << " mailbox " << mailbox << endl;

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
		{
			char nameLength = messageData[1];
			char* name = new char[nameLength];
			strncpy(name, (messageData + 2), nameLength);
			response.data = ServerCreateLock(machineID, mailbox, name);
			necessaryResponses.push(response);
			respond = true;
			break;
		}
		case DESTROY_LOCK:
		{
			int lockIndex = extractInt(messageData + 1);
			ServerDestroyLock(lockIndex);
			break;
		}
		case ACQUIRE:
		{
			int lockIndex = extractInt(messageData + 1);
			ClientRequest* temp = ServerAcquire(machineID, mailbox, lockIndex);
			if(temp == NULL) {
				respond = true;
			}
			else if(temp->respond){
				response.toMachine = temp->machineID;
				response.toMailbox = temp->mailboxNumber;
				necessaryResponses.push(response);
				respond = true;
			}
			break;
		}
		case RELEASE:
		{
			int lockIndex = extractInt(messageData + 1);
			ClientRequest* temp = ServerRelease(machineID, mailbox, lockIndex);
			if(temp == NULL) {
				respond = false;
			}
			else if(temp->respond){
				response.toMachine = temp->machineID;
				response.toMailbox = temp->mailboxNumber;
				necessaryResponses.push(response);
				respond = true;
			}
			delete temp;
			break;
		}
		case CREATE_CV:
		{
			char nameLength = messageData[1];
			char* name = new char[nameLength];
			strncpy(name, (messageData + 2), nameLength);
			response.data = ServerCreateCV(machineID, mailbox, name);
			necessaryResponses.push(response);
			respond = true;
			break;
		}
		case DESTROY_CV:
		{
			int cvToDestroy = extractInt(messageData + 1);
			ServerDestroyCV(cvToDestroy);
			break;
		}
		case SIGNAL:	//condition, then lock in message
		{
			int cvIndex = extractInt(messageData + 1);
			int lockIndex = extractInt(messageData + 5);
			ClientRequest *temp = ServerSignal(machineID, mailbox, cvIndex, lockIndex);
			if(temp == NULL) {
				respond = false;
			}
			else if(temp->respond){
				response.toMachine = temp->machineID;
				response.toMailbox = temp->mailboxNumber;
				necessaryResponses.push(response);
				respond = true;
			}
			delete temp;
			break;
		}
		case WAIT:
		{
			int cvIndex = extractInt(messageData + 1);
			int lockIndex = extractInt(messageData + 5);
			ServerWait(machineID, mailbox, cvIndex, lockIndex);
			break;
		}
		case BROADCAST:
		{
			int cvIndex = extractInt(messageData + 1);
			int lockIndex = extractInt(messageData + 5);
			ServerBroadcast(machineID, mailbox, cvIndex, lockIndex);	//adding responses to the queue is handled in broadcast
			respond = true;
			break;
		}
		case CREATE_MV:
		{
			int numEntries = extractInt(messageData + 1);
			int initialValue = extractInt(messageData + 5);
			char nameLength = messageData[9];
			char* name = new char[nameLength];
			strncpy(name, (messageData + 10), nameLength);
			response.data = ServerCreateMV(machineID, mailbox, name, numEntries, initialValue);
			necessaryResponses.push(response);

			respond = true;
			break;
		}
		case DESTROY_MV:
		{
			int mvToDestroy = extractInt(messageData + 1);
			ServerDestroyMV(mvToDestroy);
			break;
		}
		case GET_MV:
		{
			int mvIndex = extractInt(messageData + 1);
			int entryIndex = extractInt(messageData + 5);
			response.data = ServerGetMV(mvIndex, entryIndex);
			necessaryResponses.push(response);
			respond = true;
			break;
		}
		case SET_MV:
		{
			int mvIndex = extractInt(messageData + 1);
			int entryIndex = extractInt(messageData + 5);
			int value = extractInt(messageData + 9);
			ServerSetMV(mvIndex, entryIndex, value);
			break;
		}
		default:
		{
			//oops...
			cout << "Opps, no message type?" << endl;
			break;
		}

		}

		if(respond) {
			while(!necessaryResponses.empty()) {
				PacketHeader *responsePacketHeader = new PacketHeader;
				MailHeader *responseMailHeader = new MailHeader;
				char* responseData = new char[4];

				responsePacketHeader->to = necessaryResponses.front().toMachine; //machine i'm responding to
				responsePacketHeader->from = myMachineID; //this machine's number
				responseMailHeader->to = necessaryResponses.front().toMailbox;	//mailbox i'm responding to
				responseMailHeader->from = 0; //server mailbox?  TODO
				responseMailHeader->length = 4;	//length of response data

				//pack message
				compressInt(necessaryResponses.front().data, (responseData + 0));

				cout << "Server sending response message:" << endl;
				cout << "to: " << responsePacketHeader->to << " - " << responseMailHeader->to;
				cout << "  from: " << responsePacketHeader->from << " - " << responseMailHeader->from;
				//cout << "  with data: " << (int)responseData[0] << (int)responseData[1] << (int)responseData[2] << (int)responseData[3] << endl;
				printf("  with data: 0x%.2x%.2x%.2x%.2x\n", (unsigned char)responseData[0], (unsigned char)responseData[1], (unsigned char)responseData[2], (unsigned char)responseData[3]);

				//send
				bool success = postOffice->Send(*responsePacketHeader, *responseMailHeader, responseData);

				//remove response from list
				necessaryResponses.pop();
			}
		}
	}
}

