/*
 * ds1992.c
 *
 *  Created on: 11 февр. 2014 г.
 *      Author: dbkrasn
 */

#include <string.h>

unsigned char scratchpad[32];
unsigned char memory[4][32];

unsigned char mode = 0xFF;

int ds1992(unsigned char* data, int len) {
	if (mode == 0xFF) {
		mode = data[0];
	}

	if (mode == 0xAA) {
		if (data[0] == 0xAA) {
			data[1] = 0x00;
			data[2] = 0x00;
			data[3] = 0x9F;
			return 1;
		} else if (data[0] == 0xFF){
			memcpy(data, scratchpad, len);
			mode = 0xFF;
			return 1;
		}
	} else if (mode == 0x0F) {
		if (data[0] == 0x0F) {
			memcpy(scratchpad + (data[1] & 0x1F), data, len);
			mode = 0xFF;
			return 1;
		}
	}

	return 0;
}

