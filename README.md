




## STC15W4K32S4
Running at 4.3V


#### PAS1
White  
5k external pullup resistor  
3k series resistor  
Connected to P4.5 on MCU

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
Yellow  
680ohm series resistor  
0.6V diod paralell connected to 5V??  
Connected via shotkey diod array to P0.0 (RxD3)  

Default pin mapping of UART1 on MCU

#### RX
Green  
Connected to P0.1 (TxD3) (unable to trace from header)

Default pin mapping of UART1 on MCU


## NEC 79F9211
Running at 5V

Common chinese MCU for BLDC control.
Pinout seems to match standard schematic, see datasheets/China-BLDC-motor-controller-36V-250W.pdf

P150 is used for throttle input (i.e. power).
This is pin is directly connected to STC MCU via series resistor and filter capacitor.
Most likely is PWM signal from STC used to control motor power.
All other logic is in STC MCU firmware.

Unclear if there is a serial connection between STC and NEC MCU.
There seems to be a serial protocol defined for the NEC ebike controller for setting parameters.
More investigation needed.

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

#### TOOL0
Breakout to pad on PCB bottom side  
Programming tool connection?

#### TOOL1
Breakout to pad on PCB bottom side  
Programming tool connection?

