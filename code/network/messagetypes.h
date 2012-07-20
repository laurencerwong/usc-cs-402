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
#define DO_YOU_HAVE_LOCK 'n'
#define HAVE_LOCK 'o'
#define DO_NOT_HAVE_LOCK 'p'
#define DO_YOU_HAVE_CV 'q'
#define HAVE_CV 'r'
#define DO_NOT_HAVE_CV 's'
#define DO_YOU_HAVE_MV 't'
#define HAVE_MV 'u'
#define DO_NOT_HAVE_MV 'v'


//------------------------Server-Server messages-------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
/*
::::: Message Formats :::::
---= FOR QUERY/QUERY RESPONSE MESSAGES ==---
data[0] = message type
data[1:4] = array index
data[5:8] = entry index


---= FOR ALL OPERATION REQUEST MESSAGES =====
data[0] = message type

---= FOR CREATES --==========
data[1] = name size
data[2 : 2 + name size - 1] = name
---= FOR CREATE MV --======
data[1:4] = number of entries
data[5:8] = inital values
data[9] = name size
data[10 : 10 + name size - 1] = name


---== FOR ACQUIRE, RELEASE, DESTROYS ---=
data[1 : 4] = index

---== FOR SIGNAL, WAIT, BROADCAST ---=
data [1:4] = condition index
data [5: 8] = lock index








//-------------------------------------------------------------------------------------------------------------
//--------------------Client-Server messages-----------------------------------------------------------------
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
