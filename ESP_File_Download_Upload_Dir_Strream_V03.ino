
#ifdef ESP8266
#include <ESP8266WiFi.h>       // Built-in
#include <ESP8266WiFiMulti.h>  // Built-in
#include <ESP8266WebServer.h>  // Built-in
#include <ESP8266mDNS.h>
#else
#include <WiFi.h>              // Built-in
#include <WiFiMulti.h>         // Built-in
#include <ESP32WebServer.h>
#include "ESPAsyncWebServer.h"
#include <ESPmDNS.h>
#include "FS.h"
#include "SimpleTimer.h"
#include "SD.h"
#include "SPIFFS.h"
#include<cJSON.h>
// Include files for OTA
#include <WiFiClient.h>

#include <Update.h>


#endif

#include "Network.h"
#include "Sys_Variables.h"
#include "CSS.h"
#include <SD.h>
#include <SPI.h>
#include<list>


SimpleTimer timer;
#ifdef ESP8266
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);
#else
WiFiMulti wifiMulti;
ESP32WebServer server(81);
AsyncWebServer html_server(80);
ESP32WebServer OTAServer(82);
#endif

bool is_wifi_begin = false;
bool sd_mode = false;
String file_dir = "";
String subfile_dir = "";
cJSON* subfileDir;
bool wifi_cred = false;

typedef struct {

  String wifi_name;
  String wifi_pwd;
} wifi_config;

std::list<wifi_config>wifi_list;

String SSID = "VEGG_5";
String PWD = "sss3kk2aaaa4";

const char* loginIndex =
  "<form name='loginForm'>"
  "<table width='20%' bgcolor='A09F9F' align='center'>"
  "<tr>"
  "<td colspan=2>"
  "<center><font size=4><b>ESP32 Login Page</b></font></center>"
  "<br>"
  "</td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td>Username:</td>"
  "<td><input type='text' size=25 name='userid'><br></td>"
  "</tr>"
  "<br>"
  "<br>"
  "<tr>"
  "<td>Password:</td>"
  "<td><input type='Password' size=25 name='pwd'><br></td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
  "</tr>"
  "</table>"
  "</form>"
  "<script>"
  
  "function check(form)"
  "{"
  "if(form.userid.value=='admin' && form.pwd.value=='admin')"
  "{"
  "window.open('/serverIndex')"
  "}"
  "else"
  "{"
  " alert('Error Password or Username')/*displays error message*/"
  "}"
  "}"
  "</script>";

const char* serverIndex =

  "<form method='POST' action='/update' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "<div id='prg'>progress: 0%</div>";


void change_to_usb_mode()
{
  SD.end();
  SPI.end();
  digitalWrite(4, LOW);
  SD_present = false;
  timer.setTimeout(500, []()
  {
    digitalWrite(27, HIGH);
    digitalWrite(4, HIGH);
    //        digitalWrite(4,LOW);    // Buffer IC OFF
    timer.setTimeout(500, []()
    {
      digitalWrite(25, HIGH);   // Buffer IC OFF
      digitalWrite(26, LOW);
      Serial.println("CHIP DETECTED");// chip detect
      //        timer.setTimeout(500,[](){digitalWrite(4,HIGH);});
    });
  });
}

void change_to_usb_mode_at_boot() {
  SD.end();
  SPI.end();
  digitalWrite(4, LOW);
  SD_present = false;
  delay(400);
  digitalWrite(27, HIGH);
  digitalWrite(4, HIGH);
  //        digitalWrite(4,LOW);    // Buffer IC OFF
  delay(100);
  digitalWrite(25, HIGH);   // Buffer IC OFF
  digitalWrite(26, LOW);
  Serial.println("CHIP DETECTED");// chip detect

}

void change_to_sd_mode()
{
  digitalWrite(25, LOW);
  digitalWrite(4, LOW);
  timer.setTimeout(500, []()
  {
    digitalWrite(4, HIGH);
    Serial.println("SD card mode changed");
    digitalWrite(27, LOW);
    digitalWrite(26, HIGH);
    delay(20);
    digitalWrite(25, LOW); //BUffer IC LOW
    while (!SD.begin()) {
      Serial.println("SD Card not properly created");
    }
    SD_present = true;
    Serial.println("SD card done");
  });
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void save_wifi_list()
{


  cJSON * wifiList = cJSON_CreateObject();

  cJSON * wifi_name = NULL;
  cJSON * pwd = NULL;

  cJSON* wifis = cJSON_CreateArray();

  cJSON_AddItemToObject(wifiList, "wifis", wifis);

  for (auto it = wifi_list.begin(); it != wifi_list.end(); it++)
  {

    cJSON* wifiObject = cJSON_CreateObject();
    cJSON* Ob_wifi_name = cJSON_CreateString((it->wifi_name).c_str());
    cJSON * Ob_wifi_pwd = cJSON_CreateString((it->wifi_pwd).c_str());

    cJSON_AddItemToObject(wifiObject, "wifi_name", Ob_wifi_name);
    cJSON_AddItemToObject(wifiObject, "wifi_pwd", Ob_wifi_pwd);

    cJSON_AddItemToArray(wifis, wifiObject);

  }

  String WiFi_list = cJSON_Print(wifiList);

  File wifi_cred = SPIFFS.open("/wifi_cred.txt", "w");
  wifi_cred.println(WiFi_list);
  wifi_cred.close();

  if (SPIFFS.exists("/wifi_cred.txt"))Serial.println("file created successfully");

  Serial.println(WiFi_list);

  cJSON_Delete(wifiList);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void json_file_dir()
{
  cJSON *filename = NULL;
  cJSON *filenames = NULL;
  cJSON *dirnames = NULL;
  cJSON *dirname = NULL;
  cJSON *size = NULL;
  cJSON* sizearr = NULL;


  cJSON* fileDir = cJSON_CreateObject();

  filenames = cJSON_CreateArray();
  sizearr = cJSON_CreateArray();
  dirnames = cJSON_CreateArray();
 
  cJSON_AddItemToObject(fileDir, "filenames", filenames);
  cJSON_AddItemToObject(fileDir, "sizearr", sizearr);
  cJSON_AddItemToObject(fileDir, "dirnames", dirnames);

  // a loop to traverse the sd card and store the filenames in the array


  File root = SD.open("/");

  File file = root.openNextFile(); 
  
  

  while (file)
  {
     
          if (file.isDirectory()){
                             Serial.println(file.name());
                             Serial.println("111 ");
                             cJSON* dir_name = cJSON_CreateString(file.name());
                             cJSON_AddItemToArray(dirnames, dir_name);
                             
                                                     
                            }
          else{
   
    Serial.println(file.name());
    cJSON* file_name = cJSON_CreateString(file.name());
    cJSON_AddItemToArray(filenames, file_name);
                            

    int bytes = file.size();
    String fsize = "";
    if (bytes < 1024)                     fsize = String(bytes) + " B";
    else if (bytes < (1024 * 1024))        fsize = String(bytes / 1024.0, 3) + " KB";
    else if (bytes < (1024 * 1024 * 1024)) fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
    else                                  fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
          

    cJSON* file_size = cJSON_CreateString(fsize.c_str());
    cJSON_AddItemToArray(sizearr, file_size);
          }
    file = root.openNextFile();

  }

  file.close();
  file_dir = cJSON_Print(fileDir);
  File filed = SPIFFS.open("/file_dir.txt", "w");
  filed.printf("%s", file_dir.c_str());
  filed.close();

  Serial.println(file_dir);

  cJSON_Delete(fileDir);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup(void) {
  Serial.begin(115200);
  pinMode(25, OUTPUT);    //Select pin, Buffer LOW for ESP and HIGH for card reader
  pinMode(26, OUTPUT);    //SDcard chip detect (active LOW)
  pinMode(4, OUTPUT);     //HIGH to Powerup microSD card (MOSFET)
  pinMode(27, OUTPUT);    //SD RESET
  
digitalWrite(4, LOW);
digitalWrite(4,LOW);
digitalWrite(26, HIGH);
digitalWrite(27, LOW);
delay(500);
digitalWrite(4, HIGH);
digitalWrite(27, LOW);
digitalWrite(26, HIGH);
delay(20);
//  digitalWrite(25, LOW); //BUffer IC LOW
  delay(1000);

  Serial.printf("\n\n");
  Serial.println("Current version of MAYPOLE is V1.3");
  Serial.printf("\n\n");

  // Mounting the SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount SPIFFS");
  }
  else
  {
    Serial.println("SPIFFS MOUNTED");
  }


  while (!SD.begin())
  {
    Serial.println("Card Mount Failed");
    return;
  }
  Serial.println("SD CARD DONE");
  
  //sd_mode = true;
change_to_usb_mode_at_boot();
  //WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("pen_drive", "12345678");


  // Adding saved wifi APs to wifiMulti

  if (SPIFFS.exists("/wifi_cred.txt"))
  {
    File wifi_cred = SPIFFS.open("/wifi_cred.txt", "r");
    String wifi_cred_json = wifi_cred.readString();
    wifi_cred.close();

    cJSON * wifiap_json = cJSON_Parse(wifi_cred_json.c_str());
    if (wifiap_json == NULL)
    {
      const char *error_ptr = cJSON_GetErrorPtr();
      if (error_ptr != NULL)
      {
        fprintf(stderr, "Error before: %s\n", error_ptr);
      }
      cJSON_Delete(wifiap_json);
    } else {
      cJSON* wifis = cJSON_GetObjectItemCaseSensitive(wifiap_json, "wifis");
      cJSON * wifi = NULL;
      cJSON_ArrayForEach(wifi, wifis)
      {
        cJSON *wifi_name = cJSON_GetObjectItemCaseSensitive(wifi, "wifi_name");
        cJSON *wifi_pwd = cJSON_GetObjectItemCaseSensitive(wifi, "wifi_pwd");
        wifiMulti.addAP(wifi_name->valuestring, wifi_pwd->valuestring);
      }
    }
  }
  else
  {
    Serial.println("No WiFi credentials file found in SPIFFS");
  }

  wifiMulti.addAP("AndroidAP27B6", "txwm3175");
  wifiMulti.addAP("VEGG_5", "sss3kk2aaaa4");
  wifiMulti.addAP("VEGG", "sss3kk2aaaa4");
  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } 
  Serial.println(WiFi.localIP());
  delay(500);
  Serial.print(F("Initializing SD card..."));
  SD_present = true;

  // Note: Using the ESP32 and SD_Card readers requires a 1K to 4K7 pull-up to 3v3 on the MISO line, otherwise they do-not function.
  //----------------------------------------------------------------------
  ///////////////////////////// Server Commands

  //The server at port 81 which handles file upload and download
  //server.on("/",         Homepage);
  server.on("/download", File_Download);
  server.on("/upload",   File_Upload);
  server.on("/fupload",  HTTP_POST, []() {
    Serial.println("1");
    server.send(200, "text/html", webpage);
  }, handleFileUpload);
  server.on("/stream",   File_Stream);
  server.on("/delete",   File_Delete);
  server.on("/dir",      SD_dir);

  // The server at port 80 which serves html files
  html_server.on("/",  [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  html_server.on("/filedir.html", [](AsyncWebServerRequest * request) {

    json_file_dir();
    request->send(SPIFFS, "/filedir.html", "text/html");
  });

  html_server.on("/wifi-config.html", [](AsyncWebServerRequest * request) {
    
    request->send(SPIFFS, "/wifi-config.html", "text/html");
  });

   html_server.on("/subfiledir.html", [](AsyncWebServerRequest * request) {

    
    request->send(SPIFFS, "/subfiledir.html", "text/html");
  });


  html_server.on("/loadDir", [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/file_dir.txt", "text/plain");
    request->send(SPIFFS, "/subfile_dir.txt", "text/plain");
  });

  html_server.on("/connect-to-wifi", [](AsyncWebServerRequest * request) {

    if (request->hasParam("ssid") && request->hasParam("pwd"))
    {
      AsyncWebParameter* p = request->getParam("ssid");
      AsyncWebParameter* q  = request->getParam("pwd");

      SSID = String(p->value().c_str());
      PWD = String(q->value().c_str());
      wifi_cred = true;
      Serial.println(SSID);
      Serial.println(PWD);

                     wifi_config new_wifi = {
      
                          .wifi_name = SSID,
                       .wifi_pwd = PWD
                   };
    
                   wifi_list.push_back(new_wifi);
                   save_wifi_list();
      request->redirect("/");
    }
    else
    {
      request->send(SPIFFS, "/wifi-config.html", "text/html");

    }

  });

  html_server.on("/switch-mode", [](AsyncWebServerRequest * request) {

    if (!sd_mode)
    {
      //change to sd mode and send the response that we have changed to sd mode
      change_to_sd_mode();
      request->send(200, "text/plain", "SD_MODE");
      sd_mode = true;

    }
    else if (sd_mode)
    {
      // change the mode to usb and send the response that we have changed to usb mode
      change_to_usb_mode();
      request->send(200, "text/plain", "USB_MODE");
      sd_mode = false;
    }

  });


  
  html_server.on("/check-mode", [](AsyncWebServerRequest * request) {

    if (sd_mode)
    {
      //change to sd mode and send the response that we have changed to sd mode
      
      request->send(200, "text/plain", "SD_MODE");
     

    }
    else if (!sd_mode)
    {
      // change the mode to usb and send the response that we have changed to usb mode
     
      request->send(200, "text/plain", "USB_MODE");
      
    }

  });


  html_server.on("/check-status", [](AsyncWebServerRequest * request) {

    if (WiFi.status() == WL_CONNECTED)
    {
      //change to sd mode and send the response that we have changed to sd mode
      String ip_address = WiFi.localIP().toString();
      Serial.println( ip_address);
      String content = "Connected to " + SSID + " as" + " " + ip_address;
      request->send(200, "text/plain", content.c_str());
    }
    else
    {
      // change the mode to usb and send the response that we have changed to usb mode
      request->send(200, "text/plain", "notConnected");
    }

  });

html_server.on("/open", [](AsyncWebServerRequest * request){
  AsyncWebParameter* param = request->getParam("open");
  String filename = String(param->value().c_str());
   cJSON *size = NULL;
   cJSON* sizearry = NULL;
   cJSON* subfiles = NULL;
   cJSON* subfileDir = cJSON_CreateObject();
   subfiles = cJSON_CreateArray();
   sizearry = cJSON_CreateArray();

    cJSON_AddItemToObject(subfileDir, "subfiles", subfiles);
    cJSON_AddItemToObject(subfileDir, "sizearry", sizearry);
  Serial.println(filename);
    if (!filename.startsWith("/"))
    {
      filename = "/" + filename;
    }
    Serial.println(filename);
 
    File dir = SD.open(filename);
    File names = dir.openNextFile();
   while(names){
                    Serial.println(names.name());
                    
                    cJSON* file_names = cJSON_CreateString(names.name());
                    cJSON_AddItemToArray(subfiles, file_names);
                

    int bytes = names.size();
    String fsize = "";
    if (bytes < 1024)                     fsize = String(bytes) + " B";
    else if (bytes < (1024 * 1024))        fsize = String(bytes / 1024.0, 3) + " KB";
    else if (bytes < (1024 * 1024 * 1024)) fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
    else                                  fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
          

    cJSON* names_size = cJSON_CreateString(fsize.c_str());
    cJSON_AddItemToArray(sizearry, names_size);
    names = dir.openNextFile();
          }
  names.close();
  subfile_dir = cJSON_Print(subfileDir);
  File filed = SPIFFS.open("/subfile_dir.txt", "w");
  filed.printf("%s", subfile_dir.c_str());
  filed.close();

  Serial.println(subfile_dir);

  cJSON_Delete(subfileDir);
    
    request->send(SPIFFS, "/filedir.html", "text/html");
  
  });

  html_server.on("/delete", [](AsyncWebServerRequest * request) {

  

    AsyncWebParameter* param = request->getParam("delete");

    String filename = String(param->value().c_str());
    Serial.println(filename);
    if (!filename.startsWith("/"))
    {
      filename = "/" + filename;
    }
    
    Serial.println(filename);
    SD.remove(filename);


    request->redirect("/filedir.html");


  });



  html_server.on("/down-arrow.svg", [](AsyncWebServerRequest * request) {




    request->send(SPIFFS, "/down-arrow.svg");  // encoding scheme to be included


  });


  html_server.on("/trash.svg", [](AsyncWebServerRequest * request) {




    request->send(SPIFFS, "/trash.svg");  // encoding scheme to be included


  });

  OTAServer.on("/", HTTP_GET, []() {
    OTAServer.sendHeader("Connection", "close");
    OTAServer.send(200, "text/html", loginIndex);
  });

  OTAServer.on("/serverIndex", HTTP_GET, []() {
    OTAServer.sendHeader("Connection", "close");
    OTAServer.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  OTAServer.on("/update", HTTP_POST, []() {
    OTAServer.sendHeader("Connection", "close");
    OTAServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    //ESP.restart();
  }, []() {
    HTTPUpload& upload = OTAServer.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
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
  });


  ///////////////////////////// End of Request commands
  server.begin();
  html_server.begin();
  OTAServer.begin();
  json_file_dir();
  Serial.println("HTTP server started");

   Serial.println("ESP32 IP as soft AP: ");
               
  Serial.println(WiFi.softAPIP());
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop(void)
{
  timer.run();
  server.handleClient(); // Listen for client connections
  OTAServer.handleClient();
  if (wifi_cred)
  {
    WiFi.disconnect();
    WiFi.begin(SSID.c_str(), PWD.c_str());

    long t = millis();

    while (millis() - t <= 5000)
    {
      if (WiFi.status() != WL_CONNECTED) {

        delay(500);
        Serial.println("Connecting to WiFi..");
      }
      else if(WiFi.status() == WL_CONNECTED)
      {
        Serial.println(WiFi.localIP());
        wifi_cred = false;
        break;
      }

    }

    if (wifi_cred)
    {
      wifi_cred = false;
      Serial.println("Connection failed to new AP rolling back to available ones");

      if (wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
      }

    }
    else
    {
      wifi_config new_wifi = {

        .wifi_name = SSID,
        .wifi_pwd = PWD

      };
      wifi_list.push_back(new_wifi);
      save_wifi_list();
    }
  }
}

// All supporting functions from here...
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HomePage() {
  SendHTML_Header();
  webpage += F("<a href='/download'><button>Download</button></a>");
  webpage += F("<a href='/upload'><button>Upload</button></a>");
  if (sd_mode)
    webpage += F("<a href='/stream'><button>SD CARD MODE</button></a>");
  else
    webpage += F("<a href='/stream'><button>USB MODE</button></a>");
  webpage += F("<a href='/delete'><button>Delete</button></a>");
  webpage += F("<a href='/dir'><button>Directory</button></a>");
  //append_page_footer();
  SendHTML_Content();
  SendHTML_Stop(); // Stop is needed because no content length was sent
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Download() { // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  Serial.println("File_Download()");
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("download")) SD_file_download(server.arg(0));
  }
  else SelectInput("Enter filename to download", "download", "download");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_file_download(String filename) {
  Serial.println("SD_file_download():");
  if (SD_present) {
    Serial.println("SD_present()");
    if (!filename.startsWith("/"))
    {
    filename = "/" + filename;
    }
    Serial.println(filename);
    File download = SD.open(filename);
    Serial.println("filename::"+filename);
    Serial.println(download.name());

    if (download) {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
     
      download.close();
    } else ReportFileNotPresent("download");
  } else ReportSDNotPresent();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Upload() {
  if (sd_mode)
  {
    Serial.println("File upload stage-1");
    //append_page_footer();                               
    webpage += F("<h3>Select File to Upload</h3>");
    webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
    webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
    webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
    webpage += F("<a href='/'>[Back]</a><br><br>");
    //append_page_footer();
    Serial.println("File upload stage-2");
    server.send(200, "text/html", webpage);
  } else
  {
    Serial.println("File upload stage-1");
    ////append_page_footer();
       webpage += F("<h3>This is a USB mode. For file upload please change the mode</h3>");
       webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
       webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
       webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
    webpage += F("<a href='/'>[Back]</a><br><br>");
    //append_page_footer();
    Serial.println("File upload stage-2");
    server.send(200, "text/html", webpage);
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File UploadFile;
void handleFileUpload() { // upload a new file to the Filing system
    Serial.println("File upload stage-3");
  HTTPUpload& uploadfile = server.upload();

  if (uploadfile.status == UPLOAD_FILE_START)
  {
    Serial.println("File upload stage-4");
    String filename = uploadfile.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.print("Upload File Name: "); Serial.println(filename);
    SD.remove(filename);                         // Remove a previous version, otherwise data is appended the file again
    UploadFile = SD.open(filename, FILE_WRITE);  // Open the file for writing in SPIFFS (create it, if doesn't exist)
    filename = String();
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
        Serial.println("File upload stage-5");
    if (UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
  }
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    if (UploadFile)         // If the file was successfully created
    {
      UploadFile.close();   // Close the file again
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
      webpage = "";
      //append_page_footer();
            //webpage += F("<h3>File was successfully uploaded</h3>");
            //webpage += F("<h2>Uploaded File Name: "); webpage += uploadfile.filename+"</h2>";
           //webpage += F("<h2>File Size: "); webpage += file_size(uploadfile.totalSize) + "</h2><br>";
      //append_page_footer();
      
      Serial.println("Reached till here");
      webpage += F("<html>");
      webpage += F("<head>");
      webpage += F("<title>File Dir</title>");
      webpage += F("<script>var x = location.host; x=x.toString();x = x.slice(0, -3);alert(x); window.location = 'http://'+x+'/filedir.html';</script>");
      webpage += F("<body></body>"); 
      webpage += F("</head>");
      webpage += F("</html>");
      server.send (200,"text/html",webpage);
    }
    else
    {
      ReportCouldNotCreateFile("upload");
    }
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_dir() {
  if (sd_mode)
  {
    if (SD_present) {
      File root = SD.open("/");
      if (root) {
        root.rewindDirectory();
        SendHTML_Header();
        webpage += F("<h3 class='rcorners_m'>SD Card Contents</h3><br>");
        webpage += F("<table align='center'>");
        webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th></tr>");
        printDirectory("/", 0);
        webpage += F("</table>");
        SendHTML_Content();
        root.close();
      }
      else
      {
        SendHTML_Header();
        webpage += F("<h3>No Files Found</h3>");
      }
      //append_page_footer();
      SendHTML_Content();
      SendHTML_Stop();   // Stop is needed because no content length was sent
    } else ReportSDNotPresent();
  } else
  {
    Serial.println("File upload stage-1");
    //append_page_footer();
    webpage += F("<h3>This is a USB mode. For file directory change the mode.</h3>");
      webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
        webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
        webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
    webpage += F("<a href='/'>[Back]</a><br><br>");
    //append_page_footer();
    Serial.println("File upload stage-2");
    server.send(200, "text/html", webpage);
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void printDirectory(const char * dirname, uint8_t levels) {
  File root = SD.open(dirname);
  Serial.print(dirname);
#ifdef ESP8266
  root.rewindDirectory(); //Only needed for ESP8266
#endif
  if (!root) {
    return;
  }
  if (!root.isDirectory()) {
    return;
  }
  File file = root.openNextFile();
  while (file) {
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    String file_name = String(file.name());
    if (file.isDirectory())
    {
      if (file_name.indexOf(".") != 1)
      {
        Serial.println(String(file.isDirectory() ? "Dir " : "File ") + String(file.name()));
        webpage += "<tr><td>" + String(file.isDirectory() ? "Dir" : "File") + "</td><td>" + String(file.name()) + "</td><td></td></tr>";
        printDirectory(file.name(), levels - 1);
      }
    }
    else
    {

      Serial.println(file_name.indexOf("."));
      Serial.print(String(file.name())+"\t");
      if (file_name.indexOf(".") != 1)
      {

        webpage += "<tr><td>" + file_name + "</td>";
        Serial.print(String(file.isDirectory() ? "Dir " : "File ") + String(file.name()) + "\t");
        webpage += "<td>" + String(file.isDirectory() ? "Dir" : "File") + "</td>";
        int bytes = file.size();
        String fsize = "";
        if (bytes < 1024)                     fsize = String(bytes) + " B";
        else if (bytes < (1024 * 1024))        fsize = String(bytes / 1024.0, 3) + " KB";
        else if (bytes < (1024 * 1024 * 1024)) fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
        else                                  fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
        webpage += "<td>" + fsize + "</td></tr>";
        Serial.println(String(fsize));
      }
    }
    file = root.openNextFile();
  }
  file.close();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Stream() {
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("stream")) SD_file_stream(server.arg(0));
  }
  else SelectInput("Mode Changed", "stream", "stream");
  sd_mode = !sd_mode;
  if (sd_mode)
  {
    timer.setTimeout(1000, change_to_sd_mode);
  } else
  {
    timer.setTimeout(1000, change_to_usb_mode);
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_file_stream(String filename) {
  sd_mode = !sd_mode;
  if (SD_present) {
    File dataFile = SD.open("/"+filename, FILE_READ); // Now read data from SD Card
    Serial.print("Streaming file: "); Serial.println(filename);
    if (dataFile) {
       if (dataFile.available()) { // If data is available and present
         String dataType = "application/octet-stream";
          if (server.streamFile(dataFile, dataType) != dataFile.size()) {Serial.print(F("Sent less data than expected!")); }
      }
       dataFile.close(); // close the file:
    } else ReportFileNotPresent("Cstream");
   } else ReportSDNotPresent();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Delete() {
  if (sd_mode)
  {
    if (server.args() > 0 ) { // Arguments were received
      if (server.hasArg("delete")) SD_file_delete(server.arg(0));
    }
    else SelectInput("Select a File to Delete", "delete", "delete");
  } else
  {
    Serial.println("File upload stage-1");
    //append_page_footer();
    webpage += F("<h3>This is a USB mode. For file delete please change the mode</h3>");
        webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
        webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
       webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
    webpage += F("<a href='/'>[Back]</a><br><br>");
    //append_page_footer();
    Serial.println("File upload stage-2");
    server.send(200, "text/html", webpage);
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_file_delete(String filename) { // Delete the file

  if (SD_present) {
    SendHTML_Header();
    File dataFile = SD.open("/" + filename, FILE_READ); // Now read data from SD Card
    Serial.print("Deleting file: "); Serial.println(dataFile);
    if (dataFile)
    {
      if (SD.remove("/" + filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '" + filename + "' has been erased</h3>";
        webpage += F("<a href='/delete'>[Back]</a><br><br>");
      }
      else
      {
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='delete'>[Back]</a><br><br>");
      }
    } else ReportFileNotPresent("delete");
    //append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();
  } else ReportSDNotPresent();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Header() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  //append_page_footer();
  server.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Content() {
  server.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Stop() {
  server.sendContent("");
  server.client().stop(); // Stop is needed because no content length was sent
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SelectInput(String heading1, String command, String arg_calling_name) {
  SendHTML_Header();
  webpage += F("<h3>"); webpage += heading1 + "</h3>";
  webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><br>");
  //append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportSDNotPresent() {
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  //append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportFileNotPresent(String target) {
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>");
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  //();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportCouldNotCreateFile(String target) {
  SendHTML_Header();
  webpage += F("<h3>Could Not Create Uploaded File (write-protected?)</h3>");
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  ////append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
String file_size(int bytes) {
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes) + " B";
  else if (bytes < (1024 * 1024))      fsize = String(bytes / 1024.0, 3) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
  else                              fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
  return fsize;
}
