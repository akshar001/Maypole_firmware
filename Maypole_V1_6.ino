//Changes made --> 10-10-2023 
#include <SPI.h>
#include <SimpleTimer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <SD.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <ESPmDNS.h>

AsyncWebServer server(80);
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
String Version = "v.1.6";

using MyWiFiEvent_t = WiFiEvent_t;

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
  Serial.println("info.wifi_sta_disconnected.reason");
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
  byte mac[6];
  WiFi.softAPmacAddress(mac);
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "Machine_%02X:%02X:%02X", mac[3], mac[4], mac[5]);
  WiFi.softAP(macStr, "12345678");
  Serial.print("SoftAP IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Load WiFi credentials from "wificred.json"
  obj = getJSonFromFile(&doc, "/wificred.json");
  for (int i = 0; i < obj["data"].size(); i++)
  {
    String wifi_name = obj["data"][i]["wifi_name"];
    String wifi_pwd = obj["data"][i]["wifi_pwd"];
    wifiMulti.addAP(wifi_name.c_str(), wifi_pwd.c_str());
  }

  // Route for serving the JSON file
  server.on("/wificred.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    File file = SPIFFS.open("/wificred.json", "r");
    if (!file)
    {
      request->send(404, "text/plain", "File not found");
      return;
    }
    AsyncWebServerResponse *response = request->beginResponse(file, "application/json");
    request->send(response);
  });

  // Start the server
  server.begin();

 if (wifiMulti.run() == WL_CONNECTED)
  {
  //  Serial.println(WiFi.localIP());
 }
  if (got_user_network)
  {
    
       
    WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.begin(ssid.c_str(), password.c_str());


    SaveWifiCred();
  }
}

void change_to_usb_mode()
{ 
  SD.end();
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
  SD_present = true;
  timer.setTimeout(100,[]()
  {
    digitalWrite(4,HIGH);
    Serial.println("SD card mode changed");
    digitalWrite(27,LOW);
    digitalWrite(26,HIGH);
    delay(20);
    digitalWrite(25,LOW);  //BUffer IC LOW 
/*while(!SD.begin()){
      Serial.println("SD Card not properly created");
    }
    SD_present = true;
    Serial.println("SD card done");
  });*/
  
 if (!SD.begin()) {
      SD_present = false;
      Serial.println("SD card initialized failed");
    } else {
      SD_present = true;
      Serial.println("SD card initialization successfully");
    }
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
///////////////////////////////////////////////////////////////////////////////
//server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
   // handleDownload(request);
  //});
   //////////////////////////////////////////////////////////////////////////////////////////////
   // Route the URL path to the Homepage function
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  Homepage(request);
  });
  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Route for MyFiles
  server.on("/MyFiles", HTTP_GET, [](AsyncWebServerRequest *request) {
    My_Files(request);
  });
  /////////////////////////////////////////////////////////////////////////////////////////////////

  server.on("/ConnectToWifi", HTTP_GET, ConnectToWifi);

  /////////////////////////////////////////////////////////////////////////////////////////////////
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);
  }, handleFileUpload);
    /////////////////////////////////////////////////////////////////////////////////////////////////
  server.on("/sel-mode", HTTP_POST, [](AsyncWebServerRequest *request){
  change_mode();
  delay(200);
  request->redirect("/");
});
/*server.on("/sel-mode", HTTP_POST, handleSelMode);*/


///////////////////////////////////////////////////////////////////////////////////////////////

// Route for handling network settings
    server.on("/network", HTTP_POST, [](AsyncWebServerRequest *request) {
        got_user_network = true;
        ssid = request->arg("ssid");
        password = request->arg("password");
        ConnectToWifi(request);
    });

 /////////////////////////////////////////////////////////////////////////////////////////////////////
server.on("/FirmwareUpdate", HTTP_GET, [](AsyncWebServerRequest *request) {
    File file = SPIFFS.open("/Firmware_update.html");
    if (file) {
      String page = file.readString();
      file.close();
      page.replace(F("<% version %>"), Version);
      int pagesize = page.length();
      AsyncWebServerResponse *response = request->beginResponse(200, "text/html", page);
      response->addHeader("Connection", "close");
      response->setContentLength(pagesize);
      request->send(response);
    } else {
      request->send(404, "text/plain", "File Not Found");
    }
  });

//////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Define the "/update" route handler
server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Upload a firmware binary file to update.");
 //    ESP.restart();
  }, handleFirmwareUpdate);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////         
  server.on("/back.png", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/back.png", "image/png"); 
});
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 server.on("/home.png", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/home.png", "image/png"); 
});
//////////////////////////////////////////////////////////////////////////////////////////      
  server.on("/SDCard.png", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/SDCard.png", "image/png"); 
});
/////////////////////////////////// End of Request commands// Start the server
    server.begin();
    MDNS.addService("_http", "_tcp", 80);
    MDNS.addServiceTxt("_http", "_tcp", "board", "ESP32");
    Serial.println("HTTP server started");
}

void Homepage(AsyncWebServerRequest *request)
{
 // SendHTML_Header(request);
  CurrentRoot = "";
  String webpage = "";
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
   // Send_Page(request,"");
    Send_Page(request,webpage);
   // request->send(200, "text/html", webpage);
    SendHTML_Stop(); // Stop is needed because no content length was sent
}

void My_Files(AsyncWebServerRequest *request)
{
    if (request->hasParam("download"))
    {
        handleDownload(request);
        return;
    }
    else
    {
        Send_Page(request, "");
        request->send(200, "text/html", "");
    }
}

void ConnectToWifi(AsyncWebServerRequest *request)
{
    String message = "";
    if(got_user_network) 
    {
        setupWifi();
        got_user_network = false;
        printFile("/wificred.json");
        message.concat(F("<script>location.replace('/ConnectToWifi')</script>"));
    }

    // Load the HTML file from SPIFFS
    File file = SPIFFS.open("/Wifinetwork.html", "r");
    if (!file) {
        request->send(500, "text/plain", "Failed to open file");
        return;
    }

    String page = file.readString();
    file.close();

    // Replace placeholders in the HTML content
    page.replace(F("<% version %>"), Version);
    page.replace(F("<% script %>"), message);

    // Prepare the response with the appropriate headers
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", page);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");

    // Send the response
    request->send(response);
}


//// Usage example
//void handleConnectToWifi(AsyncWebServerRequest *request) {
//  ConnectToWifi(request);
//}

void handleFirmwareUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  static bool updateStarted = false;
  
  if (!index) {
    updateStarted = false;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      request->send(500, "text/plain", "Update could not begin");
      return;
    }
  }
  
  if (!updateStarted) {
    updateStarted = true;
    Serial.println("Updating firmware...");
  }
  
  if (Update.write(data, len) != len) {
    request->send(500, "text/plain", "Update failed");
    return;
  }
  
  if (final) {
    if (Update.end(true)) {
      Serial.println("Update successful. Rebooting...");
      request->send(200, "text/plain", "Update successful. Rebooting...");
      delay(1000);
      ESP.restart(); // Reboot the device
    } else {
      request->send(500, "text/plain", "Update failed");
    }
  }
}




void handleDownload(AsyncWebServerRequest *request) {
  Serial.println("Welcome to the download function");
  String filename = request->arg("download");
  
  if (!filename.startsWith("/")) {
    filename = "/" + filename;
  }

  if (SD_present) {
    File download = SD.open(MainRoot + filename, FILE_READ); // Open the file in read mode
    Serial.println(filename);
    
    if (download) {
      request->send(SD, MainRoot + filename, "application/octet-stream");
      download.close();
    } else {
      ReportFileNotPresent(request, filename);
    }
  } else {
    ReportSDNotPresent(request);
  }
}






File UploadFile;

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  static unsigned long uploadStartTime = 0;

  if (!index)
  {
    Serial.println("Welcome to file upload");
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.print("Uploading File Name: ");
    Serial.print(filename);
    Serial.print("  at path: ");
    Serial.println(MainRoot + filename);
    SD.remove(filename);                         // Remove a previous version, otherwise data is appended to the file again
    UploadFile = SD.open(MainRoot + filename, FILE_WRITE);  // Open the file for writing in SD (create it if it doesn't exist)
    uploadStartTime = millis(); // Record the start time
  }

  if (UploadFile)
  {
    if (len)
    {
      UploadFile.write(data, len); // Write the received bytes to the file
    }

    if (final)
    {
      UploadFile.close(); // Close the file

      unsigned long uploadEndTime = millis(); // Record the end time
      unsigned long uploadDuration = uploadEndTime - uploadStartTime;

      Serial.print("Upload Size: ");
      Serial.println(index + len);
      Serial.print("Upload Time: ");
      Serial.print(uploadDuration);
      Serial.println(" ms");

      request->redirect("/MyFiles");
    }
  }
  else
  {
    ReportCouldNotCreateFile(request, "upload");
  }
}


void File_Delete(AsyncWebServerRequest *request) {
  Serial.println("Welcome to delete function");
  String filename = request->getParam("delete")->value();
  if (!filename.startsWith("/")) {
    filename = "/" + filename;
  }
  String filepath = MainRoot + filename;
  Serial.print("Deleting file: "); Serial.print(filename); 
  Serial.print("  from path: "); Serial.println(filepath);

  if (SD_present && SD.exists(filepath)) {
    if (SD.remove(filepath)) {
       Serial.println("File '" + filename + "' has been erased");
      
    } else {
     Serial.println("File '" + filename + "' has not been erased");
    }
  }

  // Redirect to the main page ("/") to display the updated file list
  request->redirect("/MyFiles");
  }


String getServerArgByName(AsyncWebServerRequest* request, const String& name)
{
    for (uint8_t index = 0; index < request->args(); index++)
    {
        if (request->argName(index).equals(name))
        {
            return request->arg(index);
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


void SendHTML_Header(AsyncWebServerRequest *request)
{
  File file = SPIFFS.open("/Header.html");
  header = file.readString();
  String message = "";
  String URL = request->url();
  Serial.println(request->arg("path"));
  if (URL == "/MyFiles" && request->args() == 0)
  {
    message = "<a href=\"/MyFiles\">"; // Use the appropriate path here
  }
  else
  {
    String path = getBackPath(request->arg("path"));
    Serial.println(path);
    if (path == "")
    {
      message = "<a href=\"/\">"; // Use the appropriate path here
    }
    else
    {
      message.concat("<a href=\"/MyFiles?path=");
      message.concat(path);
      message.concat("\">");
    }
  }
  Serial.print("message: ");
  Serial.println(message);
  header.replace(F("<% Back %>"), message);
  file.close();
  webpage = "";
}

void SendHTML_Body(AsyncWebServerRequest *request)
{
  Serial.println("---------------------------------------------");
  String link = "";
  String URL = request->url();
  if (URL == "/")
  {  
    File file = SPIFFS.open("/Body.html");
    body = file.readString();
    file.close();
  }
  else if (URL == "/MyFiles" || is_from_my_files)
  {
    Serial.println("/MyFiles"); 
    File file = SPIFFS.open("/Body1.html");
    body = file.readString();
    file.close();
    
    if (request->args() > 0 || is_from_my_files)
    {
      if (is_from_my_files)
      {
        CurrentRoot = CurrentRoot;
      }
      else
      {
        if (request->argName(1) == "delete")
        {
          File_Delete(request);
        }
       CurrentRoot = getServerArgByName(request, F("path"));

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

void Send_Page(AsyncWebServerRequest *request, String page)
{
  String content;
  int page_length = 0; // Declare page_length variable
  SendHTML_Header(request);
  SendHTML_Body(request);
  SendHTML_Footer();
  
  content += header;
 // content += body;
  
  
  if (page == "")
  {
    page_length = header.length() + body.length() + footer.length();
  }
  else
  {
    page_length = header.length() + body.length() + footer.length() + page.length();
  }

  if (page != "")
  {
    content += page;
  }

  content += body;
  content += footer;
  
  request->send(200, "text/html", content);
  webpage = "";
}

void SendHTML_Content() {
  server.on("/content", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", webpage); // Send the webpage content
    webpage = ""; // Clear the webpage content after sending
  });
}


void SendHTML_Stop() {
  
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", ""); // Send empty response
    request->client()->stop(); // Stop is needed because no content length was sent
  });
}

//ReportFileNotPresent
void ReportFileNotPresent(AsyncWebServerRequest *request, String target)
{
  SendHTML_Header(request);
  webpage += F("<h3 style ='text-align:center'>File does not exist</h3>");
  SendHTML_Content(); 
  SendHTML_Body(request);
  SendHTML_Footer();
  SendHTML_Stop();
}


//ReportSDNotPresent
void ReportSDNotPresent(AsyncWebServerRequest *request)
{
  SendHTML_Header(request);
  webpage += F("<h3 style ='text-align:center'>Could Not Create Uploaded File (write-protected?)</h3>");
  SendHTML_Content(); 
  SendHTML_Body(request);
  SendHTML_Footer();
  SendHTML_Stop();
}

//ReportCouldNotCreateFile
void ReportCouldNotCreateFile(AsyncWebServerRequest *request, String target)
{
  SendHTML_Header(request);
  webpage += F("<h3 style ='text-align:center'>Could Not Create Uploaded File (write-protected?)</h3>");
  SendHTML_Content(); 
  SendHTML_Body(request);
  SendHTML_Footer();
  SendHTML_Stop();
}

String file_size(int bytes)
{
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B"; 
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}

void loop(void)
{
   if (got_user_network) {
        setupWifi();  // Connect to the Wi-Fi network in the main loop
        printFile("/wificred.json");
        got_user_network = false;
    }
  timer.run();
}
