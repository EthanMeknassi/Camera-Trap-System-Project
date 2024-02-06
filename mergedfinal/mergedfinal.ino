#include <DHT.h>
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems //cc
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems//cc
#include "driver/rtc_io.h" //cc
#include <EEPROM.h> 
#include <ESPmDNS.h>
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>
#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "esp_http_server.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <WebServer.h>
//#include <StringArray.h>
#include <FS.h>
#include "SD_MMC.h"            // SD Card ESP32
#include <FSImpl.h>
#include <vfs_api.h>
#include <EEPROM.h>            // read and write from flash memory


#include <WiFi.h>              //Built-in
#include <ESPmDNS.h>

#include <SD.h> 
#include <SPI.h>
Servo myservo; 

#define EEPROM_SIZE 1
#define servername "Camera-Trap Wireless Server" //Define the name to your server... 

bool   SD_present = false; //Controls if the SD card is present or not
String webpage = ""; //String to save the html code


// Replace with your network credentials
const char* ssid = "ESP32";
const char* password = "12345678";

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);



// Create AsyncWebServer object on port 80
WebServer server(80);
float humidity = 0;
float temp = 0;
String timestamps = "";

boolean takeNewPhoto = false;

// Photo File Name to save in SD
#define FILE_PHOTO "/photo.jpg"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int pictureNumber = 0;
String path = ""; 

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector

  // Serial port for debugging purposes
  Serial.begin(115200);
  myservo.attach(12);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.softAPIP());
  Serial.println("' to connect");
  delay(100);
  server.begin();
  Serial.println("HTTP server started");

  Serial.print(F("Initializing SD card..."));
  fs::FS &fs = SD_MMC; 

  //Note: Using the ESP32 and SD_Card readers requires a 1K to 4K7 pull-up to 3v3 on the MISO line, otherwise may not work
  if (!SD_MMC.begin())
  { 
    Serial.println(F("Card failed or not present, no SD Card data logging possible..."));
    SD_present = false; 
  } 
  else
  {
    Serial.println(F("Card initialised... file access enabled..."));
    SD_present = true; 
  }
  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  // Route for root / web page
  server.on("/", handle_OnConnect);
  server.on("/picture", newpic);
  server.on("/rotate", gotoRotate);
  server.on("/tr15", turnright15);
  server.on("/tr30", turnright30);
  server.on("/tr45", turnright45);
  server.on("/tl15", turnleft15);
  server.on("/tl30", turnleft30);
  server.on("/tl45", turnleft45);
  server.on("/neutral", neutral);
  server.on("/table", gotoTable);
  server.on("/sd",         SD_dir);


  server.begin();

}

void loop() {
  server.handleClient();
}

void handle_OnConnect() {
  timestamps = millis();
  server.send(200, "text/html", SendHTML()); 
}

void newpic(){
  capturePhotoSaveSD();
  server.send(200, "text/html", SendHTML());
}

void gotoRotate(){
  server.send(200, "text/html", Rotate());
}

void neutral(){
  server.handleClient();
  myservo.write(45);
  server.handleClient();
  server.send(200, "text/html", Rotate());
}

void turnright15(){
  server.handleClient();
  myservo.write(30);
  server.handleClient();
  server.send(200, "text/html", Rotate());
}

void turnright30(){
  server.handleClient();
  myservo.write(15);
  server.handleClient();
  server.send(200, "text/html", Rotate());
}

void turnright45(){
  server.handleClient();
  myservo.write(0);
  server.handleClient();

  server.send(200, "text/html", Rotate());
}

void turnleft15(){
  server.handleClient();
  myservo.write(60);
  server.handleClient();

  server.send(200, "text/html", Rotate());
}

void turnleft30(){
  server.handleClient();
  myservo.write(75);
  server.handleClient();

  server.send(200, "text/html", Rotate());
}

void turnleft45(){
  server.handleClient();
  myservo.write(90);
  server.handleClient();

  server.send(200, "text/html", Rotate());

}

void gotoTable(){
  server.send(200, "text/html", Table());
}

//Initial page of the server web, list directory and give you the chance of deleting and uploading
void SD_dir(){
  if (SD_present) 
  {
    

    if (server.args() > 0 ) //Arguments were received, ignored if there are not arguments
    { 
      Serial.println(server.arg(0));
  
      String Order = server.arg(0);
      Serial.println(Order);
      
      if (Order.indexOf("download_")>=0)
      {
        Order.remove(0,9);
        SD_file_download(Order);
        Serial.println(Order);
      }
  
      if ((server.arg(0)).indexOf("delete_")>=0)
      {
        Order.remove(0,7);
        SD_file_delete(Order);
        Serial.println(Order);
      }
    }
    Serial.println("yo");
    File root = SD_MMC.open("/");
    if (root) {
      root.rewindDirectory();
      SendHTML_Header();    
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
    SendHTML_Stop();   //Stop is needed because no content length was sent
  } else ReportSDNotPresent();
}

void printDirectory(const char * dirname, uint8_t levels){
  
  File root = SD_MMC.open(dirname);

  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }
  File file = root.openNextFile();

  int i = 0;
  while(file){
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    if(file.isDirectory()){
      webpage += "<tr><td>"+String(file.isDirectory()?"Dir":"File")+"</td><td>"+String(file.name())+"</td><td></td></tr>";
      printDirectory(file.name(), levels-1);
    }
    else
    {
      webpage += "<tr><td>"+String(file.name())+"</td>";
      webpage += "<td>"+String(file.isDirectory()?"Dir":"File")+"</td>";
      int bytes = file.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes)+" B";
      else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
      else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
      else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
      webpage += "<td>"+fsize+"</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/' method='post'>"); 
      webpage += F("<button type='submit' name='download'"); 
      webpage += F("' value='"); webpage +="download_"+String(file.name()); webpage +=F("'>Download</button>");
      webpage += "</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/' method='post'>"); 
      webpage += F("<button type='submit' name='delete'"); 
      webpage += F("' value='"); webpage +="delete_"+String(file.name()); webpage +=F("'>Delete</button>");
      webpage += "</td>";
      webpage += "</tr>";

    }
    file = root.openNextFile();
    i++;
  }
  file.close();

 
}

void SD_file_download(String filename)
{
  Serial.println("going to dl");
  if (SD_present) 
  { 
    File download = SD_MMC.open("/"+filename);
    if (download) 
    {
      Serial.println("if dl");
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } else ReportFileNotPresent("download"); 
  } else ReportSDNotPresent();
}

void SD_file_delete(String filename) 
{ 
  if (SD_present) { 
    SendHTML_Header();
    File dataFile = SD_MMC.open("/"+filename, FILE_READ); //Now read data from SD Card 
    if (dataFile)
    {
      if (SD.remove("/"+filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '"+filename+"' has been erased</h3>"; 
        webpage += F("<a href='/'>[Back]</a><br><br>");
      }
      else
      { 
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='/'>[Back]</a><br><br>");
      }
    } else ReportFileNotPresent("delete");
    append_page_footer(); 
    SendHTML_Content();
    SendHTML_Stop();
  } else ReportSDNotPresent();
} 

//SendHTML_Header
void SendHTML_Header()
{
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  server.send(200, "text/html", ""); //Empty content inhibits Content-length header so we have to close the socket ourselves. 
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}

//SendHTML_Content
void SendHTML_Content()
{
  server.sendContent(webpage);
  webpage = "";
}

//SendHTML_Stop
void SendHTML_Stop()
{
  server.sendContent("");
  server.client().stop(); //Stop is needed because no content length was sent
}

//ReportSDNotPresent
void ReportSDNotPresent()
{
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>"); 
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//ReportFileNotPresent
void ReportFileNotPresent(String target)
{
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

void ReportCouldNotCreateFile(String target)
{
  SendHTML_Header();
  webpage += F("<h3>Could Not Create Uploaded File (write-protected?)</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//File size conversion
String file_size(int bytes)
{
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}
void capturePhotoSaveSD( void ) {
  
  server.handleClient();

  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
    
  camera_fb_t * fb = NULL;
  
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

  // Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) +".jpg";

  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path.c_str());
  
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();
  esp_camera_fb_return(fb); 
  
  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);
  
}

String SendHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>WebServer</title>\n";
  ptr +="<style>html { font-family: Bahnschrift SemiLight Condensed; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: black;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #690;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Group 24</h1>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/sd\">SD card</a>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/picture\">Capture Picture</a>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/rotate\">Rotate the Camera </a>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/table\">Sensor results </a>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Table(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Table of results</title>\n";
  ptr +="<style>html { font-family: Bahnschrift SemiLight Condensed; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #690005;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="table {border-collapse: collapse;width: 100%;}\n";
  ptr +="th, td {text-align: left;padding: 8px;}\n";
  ptr +="th {background-color: #4CAF50;color: white;}\n";
  ptr +="tr:nth-child(even) {background-color: #f2f2f2;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Table of results</h1>\n";
  ptr +="<table>\n";
  ptr +="<tr>\n";
  ptr +="<th>Temperature </th>\n";
  ptr +="<th>Humidity </th>\n";
  ptr +="</tr>\n";
  ptr +="<tr>\n";
  ptr +="<td>Row 1, Column 1</td>\n";
  ptr +="<td>Row 1, Column 2</td>\n";
  ptr +="</tr>\n";
  ptr +="</table>\n";
  ptr +="<p>Return to Home Page: </p><a class=\"button button-off\" href=\"/\">Home</a>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Rotate(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Table of results</title>\n";
  ptr +="<style>html { font-family: Bahnschrift SemiLight Condensed; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #427BFF;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="table {border-collapse: collapse;width: 100%;}\n";
  ptr +="th, td {text-align: left;padding: 8px;}\n";
  ptr +="th {background-color: #4CAF50;color: white;}\n";
  ptr +="tr:nth-child(even) {background-color: #f2f2f2;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Rotating the camera</h1>\n";
  ptr +="<h2>Neutral Position:</h2>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/neutral\">0 degrees</a>\n";
  ptr +="<h2>Right Rotation:</h2>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/tr15\">15 degrees</a>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/tr30\">30 degrees</a>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/tr45\">45 degrees</a>\n";

  ptr +="<h2>Left Rotation:</h2>\n";

  ptr +="<p></p><a class=\"button button-off\" href=\"/tl15\">15 degrees</a>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/tl30\">30 degrees</a>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/tl45\">45 degrees</a>\n";

  ptr +="<p>Return to Home Page: </p><a class=\"button button-off\" href=\"/\">Home</a>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void append_page_header() {
  webpage  = F("<!DOCTYPE html><html>");
  webpage += F("<head>");
  webpage += F("<title>SD card</title>"); // NOTE: 1em = 16px
  webpage += F("<meta name='viewport' content='user-scalable=yes,initial-scale=1.0,width=device-width'>");
  webpage += F("<style>");//From here style:
  webpage += F("body{max-width:65%;margin:0 auto;font-family:arial;font-size:100%;}");
  webpage += F("ul{list-style-type:none;padding:0;border-radius:0em;overflow:hidden;background-color:#d90707;font-size:1em;}");
  webpage += F("li{float:left;border-radius:0em;border-right:0em solid #bbb;}");
  webpage += F("li a{color:white; display: block;border-radius:0.375em;padding:0.44em 0.44em;text-decoration:none;font-size:100%}");
  webpage += F("li a:hover{background-color:#e86b6b;border-radius:0em;font-size:100%}");
  webpage += F("h1{color:white;border-radius:0em;font-size:1.5em;padding:0.2em 0.2em;background:#d90707;}");
  webpage += F("h2{color:blue;font-size:0.8em;}");
  webpage += F("h3{font-size:0.8em;}");
  webpage += F("table{font-family:arial,sans-serif;font-size:0.9em;border-collapse:collapse;width:85%;}"); 
  webpage += F("th,td {border:0.06em solid #dddddd;text-align:left;padding:0.3em;border-bottom:0.06em solid #dddddd;}"); 
  webpage += F("tr:nth-child(odd) {background-color:#eeeeee;}");
  webpage += F(".rcorners_n {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%;color:white;font-size:75%;}");
  webpage += F(".rcorners_m {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:50%;color:white;font-size:75%;}");
  webpage += F(".rcorners_w {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:70%;color:white;font-size:75%;}");
  webpage += F(".column{float:left;width:50%;height:45%;}");
  webpage += F(".row:after{content:'';display:table;clear:both;}");
  webpage += F("*{box-sizing:border-box;}");
  webpage += F("a{font-size:75%;}");
  webpage += F("p{font-size:75%;}");
  webpage += F("</style></head><body><h1>Sd </h1>");
  webpage += F("<ul>");
  webpage += F("<li><a href='/'>Files</a></li>"); //Menu bar with commands
  webpage += F("<li><a href='/upload'>Configuration</a></li>"); 
  webpage += F("</ul>");
}
//Saves repeating many lines of code for HTML page footers 
void append_page_footer()
{ 
  webpage += F("</body></html>");
}


