# Serial Protocol

The two microcontrollers communicates directly using an async serial line.  
Baudrate is 4800.

At first glance the protocol seems to follow similar prinicples of the the display protocol which has already been reverse enginered.

Rules:
* All messages start with 0xAA
* Last byte is checksum which is the sum of all previous bytes (exculding 0xAA) truncated to 8 bits.

STC MCU issues requests and NEC MCU responds, nothing is sent by NEC without a request.


## Initialization

STC MCU continously resends first initialization command until NEC reponds which takes a few tries.
During initialization the requests are echoed as response.

An intreasting note:  
When reconfiguring with Bafang Config Tool updated parameters are not immediately sent to
motor control MCU. Power cycle occur before new parameters are sent to secondary MCU.

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
60 02 56 | 60 02 56 | Set low voltage cutoff limit 41V. See status request 42.
61 CF    | 61 CF    | Set current limit 30A. See status request 41.
64 1C    | 64 1C    | Set parameter? Value 0x1C
63 0A    | 63 0A    | This is throttle value, see below, no clue why 0x0A is sent at initialization.


## Command

Note: In table below leading message header "AA" and trailing checksum has been left out.

Request  | Response | Interpretation
-------- | -------- | --------------
63 XX    | 63 XX    | Throttle value, sent directly by STC MCU on change.


## Status
Status requests are sent frequently by the STC MCU.
There seems to be 3 types of request.

Note: In table below leading message header "AA" and trailing checksum has been left out.

Request  | Response | Interpretation
-------- | -------- | --------------
40       | 40 XX XX | Status/error flags. 0x2000 = Hall sensor error, 0x0800 = Low voltage cutoff active
41       | 41 XX    | ADC Current (6.9 steps per A)
42       | 42 XX XX | ADC Battery voltage (~14 steps per volt) (unexpected, only found voltage measuremt circuit connected to STC MCU, must be on both, whyy?)

