#include <DHT.h>
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32

#include "soc/soc.h"           // Disable brownour problems //cc
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems//cc
#include "driver/rtc_io.h" //cc
#include <EEPROM.h>            // read and write from flash memory//cc

typedef struct {
  float temperature;
  float humidity;
} mySensor_t;

#define EEPROM_SIZE 1 //define EEprom to 1 byte, will be able to gen 256 picture numbers

// Pin definition for CAMERA_MODEL_AI_THINKER //cc
#define PWDN_GPIO_NUM     32 //
#define RESET_GPIO_NUM    -1//
#define XCLK_GPIO_NUM      0//
#define SIOD_GPIO_NUM     26//
#define SIOC_GPIO_NUM     27//
  
#define Y9_GPIO_NUM       35//
#define Y8_GPIO_NUM       34//
#define Y7_GPIO_NUM       39//
#define Y6_GPIO_NUM       36//
#define Y5_GPIO_NUM       21//
#define Y4_GPIO_NUM       19//
#define Y3_GPIO_NUM       18//
#define Y2_GPIO_NUM        5//
#define VSYNC_GPIO_NUM    25//
#define HREF_GPIO_NUM     23//
#define PCLK_GPIO_NUM     22 // cc

#define DHTPIN 13
#define DHTTYPE DHT11
int count = 0; 
DHT dht(DHTPIN, DHTTYPE);

 
void setup() {

  Serial.begin(115200);
  pinMode(12,INPUT); // this initialises the pin for the motion sensor 
  pinMode(14,OUTPUT);/// initialises the pin for the servo
  dht.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  camera_fb_t * fb = NULL;
  delay(3000);
  if (digitalRead(12)==HIGH){
      delay(2000);
      Serial.println("Motion detected");
      fb = esp_camera_fb_get();  
      if (fb) {
         Serial.println("Camera capture success");
         esp_camera_fb_return(fb);
         delay(1000);
      }
      else if(!fb) {
       Serial.println("Camera capture failed");
      }
      mySensor_t sensorData; // Declare a variable of type mySensor_t
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
}
