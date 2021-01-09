# BBSHD Open Source Firmware

Work in progress for bringing life to an open source firmware for the BBSHD motor controller.

The motor controller has two microcontrollers maing things a bit more complicated.

The main controller is from STC Micro and have available toolchains and can be flashed through the standard serial communication port exposed in the main wire harness, i.e. without taking the motor apart.

There is a secondary microcontroller "79F9211" which apparently is a common controller used in chinese ebike motor controllers using some standard firmware.
This controller has no cheap availble programmer but there exist a protocol definition on how to program it, so one could most likeley be built, see doc/78Kx3-pgm.pdf.
There exist no free compiler but a propritary one (CC78K0R) can be downloaded from Renesas website, it is not possible to install without some form of product number but the installer can be extracted to get hold of the compiler binary (in violation with TOS of course).

This controller can only be flashed by taking the controller out and depotting the backside of the PCB to expose the programming pins.

A code example of a ebike motor controller written by Renesas Electronics and implemented on the 79F9211 has been found online (see misc/79F9211-bike-dump), it would seem likely bafang has based their implementation on this since they have selected this MCU for motor control.

The goal at the moment is to only reimplement the STC microcontroller firmware and keep the 79F9211 motor controller firmware as is.

## Hardware Revisions

Revision | MCU          | Released
-------- | ------------ | -----------
V1.3     | STC15W4K56S4 | ~2017
V1.4     | IAP ???      | ???
V1.5     | IAP15W4K61S4 | ~2019


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

Default pin mapping of UART1 on MCU

#### RX
Green  
External 10k pullup resistor to 5V  
Connected to P3.0 (RxD) vis series diod and 330ohm resistor.

Default pin mapping of UART1 on MCU

#### Additional UARTs
Two more UARTs are brought out to pin headers on pcb, UART3 and UART4 on STC MCU.
Those are labeled BR/BT and R/T on pcb.

## NEC 79F9211
Running at 5V

Common chinese MCU for BLDC control.
Pinout seems to resemble a standard schematic, see datasheets/China-BLDC-motor-controller-36V-250W.pdf

P150 is used as motor enable signal, controlled by STC MCU.

There is a direct UART connection between STC and NEC MCU.  
UART2 is used on STC and is directly connected to UART1 on NEC MCU.
Reverse engineered protocol definition can be found in PROTOCOL.md

Enable motor signal is used along with the set "target current" message to control motor from STC MCU.

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

#### Enable
Direct connection between STC and NEC MCU.  

Connected to P150 on NEC MCU  
Connected to P2.0 on STC MCU which is (RSTOUT_LOW) probably to force motor disable on STC MCU reset.  
Connected to P2.1 on STC MCU

#### TX
Connected to P73 (TXD) on NEC MCU  
Connected to P1.0 (RxD2) on STC MCU

#### RX
Connected to P74 on NEC MCU  
Connected to P1.1 (TxD2) on STC MCU

#### TOOL0
Breakout to pad on PCB bottom side  
Programming tool connection

#### TOOL1
Breakout to pad on PCB bottom side  
Programming tool connection


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

#### OUT2
Connected to P25 on NEC MCU.
