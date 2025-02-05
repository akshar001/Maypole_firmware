# **Maypole: Your All-in-One Smart Tool for Seamless File Management, Online Connectivity, and Remote Software Updates**

## **Getting Started with Maypole**

Welcome to the Maypole device, a versatile tool designed for easy file management, online connectivity, and remote firmware updates. This guide will help you understand its features and how to use them effectively.
We also recommend watching this informative video: <https://youtu.be/VvWGthyjWB8>.

---

## **Overview of Maypole's Capabilities**

The Maypole device is powered by an ESP32 microcontroller and serves as a smart file management system. With Maypole, you can:

- Store and organize files efficiently.
- Connect to the internet via Wi-Fi.
- Perform remote software updates without physical access to the device.
- Manage files via a web-based user interface.

---

## **1. Powering Up Maypole via USB Connection or turn-on it's switch**  
When you connect Maypole to your laptop or computer via USB, it functions as a USB flash drive. This allows you to easily transfer files using your computer's file explorer.

---

> **Note:** Follow the steps below if you want to manage files on Maypole from other devices, such as a smartphone or iPod.

---

## **2. Connecting to the Maypole Device**  

### **Step-by-Step Guide**  

#### 1. Connect to Maypole's Wi-Fi  
- Open Wi-Fi settings on your smartphone or computer.  
- Look for a network named `Machine_%02X:%02X:%02X`.(Every maypole has different digits after the word "Machine".)  
- Use the default password: `12345678` to connect.  
> **Note:** Do keep your mobile data off.

#### 2. Access the Web Interface  
- Open a web browser and enter `192.168.4.1` in the address bar.  
- Press **Enter** or **Go** to open the Maypole control panel.  

#### 3. Exploring the Interface  
Once connected, you’ll find home page which has the following options:  
- **Switch to SD Mode**  
- **My Files**  
- **Connect to Wi-Fi** – Use this option if you want to upload files from their other device which are connected to other network
- **Update Firmware**  

---

## **3. Maypole Device Modes**

Maypole operates in two primary modes:

### **1. USB Mode**
- In this mode, the Maypole device functions like a USB flash drive.(Initally by default it will be in usb mode.)
- Connect the device to a computer via a USB cable to transfer files easily.
- You can upload, download, or delete files.

### **2. SD Card Mode**
- This mode enables file management directly on an SD card.
- Access the `Switch to SD mode` option in the web interface to activate it.
- You can upload, download, or delete files stored on the SD card.
  
> **Note:** Folder management is not supported; only individual files can be handled.

---

## **4. Connecting Maypole to a Wi-Fi Network(If one wants to upload files from their other device which are connected to other network)**

To connect Maypole to a different Wi-Fi network:

1. **Open the Maypole Web Interface**
   - Go to the web browser and enter `192.168.4.1`.

2. **Enter Network Credentials**
   - Locate the `Connect to Wi-Fi` section.
   - Enter the SSID (network name) and password of the target Wi-Fi network.
   - Click `Submit` to confirm.
   - The page will refresh.

3. **Use the New IP Address**
   - Once connected, Maypole will display a new IP address at the bottom of the homepage(To go to the home page, click the home icon in the right corner of the page.).
   - Use this IP to access Maypole from any device on the same network.

---

## **5. Performing a Firmware Update (OTA)**

Maypole allows remote firmware updates using Over-The-Air (OTA) functionality. Here’s how:

1. **Access the Firmware Update Option**
   - Open the Maypole web interface and navigate to `Update Firmware`.

2. **Upload the New Firmware**
   - Select the firmware update file from your computer.
   - Click `Upload` to start the update process.

3. **Automatic Restart**
   - After updating, the device will restart automatically to apply changes.

---

## **Conclusion**

Maypole is a powerful yet user-friendly tool that simplifies file management, remote access, and software updates. By following this guide, you can maximize its capabilities and ensure seamless operation. Enjoy your experience with Maypole!
