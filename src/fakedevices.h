/*
 * fakedevices.h
 *
 *  Created on: 11 ����. 2014 �.
 *      Author: dbkrasn
 */

#ifndef FAKEDEVICES_H_
#define FAKEDEVICES_H_

typedef struct {
	unsigned char ROM[8];
	int (*processCommand)(unsigned char *bufer, int length);
} FAKEDEVICE;


#endif /* FAKEDEVICES_H_ */
