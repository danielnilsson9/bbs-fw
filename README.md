# BBSHD Open Source Firmware

**NOT READY FOR EVERYDAY USE**


Work in progress for bringing life to an open source firmware for the BBSHD motor controller.

Soon to be released. Check wiki for more documentation.  
https://github.com/danielnilsson9/bbshd-fw/wiki

Be careful if you build from source and flash this firmware, it has not yet been  
tested with the motor mounted on a bike.

**The firmware can be flashed using a standard bafang programming cable (TTL level usb to serial converter).**

## Main Benefits
* More power without hardware modifications! (max 32A).
* No upper voltage limit in software, can by default run up to 63V (maximum rating of components).
* No issue with PAS/Throttle override.
* Optional separate set of street legal & offroad assist levels.
* Possible to set road speed limit per assist level.
* Support cruise assist levels (i.e. motor power without pedal or throttle input).
* Use of speed sensor is optional.
* Use of display is optional.


![Config Tool](https://raw.githubusercontent.com/wiki/danielnilsson9/bbshd-fw/img/config_tool/config_tool3.png)

For a complete overview of what can be configured, read the configuration tool documentation:  
https://github.com/danielnilsson9/bbshd-fw/wiki/Configuration-Tool

## Limitations
* No battery SOC calculation, set display to show battery voltage instead.
* No range estimation, the range field on the display will show motor temperature instead.

## Supported Hardware Revisions

Revision | MCU          | Released    | Comment
-------- | ------------ | ----------- | --------------------
V1.4     | STC15W4K56S4 | ~2017       | V1.3 printed on PCB, sticker with 1.4.
V1.5     | IAP15W4K61S4 | ~2019       | V1.4 printed on PCB, sticker with 1.5.

## Legal
* Installing this firmware will void your warranty.
* I cannot be held responsible for any injuries caused by the use of this firmware, use at your own risk.
