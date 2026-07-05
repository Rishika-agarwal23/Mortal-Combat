#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// 🔴 MASTER (KNIGHT) ESP32 KA MAC ADDRESS YAHAN DAALEIN
uint8_t broadcastAddress[] = {0xb0, 0xcb, 0xd8, 0x0d, 0x02, 0xb4};

typedef struct struct_message {
    bool ninjaAttacked;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// Sensitivity setting (24.0 completely filters gravity and vibrations)
const float shakeThreshold = 24.0; 
unsigned long lastAttackTime = 0;

// Fixed data sent callback for ESP32 Board Manager 3.x+
void OnDataSent(const wifi_tx_info_t *txInfo, esp_now_send_status_t status) {
  // Serial debugging if needed
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (!mpu.begin()) {
    while (1) { delay(10); }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  if (esp_now_init() != ESP_OK) {
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    return;
  }
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Total movement calculation
  float totalAccel = sqrt(a.acceleration.x * a.acceleration.x + 
                          a.acceleration.y * a.acceleration.y + 
                          a.acceleration.z * a.acceleration.z);

  // Physical shake check (600ms gap to avoid double triggers)
  if (totalAccel > shakeThreshold && (millis() - lastAttackTime > 600)) {
    myData.ninjaAttacked = true;
    lastAttackTime = millis();
    
    // Send instant trigger to Master
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("💥 Ninja Attacked! Signal Sent to Master.");
    }
  } else {
    myData.ninjaAttacked = false;
  }

  delay(20); // Stability delay
}