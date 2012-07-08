/*
 * messagetypes.h
 *
 *  Created on: Jul 7, 2012
 *      Author: Keith
 */

//data[0] = message type
//data[1] name size
//data[2:2+namesize] name

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
