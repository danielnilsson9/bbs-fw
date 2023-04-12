# BBSHD/BBS02/TSDZ2 Open Source Firmware

This firmware is intended to replace the original Bafang firmware on the BBSHD/BBS02 motor controller. Almost all functionality of original firmware has been implemented and additional features have been added.

This firmware is compatible with all displays that works with the original Bafang firmware. A custom configuration tool is provided since BafangConfigTool is not compatible due to a different set of supported parameters.

The firmware is also compatible with the TongSheng TSDZ2 controller but requires a custom made cable in order to interface with Bafang compatible displays.

**Download**  
https://github.com/danielnilsson9/bbshd-fw/releases

**Install**  
https://github.com/danielnilsson9/bbshd-fw/wiki/Flashing-the-Firmware

**Configure**  
https://github.com/danielnilsson9/bbshd-fw/wiki/Configuration-Tool


If you find this project useful, consider sending a small [donation](https://www.paypal.com/donate/?business=LVAYFCMQYN8F4&no_recurring=0&item_name=BBSHD-FW&currency_code=USD) to fund further development.


## Highlights
* A bit more power without hardware modifications! (max 33A).
* No upper voltage limit in software, can by default run up to 63V (maximum rating of components).
* Support lower voltage cutoff for use with e.g. 36V battery.
* Smooth Throttle/PAS override.
* Optional separate set of street legal & offroad assist levels which can be toggled by a key combination.
* Support setting road speed limit per assist level.
* Support setting cadence limit per assist level.
* Support cruise assist levels (i.e. motor power without pedal or throttle input).
* Thermal limiting gradual ramp down.
* Low voltage gradual ramp down.
* Voltage calibration for accurate LVC and low voltage ramp down.
* Display motor/controller temperature on standard display.
* Use of speed sensor is optional.

![Config Tool](https://raw.githubusercontent.com/wiki/danielnilsson9/bbshd-fw/img/config_tool/config_tool3.png)

## Known Issues
* Compatibility issues reported with some older revisions of the BBS02 controller.


## Supported Hardware

### BBSHD

Revision | MCU          | Released    | Comment
-------- | ------------ | ----------- | --------------------
V1.4     | STC15W4K56S4 | ~2017       | V1.3 printed on PCB, sticker with 1.4.
V1.5     | IAP15W4K61S4 | ~2019       | V1.4 printed on PCB, sticker with 1.5.

### BBS02B
There are compatibility issues reported, this firmware is suspected to be incompatible with older BBS02 controllers.
If you have a newer BBS02B you are probably fine, if you have an older controller it might not be a good idéa to flash this firmware.

Revision | MCU          | Released    | Comment
-------- | ------------ | ----------- | --------------------
V1.?     | STC15F2K60S2 |             | Supported from BBS-FW version 1.1
V1.?     | IAP15F2K61S2 |             | Supported from BBS-FW version 1.1

BBS02A - No idea, not tested, not recommended to try unless you have an already bricked controller.

### TSDZ2
Compatible with TSDZ2A/B using the STM microcontroller (which is nearly all off them).

## Legal
* Installing this firmware will void your warranty.
* I cannot be held responsible for any injuries caused by the use of this firmware, use at your own risk.
