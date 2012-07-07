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
		//parse part 1
		char messageType = messageData[0];
		ClientRequest request;


		//parse part 2
		ClientRequest reply;
		switch(messageType) {	//first byte is the message type
		case CREATE_LOCK:
			reply = ServerCreateLock(request);
			break;
		case DESTROY_LOCK:
			reply = ServerDestroyLock(request);
			break;
		case ACQUIRE:

			break;
		case RELEASE:

			break;
		case CREATE_CV:

			break;
		case DESTROY_CV:

			break;
		case SIGNAL:

			break;
		case WAIT:

			break;
		case BROADCAST:

			break;
		case CREATE_MV:

			break;
		case DESTROY_MV:

			break;
		case GET_MV:

			break;
		case SET_MV:

			break;
		default:
			//oops...
			break;

		}

		//handle the message

		//send response message
	}
}

