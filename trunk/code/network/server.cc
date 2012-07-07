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
		//parse
		char messageType = messageData[0];
		ClientRequest request;
		request.machineID = messageData[1] + (messageData[4] << 8) + (messageData[3] << 16) + (messageData[2] << 24);
		request.mailboxNumber = (messageData[5] << 24) + (messageData[6] << 16) + (messageData[7] << 8) + messageData[8];

		//handle the message
		ClientRequest reply;
		switch(messageType) {	//first byte is the message type
		case CREATE_LOCK:
			reply = ServerCreateLock(request);
			break;
		case DESTROY_LOCK:
			reply = ServerDestroyLock(request);
			break;
		case ACQUIRE:
			reply = ServerAcquire(request);
			break;
		case RELEASE:
			reply = ServerRelease(request);
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

