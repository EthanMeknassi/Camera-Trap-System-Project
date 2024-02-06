#include <dht11.h>  ////library for dht11 from arduino
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32

#include "soc/soc.h"           // Disable brownour problems //cc
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems//cc
#include "driver/rtc_io.h" //cc
#include <EEPROM.h>            // read and write from flash memory//cc


void setup() {
  
  Serial.begin(115200);
  //pinMode(12,OUTPUT) ; // this initialises the pin for the motion sensor 
  pinMode(13,OUTPUT); /// initialises the pin for the servo
  //digitalWrite(13,HIGH);

///servo stuff
//this is controlled by the user, when an input is received from them the camera frame position wil change
//its controlled by a PWM pulse input to the servo
//20ms period, 1ms gets servo to the left, 1.5ms to the middle, 2ms to the right

}

void loop() {
  delay(1000);
  digitalWrite(13,HIGH);
  delayMicroseconds(2000);
  digitalWrite(13,LOW);
  delayMicroseconds(20000-2000);
  Serial.println("Right turn");
  

  //digitalWrite(13,HIGH);
  //delayMicroseconds(1000);
  //digitalWrite(13,LOW);
  //delayMicroseconds(20000-1000);
  //Serial.println("Left turn");


  //digitalWrite(13,HIGH);
  //delayMicroseconds(1500);
  //digitalWrite(13,LOW);
  //delayMicroseconds(20000-1500);
  //Serial.println("Middle turn");

}
  // put your main code here, to run repeatedly:
  