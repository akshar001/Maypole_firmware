
#ifdef ESP8266
  #include <ESP8266WiFi.h>       // Built-in
  #include <ESP8266WiFiMulti.h>  // Built-in
  #include <ESP8266WebServer.h>  // Built-in
  #include <ESP8266mDNS.h>
#else
  #include <WiFi.h>              // Built-in
  #include <WiFiMulti.h>         // Built-in
  #include <ESP32WebServer.h>    
  #include <ESPmDNS.h>
  #include "FS.h"
  #include "SimpleTimer.h"
  #include "SD.h"
  #include "SPIFFS.h"
#endif

#include "Network.h"
#include "Sys_Variables.h"
#include "CSS.h"
#include <SD.h> 
#include <SPI.h>
SimpleTimer timer;
#ifdef ESP8266
  ESP8266WiFiMulti wifiMulti; 
  ESP8266WebServer server(80);
#else
  WiFiMulti wifiMulti;
  ESP32WebServer server(80);
#endif
bool sd_mode = false;

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
  timer.setTimeout(500,[]()
  {
    digitalWrite(25,HIGH);    // Buffer IC OFF
    digitalWrite(26,LOW);
    Serial.println("CHIP DETECTED");// chip detect
    
    //        timer.setTimeout(500,[](){digitalWrite(4,HIGH);});
  });
  });
}

void change_to_sd_mode()
{
  digitalWrite(25,LOW);
  digitalWrite(4,LOW);
  timer.setTimeout(500,[]()
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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup(void){
  Serial.begin(115200);
    pinMode(25,OUTPUT);     //Select pin, Buffer LOW for ESP and HIGH for card reader 
    pinMode(26,OUTPUT);     //SDcard chip detect (active LOW)
    pinMode(4,OUTPUT);      //HIGH to Powerup microSD card (MOSFET)
    pinMode(27,OUTPUT);     //SD RESET
        digitalWrite(4,LOW);
//            digitalWrite(4,LOW);
        digitalWrite(26,HIGH);
        digitalWrite(27,LOW);
        delay(500);
      digitalWrite(4,HIGH);
      digitalWrite(27,LOW);
      digitalWrite(26,HIGH);
      delay(20);
      digitalWrite(25,LOW);  //BUffer IC LOW 
      delay(200);
      while(!SD.begin())
      {
        Serial.println("Card Mount Failed");
              return;
      }
      Serial.println("SD CARD DONE"); 
      sd_mode = true;
      WiFi.mode(WIFI_AP_STA); 
      WiFi.softAP("pen_drive","12345678");
      delay(500);
      Serial.print(F("Initializing SD card...")); 
      SD_present = true;

      // Note: Using the ESP32 and SD_Card readers requires a 1K to 4K7 pull-up to 3v3 on the MISO line, otherwise they do-not function.
      //----------------------------------------------------------------------   
      ///////////////////////////// Server Commands 
      server.on("/",         HomePage);
      server.on("/download", File_Download);
      server.on("/upload",   File_Upload);
      server.on("/fupload",  HTTP_POST,[](){ server.send(200);}, handleFileUpload);
      server.on("/stream",   File_Stream);
      server.on("/delete",   File_Delete);
      server.on("/dir",      SD_dir);
      ///////////////////////////// End of Request commands
      server.begin();
      Serial.println("HTTP server started");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop(void)
{
    timer.run();
    server.handleClient(); // Listen for client connections
}

// All supporting functions from here...
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HomePage(){
  SendHTML_Header();
  webpage += F("<a href='/download'><button>Download</button></a>");
  webpage += F("<a href='/upload'><button>Upload</button></a>");
  if(sd_mode)
    webpage += F("<a href='/stream'><button>SD CARD MODE</button></a>");
  else
    webpage += F("<a href='/stream'><button>USB MODE</button></a>");
  webpage += F("<a href='/delete'><button>Delete</button></a>");
  webpage += F("<a href='/dir'><button>Directory</button></a>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop(); // Stop is needed because no content length was sent
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Download(){ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("download")) SD_file_download(server.arg(0));
  }
  else SelectInput("Enter filename to download","download","download");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_file_download(String filename){
  if (SD_present) { 
    File download = SD.open("/"+filename);
    if (download) {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } else ReportFileNotPresent("download"); 
  } else ReportSDNotPresent();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Upload(){
  if(sd_mode)
  {
    Serial.println("File upload stage-1");
    append_page_header();
    webpage += F("<h3>Select File to Upload</h3>"); 
    webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
    webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
    webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
    webpage += F("<a href='/'>[Back]</a><br><br>");
    append_page_footer();
    Serial.println("File upload stage-2");
    server.send(200, "text/html",webpage);
  }else
  {
    Serial.println("File upload stage-1");
    append_page_header();
    webpage += F("<h3>This is a USB mode. For file upload please change the mode</h3>"); 
//    webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
//    webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
//    webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
    webpage += F("<a href='/'>[Back]</a><br><br>");
    append_page_footer();
    Serial.println("File upload stage-2");
    server.send(200, "text/html",webpage);
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File UploadFile; 
void handleFileUpload(){ // upload a new file to the Filing system
//  Serial.println("File upload stage-3");
  HTTPUpload& uploadfile = server.upload(); 
                                            
  if(uploadfile.status == UPLOAD_FILE_START)
  {
    Serial.println("File upload stage-4");
    String filename = uploadfile.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("Upload File Name: "); Serial.println(filename);
    SD.remove(filename);                         // Remove a previous version, otherwise data is appended the file again
    UploadFile = SPIFFS.open(filename, FILE_WRITE);  // Open the file for writing in SPIFFS (create it, if doesn't exist)
    filename = String();
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
//    Serial.println("File upload stage-5");
    if(UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
  } 
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    if(UploadFile)          // If the file was successfully created
    {                                    
      UploadFile.close();   // Close the file again
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
      webpage = "";
      append_page_header();
      webpage += F("<h3>File was successfully uploaded</h3>"); 
      webpage += F("<h2>Uploaded File Name: "); webpage += uploadfile.filename+"</h2>";
      webpage += F("<h2>File Size: "); webpage += file_size(uploadfile.totalSize) + "</h2><br>"; 
      append_page_footer();
      server.send(200,"text/html",webpage);
    } 
    else
    {
      ReportCouldNotCreateFile("upload");
    }
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_dir(){
  if(sd_mode)
  {
    if (SD_present) { 
      File root = SD.open("/");
      if (root) {
        root.rewindDirectory();
        SendHTML_Header();
        webpage += F("<h3 class='rcorners_m'>SD Card Contents</h3><br>");
        webpage += F("<table align='center'>");
        webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th></tr>");
        printDirectory("/",0);
        webpage += F("</table>");
        SendHTML_Content();
        root.close();
      }
      else 
      {
        SendHTML_Header();
        webpage += F("<h3>No Files Found</h3>");
      }
      append_page_footer();
      SendHTML_Content();
      SendHTML_Stop();   // Stop is needed because no content length was sent
    } else ReportSDNotPresent();
  }else
  {
    Serial.println("File upload stage-1");
    append_page_header();
    webpage += F("<h3>This is a USB mode. For file directory mode.</h3>"); 
//    webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
//    webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
//    webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
    webpage += F("<a href='/'>[Back]</a><br><br>");
    append_page_footer();
    Serial.println("File upload stage-2");
    server.send(200, "text/html",webpage);
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void printDirectory(const char * dirname, uint8_t levels){
  File root = SD.open(dirname);
  #ifdef ESP8266
  root.rewindDirectory(); //Only needed for ESP8266
  #endif
  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }
  File file = root.openNextFile();
  while(file){
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    String file_name = String(file.name());
    if(file.isDirectory())
    {
      if(file_name.indexOf(".") != 1)
      {
        Serial.println(String(file.isDirectory()?"Dir ":"File ")+String(file.name()));
        webpage += "<tr><td>"+String(file.isDirectory()?"Dir":"File")+"</td><td>"+String(file.name())+"</td><td></td></tr>";
        printDirectory(file.name(), levels-1);
      }
    }
    else
    {
      
      Serial.println(file_name.indexOf("."));
      //Serial.print(String(file.name())+"\t");
      if(file_name.indexOf(".") != 1)
      {
      
        webpage += "<tr><td>"+file_name+"</td>";
        Serial.print(String(file.isDirectory()?"Dir ":"File ")+String(file.name())+"\t");
        webpage += "<td>"+String(file.isDirectory()?"Dir":"File")+"</td>";
        int bytes = file.size();
        String fsize = "";
        if (bytes < 1024)                     fsize = String(bytes)+" B";
        else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
        else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
        else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
        webpage += "<td>"+fsize+"</td></tr>";
        Serial.println(String(fsize));
      }
    }
    file = root.openNextFile();
  }
  file.close();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Stream(){
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("stream")) SD_file_stream(server.arg(0));
  }
  else SelectInput("Mode Changed","stream","stream");
  sd_mode = !sd_mode;
  if(sd_mode)
  {
    timer.setTimeout(1000,change_to_sd_mode);
  }else
  {
    timer.setTimeout(1000,change_to_usb_mode);
  } 
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_file_stream(String filename) {
    sd_mode = !sd_mode; 
//  if (SD_present) { 
//    File dataFile = SD.open("/"+filename, FILE_READ); // Now read data from SD Card 
//    Serial.print("Streaming file: "); Serial.println(filename);
//    if (dataFile) { 
//      if (dataFile.available()) { // If data is available and present 
//        String dataType = "application/octet-stream"; 
//        if (server.streamFile(dataFile, dataType) != dataFile.size()) {Serial.print(F("Sent less data than expected!")); } 
//      }
//      dataFile.close(); // close the file: 
//    } else ReportFileNotPresent("Cstream");
//  } else ReportSDNotPresent(); 
}   
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Delete(){
  if(sd_mode)
  {
    if (server.args() > 0 ) { // Arguments were received
      if (server.hasArg("delete")) SD_file_delete(server.arg(0));
    }
    else SelectInput("Select a File to Delete","delete","delete");
  }else
  {
    Serial.println("File upload stage-1");
    append_page_header();
    webpage += F("<h3>This is a USB mode. For file delete please change the mode</h3>"); 
//    webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
//    webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
//    webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
    webpage += F("<a href='/'>[Back]</a><br><br>");
    append_page_footer();
    Serial.println("File upload stage-2");
    server.send(200, "text/html",webpage);
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_file_delete(String filename) { // Delete the file 
  if (SD_present) { 
    SendHTML_Header();
    File dataFile = SD.open("/"+filename, FILE_READ); // Now read data from SD Card 
    Serial.print("Deleting file: "); Serial.println(filename);
    if (dataFile)
    {
      if (SD.remove("/"+filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '"+filename+"' has been erased</h3>"; 
        webpage += F("<a href='/delete'>[Back]</a><br><br>");
      }
      else
      { 
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='delete'>[Back]</a><br><br>");
      }
    } else ReportFileNotPresent("delete");
    append_page_footer(); 
    SendHTML_Content();
    SendHTML_Stop();
  } else ReportSDNotPresent();
} 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Header(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves. 
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Content(){
  server.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Stop(){
  server.sendContent("");
  server.client().stop(); // Stop is needed because no content length was sent
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SelectInput(String heading1, String command, String arg_calling_name){
  SendHTML_Header();
  webpage += F("<h3>"); webpage += heading1 + "</h3>"; 
  webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportSDNotPresent(){
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>"); 
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportFileNotPresent(String target){
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportCouldNotCreateFile(String target){
  SendHTML_Header();
  webpage += F("<h3>Could Not Create Uploaded File (write-protected?)</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
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
