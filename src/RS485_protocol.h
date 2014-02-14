#ifndef _RS485_PROTOCOL_H_
#define _RS485_PROTOCOL_H_

typedef int (*WriteCallback)  (const unsigned char what);    // send a unsigned char to serial port
typedef int  (*AvailableCallback)  ();    // return number of bytes available
typedef int  (*ReadCallback)  ();    // read a unsigned char from serial port

int sendMsg (WriteCallback fSend,
              const unsigned char * data, const unsigned char length);
unsigned char recvMsg (AvailableCallback fAvailable, ReadCallback fRead,
              unsigned char * data, const unsigned char length,
              unsigned long timeout);

#endif
