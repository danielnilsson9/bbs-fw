# BBSHD Open Source Firmware

This firmware is intended to replace the original Bafang firmware on the BBSHD motor controller. Almost all functionality of original firmware has been implemented and more special features has been added.

This firmware is compatible with all displays that works with the original firmware. A custom configuration tool is provided since BafangConfigTool is not compatible due to a different set of supported parameters.

**Download**  
https://github.com/danielnilsson9/bbshd-fw/releases

**Install**  
https://github.com/danielnilsson9/bbshd-fw/wiki/Flashing-the-Firmware

**Configure**  
https://github.com/danielnilsson9/bbshd-fw/wiki/Configuration-Tool


If you find this project useful, consider sending a small [donation](https://www.paypal.com/donate/?business=LVAYFCMQYN8F4&no_recurring=0&item_name=BBSHD-FW&currency_code=USD) to fund further development.


## Highlights
* A bit more power without hardware modifications! (max 32A).
* No upper voltage limit in software, can by default run up to 63V (maximum rating of components).
* Support lower voltage cutoff for use with e.g. 36V battery.
* Smooth Throttle/PAS override.
* Optional separate set of street legal & offroad assist levels which can be toggled by a key combination.
* Support setting road speed limit per assist level.
* Support setting cadence limit per assist level.
* Support cruise assist levels (i.e. motor power without pedal or throttle input).
* Display motor/controller temperature (shown in range field on display).
* Use of speed sensor is optional.
* Use of display is optional.

![Config Tool](https://raw.githubusercontent.com/wiki/danielnilsson9/bbshd-fw/img/config_tool/config_tool3.png)

## Limitations
* No battery SOC calculation implemented yet, set display to show battery voltage instead.
* No range estimation, the range field on the display will show motor temperature instead.


## Supported Hardware

### BBSHD

Revision | MCU          | Released    | Comment
-------- | ------------ | ----------- | --------------------
V1.4     | STC15W4K56S4 | ~2017       | V1.3 printed on PCB, sticker with 1.4.
V1.5     | IAP15W4K61S4 | ~2019       | V1.4 printed on PCB, sticker with 1.5.

### BBS02

Not yet. Consider sending a [donation](https://www.paypal.com/donate/?business=LVAYFCMQYN8F4&no_recurring=0&item_name=BBSHD-FW&currency_code=USD) that will go towards obtaining a BBS02 controller for reverse engineering.

I have currently received 74 USD and will order a controller and start working on this if we reach 100.

### BBS01

No.

## Legal
* Installing this firmware will void your warranty.
* I cannot be held responsible for any injuries caused by the use of this firmware, use at your own risk.
