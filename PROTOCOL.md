# Serial Protocol

The two microcontrollers communicates directly using a an async serial line.  
Baudrate is 4800.

At first glance the protocol seems to follow the same prinicples of the the display protocol which has already been reverse enginered.

Rules:
* All messages start with 0xAA
* Last byte is checksum which is the sum of all previous bytes (exculding 0xAA) truncated to 8 bits.

STC MCU issues requests and NEC MCU responds, nothing is sent by NEC without a request.


## Initialization

STC MCU continously resends first initialization command until NEC reponds which takes a few tries.
During initialization the requests are echoed as response.

Note: In table below leading message header "AA" and trailing checksum has been left out.

Request  | Response | Interpretation
-------- | -------- | --------------
67 00    | 67 00    | Probably just an "hello" message
68 5A    | 68 5A    | Set parameter? Value 0x5A
69 11    | 69 11    | Set parameter? Value 0x11
6A 78    | 6A 78    | Set parameter? Value 0x78
6B 64    | 6B 64    | Set parameter? Value 0x64
6C 50    | 6C 50    | Set parameter? Value 0x50
6D 46    | 6D 46    | Set parameter? Value 0x46
6E 0C    | 6E 0C    | Set parameter? Value 0x0C
60 02 56 | 60 02 56 | Set parameter? Value 0x0256
61 CF    | 61 CF    | Set parameter? Value 0xCF
64 1C    | 64 1C    | Set parameter? Value 0x1C


## Command

Note: In table below leading message header "AA" and trailing checksum has been left out.

Request  | Response | Interpretation
-------- | -------- | --------------
63 1E    | 63 1E    | Throttle value


## Status
Status requests are sent frequently by the STC MCU.
There seems to be 3 types of request.

I guess that two of them would be:
* Measured current
* Error code (basing this on the fact that there exist an error code for shunt resistor error which must be detected by NEC MCU)

Note: In table below leading message header "AA" and trailing checksum has been left out.

Request  | Response | Interpretation
-------- | -------- | --------------
40       | 40 00 00 | Amp reading or power
41       | 41 00    | Error condition???
42       | 42 03 11 | ???

