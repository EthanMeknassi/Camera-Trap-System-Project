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

Servo myservo; 

typedef struct {
  float temperature;
  float humidity;
} mySensor_t; 

#define EEPROM_SIZE 1

// Replace with your network credentials
const char* ssid = "ESP32";
const char* password = "12345678";

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

#define DHTPIN 2 
#define DHTTYPE DHT11
int count = 0; 
mySensor_t sensorData;

#define PART_BOUNDARY "123456789000000000000987654321"

DHT dht(DHTPIN, DHTTYPE);
// Create AsyncWebServer object on port 80
WebServer server(80);
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
  myservo.attach(13);
  pinMode(12,INPUT); // this initialises the pin for the motion sensor 
  pinMode(14,OUTPUT);/// initialises the pin for the servo
  dht.begin();  
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.softAPIP());
  Serial.println("' to connect");
  delay(100);
  server.begin();
  Serial.println("HTTP server started");

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
  server.on("/seepic", handlepic);
  server.on("/rotate", gotoRotate);
  server.on("/tr15", turnright15);
  server.on("/tr30", turnright30);
  server.on("/tr45", turnright45);
  server.on("/tl15", turnleft15);
  server.on("/tl30", turnleft30);
  server.on("/tl45", turnleft45);
  server.on("/neutral", neutral);
  server.on("/table", gotoTable);

  server.begin();

}

void handle_OnConnect() {
  timestamps = millis();
  server.send(200, "text/html", SendHTML()); 
}

void newpic(){
  capturePhotoSaveSD();
  server.send(200, "text/html", SendHTML());
}

void handlepic(){
  File file = SD_MMC.open(FILE_PHOTO, FILE_READ);
  server.streamFile(file, String("text/plain"));
  file.close();
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

void loop(){
    if (digitalRead(12)==HIGH){
      capturePhotoSaveSD(); //takes a picture and saves it on SD card
      Serial.println("Motion detected");
      sensorData.temperature = dht.readTemperature();
      sensorData.humidity = dht.readHumidity();
      Serial.print("T = ");
      Serial.print(sensorData.temperature);
      Serial.println(" degrees");
      Serial.print("Humidity = ");
      Serial.print(sensorData.humidity);
      Serial.println(" %");
  }
  else if (digitalRead(12) == LOW){
    Serial.println("No motion detected");
  }
  server.send(200, "text/html", Table(sensorData.humidity, sensorData.temperature));
}

void loop() {
  server.handleClient();
  delay(1000);
}
// Capture Photo and Save it to SD
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
  ptr +="<p></p><a class=\"button button-off\" href=\"/seepic\">SD card</a>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/picture\">Capture Picture</a>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/rotate\">Rotate the Camera </a>\n";
  ptr +="<p></p><a class=\"button button-off\" href=\"/table\">Sensor results </a>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Table(uint8_t humidity, uint8_t temperature){
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
  ptr +="<th>Humidity </th>\n";
  ptr +="<th>Temperature </th>\n";
  ptr +="</tr>\n";
  ptr +="<tr>\n";
  ptr +="<td>"+String(humidity)+"</td>\n";
  ptr +="<td>"+String(temperature)+"</td>\n";
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
