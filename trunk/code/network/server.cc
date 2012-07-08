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
#include "messagetypes"






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
			reply = ServerDestroyLock(owner, request);
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
		case SIGNAL:
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

