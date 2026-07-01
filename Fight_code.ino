#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <esp_now.h>
#include <WiFi.h>

/* ================= CONFIGURATION ================= */
#define PLAYER_NUMBER 1

// Target MAC Address of the opposing ESP32
uint8_t opponentAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
/* ================================================= */

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_MPU6050 mpu;

const int punchLedPin = 18;
const int specialLedPin = 19;

String myName = (PLAYER_NUMBER == 1) ? "Scorpion" : "Sub-Zero";
String enemyName = (PLAYER_NUMBER == 1) ? "Sub-Zero" : "Scorpion";

int myHP = 100;
int enemyHP = 100;
bool gameOver = false;

unsigned long lastAttackTime = 0;
const int attackCooldown = 1000;

typedef struct struct_message {
  int attackType;
  int damage;
} struct_message;

struct_message outgoingAttack;
struct_message incomingAttack;
esp_now_peer_info_t peerInfo;

/*
 * Network Callbacks
 * Handles transmission success state and incoming damage packets
 */
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Packet delivered.");
  } else {
    Serial.println("Packet delivery failed.");
  }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingAttack, incomingData, sizeof(incomingAttack));
  
  myHP -= incomingAttack.damage;
  if (myHP < 0) myHP = 0;

  if (incomingAttack.attackType == 1) {
    digitalWrite(punchLedPin, HIGH);
    delay(100);
    digitalWrite(punchLedPin, LOW);
  } else if (incomingAttack.attackType == 2) {
    digitalWrite(specialLedPin, HIGH);
    delay(200);
    digitalWrite(specialLedPin, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(punchLedPin, OUTPUT);
  pinMode(specialLedPin, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Display initialization failed.");
    while(1);
  }
  
  if (!mpu.begin()) {
    Serial.println("Sensor initialization failed.");
    while(1);
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed.");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peerInfo.peer_addr, opponentAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Peer registration failed.");
    return;
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.println("FIGHT!");
  display.display();
  delay(2000);
}

void loop() {
  if (gameOver) return;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  unsigned long currentTime = millis();

  /*
   * Motion Detection Engine
   * Evaluates accelerometer and gyroscope data against established thresholds
   * to determine if an attack intent is registered.
   */
  if (currentTime - lastAttackTime > attackCooldown) {
    
    if (abs(a.acceleration.x) > 15.0) { 
      transmitAttack(1, 5); 
      lastAttackTime = currentTime;
    }
    else if (abs(g.gyro.z) > 4.0) {
      transmitAttack(2, 8); 
      lastAttackTime = currentTime;
    }
  }

  updateInterface();
  evaluateMatchState();
  delay(20); 
}

/*
 * Packet Transmission
 * Dispatches combat data to the opponent and updates local predictive state
 */
void transmitAttack(int type, int damage) {
  outgoingAttack.attackType = type;
  outgoingAttack.damage = damage;
  
  esp_err_t result = esp_now_send(opponentAddress, (uint8_t *) &outgoingAttack, sizeof(outgoingAttack));
  
  if (result == ESP_OK) {
    enemyHP -= damage; 
    if (enemyHP < 0) enemyHP = 0;
    
    if (type == 1) {
      digitalWrite(punchLedPin, HIGH); 
      delay(150); 
      digitalWrite(punchLedPin, LOW);
    } else {
      digitalWrite(specialLedPin, HIGH); 
      delay(300); 
      digitalWrite(specialLedPin, LOW);
    }
  }
}

/*
 * Graphics Rendering
 * Translates health integers into proportional pixel widths for the OLED
 */
void updateInterface() {
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print(myName);
  display.drawRect(0, 10, 52, 8, WHITE);
  int myBarWidth = map(myHP, 0, 100, 0, 50);
  display.fillRect(1, 11, myBarWidth, 6, WHITE);
  display.setCursor(0, 25);
  display.print("HP:"); display.print(myHP);

  display.setCursor(70, 0);
  display.print(enemyName);
  display.drawRect(70, 10, 52, 8, WHITE);
  int enemyBarWidth = map(enemyHP, 0, 100, 0, 50);
  display.fillRect(71, 11, enemyBarWidth, 6, WHITE);
  display.setCursor(70, 25);
  display.print("HP:"); display.print(enemyHP);

  display.display();
}

/*
 * Session Management
 * Halts the main loop and displays the final conclusion screen
 */
void evaluateMatchState() {
  if (myHP <= 0 || enemyHP <= 0) {
    gameOver = true;
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 20);
    
    if (myHP <= 0) {
      display.println("YOU LOSE!");
    } else {
      display.println("YOU WIN!");
    }
    
    display.setTextSize(1);
    display.setCursor(20, 50);
    display.println("FATALITY.");
    display.display();
  }
}