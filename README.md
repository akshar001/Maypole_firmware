# USB - T Example Code for working with SD CARD

This is a simple example code for working with USB - T.

You can connect to 'pen_drive' and password '12345678',

Type

192.168.4.1 in browser,
And you can upload, delete and edit files.

You just need to change a mode.
When you want to upload into SD card change USB mode and it will work!

In example you will find two functions,

# change_to_usb_mode()

It will change to USB mode, So PC/System which is connected to USB-T can access SD card reader DATA directly.

# change_to_sd_mode()

It wil change to SD card Mode, which is when you want to upload something to SD card via ESP32 you will enable this mode,
So ESP32 can find a SD card and write up something.


How to Compile?

The code have used Arduino for ESP32.

So you can follow this link to setup,

ESP32 with arduino

-----   https://github.com/espressif/arduino-esp32

Add it as a ZIP in add libraries,

Upload Example,
Sketch.

BOOM!
