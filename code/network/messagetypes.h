/*
 * messagetypes.h
 *
 *  Created on: Jul 7, 2012
 *      Author: Keith
 */



char CREATE_LOCK = 'a';
char DESTROY_LOCK = 'b';
char ACQUIRE = 'c';
char RELEASE = 'd';
char CREATE_CV = 'e';
char DESTROY_CV = 'f';
char SIGNAL = 'g';
char WAIT = 'h';
char BROADCAST = 'i';
char CREATE_MV = 'j';
char DESTROY_MV = 'k';
char GET_MV = 'l';
char SET_MV = 'm';

/*
Note: When ints

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
data[5] = nameLength, the length of the name for this MV array
data[6:6 + nameLength] = the data for the name itself

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
