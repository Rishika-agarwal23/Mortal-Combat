#include <WiFi.h>

void setup(){
  Serial.begin(115200);
  
  // Set Wi-Fi to Station mode to get the MAC address
  WiFi.mode(WIFI_MODE_STA);
  
  Serial.println();
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop(){
  // Nothing needed in the loop
}