# **Maypole: Your All-in-One Smart Tool for Seamless File Management, Online Connectivity, and Remote Software Updates.**

## **Maypole Setup Guide: Getting Familiar with its Applications**

Welcome to the world of Maypole, a versatile device designed to simplify various tasks including file management and connectivity. This guide will walk you through the process of setting up Maypole by obtaining its source code from GitHub, either through cloning the repository or downloading it as a ZIP archive.

## **Table of contens**

1.  **Overview of the Maypole's Capabilities**

2.  **Setup Guide**

3.  **Connecting to the Maypole Device**

4.  **Modes of Operation**

5.  **Connecting Maypole to a New Wi-Fi Network**

6.  **Performing Firmware Update via User Interface**

## **1. Overview of the Maypole's Capabilities: What is Maypole?**

The Maypole device is like a smart tool made using a special computer chip called ESP32. It can do different things, kind of like a multi-purpose tool. You can use it to easily work with your files, connect it to the internet through WiFi, and even update its software from far away. This device is made to make tasks like storing and managing files simpler and more convenient for you.

## **2. Setup Guide**

### **Accessing the GitHub Repository**

- Open your preferred web browser and navigate to the Maypole GitHub repository: <https://github.com/akshar001/Maypole_firmware>

### **Cloning the Repository**

1. Open your terminal or command prompt.

2. Clone the Maypole repository from GitHub by executing the following command:

     ```git clone https://github.com/akshar001/Maypole_firmware.git```

### **Downloading as ZIP**

1. Visit the Maypole repository on GitHub at the URL: <https://github.com/akshar001/Maypole_firmware>

2. Click on the green "Code" button on the top right of the repository page.

3. Select "Download ZIP" from the dropdown menu.

4. Save the ZIP file to your desired location on your computer.

### **Installing Required Libraries**

To start with Maypole you need some arduino libraries.

1. **SimpleTimer Lib:** <https://github.com/jfturcot/SimpleTimer>

2. **ArduinoJson Lib:** <https://github.com/bblanchon/ArduinoJson>

3. **Update Lib:** <https://github.com/espressif/arduino-esp32/tree/master/libraries/Update/src>

### **Running the Project**

- Launch the Arduino IDE and open the Maypole project that you either cloned from the GitHub repository or downloaded as a ZIP file. This project contains the necessary code and configurations to operate the Maypole device. Once you have the project open in the Arduino IDE, you can proceed to upload the firmware directly to the Maypole device.

- Additionally, don't forget to upload the SPIFFS (SPI Flash File System) data. Once you've successfully uploaded the firmware, you'll need to configure and upload the SPIFFS data using the Arduino for ESP32 plugin. This process allows you to upload files and data that the Maypole device will utilize.

- For detailed instructions on how to set up SPIFFS upload using the Arduino IDE, you can follow the guide provided at: <https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/>

- You'll find the necessary data folder included in the Maypole project repository. This folder contains essential files that the Maypole device requires for its functionalities. By uploading this data using the SPIFFS uploader, you'll ensure that the device has access to the required resources for seamless operation.

## **3. Connecting to the Maypole Device**

- After completing the uploading process, here's a helpful tip for effectively operating the Maypole device:

- For a comprehensive visual guide on operating the Maypole device and making the most of its features, we recommend watching this informative video: <https://youtu.be/VvWGthyjWB8>. This video provides step-by-step instructions and demonstrations that will enhance your understanding of how to interact with the Maypole device and utilize its functionalities to the fullest.

- ### **Steps to follow**

  To establish a connection with the Maypole device using your smartphone or computer, follow these straightforward steps:

1. ### **Network Connection**

      - On your smart device (phone or PC), access the Wi-Fi settings. Locate and select the network named ```"Machine_%02X:%02X:%02X"```. Connect to this network and enter the password ```'12345678'```.

2. ### **Web Browser Access**

      - Open a web browser on your device.

3. ### **Entering IP Address**

      - In the address bar of your web browser, type: ```192.168.4.1```.

4. ### **Interaction with Maypole**

      - Press Enter or Go in your browser. Your browser will establish a connection with the Maypole device. Initially, the device will be in USB mode by default. You'll see options such as ```Switch to SD mode```, ```My Files```, ```Connect to WiFi```, and ```Update Firmware```.

## 4. **Maypole Device Mode Operations**

- The Maypole device offers two distinct modes of operation: USB Mode and SD Card Mode. Each mode serves specific purposes and provides unique ways to manage and interact with files. Let's explore these modes in detail:

 1. **USB Mode:**
    - **Description:** In USB Mode, the Maypole device emulates a USB storage device that you would typically connect to a computer.

    - **Usage:** To utilize this mode, connect the Maypole device to your computer using a USB cable.

    - **Functionality:** Once connected, your computer recognizes the Maypole device as a standard storage unit, similar to a USB flash drive. This allows you to seamlessly transfer files to and from the device using your computer's file explorer.

    - **Benefit:** USB Mode simplifies file management by enabling direct drag-and-drop actions without the need for additional software.

 2. **SD-card Mode:**

    - **Description:** SD Card Mode empowers the Maypole device to interact with an external SD card.

    - **Purpose:** This mode is particularly handy when managing files on an SD card without requiring a direct computer connection.

    - **Activation:** To switch to SD Card Mode, access the Maypole device's user interface via a web browser.

    - **Transition:** Within the interface, locate an option or button to activate SD Card Mode.

    - **Functionality:** Once enabled, the Maypole device takes over the management of files on the connected SD card. You can perform tasks such as uploading, deleting, or editing files through the interface.

    - **Advantage:** SD Card Mode offers remote file management capabilities, making it convenient for tasks that don't require direct computer involvement.

- In SD Card Mode, the Maypole device facilitates seamless file management with the following operations:
     1. **File Upload:**
        - Easily transfer files from your computer to the Maypole device.
        - Utilize a user-friendly interface for selecting and uploading files.
        - Uploaded files are securely stored on the device's storage.
     2. **File Download:**
        - Request and download files stored on the Maypole device.
        - Access files remotely without direct device interaction.
        - Retrieve files conveniently through the device's interface.
     3. **File Deletion:**
        - Request deletion of specific files from the device's storage.
        - Remove unnecessary files to maintain an organized storage space.
        - Streamline file management within the Maypole device.

- These file operations empower users to efficiently manage their files in SD Card Mode, enhancing accessibility, organization, and control over stored content.

## **5. Connecting Maypole to a New Wi-Fi Network**

- ### **Entering Network Details**
  - Within the user interface, locate the ```"Connect to Wi-Fi"``` section. Here, you'll discover fields to input the SSID (network name) and password of the new Wi-Fi network.

- ### **Submission**
  - Provide accurate SSID and password for the desired Wi-Fi network. After entering the required information, click the ```"Submit"``` button.

- ### **New IP Address***
    - The Maypole device will use the provided SSID and password to establish a connection with the chosen Wi-Fi network. Upon successful connection, the Maypole will receive a new IP address on the selected network. This new IP address enables remote access and management of the Maypole device from any device connected to the same Wi-Fi network. Use the provided IP address to access the Maypole's user interface via a browser, facilitating convenient control and interaction.

- By following these steps, you can effortlessly connect the Maypole device to a new Wi-Fi network, enhancing its accessibility and allowing remote control and management.

## **6. Performing Firmware Update via User Interface (OTA)**

- The Maypole device offers an Over-The-Air (OTA) update mechanism, allowing users to upload new firmware wirelessly after making code changes and compiling in the Arduino IDE. This feature eliminates the need for physical connections and simplifies the process of updating the device's firmware.

- Here's how OTA works with the Maypole device:
    - **Initiating the OTA Update:** Access the Maypole device's user interface through your browser, typically by entering its IP address. Look for an option related to "Firmware Update" or "Update Firmware."

    - **Selecting the New Firmware:** Once in the update section, you'll be prompted to select the updated firmware file that you've compiled in the Arduino IDE. Ensure you have the new firmware file accessible on your computer or device.

    - **Initiating the Update:** Trigger the OTA update process by clicking the appropriate button or link provided in the interface The Maypole device will receive the new firmware wirelessly from your connected device.

    - **Monitoring the Progress:** As the update progresses, you'll likely see indicators or progress bars showing the advancement of the update.

    - **Automatic Restart:** After successfully receiving and installing the new firmware, the Maypole device will automatically restart to apply the update.

- The OTA update capability in the Maypole device streamlines the firmware update process, enabling users to make changes, compile, and wirelessly update the device's firmware without the need for physical intervention. This feature contributes to the device's ease of use and flexibility in adapting to new functionalities or improvements.
