# Serial Protocol

The two microcontrollers communicates directly using an async serial line.  
Baudrate is 4800.

The protocol follows similar prinicples of the the display protocol which has earlier been reverse engineered.

Rules:
* All messages start with 0xAA
* Last byte is checksum which is the sum of all previous bytes (exculding 0xAA) truncated to 8 bits.

STC MCU issues requests and NEC MCU responds, nothing is sent by NEC without a request.


## Initialization

STC MCU continously resends first initialization command until NEC reponds which takes a few tries.
During initialization the requests are echoed as response.

An intreasting note:  
When reconfiguring with Bafang Config Tool updated parameters are not immediately sent to
motor control MCU. Power cycle must occur before new parameters are sent to secondary MCU.

Note: In table below the leading message header "AA" and trailing checksum has been left out.

Request  | Response | Interpretation
-------- | -------- | --------------
67 00    | 67 00    | Probably just a "hello" message,
68 5A    | 68 5A    | Set parameter? Value 0x5A
69 11    | 69 11    | Set parameter? Value 0x11
6A 78    | 6A 78    | Set parameter? Value 0x78
6B 64    | 6B 64    | Set parameter? Value 0x64
6C 50    | 6C 50    | Set parameter? Value 0x50
6D 46    | 6D 46    | Set parameter? Value 0x46
6E 0C    | 6E 0C    | Set parameter? Value 0x0C
60 02 56 | 60 02 56 | Set low voltage cutoff limit 41V. See status request 42.
61 CF    | 61 CF    | Set battery max current limit 30A. See status request 41.

The parameters which have not been identified above are hardcoded in the standard firmware to these values.
It could potentially be motor parameters for FOC or current ramp up/down etc. I have no clue (anyone here peeking who have the info, feel free to contact me).

## Commands

Note: In table below the leading message header "AA" and trailing checksum has been left out.

Request  | Response | Interpretation
-------- | -------- | --------------
63 XX    | 63 XX    | Target Speed (0 - 255)
64 XX    | 64 XX    | Target Current % (0 - 100), percent of max current


## Status
Status requests are sent frequently by the STC MCU.
There seems to be 3 types of request.

Note: In table below the leading message header "AA" and trailing checksum has been left out.

Request  | Response | Interpretation
-------- | -------- | --------------
40       | 40 XX XX | Status/error flags.
41       | 41 XX    | ADC Battery Current (6.9 steps per amp)
42       | 42 XX XX | ADC Battery voltage (~14 steps per volt) (unexpected, only found voltage measuremt circuit connected to STC MCU, but must be on both)


### Status Flags
uint16

#### Byte 1

B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0
-- | ---| -- | -- | -- | -- | -- | --
-  |-   |-   |-   |-   |-   |-   |-

#### Byte 2

B7 | B6 | B5                | B4 | B3                 | B2                     | B1 | B0
---|----|-------------------|----|--------------------|------------------------|----|----------------------
-  |-   | Hall Sensor Fault |-   | Low Voltage Cutoff | Motor Ctrl Disabled    |-   | Motor Ctrl Disabled

