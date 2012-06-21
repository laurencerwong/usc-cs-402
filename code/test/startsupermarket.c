/*
 * startsupermarket.c
 *
 *  Created on: Jun 21, 2012
 *      Author: Robert
 */

int main(){
	Exec("../test/supermarket", sizeof("../test/supermarket"), "supermarket thread 1", sizeof("supermarket thread 1"));
	Exec("../test/supermarket", sizeof("../test/supermarket"), "supermarket thread 2", sizeof("supermarket thread 2"));
	Exit(0);
}
