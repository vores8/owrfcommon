#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <nrf24l01.h>
#include <debugprint.h>

#include "owrfreadwrite.h"
#include <RS485_protocol.h>

#define RW_TIMEOUT 1000
#define PAUSE 2

int nrf_write(const unsigned char what) {
	unsigned char b = what;
	int result = nrf24l01_write(&b, 1);
	if (result != 1)
		debugPrint("nrf_write %x FAIL\r\n", what);
	return result;
}
int nrf_available() {
	int pipe;
	if (nrf24l01_readready(&pipe)) {
		return NRF24L01_PAYLOAD;
	}
	return 0;
}

int nrf_read() {
	unsigned char b;
	nrf24l01_read(&b, 1);
	return b;
}

int owrf_read(unsigned char * buffer) {
	unsigned char length = -1;
	if (recvMsg(nrf_available, nrf_read, &length, 1, RW_TIMEOUT) == 1) {
		_delay_ms(PAUSE);
		sendMsg(nrf_write, &length, 1);
		_delay_ms(PAUSE);
		return recvMsg(nrf_available,   // return available count
				nrf_read,             // read one byte
				buffer,                    // buffer to receive into
				length,              // maximum buffer size
				RW_TIMEOUT);
	}
	return -1;
}

int owrf_write(unsigned char * buffer, int length) {
	unsigned char b = length;
	if (sendMsg(nrf_write, &b, 1) != 1) {
		return 0;
	}
	_delay_ms(PAUSE);
	if (recvMsg(nrf_available, nrf_read, &b, 1, RW_TIMEOUT) != 1) {
		if (b != length)
			return 0;
	}
	_delay_ms(PAUSE);
	return sendMsg(nrf_write, buffer, length);
}
