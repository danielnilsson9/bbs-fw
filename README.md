# BBSHD Open Source Firmware

Work in progress for bringing life to an open source firmware for the BBSHD motor controller.

Soon to be released. Check wiki for more documentation.  
https://github.com/danielnilsson9/bbshd-fw/wiki

Be careful if you build from source and flash this firmware, it has not yet been  
tested with the motor mounted on a bike.


## Main Benefits
* More power (max 32A)
* No high voltage limit, can by default run up to 63V (maximum rating of components)
* No issue with PAS/Throttle ovveride
* Optional separate set of street legal & offroad assist levels.
* Possible to set road speed limit per assist level
* Support cruise assist level (i.e. motor power without pedal or throttle input)
* Optional use of speed sensor
* Optional use of display

For a complete overview of what can be configured, read the configuration tool documentation:  
https://github.com/danielnilsson9/bbshd-fw/wiki/Configuration-Tool


## Hardware Revisions

Revision | MCU          | Released    | Comment
-------- | ------------ | ----------- | --------------------
V1.4     | STC15W4K56S4 | ~2017       | V1.3 printed on PCB
V1.5     | IAP15W4K61S4 | ~2019       | V1.4 printed on PCB

Nothing noticeable changed on PCB:s, pin compataible MCU.  
Different firmware required due to small change in eeprom layout.
