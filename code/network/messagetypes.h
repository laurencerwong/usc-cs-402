/*
 * messagetypes.h
 *
 *  Created on: Jul 7, 2012
 *      Author: Keith
 */



#define CREATE_LOCK 'a'
#define DESTROY_LOCK 'b'
#define ACQUIRE 'c'
#define RELEASE 'd'
#define CREATE_CV 'e'
#define DESTROY_CV 'f'
#define SIGNAL 'g'
#define WAIT 'h'
#define BROADCAST 'i'
#define CREATE_MV 'j'
#define DESTROY_MV 'k'
#define GET_MV 'l'
#define SET_MV 'm'

/*
Note: When ints are in messages, the MSByte comes first, ie. read the int from left to right in the order the bytes are received

::::: Message Formats :::::
-What the server expects to get from the client

---== FOR ALL MESSAGES ==---
data[0] = message type


---- Lock messages ----
Create:
data[1] = nameLength, the length of the name for this lock
data[2:2 + nameLength] = the data for the name itself

Destroy:
data[1:4] = lockIndex, the index of the lock that we want to delete

Acquire:
data[1:4] = lockIndex, the index of the lock that we want to acquire

Release:
data[1:4] = lockIndex, the index of the lock that we want to release


---- CV messages ----
Create:
data[1] = nameLength, the length of the name for this CV
data[2:2 + nameLength] = the data for the name itself

Destroy:
data[1:4] = lockIndex, the index of the CV that we want to delete

Signal:
data[1:4] = cvIndex, the index of the CV you want to signal
data[5:8] = lockIndex, the index of the lock you want to use

Wait:
data[1:4] = cvIndex, the index of the CV you want to signal
data[5:8] = lockIndex, the index of the lock you want to use

Broadcast:
data[1:4] = cvIndex, the index of the CV you want to signal
data[5:8] = lockIndex, the index of the lock you want to use


---- MV messages ----
Create:
data[1:4] = numEntries, the number of entries you watn to create in this MV array
data[5:8] = initialValue, the value that every entry in the new MV will be initialized to
data[9] = nameLength, the length of the name for this MV array
data[10:10 + nameLength] = the data for the name itself

Destroy:
data[1:4] = mvToDestroy, the index of the MV array that we are deleting

Get:
data[1:4] = mvIndex, the index telling which MV array to use
data[5:8] = entryIndex, tells us which entry out of the MV array we want to read

Set:
data[1:4] = mvIndex, the index telling which MV array to use
data[5:8] = entryIndex, tells us which entry out of the MV array we want to read
data[9:12] = value, the value we want to set this entry in the given MV array



*/
