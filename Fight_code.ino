// SECTION 1: CONFIGURATION & LIBRARIES
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <esp_now.h>
#include <WiFi.h>

#define PLAYER_NUMBER 1
uint8_t opponentAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

// SECTION 2: HARDWARE & COMBAT VARIABLES
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_MPU6050 mpu;

const int punchLedPin = 18;
const int specialLedPin = 19;

String myName = (PLAYER_NUMBER == 1) ? "Knox" : "Tyrant";
String enemyName = (PLAYER_NUMBER == 1) ? "Tyrant" : "Knox";

int myHP = 100;
int enemyHP = 100;
bool gameOver = false;

int frame = 0;
unsigned long lastFrameTime = 0;
const int frameDelay = 300; 

unsigned long lastAttackTime = 0;
const int attackCooldown = 1000;
bool isAttacking = false;
bool weak = false;

typedef struct struct_message {
  int attackType;
  int damage;
} struct_message;

struct_message outgoingAttack;
struct_message incomingAttack;
esp_now_peer_info_t peerInfo;

// SECTION 3: EXTERNAL CHARACTER REFERENCE
// Ensure your drawNinja function is defined in another tab (e.g., Character.ino)
extern void drawNinja(int x, int y, bool weak);

// SECTION 4: NETWORK COMMUNICATION
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

// ESP-NOW Compatibility check for ESP32 Core v3.x vs older versions
#if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
#else
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif
  memcpy(&incomingAttack, incomingData, sizeof(incomingAttack));
  
  myHP -= incomingAttack.damage;
  if (myHP < 0) myHP = 0;

  if (incomingAttack.attackType == 1) {
    digitalWrite(punchLedPin, HIGH); delay(100); digitalWrite(punchLedPin, LOW);
  } else if (incomingAttack.attackType == 2) {
    digitalWrite(specialLedPin, HIGH); delay(200); digitalWrite(specialLedPin, LOW);
  }
}

// SECTION 5: INITIALIZATION
void setup() {
  Serial.begin(115200);

  pinMode(punchLedPin, OUTPUT);
  pinMode(specialLedPin, OUTPUT);

  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while(1); // Halt if OLED fails
  }
  
  if (!mpu.begin()) {
    while(1); // Halt if MPU fails
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;

  esp_now_register_send_cb((esp_now_send_cb_t)OnDataSent);
  esp_now_register_recv_cb((esp_now_recv_cb_t)OnDataRecv);

  memcpy(peerInfo.peer_addr, opponentAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(20, 25);
  display.println("FIGHT!");
  display.display();
  delay(2000);
}

// SECTION 6: MAIN COMBAT LOOP (LOGIC & SYNC)
void loop() {
  if (gameOver) return;

  display.clearDisplay();
  unsigned long currentTime = millis();
  
  weak = (myHP <= 15);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Motion Detection Engine
  if (!weak && currentTime - lastAttackTime > attackCooldown) {
    
    // Punch Detected (Forward Acceleration)
    if (abs(a.acceleration.x) > 15.0) { 
      frame = 3; 
      isAttacking = true;
      transmitAttack(1, 5); 
      lastAttackTime = currentTime;
    }
    // Special Move Detected (Wrist Twist)
    else if (abs(g.gyro.z) > 4.0) {
      frame = 2; 
      isAttacking = true;
      transmitAttack(2, 8); 
      lastAttackTime = currentTime;
    }
  }

  // Animation Synchronizer
  if (isAttacking) {
    if (currentTime - lastAttackTime > 300) {
      isAttacking = false;
      frame = 0;
    }
  } else if (!weak) {
    if (currentTime - lastFrameTime >= frameDelay) {
      frame = (frame == 0) ? 1 : 0; 
      lastFrameTime = currentTime;
    }
  }

  // Draw Central Character
  drawNinja(55, 20, weak);

  // Draw Top UI: Enemy Health Bar
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(enemyName);
  display.print(" HP: ");
  display.print(enemyHP);
  display.drawRect(70, 0, 52, 8, WHITE);
  int enemyFill = map(enemyHP, 0, 100, 0, 50);
  if(enemyFill > 0) display.fillRect(71, 1, enemyFill, 6, WHITE);

  // Draw Bottom UI: My Health Bar
  display.drawRect(30, 48, 70, 8, WHITE);
  int fill = map(myHP, 0, 100, 0, 68);
  if(fill > 0) display.fillRect(31, 49, fill, 6, WHITE);
  display.setCursor(45, 57);
  display.print(myName);

  display.display();

  // Win/Loss Condition Check
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
    display.setCursor(30, 50);
    display.println("FATALITY");
    display.display();
  }

  delay(20);
}
