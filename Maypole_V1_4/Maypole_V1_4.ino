#include <SPI.h>
#include <SimpleTimer.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <SD.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <ESPmDNS.h>

WebServer server(80);
WiFiMulti wifiMulti;
DynamicJsonDocument doc(2048);
JsonObject obj; 
SimpleTimer timer;

const char* host = "maypole";
String ssid = "";
String password =  "";
String header,body,footer;
String webpage = "";
String CurrentRoot = "";
String MainRoot;
bool SD_present; 
bool is_from_my_files = false;
bool got_user_network = false;
String Version = "v.1.4";


void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Connected to AP successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
}

JsonObject getJSonFromFile(DynamicJsonDocument *doc, String filename, bool forceCleanONJsonError = true ) 
{
    File myWifilist;
    myWifilist = SPIFFS.open(filename);
    if (myWifilist) 
    {
        DeserializationError error = deserializeJson(*doc, myWifilist);
        if (error) {
            // if the file didn't open, print an error:
            Serial.print(F("Error parsing JSON "));
            Serial.println(error.c_str());
 
            if (forceCleanONJsonError){
                return doc->to<JsonObject>();
            }
        }
 
        myWifilist.close();
 
        return doc->as<JsonObject>();
    } 
    else 
    {
        Serial.print(F("Error opening (or file not exists) "));
        Serial.println(filename);
        Serial.println(F("Empty json created"));
        return doc->to<JsonObject>();
    }
 
}
 
bool saveJSonToAFile(DynamicJsonDocument *doc, String filename) 
{
    Serial.println(F("Open file in write mode"));
    File myWifilist;
    myWifilist = SPIFFS.open(filename, FILE_WRITE);
    if (myWifilist) 
    {
        Serial.print(F("Filename --> "));
        Serial.println(filename);
        Serial.print(F("Start write..."));
        serializeJson(*doc, myWifilist);
        Serial.print(F("..."));
        myWifilist.close();
        Serial.println(F("done."));
        return true;
    } 
    else 
    {
        Serial.print(F("Error opening "));
        Serial.println(filename);
        return false;
    }
}

void printFile(const char *filename) 
{
    File file = SPIFFS.open(filename);
    if (!file) 
    {
        Serial.println(F("Failed to read file"));
        return;
    }
    while (file.available()) 
    {
        String payload = file.readString();
        Serial.print(payload);
    }
    Serial.println();
    file.close();
}
 

void SaveWifiCred()
{
    obj = getJSonFromFile(&doc, "/wificred.json");
    JsonArray data;
    if (!obj.containsKey(F("data"))) {
        Serial.println(F("Not find data array! Crete one!"));
        data = obj.createNestedArray(F("data"));
    } else {
        Serial.println(F("Find data array!"));
        data = obj[F("data")];
    }
 
    JsonObject objArrayData = data.createNestedObject();
 
    objArrayData["wifi_name"] = ssid;
    objArrayData["wifi_pwd"] = password;
 
    boolean isSaved = saveJSonToAFile(&doc, "/wificred.json");
 
    if (isSaved)
    {
        Serial.println("File saved!");
    }else
    {
        Serial.println("Error on save File!");
    } 
    return; 
}

void setupWifi()
{
  WiFi.mode(WIFI_AP_STA); 
  WiFi.softAP("pen_drive","12345678");
  Serial.println(WiFi.softAPIP());
  obj = getJSonFromFile(&doc, "/wificred.json");
  for(int i=0;i<obj["data"].size();i++)
  {
    String wifi_name = obj["data"][i]["wifi_name"];
    String wifi_pwd = obj["data"][i]["wifi_pwd"];
    wifiMulti.addAP(wifi_name.c_str(),wifi_pwd.c_str());
  } 
  if(wifiMulti.run() == WL_CONNECTED)
  {
    Serial.println(WiFi.localIP());
  }
  if(got_user_network)
  {
    WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.begin(ssid.c_str(),password.c_str()); 
    SaveWifiCred();  
  }

}

void change_to_usb_mode()
{
  SD.end();
  SPI.end();
  digitalWrite(4,LOW);
  SD_present = false;
  timer.setTimeout(500,[]()
  {
    digitalWrite(27,HIGH);
    digitalWrite(4,HIGH);
  //        digitalWrite(4,LOW);    // Buffer IC OFF  
  timer.setTimeout(100,[]()
  {
    digitalWrite(25,HIGH);    // Buffer IC OFF
    digitalWrite(26,LOW);
    Serial.println("CHIP DETECTED");// chip detect
  });
  });
  Serial.print(F("USB mode Initialized."));
}

void change_to_sd_mode()
{
  Serial.print(F("Initializing SD card..."));
  digitalWrite(25,LOW);
  digitalWrite(4,LOW);
  //SD_present = true;
  timer.setTimeout(100,[]()
  {
    digitalWrite(4,HIGH);
    Serial.println("SD card mode changed");
    digitalWrite(27,LOW);
    digitalWrite(26,HIGH);
    delay(20);
    digitalWrite(25,LOW);  //BUffer IC LOW 
    while(!SD.begin()){
      Serial.println("SD Card not properly created");
    }
    SD_present = true;
    Serial.println("SD card done");
  });
}

void setup(void)
{
      Serial.begin(115200);
      pinMode(25,OUTPUT);     //Select pin, Buffer LOW for ESP and HIGH for card reader 
      pinMode(26,OUTPUT);     //SDcard chip detect (active LOW)
      pinMode(4,OUTPUT);      //HIGH to Powerup microSD card (MOSFET)
      pinMode(27,OUTPUT);     //SD RESET
      Serial.print(F("Initializing USB mode as Default..."));
      SPIFFS.begin();
      MDNS.begin(host);
      if (!MDNS.begin(host)) {
        Serial.println("Error startng mDNS responder");
      }
      Serial.println("mDNS server started");
      change_to_usb_mode(); 
      setupWifi();
      //WiFi.softAP("pen_drive","12345678");
      ///////////////////////////// Server Commands 
      server.on("/",              Homepage);
      server.on("/MyFiles",       My_Files);
      server.on("/ConnectToWifi", ConnectToWifi);
      server.on("/upload" ,HTTP_POST, [](){
            is_from_my_files = true; My_Files();},handleFileUpload);
      server.on("/sel-mode",HTTP_POST, [](){ change_mode(); delay(200);
            server.sendHeader(F("Location"), F("/"));server.send(303);
            });
      server.on("/network" ,HTTP_POST, [](){ got_user_network = true;
            ssid = server.arg(0); password = server.arg(1); ConnectToWifi();});

      server.on("/FirmwareUpdate", HTTP_GET, []() {
        File file = SPIFFS.open("/Firmware_update.html");
        String page = file.readString();
        file.close();
        page.replace(F("<% version %>"),Version);
        int pagesize = page.length();
        server.sendHeader("Connection", "close");
        server.setContentLength(pagesize);
        server.send(200, "text/html", page);
      });
      server.on("/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
        }, []() { Update_Firmware(); });
         
      server.on("/back.png",[](){
        File file_img = SPIFFS.open("/back.png");
        server.streamFile(file_img,"image/png");
        file_img.close();
      });
      server.on("/home.png",[](){
        File file_img = SPIFFS.open("/home.png");
        server.streamFile(file_img,"image/png");
        file_img.close();
      });
      server.on("/SDCard.png",[](){
        File file_img = SPIFFS.open("/SDCard.png");
        server.streamFile(file_img,"image/png");
        file_img.close();
      });
      ///////////////////////////// End of Request commands
      server.begin();
      MDNS.addService("_http", "_tcp", 80);
      MDNS.addServiceTxt("_http", "_tcp", "board", "ESP32");
      Serial.println("HTTP server started");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop(void)
{
    timer.run();
    server.handleClient(); // Listen for client connections
}


void Homepage()
{
  CurrentRoot = "";
    if(SD_present)
    {
      webpage += F("<div class = 'container'><form method = 'POST' action = '/sel-mode'><button id='sel-mode' class='btns'> &nbsp;Switch to USB Mode</button></form>");
      webpage += F("<a href = '/MyFiles'><button id = 'my_files' class = 'btns'> &nbsp;My Files</button></a><div>");
    }
    else
    {
      webpage += F("<div class = 'container'><form method = 'POST' action = '/sel-mode'><button id='sel-mode' class='btns'> &nbsp;Switch to SD Mode</button></form>");
      webpage += F("<a href = '/MyFiles'><button id = 'my_files' class = 'btns' disabled> &nbsp;My Files</button></a><div>");
    }
    Send_Page(webpage);
    SendHTML_Stop(); // Stop is needed because no content length was sent
}


void My_Files()
{
    if (server.argName(1) == "download")
    {
        File_Download();
        return;
    }
    else
    {
      Send_Page("");
      SendHTML_Stop();
    }
}

void ConnectToWifi()
{
    String message = "";
    if(got_user_network) 
    {
      setupWifi();
      got_user_network =  false;
      printFile("/wificred.json");
      message.concat(F("<script>location.replace(/ConnectToWifi)</script>"));
    }
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
    server.sendHeader("Pragma", "no-cache"); 
    File file = SPIFFS.open("/Wifinetwork.html");
    String page = file.readString();
    file.close();
    page.replace(F("<% version %>"),Version);
    page.replace(F("<% script %>"),message);
    int pagesize = page.length();
    server.setContentLength(pagesize);
    server.send(200, "text/html", "" ); 
    server.sendContent(page);
    server.send(200, "text/html", "");
    server.client().stop();
}


void Update_Firmware()
{
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) 
    {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (upload.name == "filesystem") 
      {
        if (!Update.begin(SPIFFS.totalBytes(), U_SPIFFS))//start with max available size 
        {
           Update.printError(Serial);
         }
      }
      else
      {
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
          Update.printError(Serial);
        }
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Download()
{
  Serial.println("Welcome to download fuction");
  String filename = getServerArgByName(F("download"));
  if(!filename.startsWith("/")) filename = "/"+filename;
  if (SD_present) { 
    File download = SD.open(MainRoot+filename);
    Serial.println(filename);
    if (download) 
    {
      //server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
      Serial.println("File Downloaded");
    } else ReportFileNotPresent(); 
  } //else ReportSDNotPresent();
}

//---------------------------------------------------------------------------------------------------------------------------
File UploadFile; 

void handleFileUpload()
{ // upload a new file to the Filing system
//  Serial.println("File upload stage-3");
  HTTPUpload& uploadfile = server.upload();
  Serial.println("Welecome to file upload"); 
                                            
  if(uploadfile.status == UPLOAD_FILE_START)
  {
    Serial.println("File upload stage-4");
    String filename = uploadfile.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("Uploading File Name: "); Serial.print(filename);
    Serial.print("  at path: "); Serial.println(MainRoot+filename);
    SD.remove(filename);                         // Remove a previous version, otherwise data is appended the file again
    UploadFile = SD.open(MainRoot+filename, FILE_WRITE);  // Open the file for writing in SPIFFS (create it, if doesn't exist)
    filename = String();
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
    Serial.println("File upload stage-5");
    if(UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
  } 
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    Serial.println("File upload stage-6");
    if(UploadFile)          // If the file was successfully created
    {                                    
      UploadFile.close();   // Close the file again
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
    } 
    else
    {
      ReportCouldNotCreateFile();
    }
  }
}

//--------------------------------------------------------------------------------------------------------------------------
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Delete() // Delete the file
{  
    Serial.println("Welcome to delete function");
    String filename = getServerArgByName(F("delete"));
    if(!filename.startsWith("/")) filename = "/"+filename;
    File dataFile = SD.open(MainRoot+filename, FILE_READ); // Now read data from SD Card 
    String filepath = MainRoot+filename;
    Serial.print("Deleting file: "); Serial.print(filename); 
    Serial.print("  from path: ");Serial.println(filepath);
    if (!dataFile)
    {       
        dataFile.close();
       // My_Files();
        return;
    }
    dataFile.close();
    SD.remove(filepath);
    Serial.println(F("File deleted successfully"));
   // is_from_my_files = true;
   // My_Files();
}


String getServerArgByName(String name)
{
    for (uint8_t index = 0; index < server.args(); index++)
    {
        if (server.argName(index).equals(name))
        {
            return server.arg(index);
        }
    }
    return "";
}


String SD_Dir(String ROOT)
{
    Serial.print("MainRoot : ");
    Serial.println(ROOT);
    MainRoot = ROOT;
    body.replace(F("<% Directory %>"), MainRoot);
    File root = SD.open(ROOT);
    String htmlTableData = "";
    while (true)
    {
        File file = root.openNextFile();
        if (!file)
        {
            break;
        }
        
        String fileName = String(file.name());
        Serial.print("fileName:  ");
        Serial.println(fileName);
        String fileEntryData = "";
        if (file.isDirectory())
        {
            fileEntryData.concat(F("<td><u><a href=\"MyFiles?path="));
            fileEntryData.concat(CurrentRoot+ "/" +fileName);
            fileEntryData.concat(F("\" title=\"Folder\">"));
            fileEntryData.concat(fileName);
            fileEntryData.concat(F("</a></u></td><td>-</td><td>-</td>"));
        }
        else
        {
            fileEntryData.concat(F("<td>"));
            fileEntryData.concat(fileName);
            fileEntryData.concat(F("</td><td>"));
            String fileSize = file_size(file.size());
            fileEntryData.concat(fileSize);
            fileEntryData.concat(F("</td><td><a href=\"MyFiles?path="));
            fileEntryData.concat(MainRoot);
            fileEntryData.concat(F("&download="));
            fileEntryData.concat(fileName);
            fileEntryData.concat(F("\" title=\"Download\">DL</a>"));
            fileEntryData.concat(F("<a href=\"MyFiles?path="));
            fileEntryData.concat(MainRoot);
            fileEntryData.concat(F("&delete="));
            fileEntryData.concat(fileName);
            fileEntryData.concat(F("\" title=\"Delete\">DEL</a>"));
            fileEntryData.concat(F("</td>"));
        }
        htmlTableData.concat(F("<tr>"));
        htmlTableData.concat(fileEntryData);
        htmlTableData.concat(F("</tr>"));
    }
    //Serial.println("`````````````````````````````````````````````````````````````");
    return htmlTableData;
}

void change_mode()
{
 if(SD_present)
 {
  change_to_usb_mode();
 }
 else
 {
  change_to_sd_mode();
 }

}


String getBackPath(String str)
{
  String temp = "",a ="";
  char deli = '/';

  for(int i=0; i<(int)str.length(); i++)
  {
    if(str[i] != deli)
    {
      temp += str[i];
    }
    else
    {
      if(temp != "")
      {
        a += '/'+temp;
      }
      else
      {
        a += temp;
      }
      temp = "";
    }
  }
  return a;
}



//----------------------------------------------------------------------------------------------------------

void SendHTML_Header()
{
 // server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
 // server.sendHeader("Pragma", "no-cache"); 
 // server.sendHeader("Expires", "-1"); 
  //server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  File file = SPIFFS.open("/Header.html");
  header = file.readString(); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  String message = "",Path = "";
  String URI = server.uri();
  Serial.println(getServerArgByName(F("path")));
  if(URI == "/MyFiles" && server.args()== 0)
  {
    Path = "/" ;
    //CurrentRoot = "";
  }
  else
  {
    Path = getBackPath(getServerArgByName(F("path")));
    Serial.println(Path);
    if(Path == "") 
    {
      Path = "/";
      //CurrentRoot = "";
    }
  }
  message.concat(F("<a href=\"MyFiles?path="));
  message.concat(Path);
  message.concat(F("\">"));
  Path = "";
  Serial.print("message: "); Serial.println(message);
  header.replace(F("<% Back %>"), message);
  //server.send(200, "text/html", "" ); 
  //server.sendContent(header);
  file.close();
  //webpage = "";
}

void SendHTML_Body()
{
  Serial.println("---------------------------------------------");
  String link = "";
  String URI = server.uri();
  if(URI == "/")
  {  
    File file = SPIFFS.open("/Body.html");
    body = file.readString();
    file.close();
  }
  else if(URI == "/MyFiles" || is_from_my_files)
  {
    Serial.println("/MyFiles"); 
    File file = SPIFFS.open("/Body1.html");
    body = file.readString();
    file.close();
    
    if (server.args()>0 || is_from_my_files)
    {
      if(is_from_my_files)
      {
        CurrentRoot = CurrentRoot;
      }
      else
      {
        if(server.argName(1) == "delete")
        {
          File_Delete();
        }
        CurrentRoot = getServerArgByName(F("path"));
      }
      body.replace(F("<% files %>"), SD_Dir(CurrentRoot));
    }
    else
    {
      body.replace(F("<% files %>"), SD_Dir("/"));
    }
  }
 // Serial.print("PrevRoot: "); Serial.println(PrevRoot.back());
  Serial.print("CurrRoot: "); Serial.println(CurrentRoot);
  Serial.println("`````````````````````````````````````````````````````````````");
  //int lengthofbody = body.length();
  //server.sendContent(body);
  Serial.println("Sent Body");
  //webpage = "";
}


void SendHTML_Footer()
{
  Serial.println("Welcome to Footer");
  File file = SPIFFS.open("/Footer.html");
  footer = file.readString();
  String ip_address = WiFi.localIP().toString();
  String wifi_ssid = WiFi.SSID();
  footer.replace(F("<% ssid %>"), wifi_ssid.c_str());
  footer.replace(F("<% ip %>"), ip_address.c_str());
  footer.replace(F("<% version %>"), Version);
  String page = "";
  if(is_from_my_files)
  {
    Serial.println("Sending Redirect link");
    is_from_my_files = false;
    page.concat(F("<script>"));
    page.concat(F("location.replace("));
    page.concat(F("\"MyFiles?path="));
    if(MainRoot != "")
    {
      page.concat(MainRoot);
    }
    else
    {
      page.concat("/");
    }
    page.concat(F("\")"));
    page.concat(F("</script>"));
    footer.replace(F("<% script %>"), page);
  }
  else
  {
    footer.replace(F("<% script %>"), "");
  }
  //server.sendContent(footer);
  //server.send(200, "text/html", "");
  file.close();
  //webpage = "";
}


void Send_Page(String page)
{
  int page_length;
  SendHTML_Header();
  SendHTML_Body();
  SendHTML_Footer();
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1");
  if(page == "")
  {
    page_length = header.length() + body.length() + footer.length();
  }
  else
  {
    page_length = header.length() + body.length() + footer.length()+ page.length();
  } 
  server.setContentLength(page_length);
  server.send(200, "text/html", "" );
  server.sendContent(header);
  if(page != "")
  {
    SendHTML_Content();
  }
  server.sendContent(body);
  server.sendContent(footer);
  server.send(200, "text/html", "");
  webpage = "";
}

void SendHTML_Content()
{
  server.sendContent(webpage);
  webpage = "";

}

void SendHTML_Stop()
{
  server.sendContent("");
  server.client().stop();
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportFileNotPresent(){
  SendHTML_Header();
  webpage += F("<h3 style ='text-align:center'>File does not exist</h3>");
  SendHTML_Content(); 
  SendHTML_Body();
  SendHTML_Footer();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportCouldNotCreateFile(){
  SendHTML_Header();
  webpage += F("<h3 style ='text-align:center'>Could Not Create Uploaded File (write-protected?)</h3>");
  SendHTML_Content(); 
  SendHTML_Body();
  SendHTML_Footer();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
String file_size(int bytes){
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}
