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
int nextQueryID = 0;
void compressInt(int, char*);
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

enum ServerAction{ Query_All_Servers, Respond_Once_To_Server, Respond_Once_To_Client, Do_Bookkeeping, No_Action, Create_Query
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
class QueryStatus {
public:
	PacketHeader *packetHeader;
	MailHeader *mailHeader;
	char* data;
	int numResponsesReceived;
	int id;
	StructureType type;

	QueryStatus(PacketHeader* ph, MailHeader* mh, char* inData){
		packetHeader = ph;
		mailHeader = mh;
		data = inData;
		numResponsesReceived = 0;
		id = nextQueryID;
		nextQueryID++;
	}
};


/**Allows us to remember what we want to create for a client
 * until we gather enough information from other servers to either
 * create the object or forward to another server
 */
class PendingCreate{
public:
	char* name;
	StructureType type; //allows us to have same names across different types
	bool *responseTracker; //array for tracking which servers have responded. indexed by machine IDs
	deque<QueryStatus*> waitingClients; //clients needing a response regarding the create
	deque<ResponseQueueEntry*> serverResponseQueue; //servers we should message once we determine if we are/are not creating lock
	int numResponses; //number of servers we've heard back from regarding create
	int ID; //unique identifier for this create
	PendingCreate(char* n, StructureType t){
		this->name = n;
		this->type = t;
		this->responseTracker = new bool[totalNumServers];
		this->responseTracker[myMachineID] = true;
		numResponses = 0;
		ID = nextQueryID;
		nextQueryID++;
	}
	~PendingCreate(){
		/**for(unsigned int i = 0; i < serverResponseQueue.size(); i++){
			delete serverResponseQueue.at(i);
		}*/
		delete[] responseTracker;
	}
	bool isAMatch(char* otherName, StructureType structType){
		return !strcmp(name, otherName) && structType == type;
	}

	/**Call this if we had a pending create and another server ended up having the item.
	 *
	 */
	void forwardWaitingClientMessagesToServer(int recipientMachine){
		for(unsigned int i = 0; i < waitingClients.size(); i++){
			QueryStatus* temp = waitingClients.at(i);
			temp->packetHeader->to = recipientMachine;
			temp->mailHeader->to = 1; //send to other machine's ServerToServerHandler, which must handle creates anyways
			postOffice->Send(*(temp->packetHeader), *(temp->mailHeader), temp->data);
			//delete temp
		}
	}

	/**Call this if we had a pending create and this server ended up making the item in question.
	 *
	 */
	void messageBackClients(int encodedIndex){
		for(unsigned int i = 0; i < waitingClients.size(); i++){
			QueryStatus* temp = waitingClients.at(i);
			temp->packetHeader->to = temp->packetHeader->from;
			temp->mailHeader->to = temp->mailHeader->from;
			temp->packetHeader->from = myMachineID;
			temp->mailHeader->from = 1;
			compressInt(encodedIndex, temp->data);
			temp->mailHeader->length = 4;
			postOffice->Send(*(temp->packetHeader), *(temp->mailHeader), temp->data);
			//delete temp;
		}
	}

	/**call this if we've done the create, so other servers will forward us
	 * waiting client messages for Creates
	 */
	void notifyServersOfCreation(char* outData){
		PacketHeader* outPacketHeader = new PacketHeader;
		MailHeader* outMailHeader = new MailHeader;
		outPacketHeader->from = myMachineID;
		outMailHeader->from = 1;
		outMailHeader->length = 5;
		for(unsigned int j = 0; j < serverResponseQueue.size(); j++){
			//package message based on remembered info in PendingCreate
			ResponseQueueEntry* temp = serverResponseQueue.at(j);
			outPacketHeader->to = temp->replyMachine;
			outMailHeader->to = temp->replyMailbox;

			compressInt(temp->replyMessageID, outData + 1);
			postOffice->Send(*outPacketHeader, *outMailHeader, outData);
		}
		delete outPacketHeader;
		delete outMailHeader;
	}
};


deque<PendingCreate*> pendingCreates; //where we store PendingCreate objects

BitMap *serverLockMap;
BitMap *serverConditionMap;
BitMap *serverMVMap;

int serverLockArraySize = 0;
int serverConditionArraySize = 0;
int serverMVArraySize = 0;
ServerLockEntry *serverLockTable;
ServerConditionEntry *serverConditionTable;
ServerMVEntry *serverMVTable;


deque<QueryStatus*> queryQueue;


struct ServerResponse {
	int toMachine;
	int toMailbox;
	int data;
	//int operationSuccessful;
};
queue<ServerResponse> necessaryResponses;

/**Where we store client requests for CV operations while we wait
 * for lock validation (lock exists + client owns it)
 */
struct CVLockTracker {

	//address to respond to client at
	int clientMachineID;
	int clientMailbox;

	int cvNum; //remember which CV they want to operate on
	int lockNum; //remember which lock they say CV is associated wit
	char messageType;
};

vector<CVLockTracker> cvLockTrackerList;

/**encapsulation for Server function responses to Clients
 *
 */
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
		printf("0x%.2x  ", (unsigned char)responseData[i]);
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

/**encapsulation to see if, on a create, we already have an object created
 * saves other code from tedious string comparisons
 */
int checkIfLockExists(char* name){
	//check if there already is a lock with this name
	if(serverLockArraySize == 0) return -1;
	for(int i = 0; i < MAX_SERVER_LOCKS; i++) {
		if(serverLockMap->Test(i) == 1) {
			if(strcmp(serverLockTable[i].lock->getName(), name) == 0) {	//if they have the same name
				printf("A Lock with name %s has already been created at index %d\n", name, i);
				return i;
			}
		}
	}
	return -1;
}

/**encapsulation to see if, on a create, we already have an object created
 * saves other code from tedious string comparisons
 */
int checkIfCVExists(char* name){
	//check if there already is a CV with this name
	if(serverConditionArraySize == 0) return -1;
	for(int i = 0; i < MAX_SERVER_CVS; i++) {
		if(serverConditionMap->Test(i) == 1) {
			if(strcmp(serverConditionTable[i].condition->getName(), name) == 0) {	//if they have the same name
				printf("A CV with name %s has already been created at index %d\n", name, i);
				return i;
			}
		}
	}
	return -1;
}

/**encapsulation to see if, on a create, we already have an object created
 * saves other code from tedious string comparisons
 */
int checkIfMVExists(char* name){
	//check if there already is a lock with this name
	if(serverMVArraySize == 0) return -1;
	for(int i = 0; i < MAX_SERVER_MV_ARRAYS; i++) {
		if(serverMVMap->Test(i) == 1) {
			if(strcmp(serverMVTable[i].name, name) == 0) {	//if they have the same name
				printf("An MV with name %s has already been created at index %d\n", name, i);
				return i;
			}
		}
	}
	return -1;
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
		printf("Server %d Destroying ServerLock %d: %s\n", myMachineID, lockIndex, serverLockTable[lockIndex].lock->getName());
		delete serverLockTable[lockIndex].lock;
		serverLockMap->Clear(lockIndex);
	}
	else{ //some thread is depending on the thread being there, so just defer deletion
		serverLockTable[lockIndex].isToBeDeleted = true;
	}
}

ClientRequest* ServerAcquire(int machineID, int mailbox, int lockIndex) {
	if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
		cout << "server lock array size is " << serverLockArraySize <<  endl;
		printf("Thread %s called Acquire with an invalid index %d\n", currentThread->getName(), lockIndex);
		return NULL;
	}
	if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
		printf("Thread %s called Acquire on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
		return NULL;
	}

	//validation done, ok to perform operation
	printf("Server %d Acquiring ServerLock %d: %s\n", myMachineID, lockIndex, serverLockTable[lockIndex].lock->getName());
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
	printf("Server %d Releasing ServerLock %d: %s\n", myMachineID, lockIndex, serverLockTable[lockIndex].lock->getName());
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

ClientRequest* ServerSignal(int machineID, int mailbox, int conditionIndex, int lockNum){
	cout << "------------------------------------------ IN SERVERSignal---------------" << endl;
	if(conditionIndex < 0 || conditionIndex > serverConditionArraySize -1){ //array index is out of bounds
		printf("Thread %s called Signal with an invalid index %d\n", currentThread->getName(), conditionIndex);
		return NULL;
	}
	if(!serverConditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Signal on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		return NULL;
	}
	if(lockNum < 0 ){ //array index is out of bounds
		printf("Thread %s called Signal with an invalid lock number %d\n", currentThread->getName(), lockNum);
		return NULL;
	}

	//validation done, ok to perform operation
	printf("Server %d Signalling ServerCondition %d: %s with lock %d\n", myMachineID, conditionIndex, serverConditionTable[conditionIndex].condition->getName(), lockNum);
	return serverConditionTable[conditionIndex].condition->Signal(lockNum, new ClientRequest(machineID, mailbox));
}

void ServerWait(int machineID, int mailbox, int conditionIndex, int lockNum){
	cout << "------------------------------------------ IN SERVERWAIT---------------" << endl;
	if(conditionIndex < 0){ //array index is out of bounds
		printf("Thread %s called Wait with an invalid index %d\n", currentThread->getName(), conditionIndex);
		return;
	}
	if(!serverConditionMap->Test(conditionIndex)){ //condition has not been instantiated at this index
		printf("Thread %s called Wait on a condition that does not exist: %d\n", currentThread->getName(), conditionIndex);
		return;
	}


	//validation done, ok to perform operation
	serverConditionTable[conditionIndex].condition->Wait(lockNum, new ClientRequest(machineID, mailbox));
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
	cout << "------------------------------------------ IN SERVERBroadcast---------------" << endl;
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
		//delete cr;	//TODO deleted this to be safe?
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
		printf("Thread %s tried to get a MV at invalid entry index %d in MV array %d.  There are only %d entries!\n", currentThread->getName(), entryIndex, mvIndex, serverMVTable[mvIndex].numEntries);
		return -1;
	}
	int value = serverMVTable[mvIndex].mvEntries[entryIndex];
	printf("Getting MV %s (%d) entry %d, value %d\n", serverMVTable[mvIndex].name, mvIndex, entryIndex, value);
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

	printf("Setting MV %s (%d) entry %d to value %d\n", serverMVTable[mvIndex].name, mvIndex, entryIndex, value);
	serverMVTable[mvIndex].mvEntries[entryIndex] = value;
}

//takes buf[0:3] and makes an int, where 0 is the MSByte of the int
int extractInt(char *buf) {
	int a, b, c, d;
	a = (buf[0] << 24) & 0xff000000;
	b = (buf[1] << 16) & 0x00ff0000;
	c = (buf[2] << 8) & 0x0000ff00;
	d = (buf[3] << 0) & 0x000000ff;

	return a + b + c + d;
}

void compressInt(int x, char *dest) {
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
	case DO_YOU_HAVE_LOCK_NAME:
		messageTypeName = "Do you have lock name";
		break;
	case DO_YOU_HAVE_CV_NAME:
		messageTypeName = "Do you have CV name";
		break;
	case DO_YOU_HAVE_MV_NAME:
		messageTypeName = "Do you have MV name";
		break;
	case DO_NOT_HAVE_LOCK_NAME:
		messageTypeName = "I do not have lock name";
		break;
	case DO_NOT_HAVE_CV_NAME:
		messageTypeName = "I do not have CV name";
		break;
	case DO_NOT_HAVE_MV_NAME:
		messageTypeName = "I do not have MV name";
		break;
	case HAVE_LOCK_NAME:
		messageTypeName = "I do have lock name";
		break;
	case HAVE_CV_NAME:
		messageTypeName = "I do have CV name";
		break;
	case HAVE_MV_NAME:
		messageTypeName = "I do have MV name";
		break;
	case DOES_CLIENT_HAVE_LOCK:
		messageTypeName = "Does Client Have Lock";
		break;
	case CV_LOCK_TRACKER_RESPONSE:
		messageTypeName = "CV Lock Tracker Response";
		break;
	case NO_ACTION:
		messageTypeName = "No action";
		break;
	default:
		messageTypeName = "UNKNOWN";
		printf("Get message type name got message with no type!  Type field was: %.2x\n", messageType);
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

/** encapsulate the logic behind whether we can reply back with regards to a pending create on our server.  Basically, we should respond remember who
 * is requesting the wait and message them back the eventual index if we end up creating the object.  However, if our machineID is greater than the requesting server and
 * we did not already have the other server process our create request, our convention is that this server will not create the object and can message back in the negative.
 */
void determineResponseToCreateRequest(StructureType type, char* name, ServerAction* serverAction, char* responseMessage, PacketHeader* packetHeader, MailHeader* mailHeader, char* inData){
	bool receivedResponse = false;
	PendingCreate* pc = NULL; //temp pointer to remember which PendingCreate object we should be modifying
	for(unsigned int i = 0; i < pendingCreates.size(); i++){ //check for pending create
		if(pendingCreates.at(i)->isAMatch(name, type)){
			//I do have a pending create for this lock/CV/MV
			pc = pendingCreates.at(i); //
			int serverRequesting = packetHeader->from;
			if(pc->responseTracker[serverRequesting]){ //has the requesting server told me it doesn't have this object previously?
				//if so, put it on queue until we know where the object is
				pc->serverResponseQueue.push_back(new ResponseQueueEntry(packetHeader->from, mailHeader->from, extractInt(inData + 1)));
				*serverAction = No_Action;
				receivedResponse = true;
				break;
			}
		}
	}
	if(pc == NULL){ //we don't have a pending Request for the item
			switch(type){
				case Lock:
					responseMessage[0] = DO_NOT_HAVE_LOCK_NAME;
					break;
				case CV:
					responseMessage[0] = DO_NOT_HAVE_CV_NAME;
					break;
				case MV:
					responseMessage[0] = DO_NOT_HAVE_MV_NAME;
					break;
				default:
					cout << "error in determineResponseToCreateRequest: no valid structure type" << endl;
					break;
			}
					(*serverAction) = Respond_Once_To_Server;


	}
	if(!receivedResponse && pc != NULL){ // do this only if we haven't heard an answer from the requesting server
		if(myMachineID < packetHeader->from){ //gives precedence to server with lower ID. if my ID is lower, let's remember to tell this guy later
			pc->serverResponseQueue.push_back(new ResponseQueueEntry(packetHeader->from, mailHeader->from, extractInt(inData + 1)));
			*serverAction = No_Action;
			cout << "should be resolved for cv " << name << endl;
		}
		else{
			cout << "(box 1) determines other servers should create object " << name << endl;
			//if we have the higher ID and had not already heard a "I don't have" from the other server, we can tell the other server definitively we don't have the lock
			//giving the burden of greating it to the other person
			switch(type){
			case Lock:
				responseMessage[0] = DO_NOT_HAVE_LOCK_NAME;
				break;
			case CV:
				responseMessage[0] = DO_NOT_HAVE_CV_NAME;
				break;
			case MV:
				responseMessage[0] = DO_NOT_HAVE_MV_NAME;
				break;
			default:
				cout << "error in determineResponseToCreateRequest: no valid structure type" << endl;
				break;
			}
			(*serverAction) = Respond_Once_To_Server;
		}
	}
}

void LockWatcher(){
	while(true){
		PacketHeader* packetHeader = new PacketHeader;
		MailHeader* mailHeader = new MailHeader;
		packetHeader->from = myMachineID;
		char* data = new char[MaxMailSize];
		mailHeader->to = 2;

		for( int i; i < serverLockArraySize; i++){
			if(serverLockMap->Test(i)){
				ClientRequest* temp = serverLockTable[i].lock->getOwner();
				if(temp == NULL) continue;
				bool success = postOffice->Send(*packetHeader, *mailHeader, data );
				if(!success){
					temp = serverLockTable[i].lock->Release(temp);
					if(temp->respond){
						packetHeader->to = temp->machineID;
						mailHeader->to = temp->mailboxNumber;
						postOffice->Send(*packetHeader, *mailHeader, data );
					}
				}

			}
			usleep(50);
		}
	}
}
void ServerToServerMessageHandler(){
	Thread* t = new Thread("lock pinger");
	t->mailboxNum = 2;
	t->threadID =2 ;
	//t->Fork((VoidFunctionPtr) LockWatcher, 0);
	while(true){ //infinitely check for messages to handle
		PacketHeader *packetHeader = new PacketHeader;
		MailHeader *mailHeader = new MailHeader;
		char* inData = new char[MaxMailSize];
		cout << "\nbox 1 about to do receive" << endl;
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
				cout << "Server (box 1) received message of type: " << messageTypeName << " from machine "
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

				/**The following three cases will result in a message being sent with the data format
				 * outData[0] = message type
				 * outData[1:4] = message ID of incoming message
				 */
				case DO_YOU_HAVE_LOCK:{ //cases where other server is trying to perform Acquire, Release, or DestroyLock
					int index = extractInt(inData + 5);
					cout << "Lock being examined is encoded index " << endl;
					index = decodeIndex(index);
					if( index > 0 && index < serverLockArraySize){
						cout << "inside if statement" << endl;
						outData[0] = (serverLockMap->Test(index)) ? HAVE_LOCK : DO_NOT_HAVE_LOCK;
					}
					else outData[0] = DO_NOT_HAVE_LOCK;
					action = Respond_Once_To_Server;
					outMailHeader->length = 5;
					break;
				}
				case DO_YOU_HAVE_CV:{ //cases where other server is trying to perform Signal, Broadcast, Wait, or DestroyCondition
					int index = extractInt(inData + 5);
					index = decodeIndex(index);
					if(index > 0 && index < serverConditionArraySize){
						outData[0] = (serverConditionMap->Test(index)) ? HAVE_CV : DO_NOT_HAVE_CV;
					}
					else outData[0] = DO_NOT_HAVE_CV;
					action = Respond_Once_To_Server;
					outMailHeader->length = 5;
					break;
				}
				case DO_YOU_HAVE_MV:{ //cases where other server is trying to perform SetMV, GetMV, or DestroyMV
					int index = extractInt(inData + 5);
					index = decodeIndex(index);
					cout << "Server (box 1) testing MV of encoded index " << index << endl;
					if(index > 0 && index < serverMVArraySize){
						outData[0] = (serverMVMap->Test(index)) ? HAVE_MV : DO_NOT_HAVE_MV;
					}
					else{
						outData[0] = DO_NOT_HAVE_MV;
					}
					action = Respond_Once_To_Server;
					outMailHeader->length = 5;
					break;
				}


				case DO_YOU_HAVE_LOCK_NAME:{//case where another server is looking to create a Lock
					int nameLength = inData[5];
					char* name = new char[nameLength];
					strncpy(name, inData + 6, nameLength);
					if(checkIfLockExists(name) != -1){ //do I have it already created?
						outData[0] = HAVE_LOCK_NAME;
						action = Respond_Once_To_Server;
					}
					else{ //I do not already have it
						determineResponseToCreateRequest(Lock, name, &action, outData, packetHeader, mailHeader, inData);
					}
					outMailHeader->length = 5;
					//outMailHeader->length = 5 + 1 /*char that tells size of name*/ + nameLength;
					break;
				}

				case DO_YOU_HAVE_CV_NAME:{ //means another server is looking to create a CV
					int nameLength = inData[5];
					char* name = new char[nameLength];
					strncpy(name, inData + 6, nameLength);
					if(checkIfCVExists(name) != -1){ //if the CV has already been created on this server, reply in the affirmative
						outData[0] = HAVE_CV_NAME;
						action = Respond_Once_To_Server;
					}
					else{
						determineResponseToCreateRequest(CV, name, &action, outData, packetHeader, mailHeader, inData);
					}
					outMailHeader->length = 5;
					//outMailHeader->length = 5 + 1 /*char that tells size of name*/ + nameLength;
					break;
				}

				case DO_YOU_HAVE_MV_NAME:{ //case where another server is looking to create an MV
					int nameLength = inData[5];
					char* name = new char[nameLength];
					strncpy(name, inData + 6, nameLength);
					if(checkIfMVExists(name) != -1){ //if the MV has already been created here, reply in the affirmative
						outData[0] = HAVE_MV_NAME;
						action = Respond_Once_To_Server;
					}
					else{
						determineResponseToCreateRequest(MV, name, &action, outData, packetHeader, mailHeader, inData);
					}
					outMailHeader->length = 5;
					//outMailHeader->length = 5 + 1 /*char that tells size of name*/ + nameLength;
					break;
				}

				case HAVE_LOCK_NAME:
				case HAVE_CV_NAME:
				case HAVE_MV_NAME:{
					int pendingCreateID = extractInt(inData + 1);
					PendingCreate* pc;
					//find the pending create this was resopnding to
					for(unsigned int i = 0; i < pendingCreates.size(); i++){
						if(pendingCreates.at(i)->ID == pendingCreateID){
							pc = pendingCreates.at(i);
							pc->forwardWaitingClientMessagesToServer(packetHeader->from);
							pendingCreates.erase(pendingCreates.begin() + i);

							break;
						}
					}
					action = No_Action;
					break;
				}
				case DO_NOT_HAVE_LOCK_NAME:
				case DO_NOT_HAVE_CV_NAME:
				case DO_NOT_HAVE_MV_NAME:{
					int id = extractInt(inData + 1);
					PendingCreate* temp;
					int tempIndex;
					action = No_Action;
					for(unsigned int i = 0; i < pendingCreates.size(); i++){
						cout << "we're trying to find id of " << id << endl;
						cout << " a pending create has an ID of " << pendingCreates.at(i)->ID << " and is named " << pendingCreates.at(i)->name << endl;
						if(pendingCreates.at(i)->ID == id){

							temp = pendingCreates.at(i);
							cout << "updating response count for " << temp->name << endl;
							temp->numResponses++;
							temp->responseTracker[packetHeader->from] = true;
							if(temp->numResponses == totalNumServers -1) tempIndex = i;
							break;
						}

					}
					if(temp == NULL){
						action = No_Action;
						break;
					}
					if(temp->numResponses == totalNumServers - 1){

						QueryStatus* temp2 = temp->waitingClients.front();
						int index;
						switch(temp->type){
						case Lock:{

							char* name = new char[temp2->data[1]];
							strncpy(name, temp2->data + 2, temp2->data[1]);
							index = encodeIndex(ServerCreateLock(name));
							cout << "Server " << myMachineID << " (box 1) created a Lock named " << name << " of encoded index " << index << endl;
							outData[0] = HAVE_LOCK_NAME;
							break;
						}
						case CV:{
							char* name = new char[temp2->data[1]];
							strncpy(name, temp2->data + 2, temp2->data[1]);
							index = encodeIndex(ServerCreateCV(name));
							cout << "Server " << myMachineID << " (box 1) created a CV named " << name << " of encoded index " << index << endl;
							outData[0] = HAVE_CV_NAME;
							break;
						}
						case MV:{
							char* name = new char[temp2->data[9]];
							strncpy(name, temp2->data + 10, temp2->data[9]);
							int numEntries = extractInt(temp2->data + 1); //numEntries should be in data[1:4]
							int val = extractInt(temp2->data + 5); //initial value should be in data[5:8]
							index = encodeIndex(ServerCreateMV(name, numEntries, val)); //writes the index into the
							cout << "Server " << myMachineID << " (box 1) created a MV named " << name << " of encoded index " << index << endl;
							outData[0] = HAVE_MV_NAME;
							break;
						}
						default:
							cout << "Error case received in ServerToServerHandler in DO_NOT_HAVE_XXX_NAME switch block.  PendingCreate's temp not set" << endl;
							break;
						}
						outPacketHeader->from = myMachineID;
						outMailHeader->from = 1;

						//tell every server with a PendingCreate on the object we just made
						//that we have it
						temp->notifyServersOfCreation(outData);
						temp->messageBackClients(index);
						action = No_Action;

						pendingCreates.erase(pendingCreates.begin() + tempIndex);
					}
					break;
				}

				/**These cases are passed on from the Server function on our machine.
				 * Thus, there will be no messageID, and we must put one in if we are to send
				 * off a message to another ServerToServerHandler.
				 */


				/**the following three messages must query other servers to see if an object has been created.
				 * messages are sent with the following format:
				 * data[0] = message type
				 * data[1:4] = my ID for the query
				 * data[5] = name length
				 * data[6 : 6 + name length] = char array for name
				 */
				case CREATE_LOCK:{
					int nameLength = inData[1];
					char* name = new char[nameLength];
					strncpy(name, inData + 2, nameLength); //parse message
					int index = checkIfLockExists(name);
					if(index != -1){ //do I have it already created?
						compressInt(encodeIndex(index), outData);
						action = Respond_Once_To_Client;
						break;
					}
					else{
						//package response queries
						outData[0] = DO_YOU_HAVE_LOCK_NAME;
						outData[5] = nameLength;
						strncpy(outData + 6, inData + 2, nameLength);
						outMailHeader->length = 5 + 1 + nameLength;
						action = Create_Query; //need to poll other servers before deciding action
						bool createNew = true;
						for(unsigned int i = 0; i < pendingCreates.size(); i++){
							if(pendingCreates.at(i)->isAMatch(name, Lock)){
								pendingCreates.at(i)->waitingClients.push_back(new QueryStatus(packetHeader, mailHeader, inData));
								cout << "Server (box 1) adding lock to pending Create" << endl;
								createNew = false;
								action = No_Action;
								break;
							}
						}
						if(createNew){
							cout << "Server (box 1) adding PendingCreate for Lock" << endl;
							pendingCreates.push_back(new PendingCreate(name, Lock)); //so we can track responses from other servers and remember we are trying to create
							compressInt(pendingCreates.back()->ID, outData + 1);
							pendingCreates.back()->waitingClients.push_back(new QueryStatus(packetHeader, mailHeader, inData));
						}
					}
					break;
				}
				case CREATE_CV:{
					int nameLength = inData[1];
					char* name = new char[nameLength];
					strncpy(name, inData + 2, nameLength); //parse message
					int index = checkIfCVExists(name);
					if(index != -1){ //do I have it already created?
						compressInt(encodeIndex(index), outData);
						action = Respond_Once_To_Client;
						break;
					}
					else{
						//package response queries
						outData[0] = DO_YOU_HAVE_CV_NAME;
						outData[5] = nameLength;
						strncpy(outData + 6, inData + 2, nameLength);
						outMailHeader->length = 5 + 1 + nameLength;
						action = Create_Query; //need to poll other servers before deciding action
						bool createNew = true;
						for(unsigned int i = 0; i < pendingCreates.size(); i++){
							if(pendingCreates.at(i)->isAMatch(name, CV)){
								cout << "Server (box 1) adding CV to  pre-existing pending Create for " << name << endl;
								pendingCreates.at(i)->waitingClients.push_back(new QueryStatus(packetHeader, mailHeader, inData));
								createNew = false;
								action = No_Action;
								break;
							}
						}
						if(createNew){

							cout << "Server (box 1) adding PendingCreate for CV" << endl;
							pendingCreates.push_back(new PendingCreate(name, CV)); //so we can track responses from other servers and remember we are trying to create
							pendingCreates.back()->waitingClients.push_back(new QueryStatus(packetHeader, mailHeader, inData));
							compressInt(pendingCreates.back()->ID, outData + 1);
						}

					}
					break;
				}
				case CREATE_MV:{
					int nameLength = inData[9];
					char* name = new char[nameLength];
					strncpy(name, inData + 10, nameLength); //parse message
					int index = checkIfMVExists(name);
					if(index != -1){ //do I have it already created?
						compressInt(encodeIndex(index), outData);
						action = Respond_Once_To_Client;
						break;
					}
					else{
						//package response queries
						outData[0] = DO_YOU_HAVE_MV_NAME;
						outData[5] = nameLength;
						strncpy(outData + 6, inData + 2, nameLength);
						outMailHeader->length = 5 + 1 + nameLength;
						action = Create_Query; //need to poll other servers before deciding action
						bool createNew = true;
						for(unsigned int i = 0;  i < pendingCreates.size(); i++){
							if(pendingCreates.at(i)->isAMatch(name, MV)){
								pendingCreates.at(i)->waitingClients.push_back(new QueryStatus(packetHeader, mailHeader, inData));
								compressInt(pendingCreates.at(i)->ID, outData + 1);
								createNew = false;
								action = No_Action;
								break;
							}
						}
						if(createNew){
							pendingCreates.push_back(new PendingCreate(name, MV)); //so we can track responses from other servers and remember we are trying to create
							pendingCreates.back()->waitingClients.push_back(new QueryStatus(packetHeader, mailHeader, inData));
							compressInt(pendingCreates.back()->ID, outData + 1);
						}
					}
					break;
				}

				/**The following messages need a format of the following:
				 * data[0] = message type
				 * data[1: 4] = my id for the query
				 * data[5:8] = encoded index of the object
				 */
				case ACQUIRE:
				case RELEASE:
				case DESTROY_LOCK:{
					outData[0] = DO_YOU_HAVE_LOCK;
					compressInt(extractInt(inData + 1), outData + 5);
					//strncpy(outData + 5, inData + 1, 4);
					action = Query_All_Servers;
					outMailHeader->length = 9;
					break;
				}


				/**when we get these CV operation messages, the lock validation/ownership should have already been performed which is why we
				 * only send one message with the CV index
				 */
				case DESTROY_CV:{
					outData[0] = DO_YOU_HAVE_CV;
					compressInt(extractInt(inData + 1), outData + 5);
					//strncpy(outData + 5, inData + 1, 4);
					action = Query_All_Servers;
					outMailHeader->length = 9;
					break;
				}
				case SIGNAL:	//condition, then lock in message
				case WAIT:
				case BROADCAST:{
					outData[0] = DO_YOU_HAVE_CV;
					compressInt(extractInt(inData + 1), outData + 5); // i had inData + 5, why?
					//strncpy(outData + 5, inData + 5, 4);
					action = Query_All_Servers;
					outMailHeader->length = 9;
					break;
				}


				case DESTROY_MV:
				case GET_MV:
				case SET_MV:{
					outData[0] = DO_YOU_HAVE_MV;
					compressInt(extractInt(inData + 1), outData + 5);
					//strncpy(outData + 5, inData + 1, 4);
					action = Query_All_Servers;
					outMailHeader->length = 9;
					break;
				}

				case HAVE_LOCK:
				case HAVE_CV:
				case HAVE_MV:{
					action = Do_Bookkeeping;
					int id = extractInt(inData + 1); //get our query ID so we can find the pending QuerySTatus object
					for(unsigned int i = 0; i < queryQueue.size(); i++){ //search and ifnd the matching query
						QueryStatus* temp = queryQueue.at(i);
						if(temp->id == id){
							temp->packetHeader->to = packetHeader->from; //repackage original client message to send to machine with the object
							temp->mailHeader->to = 0; //send to mailbox 0, this allows the server to handle the message like a miss never happend
							postOffice->Send(*temp->packetHeader, *temp->mailHeader, temp->data); //send the message
							queryQueue.erase(queryQueue.begin() + i); //delete the query
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
					action = No_Action;

					int id = extractInt(inData + 1); //get our query ID so we can find the pending QuerySTatus object
					for(unsigned int i = 0; i < queryQueue.size(); i++){ //find the query so we can update it
						QueryStatus* temp = queryQueue.at(i);
						if(temp->id == id){
							temp->numResponsesReceived++; //track how many servers we've heard from
							if(temp->numResponsesReceived == totalNumServers - 1){ //we've heard a no from all servers

								switch(temp->type){ //this means the object didn't exist on any server!
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
								cout << "Server: " << myMachineID << " Mailbox number: 1" << endl;
								cout << "message data: " << messageTypeName << inData + 1 << endl;
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
						//everything is pre packaged
						cout << "Server " << myMachineID << " (box 1) found object already created and is messaging back client " << packetHeader->from << "-" << mailHeader->from << endl;
						outPacketHeader->to = packetHeader->from;
						outPacketHeader->from = myMachineID;
						outMailHeader->to = mailHeader->from;
						outMailHeader->from = 1;
						outMailHeader->length = 4; // only message format we ever have sends one int to a client with the result of a create
													//any other server messages to clients come from the other function
						postOffice->Send(*outPacketHeader, *outMailHeader, outData);
						break;
				}
				case Respond_Once_To_Server:{

						//code is always identical
						outPacketHeader->from = myMachineID;
						outPacketHeader->to = packetHeader->from;
						outMailHeader->from = mailHeader->to;
						outMailHeader->to = mailHeader->from;
						cout << "Server " << myMachineID << " (box 1) sending message of type " << getMessageTypeName(outData[0]) << " to Server " << outPacketHeader->to << " box " << outMailHeader->to <<endl;
						compressInt(extractInt(inData + 1), outData + 1);
						//outMailHeaderSize should already be set
						//strncpy(outData + 1, inData + 1, 4); //copy message ID into response
						cout << "out id " << extractInt(outData + 1);
						bool success = postOffice->Send(*outPacketHeader, *outMailHeader, outData);

						break;
				}
				case Create_Query:{
					//send message to all servers
					cout << "Server " << myMachineID << " (box 1) sending message of type " << getMessageTypeName(outData[0]) << " to all server box 1s in case Create_Query" << endl;
					for(int i = 0; i < totalNumServers; i++){
						if(i == myMachineID) continue;
						outPacketHeader->from = myMachineID;
						outPacketHeader->to = i;
						outMailHeader->from = 1;
						outMailHeader->to = 1;
						cout << " id " << extractInt(outData + 1) << endl;
						bool success = postOffice->Send(*outPacketHeader, *outMailHeader, outData);

					}
					break;
				}
				case Query_All_Servers:{

						//instantiate and set up new QueryStatus
						QueryStatus* qs = new QueryStatus(packetHeader, mailHeader, inData);
						qs->type = getType(messageType);
						//grab and package a query id.  this will be so we can
						//match responses from other servers with this query
						compressInt(qs->id, outData + 1);
						cout << " encoded index of the object being queried about is " << extractInt(inData + 1) << endl;
						nextQueryID++;
						cout << "Server " << myMachineID << " (box 1) sending message of type " << getMessageTypeName(outData[0]) << " to all server box 1s in case Query_All_Servers" << endl;
						//send message to all servers
						queryQueue.push_back(qs);
						for(int i = 0; i < totalNumServers; i++){
							if(i == myMachineID) continue;
							outPacketHeader = new PacketHeader;
							outMailHeader = new MailHeader;
							outPacketHeader->from = myMachineID;
							outPacketHeader->to = i;
							outMailHeader->from = 1;
							outMailHeader->to = 1;
							outMailHeader->length = 9;
							bool success = postOffice->Send(*outPacketHeader, *outMailHeader, outData);

						}
						break;
				}

				case No_Action:
				case Do_Bookkeeping:

					break;
				default: break;
				}


				delete outPacketHeader;
				delete outMailHeader;
				//delete[] outData;
	}
}

void Server() {
	cout << "Nachos server starting up with machine ID " << myMachineID << endl;
	cout << " -note, all clients expect a server with machineID 0" << endl;
	Thread* t = new Thread("ServerToServerHandler");
	t->mailboxNum = 1;
	t->threadID = 1;
	if(totalNumServers > 1)t->Fork((VoidFunctionPtr) ServerToServerMessageHandler, 0);
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

		string messageTypeName = getMessageTypeName(messageType);


		cout << "\nServer (box 0) received message of type: " << messageTypeName << " from machine "
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
				if(!serverLockMap->Test(i)) {
					continue;
				}
				if(!strcmp(name, serverLockTable[i].lock->getName())) {
					response.data = encodeIndex(i);
					necessaryResponses.push(response);
					respond = true;
					alreadyHad = true;
					break;
				}
			}
			if(!alreadyHad) {	//inform my server-server thread to handle creating the object
				printf("Server %d did not have lock %s\n", myMachineID, name);
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
				printf("DESTROY_LOCK: I don't think Lock (num %d) should be on my machine (%d), machine %d should have it\n", lockNum, myMachineID, lockMachineID);
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
			}
			else {
				int lockIndex = decodeIndex(lockNum);
				ServerDestroyLock(lockIndex);
			}
			break;
		}
		case ACQUIRE:
		{
			int lockNum = extractInt(messageData + 1);
			int lockIndex = decodeIndex(lockNum);
			int lockMachineID = decodeMachineIDFromLockNumber(lockNum);

			if(lockMachineID != myMachineID) {
				printf("ACQUIRE: I don't think Lock (num %d) should be on my machine (%d), machine %d should have it\n", lockNum, myMachineID, lockMachineID);
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
			else {
				respond = false;
			}
			break;
		}
		case RELEASE:
		{
			int lockNum = extractInt(messageData + 1);
			int lockIndex = decodeIndex(lockNum);
			int lockMachineID = decodeMachineIDFromLockNumber(lockNum);

			if(lockMachineID != myMachineID) {
				printf("RELEASE: I don't think Lock (num %d) should be on my machine (%d), machine %d should have it\n", lockNum, myMachineID, lockMachineID);
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
				break;
			}

			ClientRequest* temp = ServerRelease(messageFromMachineID, messageFromMailbox, lockIndex);
			if(temp == NULL) {
				necessaryResponses.push(response);
				respond = true;
			}
			else if(temp->respond){
				necessaryResponses.push(response);	//hacky way of responding to two people without having to create an additional ServerResponse object
				response.toMachine = temp->machineID;
				response.toMailbox = temp->mailboxNumber;
				necessaryResponses.push(response);
				respond = true;
			}
			else {
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
				if(!serverConditionMap->Test(i)) {
					continue;
				}
				if(!strcmp(name, serverConditionTable[i].condition->getName())) {
					response.data = encodeIndex(i);
					necessaryResponses.push(response);
					respond = true;
					alreadyHad = true;
					break;
				}
			}
			if(!alreadyHad) {	//inform my server-server thread to handle creating the object
				printf("Server %d did not have CV %s\n", myMachineID, name);
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
			int lockNum = extractInt(messageData + 9);
			int lockIndex = decodeIndex(lockNum);
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
				//delete cr;
			}

			char *rData;
			int responseLength = 14;
			rData = new char[responseLength];

			rData[0] = CV_LOCK_TRACKER_RESPONSE;
			compressInt(clientMID, rData + 1);
			compressInt(clientMBX, rData + 5);
			compressInt(lockNum, rData + 9);
			if(held) {
				rData[13] = 1;
			}
			else {
				rData[13] = 0;
			}

			sendMessageWithData(myMachineID, 0, messageFromMachineID, messageFromMailbox, responseLength, rData);

			respond = false;
			break;
		}

		case CV_LOCK_TRACKER_RESPONSE:
		{

			cout << "------------------IN CV_LOCK_TRACKER_RESONSE HANDLER" << endl;
			int clientMID = extractInt(messageData + 1);
			int clientMBX = extractInt(messageData + 5);
			int lockNum = extractInt(messageData + 9);
			int cvNum;
			char operation;
			int cvltListEntry = -1;

			response.toMachine = clientMID;
			response.toMailbox = clientMBX;

			for(unsigned int i = 0; i < cvLockTrackerList.size(); i++) {
				/**if(cvLockTrackerList.at(i).clientMachineID == clientMID){
					ASSERT(FALSE);
				}
				if(cvLockTrackerList.at(i).clientMailbox == clientMBX ){
					//ASSERT(FALSE);
				}
				if(cvLockTrackerList.at(i).lockNum == lockNum){
					//ASSERT(FALSE);
				}*/
				if( cvLockTrackerList.at(i).clientMachineID == clientMID &&
						cvLockTrackerList.at(i).clientMailbox == clientMBX &&
						cvLockTrackerList.at(i).lockNum == lockNum) {
					//ASSERT(FALSE);
					cout << "------------------FOUND CV_LOCK_TRACKER MATCH ------------" << endl;
					cvNum = cvLockTrackerList.at(i).cvNum;
					operation = cvLockTrackerList.at(i).messageType;
					cvltListEntry = i;
					break;
				}
			}
			if(cvltListEntry == -1) {
				printf("Got a CV_LOCK_TRACKER_RESPONSE, but had no corresponding entry in my CVLT List!\n");
				printf("  -ClientMID: %d  ClientMBX: %d  lockNum: %d", clientMID, clientMBX, lockNum);
			}
			else {
				cvLockTrackerList.erase(cvLockTrackerList.begin() + cvltListEntry);
			}

			if(messageData[13] == FALSE) {	//if lock was not owned by this client
				necessaryResponses.push(response);	//TODO, need to send to client maybe instead of guy who sent me CVLT resp?
				respond = true;
				printf("Client %d - %d called %s on CV %d with lock that it does not have (lock num %d)\n", clientMID, clientMBX, getMessageTypeName(operation).c_str(), cvNum, lockNum);
			}
			else {
				//yes, he had it, now proceed.  do we have the CV?

				if(decodeMachineIDFromCVNumber(cvNum) == myMachineID) {

					if(operation == SIGNAL) {

						ClientRequest *temp = ServerSignal(clientMID, clientMBX, decodeIndex(cvNum), lockNum);
						if(temp == NULL) {
							//respond = false;
						}
						else if(temp->respond){
							response.toMachine = temp->machineID;	//TODO **done?**, these will be from the server now, so we have to get CVLT data instead
							response.toMailbox = temp->mailboxNumber;
							necessaryResponses.push(response);
							respond = true;
						}

						response.toMachine = clientMID;	//respond to guy who sent signal
						response.toMailbox = clientMBX;
						necessaryResponses.push(response);
						respond = true;

						//delete temp;	//TODO commented out for safety?
					}
					else if(operation == WAIT) {
						ServerWait(clientMID, clientMBX, decodeIndex(cvNum), lockNum);
						//TODO release lock

						char *releaseData = new char[5];
						releaseData[0] = RELEASE;
						compressInt(lockNum, releaseData + 1);
						sendMessageWithData(myMachineID, 0, decodeMachineIDFromLockNumber(lockNum), 0, 5, releaseData);

						respond = false;
					}
					else if(operation == BROADCAST) {
						ServerBroadcast(messageFromMachineID, messageFromMailbox, decodeIndex(cvNum), lockNum);	//adding responses to the queue is handled in broadcast
						response.toMachine = clientMID;
						response.toMailbox = clientMBX;
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
			int cvNum = extractInt(messageData + 1);
			int lockNum   = extractInt(messageData + 5);
			int lockOwnerMachineID = decodeMachineIDFromLockNumber(lockNum);	//get machine who has the lock

			if(lockOwnerMachineID < 0 || lockOwnerMachineID > totalNumServers) {
				cout << "Invalid lock " << lockNum << " passed to SIGNAL, cannot decode lock owner" << endl;
				necessaryResponses.push(response);
				respond = true;
			}

			if(lockOwnerMachineID == myMachineID) {	//if i am the owner of the lock
				//proceed as normal
				int lockIndex = decodeIndex(lockNum);

				if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
					printf("Thread %s called Signal with an invalid lock index %d\n", currentThread->getName(), lockIndex);
					necessaryResponses.push(response);
					respond = true;
					return;
				}
				if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
					printf("Thread %s called Signal on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
					necessaryResponses.push(response);
					respond = true;
					return;
				}

				//verify who owns the CV
				int cvOwnerMachineID = decodeMachineIDFromCVNumber(cvNum);

				if(cvOwnerMachineID == myMachineID) {	//If I also own the CV, do normal operation
					int cvIndex = decodeIndex(cvNum);

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
					response.toMachine = messageFromMachineID;
					response.toMailbox = messageFromMailbox;
					necessaryResponses.push(response);
					respond = true;
					//delete temp;
				}
				else {	//I don't have the CV, send it to server-server thread
					sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
					respond = false;
				}
			}
			else {
				//some other server has the lock, do messaging and CVLT
				int lockOwnerMailbox = 0;	//we know that in this case, we have to send to mailbox 0
				char *queryData;
				int queryDataLength = 13;
				queryData = new char[queryDataLength];

				queryData[0] = DOES_CLIENT_HAVE_LOCK;
				compressInt(messageFromMachineID, queryData + 1);
				compressInt(messageFromMailbox, queryData + 5);
				compressInt(lockNum, queryData + 9);

				CVLockTracker cvlt;
				cvlt.messageType = messageType;
				cvlt.cvNum = cvNum;
				cvlt.lockNum = lockNum;
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

			int cvNum = extractInt(messageData + 1);
			int lockNum = extractInt(messageData + 5);
			int lockOwnerMachineID = decodeMachineIDFromLockNumber(lockNum);	//get machine who has the lock

			if(lockOwnerMachineID < 0 || lockOwnerMachineID > totalNumServers) {
				cout << "Invalid lock " << lockNum << " passed to BROADCAST, cannot decode lock owner" << endl;
				necessaryResponses.push(response);
				respond = true;
			}

			if(lockOwnerMachineID == myMachineID) {	//if i am the owner of the lock
				//proceed as normal
				int lockIndex = decodeIndex(lockNum);
				if(lockIndex < 0 || lockIndex > serverLockArraySize -1){ //array index is out of bounds
					printf("Thread %s called Broadcast with an invalid lock index %d\n", currentThread->getName(), lockIndex);
					necessaryResponses.push(response);
					respond = true;
					return;
				}
				if(!serverLockMap->Test(lockIndex)){ //lock has not been instantiated at this index
					printf("Thread %s called Broadcast on a lock that does not exist: %d\n", currentThread->getName(), lockIndex);
					necessaryResponses.push(response);
					respond = true;
					return;
				}

				int cvOwnerMachineID = decodeMachineIDFromCVNumber(cvNum);
				if(cvOwnerMachineID == myMachineID) {	//If I also own the CV, do normal operation
					int cvIndex = decodeIndex(cvNum);
					ServerBroadcast(messageFromMachineID, messageFromMailbox, cvIndex, lockIndex);	//adding responses to the queue is handled in broadcast
					necessaryResponses.push(response);
					respond = true;
				}
				else {	//I don't have the CV, send it to server-server thread
					sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
					respond = false;
				}

			}
			else {
				//some other server has it, do messaging and CVLT
				int lockOwnerMailbox = 0;	//we know that in this case, we have to send to mailbox 0
				char *queryData;
				int queryDataLength = 13;
				queryData = new char[queryDataLength];

				queryData[0] = DOES_CLIENT_HAVE_LOCK;
				compressInt(messageFromMachineID, queryData + 1);
				compressInt(messageFromMailbox, queryData + 5);
				compressInt(lockNum, queryData + 9);

				CVLockTracker cvlt;
				cvlt.messageType = messageType;
				cvlt.cvNum = cvNum;
				cvlt.lockNum = lockNum;
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
			int cvNum = extractInt(messageData + 1);
			int lockNum = extractInt(messageData + 5);
			int lockOwnerMachineID = decodeMachineIDFromLockNumber(lockNum);	//get machine who has the lock

			if(lockOwnerMachineID < 0 || lockOwnerMachineID > totalNumServers) {
				cout << "Invalid lock " << lockNum << " passed to WAIT, cannot decode lock owner" << endl;
				necessaryResponses.push(response);
				respond = true;
			}

			if(lockOwnerMachineID == myMachineID) {	//if i am the owner of the lock
				//proceed as normal
				int lockIndex = decodeIndex(lockNum);

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


				int cvOwnerMachineID = decodeMachineIDFromCVNumber(cvNum);
				if(cvOwnerMachineID == myMachineID) {	//If I also own the CV, do normal operation
					int cvIndex = decodeIndex(cvNum);
					ClientRequest *relResp = ServerRelease(messageFromMachineID, messageFromMailbox, lockIndex);

					if(relResp->respond == true) {
						response.toMachine = relResp->machineID;
						response.toMailbox = relResp->mailboxNumber;
						necessaryResponses.push(response);
						respond = true;
					}

					ServerWait(messageFromMachineID, messageFromMailbox, cvIndex, lockIndex);
				}
				else {	//I don't have the CV, send it to server-server thread
					sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
					respond = false;
				}
			}
			else {	//I am not the owner of the lock
				int lockOwnerMailbox = 0;	//we know that in this case, we have to send to mailbox 0
				char *queryData;
				int queryDataLength = 13;
				queryData = new char[queryDataLength];

				queryData[0] = DOES_CLIENT_HAVE_LOCK;
				compressInt(messageFromMachineID, queryData + 1);
				compressInt(messageFromMailbox, queryData + 5);
				compressInt(lockNum, queryData + 9);

				CVLockTracker cvlt;
				cvlt.messageType = messageType;
				cvlt.cvNum = cvNum;
				cvlt.lockNum = lockNum;
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
				if(!serverMVMap->Test(i)) {
					continue;
				}
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
					response.data = encodeIndex(ServerCreateMV(name, numEntries, initialValue));
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
			int mvToDestroyNum = extractInt(messageData + 1);
			int mvMachineID = decodeMachineIDFromMVNumber(mvToDestroyNum);

			if(mvMachineID != myMachineID) {
				printf("DESTROY_MV: I don't think MV (num %d) should be on my machine (%d), machine %d should have it\n", mvToDestroyNum, myMachineID, mvMachineID);
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
				break;
			}

			int mvToDestroy = decodeIndex(mvToDestroyNum);

			ServerDestroyMV(mvToDestroy);
			break;
		}
		case GET_MV:
		{
			int mvNum = extractInt(messageData + 1);
			//int mvIndex = mvNum;
			int mvMachineID = decodeMachineIDFromMVNumber(mvNum);

			if(mvMachineID != myMachineID) {
				printf("GET_MV: I don't think MV (num %d) should be on my machine (%d), machine %d should have it\n", mvNum, myMachineID, mvMachineID);
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
				break;
			}

			int mvIndex = decodeIndex(mvNum);

			int entryIndex = extractInt(messageData + 5);
			response.data = ServerGetMV(mvIndex, entryIndex);
			necessaryResponses.push(response);
			respond = true;
			break;
		}
		case SET_MV:
		{
			int mvNum = extractInt(messageData + 1);

			int mvMachineID = decodeMachineIDFromMVNumber(mvNum);
			//cout << "server got a setMV message thinking mv should be on machine " << mvMachineID << endl;
			if(mvMachineID != myMachineID) {
				printf("SET_MV: I don't think MV (num %d) should be on my machine (%d), machine %d should have it", mvNum, myMachineID, mvMachineID);
				sendMessageWithData(messageFromMachineID, messageFromMailbox, myMachineID, 1, messageLength, messageData);
				respond = false;
				break;
			}

			int mvIndex = decodeIndex(mvNum);

			necessaryResponses.push(response);
			respond = true;
			int entryIndex = extractInt(messageData + 5);
			int value = extractInt(messageData + 9);
			ServerSetMV(mvIndex, entryIndex, value);
			break;
		}
		case NO_ACTION:
		{
			cout << "Received message response, no action necessary" << endl;
			respond = false;
			break;
		}
		default:
		{
			//oops...
			printf("Received message with no type!  Type field was: %.2x\n", messageType);
			respond = false;
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



