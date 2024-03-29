// ------------------------------------ WEBSERVER -----------------------------------------------

// Necessary includes
#include <WiFi.h>
#include <WebServer.h>

// SSID & Password for AP connection
const char* ssid = "ESP33"; 
const char* password = "12345678";

// Put IP Address details
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// Start webserver on port 80
WebServer server(80);
uint8_t LED1pin = 4;
bool LED1status = LOW;
float weight = 0;
float temp = 0;
// ------------------------------------ SETUP -----------------------------------------------

void setup() {

  // Set baud rate
  Serial.begin(115200);

  // Webserver setup
  pinMode(LED1pin, OUTPUT);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.softAPIP());
  Serial.println("' to connect");
  delay(100);
  server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

// ------------------------------------ LOOP -----------------------------------------------

void loop() {
  delay(500);
  server.handleClient();
  if(LED1status)
  {digitalWrite(LED1pin, HIGH); weight = 234; temp = 400;}
  else
  {digitalWrite(LED1pin, LOW); weight = 1; temp = 0;}
}

void handle_OnConnect() {
  LED1status = LOW;
  Serial.println("GPIO4 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status, weight, temp)); 
}

void handle_led1on() {
  LED1status = HIGH;
  Serial.println("GPIO4 Status: ON");
  server.send(200, "text/html", SendHTML(true, weight, temp)); 
}

void handle_led1off() {
  LED1status = LOW;
  Serial.println("GPIO4 Status: OFF");
  server.send(200, "text/html", SendHTML(false, weight, temp)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}



String SendHTML(uint8_t led1stat, uint8_t weight, uint8_t temp){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
   if(led1stat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";}
  else
  {ptr +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";}
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}