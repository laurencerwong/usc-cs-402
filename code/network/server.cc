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
#include <vector>
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

enum ServerAction{ Query_All_Servers, Respond_Once_To_Server, Respond_Once_To_Client, Do_Bookkeeping, No_Action, Do_Create
};
enum StructureType {Lock, CV, MV};
class ResponseQueueEntry{
public:
	int replyMachine;
	int replyMailbox;
	int replyMessageID;

	ResponseQueueEntry(int rM, int rMb, int rmID){
		replyMachine = rM;
		replyMailbox = rMb;
		replyMessageID = rmID;
	}

};
class PendingCreate{
public:
	char* name;
	StructureType type;
	bool *responseTracker;
	deque<ResponseQueueEntry*> responseQueue;
	int numResponses;
	PendingCreate(char* n, StructureType t){
		this->name = n;
		this->type = t;
		this->responseTracker = new bool[totalNumServers];
		this->responseTracker[myMachineID] = true;
		numResponses = 0;
	}
	~PendingCreate(){
		for(unsigned int i = 0; i < responseQueue.size(); i++){
			delete responseQueue.at(i);
		}
		delete[] responseTracker;
	}
	bool isAMatch(char* otherName, StructureType structType){
		return !strcmp(name, otherName) && structType == type;
	}
};

deque<PendingCreate*> pendingCreates;

BitMap *serverLockMap;
BitMap *serverConditionMap;
BitMap *serverMVMap;

int serverLockArraySize = 0;
int serverConditionArraySize = 0;
int serverMVArraySize = 0;
ServerLockEntry *serverLockTable;
ServerConditionEntry *serverConditionTable;
ServerMVEntry *serverMVTable;

struct QueryStatus {
	PacketHeader *packetHeader;
	MailHeader *mailHeader;
	char* data;
	int numResponsesReceived;
	int id;
	bool isCreateOperation;
	StructureType type;
};
deque<QueryStatus*> queryQueue;
int nextQueryID = 0;

struct ServerResponse {
	int toMachine;
	int toMailbox;
	int data;
	//int operationSuccessful;
};
queue<ServerResponse> necessaryResponses;


struct CVLockTracker {
	int clientMachineID;
	int clientMailbox;
	int cvIndex;
	int lockIndex;
	char messageType;
};

vector<CVLockTracker> cvLockTrackerList;


bool sendMessageWithData(int fromMachine, int fromMailbox, int toMachine, int toMailbox, int dataLength, char* data) {
	PacketHeader *responsePacketHeader = new PacketHeader;
	MailHeader *responseMailHeader = new MailHeader;
	char* responseData = new char[dataLength];

	responsePacketHeader->to = toMachine; //machine i'm responding to
	responsePacketHeader->from = fromMachine; //this machine's number
	responseMailHeader->to = toMailbox;	//mailbox i'm responding to
	responseMailHeader->from = fromMailbox; //server mailbox?  TODO
	responseMailHeader->length = dataLength;	//length of response data

	//pack message
	for(int i = 0; i < dataLength; i++) {
		responseData[i] = data[i];
	}

	cout << "Server sending message:" << endl;
	cout << "to: " << responsePacketHeader->to << " - " << responseMailHeader->to;
	cout << "  from: " << responsePacketHeader->from << " - " << responseMailHeader->from;
	//cout << "  with data: " << (int)responseData[0] << (int)responseData[1] << (int)responseData[2] << (int)responseData[3] << endl;
	printf("  with data:  ");
	for(int i = 0; i < dataLength; i++) {
		printf("0x%.2x  ", (unsigned char)responseData[0]);
	}
	printf("\n");

	//send
	bool success = postOffice->Send(*responsePacketHeader, *responseMailHeader, responseData);
	return success;
}

int encodeIndex(int internalIndex){
	return (myMachineID * MAX_SERVER_LOCKS) + internalIndex;
}

int decodeIndex(int encodedIndex){
	return encodedIndex - (myMachineID * MAX_SERVER_LOCKS);
}

int decodeMachineIDFromLockNumber(int lockNum) {
	return lockNum / MAX_SERVER_LOCKS;
}
int decodeMachineIDFromCVNumber(int cvNum) {
	return cvNum / MAX_SERVER_CVS;
}
int decodeMachineIDFromMVNumber(int mvNum) {
	return mvNum / MAX_SERVER_MV_ARRAYS;
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
	if(conditionIndex < 0 || decodeIndex(conditionIndex) > serverConditionArraySize -1){ //array index is out of bounds
		printf("Thread %s called Signal with an invalid index %d\n", currentThread->getName(), conditionIndex);
		return NULL;
	}
	if(!serverConditionMap->Test(decodeIndex(conditionIndex))){ //condition has not been instantiated at this index
		printf("Thread %s called Signal on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		return NULL;
	}
	if(lockIndex < 0 ){ //array index is out of bounds
		printf("Thread %s called Signal with an invalid lock index %d\n", currentThread->getName(), lockIndex);
		return NULL;
	}

	//validation done, ok to perform operation
	return serverConditionTable[conditionIndex].condition->Signal(lockIndex, new ClientRequest(machineID, mailbox));
}

void ServerWait(int machineID, int mailbox, int conditionIndex, int lockIndex){

	if(conditionIndex < 0){ //array index is out of bounds
		printf("Thread %s called Wait with an invalid index %d\n", currentThread->getName(), conditionIndex);
		return;
	}
	if(!serverConditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Wait on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		return;
	}


	//validation done, ok to perform operation
	serverConditionTable[conditionIndex].condition->Wait(lockIndex, new ClientRequest(machineID, mailbox));
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

	if(conditionIndex < 0){ //array index is out of bounds
		printf("Thread %s called Broadcast with an invalid index %d\n", currentThread->getName(), conditionIndex);
		return;
	}
	if(!serverConditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Broadcast on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		return;
	}


	//validation done, ok to perform operation
	//signal everyone in the CV and remember who to respond to
	while(serverConditionTable[conditionIndex].condition->hasWaiting()) {
		ClientRequest *cr = serverConditionTable[conditionIndex].condition->Signal(lockIndex, new ClientRequest(machineID, mailbox));
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
	int a, b, c, d;
	a = (buf[0] << 24) & 0xff000000;
	b = (buf[0] << 16) & 0x00ff0000;
	c = (buf[0] << 8) & 0x0000ff00;
	d = (buf[0] << 0) & 0x000000ff;

	return a + b + c + d;
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
	case CREATE_MV:
		return true;
		break;
	}
	return false;
}


void determineResponseToCreateRequest(StructureType type, char* name, ServerAction* serverAction, char* responseMessage, PacketHeader* packetHeader, MailHeader* mailHeader, char* inData){
	bool receivedResponse = false;
	PendingCreate* pc;
	for(unsigned int i = 0; i < pendingCreates.size(); i++){ //check for pending create
		if(pendingCreates.at(i)->isAMatch(name, type)){
			//I do have a pending create
			pc = pendingCreates.at(i);
			int serverRequesting = packetHeader->from;
			if(pc->responseTracker[serverRequesting]){ //has the requesting server told me it doesn't have this object previously?
				//if so, put it on queue until we know where the object is
				pc->responseQueue.push_back(new ResponseQueueEntry(packetHeader->from, mailHeader->from, extractInt(inData + 1)));
				*serverAction = No_Action;
				receivedResponse = true;
				break;
			}
		}
	}
	if(!receivedResponse){ // do this only if we haven't heard an answer from the requesting server
		if(myMachineID < packetHeader->from){ //gives precedence to server with lower ID. if my ID is lower, let's remember to tell this guy later
			pc->responseQueue.push_back(new ResponseQueueEntry(packetHeader->from, mailHeader->from, extractInt(inData + 1)));
			*serverAction = No_Action;
		}
		else{
			*responseMessage = DO_NOT_HAVE_LOCK;
			*serverAction = Respond_Once_To_Server;
		}
	}
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
				char* outData = new char[MaxMailSize];
				ServerAction action;

				//handle the message
				switch(messageType) {	//first byte is the message type
				/**These cases would be received from another ServerToServerHandler.
				 * Thus, they will have messageIDs in inData[1:4].
				 */
				case DO_YOU_HAVE_LOCK:{
					int index = extractInt(inData + 5);
					outData[0] = (decodeIndex(serverLockMap->Test(index))) ? HAVE_LOCK : DO_NOT_HAVE_LOCK;
					action = Respond_Once_To_Server;
					outMailHeader->length = 5;
					break;
				}
				case DO_YOU_HAVE_LOCK_NAME:{
					int nameLength = inData[5];
					char* name = new char[nameLength];
					strncpy(name, inData + 6, nameLength);
					if(checkIfLockExists(name)){ //do I have it already created?
						outData[0] = HAVE_LOCK;
						action = Respond_Once_To_Server;
					}
					else{ //I do not already have it
						determineResponseToCreateRequest(Lock, name, &action, outData, packetHeader, mailHeader, inData);
					}
					outMailHeader->length = 5 + 1 /*char that tells size of name*/ + nameLength;
					break;
				}
				case DO_YOU_HAVE_CV:{
					int index = extractInt(inData + 5);
					outData[0] = (serverConditionMap->Test(decodeIndex(index))) ? HAVE_CV : DO_NOT_HAVE_CV;
					action = Respond_Once_To_Server;
					outMailHeader->length = 5;
					break;
				}
				case DO_YOU_HAVE_CV_NAME:{
					int nameLength = inData[5];
					char* name = new char[nameLength];
					strncpy(name, inData + 6, nameLength);
					if(checkIfCVExists(name)){
						outData[0] = HAVE_CV;
						action = Respond_Once_To_Server;
					}
					else{
						determineResponseToCreateRequest(CV, name, &action, outData, packetHeader, mailHeader, inData);
					}
					outMailHeader->length = 5 + 1 /*char that tells size of name*/ + nameLength;
					break;
				}
				case DO_YOU_HAVE_MV:{
					int index = extractInt(inData + 5);
					outData[0] = (decodeIndex(serverMVMap->Test(index))) ? HAVE_MV : DO_NOT_HAVE_MV;
					action = Respond_Once_To_Server;
					outMailHeader->length = 5;
					break;
				}
				case DO_YOU_HAVE_MV_NAME:{
					int nameLength = inData[5];
					char* name = new char[nameLength];
					strncpy(name, inData + 6, nameLength);
					if(checkIfMVExists(name)){
						outData[0] = HAVE_MV;
						action = Respond_Once_To_Server;
					}
					else{
						determineResponseToCreateRequest(MV, name, &action, outData, packetHeader, mailHeader, inData);
					}
					outMailHeader->length = 5 + 1 /*char that tells size of name*/ + nameLength;
					break;
				}

				/**These cases are passed on from the Server function on our machine.
				 * Thus, there will be no messageID, and we must put one in if we are to send
				 * off a message to another ServerToServerHandler.
				 */
				case CREATE_LOCK:{
					int nameLength = inData[1];
					char* name = new char[nameLength];
					strncpy(name, inData + 2, nameLength);
					outData[0] = DO_YOU_HAVE_LOCK_NAME;
					outData[5] = nameLength;
					strncpy(outData + 6, inData + 2, nameLength);
					outMailHeader->length = 5 + 1 + nameLength;
					action = Query_All_Servers;
					pendingCreates.push_back(new PendingCreate(name, Lock));
					break;
				}//inData[1] = nameLength, inData[2:2+nameLength] = name
				case ACQUIRE:
				case RELEASE:
				case DESTROY_LOCK:{
					outData[0] = DO_YOU_HAVE_LOCK;
					strncpy(outData + 5, inData + 1, 4);
					action = Query_All_Servers;
					outMailHeader->length = 9;
					break;
				}


				case CREATE_CV:{
					int nameLength = inData[1];
					char* name = new char[nameLength];
					strncpy(name, inData + 2, nameLength);
					outData[0] = DO_YOU_HAVE_CV_NAME;
					outData[5] = nameLength;
					strncpy(outData + 6, inData + 2, nameLength);
					action = Query_All_Servers;
					outMailHeader->length = 5 + 1 + nameLength;
					pendingCreates.push_back(new PendingCreate(name, CV));
					break;
				}
				case DESTROY_CV:{
					outData[0] = DO_YOU_HAVE_CV;
					strncpy(outData + 5, inData + 1, 4);
					action = Query_All_Servers;
					outMailHeader->length = 9;
					break;
				}
				case SIGNAL:	//condition, then lock in message
				case WAIT:
				case BROADCAST:{
					outData[0] = DO_YOU_HAVE_CV;
					strncpy(outData + 5, inData + 5, 4);
					action = Query_All_Servers;
					outMailHeader->length = 9;
					break;
				}

				case CREATE_MV:{
					int nameLength = inData[1];
					char* name = new char[nameLength];
					strncpy(name, inData + 2, nameLength);
					outData[0] = DO_YOU_HAVE_MV_NAME;
					outData[5] = nameLength;
					strncpy(outData + 6, inData + 2, nameLength);
					action = Query_All_Servers;
					outMailHeader->length = 5 + 1 + nameLength;
					pendingCreates.push_back(new PendingCreate(name, MV));
					break;
				}
				case DESTROY_MV:
				case GET_MV:
				case SET_MV:{
					outData[0] = DO_YOU_HAVE_MV;
					strncpy(outData + 5, inData + 1, 4);
					action = Query_All_Servers;
					outMailHeader->length = 9;
					break;
				}
				case HAVE_LOCK:
				case HAVE_CV:
				case HAVE_MV:{
					action = Do_Bookkeeping;
					int id = extractInt(inData + 1);
					for(unsigned int i = 0; i < queryQueue.size(); i++){
						QueryStatus* temp = queryQueue.at(i);
						if(temp->id == id){
							temp->packetHeader->to = packetHeader->from;
							temp->mailHeader->to = 0;
							postOffice->Send(*temp->packetHeader, *temp->mailHeader, temp->data);
							queryQueue.erase(queryQueue.begin() + i);
							/*
							 * delete temp;
							 */
							break;
						}
					}
					break;
				}
				case DO_NOT_HAVE_LOCK:
				case DO_NOT_HAVE_CV:
				case DO_NOT_HAVE_MV:{
					action = Do_Bookkeeping;

					int id = (int) inData[1] << 24  + (int)inData[2] << 16 + (int)inData[3] << 8 + (int) inData[4];
					for(unsigned int i = 0; i < queryQueue.size(); i++){
						QueryStatus* temp = queryQueue.at(i);
						if(temp->id == id){
							temp->numResponsesReceived++; //separate variable for No/yes count? if so that means we would wait for everyone to message back before sending message off
							if(temp->numResponsesReceived == totalNumServers - 1){
								//do creates

								if(temp->isCreateOperation){
									char* name = new char[temp->data[1]];
									strncpy(name, temp->data + 2, temp->data[1]);
									switch(temp->type){
									case Lock:{
										compressInt(encodeIndex(ServerCreateLock(name)), inData);
										outData[0] = HAVE_LOCK;
										break;
									}
									case CV:{
										compressInt(encodeIndex(ServerCreateCV(name)), inData);
										outData[0] = HAVE_CV;
										break;
									}
									case MV:{;
										int numEntries = (int)(temp->data[1]) << 24 + (int)(temp->data[2]) << 16 + (int)(temp->data[3]) << 8 + (int) temp->data[4];
										int val = (int)(temp->data[6]) << 24 + (int)(temp->data[6]) << 16 + (int)(temp->data[7]) << 8 + (int) temp->data[8];
										compressInt(encodeIndex(ServerCreateMV(name, numEntries, val)), inData);
										outData[0] = HAVE_MV;
										break;
									}
									default:
										break;
									}
									PendingCreate* pc;
									for(unsigned int j = 0; j < pendingCreates.size(); j++){
										if(pendingCreates.at(j)->isAMatch(name, temp->type)){
											pc = pendingCreates.at(j);
											pendingCreates.erase(pendingCreates.begin() + j);
										}
									}
									outPacketHeader->from = myMachineID;
									outMailHeader->from = 1;
									for(unsigned int j = 0; j < pc->responseQueue.size(); j++){
										ResponseQueueEntry* temp2 = pc->responseQueue.at(j);
										outPacketHeader->to = temp2->replyMachine;
										outMailHeader->to = temp2->replyMailbox;
										compressInt(temp2->replyMessageID, outData + 1);
									}
									mailHeader = temp->mailHeader;
									packetHeader = temp->packetHeader;
									outData = temp->data;
									action = Respond_Once_To_Client;
									delete pc;

								}
								else switch(temp->type){
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
				}
				default:
				{
					//oops...
					cout << "Opps, no message type?" << endl;
					break;
				}

				}

				switch(action){
				case Respond_Once_To_Client:{
						postOffice->Send(*packetHeader, *mailHeader, outData);
						break;
				}
				case Respond_Once_To_Server:{
						outPacketHeader->from = myMachineID;
						outPacketHeader->to = packetHeader->from;
						outMailHeader->from = mailHeader->to;
						outMailHeader->to = mailHeader->from;
						//outMailHeaderSize should already be set
						strncpy(outData + 1, inData + 1, 4); //copy message ID into response
						bool success = postOffice->Send(*outPacketHeader, *outMailHeader, outData);

						break;
				}
				case Query_All_Servers:{
						QueryStatus* qs = new QueryStatus;
						qs->packetHeader = packetHeader;
						qs->mailHeader = mailHeader;
						qs->data = inData;
						qs->id = nextQueryID;
						qs->isCreateOperation = isCreation(messageType);
						qs->type = getType(messageType);
						compressInt(nextQueryID, outData + 1);
						nextQueryID++;
						for(int i = 0; i < totalNumServers; i++){
							if(i == myMachineID) continue;
							outPacketHeader = new PacketHeader;
							outMailHeader = new MailHeader;
							outPacketHeader->from = myMachineID;
							outPacketHeader->to = i;
							outMailHeader->from = 1;
							outMailHeader->to = mailHeader->from;
							outMailHeader->length = 5;
							bool success = postOffice->Send(*outPacketHeader, *outMailHeader, outData);
							queryQueue.push_back(qs);
						}
						break;
				}

				case No_Action:
				case Do_Bookkeeping:

					break;
				default: break;
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
		int messageLength = mailHeader->length;
		int messageFromMachineID = packetHeader->from;
		int messageFromMailbox = mailHeader->from;

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
			/*case DOES_CLIENT_HAVE_LOCK:
			messageTypeName = "Does Client Have Lock";
			break;*/
		case CV_LOCK_TRACKER_RESPONSE:
			messageTypeName = "CV Lock Tracker Response";
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
				<< messageFromMachineID << " mailbox " << messageFromMailbox << endl;

		int replyMachineID = messageFromMachineID;
		int replyMailbox = messageFromMailbox;
		bool respond = false;
		int replyData = 0;

		ServerResponse response;
		response.toMachine = messageFromMachineID;
		response.toMailbox = messageFromMailbox;
		response.data = -1;

		//handle the message
		switch(messageType) {	//first byte is the message type

		case CREATE_LOCK:	//data[1] = nameLength, data[2:2+nameLength] = name
		{
			char nameLength = messageData[1];
			char* name = new char[nameLength];
			strncpy(name, (messageData + 2), nameLength);
			bool alreadyHad = false;

			for(int i = 0; i < serverLockArraySize; i++) {
				if(!strcmp(name, serverLockTable[i].lock->getName())) {
					response.data = encodeIndex(i);
					necessaryResponses.push(response);
					respond = true;
					alreadyHad = true;
					break;
				}
			}
			if(!alreadyHad) {	//inform my server-server thread to handle creating the object
				if(totalNumServers == 1) {
					response.data = ServerCreateLock(name);
					necessaryResponses.push(response);
					respond = true;
				}
				else {
					sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
					respond = false;
				}
			}
			break;
		}
		case DESTROY_LOCK:
		{
			int lockNum = extractInt(messageData + 1);
			int lockMachineID = decodeMachineIDFromLockNumber(lockNum);

			if(lockMachineID != myMachineID) {
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
			}
			else {
				ServerDestroyLock(lockNum);
			}
			break;
		}
		case ACQUIRE:
		{
			int lockIndex = extractInt(messageData + 1);
			int lockMachineID = decodeMachineIDFromLockNumber(lockIndex);

			if(lockMachineID != myMachineID) {
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
				break;
			}

			ClientRequest* temp = ServerAcquire(messageFromMachineID, messageFromMailbox, lockIndex);
			if(temp == NULL) {
				necessaryResponses.push(response);
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
			int lockMachineID = decodeMachineIDFromLockNumber(lockIndex);

			if(lockMachineID != myMachineID) {
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
				break;
			}

			ClientRequest* temp = ServerRelease(messageFromMachineID, messageFromMailbox, lockIndex);
			if(temp == NULL) {
				respond = false;
			}
			else if(temp->respond){
				response.toMachine = temp->machineID;
				response.toMailbox = temp->mailboxNumber;
				necessaryResponses.push(response);
				respond = true;
			}
			break;
		}
		case CREATE_CV:
		{
			char nameLength = messageData[1];
			char* name = new char[nameLength];
			strncpy(name, (messageData + 2), nameLength);
			bool alreadyHad = false;

			for(int i = 0; i < serverConditionArraySize; i++) {
				if(!strcmp(name, serverConditionTable[i].condition->getName())) {
					response.data = encodeIndex(i);
					necessaryResponses.push(response);
					respond = true;
					alreadyHad = true;
					break;
				}
			}
			if(!alreadyHad) {	//inform my server-server thread to handle creating the object
				if(totalNumServers == 1) {
					response.data = ServerCreateCV(name);
					necessaryResponses.push(response);
					respond = true;
				}
				else {
					sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
					respond = false;
				}
			}
			break;
		}
		case DESTROY_CV:
		{
			int cvToDestroy = extractInt(messageData + 1);
			int cvMachineID = decodeMachineIDFromCVNumber(cvToDestroy);

			if(cvMachineID != myMachineID) {
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
				break;
			}

			ServerDestroyCV(cvToDestroy);
			break;
		}

		//========

		case DOES_CLIENT_HAVE_LOCK:
		{
			int clientMID = extractInt(messageData + 1);
			int clientMBX = extractInt(messageData + 5);
			int lockIndex = extractInt(messageData + 9);
			bool held = false;
			bool validLock = true;

			if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
				printf("Thread %s was asked if client %d - %d had a lock at an invalid lock index %d\n", currentThread->getName(), clientMID, clientMBX, lockIndex);
				validLock = false;
				return;
			}
			if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
				printf("Thread %s was asked if client %d - %d had a lock that does not exist: %d\n", currentThread->getName(), clientMID, clientMBX, lockIndex);
				validLock = false;
				return;
			}

			//now test if the owner is correct
			if(validLock) {
				ClientRequest *cr = new ClientRequest(clientMID, clientMBX);
				held = serverLockTable[lockIndex].lock->isHeldByRequester(cr);
				delete cr;
			}

			char *rData;
			int responseLength = 14;
			rData = new char[responseLength];

			rData[0] = CV_LOCK_TRACKER_RESPONSE;
			compressInt(clientMID, rData + 1);
			compressInt(clientMBX, rData + 5);
			compressInt(lockIndex, rData + 9);
			rData[13] = held;

			sendMessageWithData(myMachineID, 0, messageFromMachineID, messageFromMailbox, responseLength, rData);

			respond = false;
			break;
		}

		case CV_LOCK_TRACKER_RESPONSE:
		{
			int clientMID = extractInt(messageData + 1);
			int clientMBX = extractInt(messageData + 5);
			int lockNum = extractInt(messageData + 9);
			int cvNum;
			char operation;
			int cvltListEntry = -1;

			response.toMachine = clientMID;
			response.toMailbox = clientMBX;

			for(unsigned int i = 0; i < cvLockTrackerList.size(); i++) {
				if( cvLockTrackerList.at(i).clientMachineID == clientMID &&
						cvLockTrackerList.at(i).clientMailbox == clientMBX &&
						cvLockTrackerList.at(i).lockIndex == lockNum) {
					cvNum = cvLockTrackerList.at(i).cvIndex;
					operation = cvLockTrackerList.at(i).messageType;
					cvltListEntry = i;
					break;
				}
			}
			if(cvltListEntry == -1) {
				printf("Got a CV_LOCK_TRACKER_RESPONSE, but had no corresponding entry in my CVLT List!\n");
			}
			else {
				cvLockTrackerList.erase(cvLockTrackerList.begin() + cvltListEntry);
			}

			if(messageData[13] == FALSE) {	//if lock was not owned by this client
				necessaryResponses.push(response);
				respond = true;
				printf("Client %d - %d called %s on CV %d with lock that it does not have (lock %d)\n", clientMID, clientMBX, getMessageTypeName(operation).c_str(), cvNum, lockNum);
			}
			else {
				//yes, he had it, now proceed.  do we have the CV?

				if(decodeMachineIDFromCVNumber(cvNum) == myMachineID) {

					if(operation == SIGNAL) {
						if(lockNum < 0 || lockNum > serverLockArraySize -1){ //array index is out of bounds
							printf("Thread %s called Signal with an invalid lock index %d\n", currentThread->getName(), lockNum);
							necessaryResponses.push(response);
							respond = true;
							return;
						}
						if(!serverLockMap->Test(lockNum)){ //lock has not been instantiated at this index
							printf("Thread %s called Signal on a lock that does not exist: %d\n", currentThread->getName(), lockNum);
							necessaryResponses.push(response);
							respond = true;
							return;
						}

						ClientRequest *temp = ServerSignal(clientMID, clientMBX, cvNum, lockNum);
						if(temp == NULL) {
							respond = false;
						}
						else if(temp->respond){
							response.toMachine = clientMID;	//TODO **done?**, these will be from the server now, so we have to get CVLT data instead
							response.toMailbox = clientMBX;
							necessaryResponses.push(response);
							respond = true;
						}
						delete temp;
					}
					else if(operation == WAIT) {
						if(lockNum < 0 || lockNum > serverLockArraySize -1){ //array index is out of bounds
							printf("Thread %s called Wait with an invalid lock index %d\n", currentThread->getName(), lockNum);
							necessaryResponses.push(response);
							respond = true;
							return;
						}
						if(!serverLockMap->Test(lockNum)){ //lock has not been instantiated at this index
							printf("Thread %s called Wait on a lock that does not exist: %d\n", currentThread->getName(), lockNum);
							necessaryResponses.push(response);
							respond = true;
							return;
						}
						ServerWait(clientMID, clientMBX, cvNum, lockNum);
					}
					else if(operation == BROADCAST) {
						if(lockNum < 0 || lockNum > serverLockArraySize -1){ //array index is out of bounds
							printf("Thread %s called Broadcast with an invalid lock index %d\n", currentThread->getName(), lockNum);
							necessaryResponses.push(response);
							respond = true;
							return;
						}
						if(!serverLockMap->Test(lockNum)){ //lock has not been instantiated at this index
							printf("Thread %s called Broadcast on a lock that does not exist: %d\n", currentThread->getName(), lockNum);
							necessaryResponses.push(response);
							respond = true;
							return;
						}

						ServerBroadcast(messageFromMachineID, messageFromMailbox, cvNum, lockNum);	//adding responses to the queue is handled in broadcast
						necessaryResponses.push(response);
						respond = true;
					}
					else {
						printf("Unknown operation in CVLT response???\n");
					}
				}
				else {	//we don't havce the CV!  message our server message handler to take care of it
					char *forwardData = new char[9];
					forwardData[0] = operation;
					compressInt(cvNum, forwardData + 1);
					compressInt(lockNum, forwardData + 5);
					sendMessageWithData(clientMID, clientMBX, myMachineID, 1, 9, forwardData);
					respond = false;
				}
			}

			break;
		}

		case SIGNAL:
		{
			int cvIndex = extractInt(messageData + 1);
			int lockIndex = extractInt(messageData + 5);
			int lockOwnerMachineID = decodeMachineIDFromLockNumber(lockIndex);	//get machine who has the lock

			if(lockOwnerMachineID < 0 || lockOwnerMachineID > totalNumServers) {
				cout << "Invalid lock " << lockIndex << " passed to SIGNAL, cannot decode lock owner" << endl;
				necessaryResponses.push(response);
				respond = true;
			}

			if(lockOwnerMachineID == myMachineID) {	//if i am the owner of the lock
				//proceed as normal

				if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
					printf("Thread %s called Signal with an invalid lock index %d\n", currentThread->getName(), lockIndex);
					respond = true;
					return;
				}
				if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
					printf("Thread %s called Signal on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
					respond = true;
					return;
				}

				ClientRequest *temp = ServerSignal(messageFromMachineID, messageFromMailbox, cvIndex, lockIndex);
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
			}
			else {
				//some other server has it, do messaging and CVLT
				int lockOwnerMailbox = 0;	//we know that in this case, we have to send to mailbox 0
				char *queryData;
				int queryDataLength = 13;
				queryData = new char[queryDataLength];

				queryData[0] = DOES_CLIENT_HAVE_LOCK;
				compressInt(lockOwnerMachineID, queryData + 1);
				compressInt(lockOwnerMailbox, queryData + 5);
				compressInt(lockIndex, queryData + 9);

				CVLockTracker cvlt;
				cvlt.messageType = messageType;
				cvlt.cvIndex = cvIndex;
				cvlt.lockIndex = lockIndex;
				cvlt.clientMachineID = messageFromMachineID;
				cvlt.clientMailbox = messageFromMailbox;
				cvLockTrackerList.push_back(cvlt);

				//send does client have lock
				sendMessageWithData(myMachineID, 0, lockOwnerMachineID, lockOwnerMailbox, queryDataLength, queryData);

				respond = false;
			}
			break;
		}
		case BROADCAST:
		{

			int cvIndex = extractInt(messageData + 1);
			int lockIndex = extractInt(messageData + 5);
			int lockOwnerMachineID = decodeMachineIDFromLockNumber(lockIndex);	//get machine who has the lock

			if(lockOwnerMachineID < 0 || lockOwnerMachineID > totalNumServers) {
				cout << "Invalid lock " << lockIndex << " passed to BROADCAST, cannot decode lock owner" << endl;
				necessaryResponses.push(response);
				respond = true;
			}

			if(lockOwnerMachineID == myMachineID) {	//if i am the owner of the lock
				//proceed as normal

				if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
					printf("Thread %s called Broadcast with an invalid lock index %d\n", currentThread->getName(), lockIndex);
					respond = true;
					return;
				}
				if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
					printf("Thread %s called Broadcast on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
					respond = true;
					return;
				}

				ServerBroadcast(messageFromMachineID, messageFromMailbox, cvIndex, lockIndex);	//adding responses to the queue is handled in broadcast
				respond = true;
			}
			else {
				//some other server has it, do messaging and CVLT
				int lockOwnerMailbox = 0;	//we know that in this case, we have to send to mailbox 0
				char *queryData;
				int queryDataLength = 13;
				queryData = new char[queryDataLength];

				queryData[0] = DOES_CLIENT_HAVE_LOCK;
				compressInt(lockOwnerMachineID, queryData + 1);
				compressInt(lockOwnerMailbox, queryData + 5);
				compressInt(lockIndex, queryData + 9);

				CVLockTracker cvlt;
				cvlt.messageType = messageType;
				cvlt.cvIndex = cvIndex;
				cvlt.lockIndex = lockIndex;
				cvlt.clientMachineID = messageFromMachineID;
				cvlt.clientMailbox = messageFromMailbox;
				cvLockTrackerList.push_back(cvlt);

				//send does client have lock
				sendMessageWithData(myMachineID, 0, lockOwnerMachineID, lockOwnerMailbox, queryDataLength, queryData);

				respond = false;
			}
			break;
		}
		case WAIT:
		{
			int cvIndex = extractInt(messageData + 1);
			int lockIndex = extractInt(messageData + 5);
			int lockOwnerMachineID = decodeMachineIDFromLockNumber(lockIndex);	//get machine who has the lock

			if(lockOwnerMachineID < 0 || lockOwnerMachineID > totalNumServers) {
				cout << "Invalid lock " << lockIndex << " passed to WAIT, cannot decode lock owner" << endl;
				necessaryResponses.push(response);
				respond = true;
			}

			if(lockOwnerMachineID == myMachineID) {	//if i am the owner of the lock
				//proceed as normal

				if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
					printf("Thread %s called Wait with an invalid lock index %d\n", currentThread->getName(), lockIndex);
					respond = true;
					return;
				}
				if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
					printf("Thread %s called Wait on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
					respond = true;
					return;
				}
				ServerWait(messageFromMachineID, messageFromMailbox, cvIndex, lockIndex);
			}
			else {
				int lockOwnerMailbox = 0;	//we know that in this case, we have to send to mailbox 0
				char *queryData;
				int queryDataLength = 0;

				queryDataLength = 5;
				queryData = new char[queryDataLength];
				queryData[0] = RELEASE;
				compressInt(lockIndex, queryData + 1);

				CVLockTracker cvlt;
				cvlt.messageType = messageType;
				cvlt.cvIndex = cvIndex;
				cvlt.lockIndex = lockIndex;
				cvlt.clientMachineID = messageFromMachineID;
				cvlt.clientMailbox = messageFromMailbox;
				cvLockTrackerList.push_back(cvlt);

				//send does client have lock
				sendMessageWithData(myMachineID, 0, lockOwnerMachineID, lockOwnerMailbox, queryDataLength, queryData);

				respond = false;
			}
			break;
		}

		//========


		case CREATE_MV:
		{
			int numEntries = extractInt(messageData + 1);
			int initialValue = extractInt(messageData + 5);
			char nameLength = messageData[9];
			char* name = new char[nameLength];
			strncpy(name, (messageData + 10), nameLength);
			bool alreadyHad = false;

			for(int i = 0; i < serverMVArraySize; i++) {
				if(!strcmp(name, serverMVTable[i].name)) {
					response.data = encodeIndex(i);
					necessaryResponses.push(response);
					respond = true;
					alreadyHad = true;
					break;
				}
			}
			if(!alreadyHad) {	//inform my server-server thread to handle creating the object
				if(totalNumServers == 1) {
					response.data = ServerCreateMV(name, numEntries, initialValue);
					necessaryResponses.push(response);
					respond = true;
				}
				else {
					sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
					respond = false;
				}
			}
			break;

		}
		case DESTROY_MV:
		{
			int mvToDestroy = extractInt(messageData + 1);
			int mvMachineID = decodeMachineIDFromMVNumber(mvToDestroy);

			if(mvMachineID != myMachineID) {
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
				break;
			}

			ServerDestroyMV(mvToDestroy);
			break;
		}
		case GET_MV:
		{
			int mvIndex = extractInt(messageData + 1);
			int mvMachineID = decodeMachineIDFromMVNumber(mvIndex);

			if(mvMachineID != myMachineID) {
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
				break;
			}

			int entryIndex = extractInt(messageData + 5);
			response.data = ServerGetMV(mvIndex, entryIndex);
			necessaryResponses.push(response);
			respond = true;
			break;
		}
		case SET_MV:
		{
			int mvIndex = extractInt(messageData + 1);

			int mvMachineID = decodeMachineIDFromMVNumber(mvIndex);
			if(mvMachineID != myMachineID) {
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
				break;
			}

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

