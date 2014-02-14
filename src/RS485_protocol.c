/*
 RS485 protocol library.

 Devised and written by Nick Gammon.
 Date: 14 November 2011
 Version: 1.1

 Version 1.1 reset the timeout period after getting STX.

 Licence: Released for public use.


 Can send from 1 to 255 bytes from one node to another with:

 * Packet start indicator (STX)
 * Each data byte is doubled and inverted to check validity
 * Packet end indicator (ETX)
 * Packet CRC (checksum)


 To allow flexibility with hardware (eg. Serial, SoftwareSerial, I2C)
 you provide three "callback" functions which send or receive data. Examples are:

 void fWrite (const byte what)
 {
 Serial.write (what);
 }

 int fAvailable ()
 {
 return Serial.available ();
 }

 int fRead ()
 {
 return Serial.read ();
 }

 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <debugPrint.h>
#include <millis.h>
#include <RS485_protocol.h>

#define TRUE 1
#define FALSE 0
#define int int

#define STX '\2'
#define ETX '\3'

// calculate 8-bit CRC
static unsigned char crc8 (const unsigned char *addr, unsigned char len)
{
  unsigned char crc = 0;
  while (len--)
    {
    unsigned char inbyte = *addr++;
    for (unsigned char i = 8; i; i--)
      {
      unsigned char mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix)
        crc ^= 0x8C;
      inbyte >>= 1;
      }  // end of for
    }  // end of while
  return crc;
}  // end of crc8

// send a byte complemented, repeated
// only values sent would be (in hex):
//   0F, 1E, 2D, 3C, 4B, 5A, 69, 78, 87, 96, A5, B4, C3, D2, E1, F0
int sendComplemented (WriteCallback fSend, const unsigned char what)
{
unsigned char c;

  // first nibble
  c = what >> 4;
  if (fSend ((c << 4) | (c ^ 0x0F)) != 1) {
	  debugPrint("complemented 1st nibble failed\r\n");
	  return 0;
  }

  // second nibble
  c = what & 0x0F;
  if (fSend ((c << 4) | (c ^ 0x0F)) != 1) {
	  debugPrint("complemented 2nd nibble failed\r\n");
	  return 0;
  }
  return 1;
}  // end of sendComplemented

// send a message of "length" bytes (max 255) to other end
// put STX at start, ETX at end, and add CRC
int sendMsg (WriteCallback fSend, const unsigned char * data, const unsigned char length)
{
  if (fSend (STX) != 1) {  // sendSTX
	  debugPrint("send STX FAIL\r\n");
	  return 0;
  }
  for (unsigned char i = 0; i < length; i++)
    if (sendComplemented (fSend, data [i]) != 1)
    	return 0;
  if (fSend (ETX) != 1) {  // sendETX
	  debugPrint("send ETX FAIL\r\n");
	  return 0;
  }
  return sendComplemented (fSend, crc8 (data, length));
}  // end of sendMsg

// receive a message, maximum "length" bytes, timeout after "timeout" milliseconds
// if nothing received, or an error (eg. bad CRC, bad data) return 0
// otherwise, returns length of received data
unsigned char recvMsg (AvailableCallback fAvailable,   // return available count
              ReadCallback fRead,             // read one byte
              unsigned char * data,                    // buffer to receive into
              const unsigned char length,              // maximum buffer size
              unsigned long timeout)          // milliseconds before timing out
  {

  unsigned long start_time = millis ();

  int have_stx = FALSE;

  // variables below are set when we get an STX
  int have_etx = FALSE;
  unsigned char input_pos = 0;
  int first_nibble = TRUE;
  unsigned char current_byte = 0;

  while (millis () - start_time < timeout)
//  while (timeout-- > 0)
    {
    if (fAvailable () > 0)
      {
      unsigned char inByte = fRead ();

      switch (inByte)
        {

        case STX:   // start of text
          have_stx = TRUE;
          have_etx = FALSE;
          input_pos = 0;
          first_nibble = TRUE;
          start_time = millis ();  // reset timeout period
          break;

        case ETX:   // end of text
          have_etx = TRUE;
          break;

        default:
          // wait until packet officially starts
          if (!have_stx)
            break;

          // check byte is in valid form (4 bits followed by 4 bits complemented)
          if ((inByte >> 4) != ((inByte & 0x0F) ^ 0x0F) )
            return 0;  // bad character

          // convert back
          inByte >>= 4;

          // high-order nibble?
          if (first_nibble)
            {
            current_byte = inByte;
            first_nibble = FALSE;
            break;
            }  // end of first nibble

          // low-order nibble
          current_byte <<= 4;
          current_byte |= inByte;
          first_nibble = TRUE;

          // if we have the ETX this must be the CRC
          if (have_etx)
            {
            if (crc8 (data, input_pos) != current_byte)
              return 0;  // bad crc
            return input_pos;  // return received length
            }  // end if have ETX already

          // keep adding if not full
          if (input_pos < length)
            data [input_pos++] = current_byte;
          else
            return 0;  // overflow
          break;

        }  // end of switch
      }  // end of incoming data
//    _delay_ms(1);
    } // end of while not timed out

  return 0;  // timeout
} // end of recvMsg

