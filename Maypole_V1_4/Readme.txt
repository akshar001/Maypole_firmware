This is the 4th version of Maypole.

In this we update maypole with functionality such as:
    1. It can now work simultaneously in both mode STA/AP.
    2. You can now add your wifi to it and it can also reconnect on boot.
    3. You can now develop the filesystem in SD card.
    4. We added the OTA functionality to update firmware and filesystem.
    5. It will now by default boot in USB mode.
    6. You can now download file from "DL", delete file from "DEL".
    7. For uploading the file choose file in form seen in middle of My Files page and click on upload to upload it.

For accessing the Folder/Files of you SD card on webserver you need to change to SD Card mode first.
To connect to your wifi go to Connect to wifi and add your "Username" in "ssid" and "Password" in "password" field. It will connect to your network and save it for future reconnection.
For updating Firmware/Filesystem go to Update firmware and and choose bin File. After update you need to recconect to ESP module and go Homepage.

For accessing the Webpage go to URL: 192.168.4.1 or http://maypole.local

For Maypole as AP.
UserName : pen_drive
Password : 12345678

Addresses for directly flashing bin files:

boot_app0.bin                           :  0xe000 
Maypole_V1_4.ino.bin                    :  0x10000 
Maypole_V1_4.ino.bootloader.bin         :  0x1000
Maypole_V1_4.ino.partitions.bin         :  0x8000
Maypole_V1_4.spiffs.bin                 :  0x290000

Set Baud: 921600
