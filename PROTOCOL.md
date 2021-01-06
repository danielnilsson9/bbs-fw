# Serial Protocol

The two microcontrollers communicated directly using a an async serial line.  
Baudrate is 4800.

By first glance the protocol seems to follow the same prinicples of the the display protocol which has already been reverse enginered.

Rules:
* All messages start with 0xAA
* Last byte is checksum which is the sum of all previous bytes (exculding 0xAA) truncated to 8 bits.


