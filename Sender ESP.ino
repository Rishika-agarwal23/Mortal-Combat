#include <WiFi.h>
#include <esp_now.h>

// Replace with receiver's MAC address
uint8_t receiver[] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC};

int number = 0;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  esp_now_init();

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, receiver, 6);
  esp_now_add_peer(&peer);
}

void loop() {
  number++;

  esp_now_send(receiver, (uint8_t *)&number, sizeof(number));

  Serial.print("Sent: ");
  Serial.println(number);

  delay(1000);
}
