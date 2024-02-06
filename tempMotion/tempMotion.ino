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
#include <EEPROM.h>    

typedef struct {
  float temperature;
  float humidity;
} mySensor_t;     

const char* ssid = "ESP32";
const char* password = "12345678";

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

#define DHTPIN 13 
#define DHTTYPE DHT11
int count = 0; 
mySensor_t sensorData; // Declare a variable of type mySensor_t

DHT dht(DHTPIN, DHTTYPE);

WebServer server(80);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector

  // Serial port for debugging purposes
  Serial.begin(115200);
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

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  server.on("/", handle_OnConnect);
  server.on("/rotate", gotoRotate);
  server.on("/table", gotoTable);

  server.begin();

}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML()); 
}

void gotoRotate(){
  server.send(200, "text/html", Rotate());
}

void gotoTable(){
    if (digitalRead(12)==HIGH){
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

