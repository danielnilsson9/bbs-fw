




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

#### CLKO
External clock output connected to external clock input on NEC MCU.  
TODO: measure frequency



## NEC 79F9211
Running at 5V

#### P120
Connected to P5.0 (RxD3_2) on STC
TODO: function unknown, not a TX pin on NEC









