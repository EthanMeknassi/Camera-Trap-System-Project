#include <dht11.h>  ////library for dht11 from arduino
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32

#include "soc/soc.h"           // Disable brownour problems //cc
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems//cc
#include "driver/rtc_io.h" //cc
#include <EEPROM.h>            // read and write from flash memory//cc



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


#define DHT11PIN 2 // pin for dht11
dht11 DHT11;

int count = 0; // define counter so that i can keep track of number of pictures and what data goes with what


 
void setup() {
  //// problem 1 - camera runs off 115400
///cc below
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
  //cc anove
  config.frame_size = FRAMESIZE_SVGA; // 8000x600
  config.jpeg_quality = 12; // 0-63, the lower the number the better the quality
  config.fb_count = 1; //frame buffer has effect on resolution


     // Init Camera
  esp_err_t err = esp_camera_init(&config);
  camera_fb_t * fb = NULL;
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(12,INPUT); // this initialises the pin for the motion sensor 
  //pinmode(14,OUTPUT)/// initialises the pin for the servo
}

 
void loop() {
  // put your main code here, to run repeatedly:
   camera_fb_t * fb = NULL;
   if (digitalRead(12)==HIGH){
       Serial.println("Motion detected");
      fb = esp_camera_fb_get();  
      if (fb) {
         Serial.println("Camera capture success");
         esp_camera_fb_return(fb);
         delay(1000); // cc 
             }
      else if(!fb) {
       Serial.println("Camera capture failed");
    }
  }
  else {
    Serial.println("No motion detected");
  }
}

