# Maypole Example Code for working with SD CARD

This is a simple example code for working with Maypole.

We have tested the Maypole with latest esp32 Arduino Firmware V2.2.0.

You need to copy paste two libraries from Lib folder into your Arduino lib folder.

Select the ESP32 Dev Module as a board!

You also need to install SimplTimer lib,
https://github.com/jfturcot/SimpleTimer

After it just open the sketch and upload the firmware directly to maypole.



You also need to upload spiffs data, you need to setup a spiffs upload using arduino for esp32 plugin 

https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/

The data folder is already provided just upload it!

After uploading everythin here is a tips for operating a maypole!


Please visit a video https://youtu.be/VvWGthyjWB8


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
