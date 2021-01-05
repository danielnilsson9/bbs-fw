


## Hardware Revisions

Revision | MCU 
-------- | ------------
V1.3     | STC15W4K32S4
V1.4     | ???
V1.5     | IAP15W4K61S4






## Pins

Function               | Type       | STC15W4K32S4 | D79F9211 | Comment
---------------------- | ---------- | ------------ | -------- | -------
BRAKE (blue)           | External   | P2.4         | P124     | 










## STC15W4K32S4
Running at 4.3V


#### PAS1
White  
5k external pullup resistor  
3k series resistor  
Connected to P4.5 on STC MCU

#### PAS2
Grey   
5k external pullup resistor  
3k series resistor  
Connected to P4.6 on MCU

#### BRAKE 
White
5V active low  
Conencted to to P2.4 on MCU via 0.6V diode for voltage drop?

#### THROTTLE
Blue  
3k series resistor  
9k external pulldown on pcb  
Connected to P1.3(ADC3) on MCU

#### TX
Green  
Complex net involving diod, possibly multiple transistors before reaching MCU.
Connected to P3.1 (TxD) on STC MCU

#### RX
Green  
Connected to P3.0 (RxD) vis series diod and 330ohm resistor.
External 10k pullup resistor to 5V 

Default pin mapping of UART1 on MCU


## NEC 79F9211
Running at 5V

Common chinese MCU for BLDC control.
Pinout seems to match standard schematic, see datasheets/China-BLDC-motor-controller-36V-250W.pdf

P150 is used for throttle input (i.e. power).
This is pin is directly connected to STC MCU via series resistor and filter capacitor.
Most likely is PWM signal from STC used to control motor power.
All other logic is in STC MCU firmware.

There seems to be a direct UART connection between STC and NEC MCU.  
UART2 is used on STC and is directly connected to UART0 on NEC MCU.

There seems to be a serial protocol defined for the NEC ebike controller for setting parameters.
More investigation needed into this is needed.

It would seem likely that parameters are transfered when Bafang Config Tool is used to upload parameters to STC MCU.  
At least max current would be needed by NEC MCU since motor control and current sense is implemented there.

#### HALL U
White  
3.3k series resistor  
Connected to P120 on NEC MCU  
Connected to P5.0 on STC MCU

#### HALL V
Blue  
3.3k series resistor  
Connected to P122 on NEC MCU  
Connected to P3.4 on STC MCU

#### HALL W
Grey
3.3k series resistor  
Connected to P121 on NEC MCU  
Connected to P0.6 on STC MCU

#### Throttle
Only a connection between STC and NEC MCU.  
Not connected to actual throttle signal in any way.  

Connected to P150 on NEC MCU  
Connected to P2.0 on STC MCU which is (RSTOUT_LOW) probably to force throttle low on controller reset.  
Connected to P4.3 (PWM4_2) on STC MCU (power control output signal)

#### TX
Connected to P73 (TXD) on NEC MCU  
Connected to P1.0 (RxD2) on STC MCU

#### RX
Connected to P74 on NEC MCU
Connected to P1.1 (TxD2) on STC MCU

#### TOOL0
Breakout to pad on PCB bottom side  
Programming tool connection?

#### TOOL1
Breakout to pad on PCB bottom side  
Programming tool connection?



## LM358
Current sensing.  
VCC 5V

#### IN1
Unused

#### OUT1
Unused

#### IN2
Connected to shunt resistors though some opamp configuration.
Cannot figure out complex input configuration, feedback gain looks to be 10k/1k.

TODO: Measure

#### OUT2
Current sense but unexpected input pin??
Connected to P25 on NEC MCU.
