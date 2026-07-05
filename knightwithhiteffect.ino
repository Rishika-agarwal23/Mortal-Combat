#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <esp_now.h>
#include <WiFi.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_MPU6050 mpu;

// 🔴 REMOTE (NINJA) ESP32 KA MAC ADDRESS YAHAN DAALEIN
uint8_t remoteMac[] = {0xb0, 0xcb, 0xd8, 0x0c, 0xd6, 0x6c};

typedef struct struct_message {
    bool ninjaAttacked;
} struct_message;
struct_message incomingData;

// Animation Tickers
int idle_frame = 0;
unsigned long lastFrameTime = 0;
int frameDelay = 250;

int knight_energy = 100;
int ninja_energy = 100;

// Attack States
unsigned long knight_attackTimer = 0;
bool knight_isAttacking = false;
int knight_attackFrame = 0;

unsigned long ninja_attackTimer = 0;
bool ninja_isAttacking = false;
int ninja_attackFrame = 0;

// 🛠️ NEW: Hurt/Impact States
bool knight_isHurt = false;
unsigned long knight_hurtTimer = 0;
bool ninja_isHurt = false;
unsigned long ninja_hurtTimer = 0;

const float shakeThreshold = 24.0; 
unsigned long lastLocalAttackTime = 0;

// --- IMPACT EFFECT (💥 HIT SPARK) ---
void drawHitSpark(int x, int y) {
  display.drawLine(x-4, y-4, x+4, y+4, WHITE);
  display.drawLine(x+4, y-4, x-4, y+4, WHITE);
  display.drawLine(x, y-6, x, y+6, WHITE);
  display.drawLine(x-6, y, x+6, y, WHITE);
}

// --- KNIGHT GRAPHICS ---
void drawSword(int x, int y, bool attack) {
  if(attack) {
    display.drawLine(x, y, x+18, y-12, WHITE);
    display.drawLine(x, y+2, x+18, y-10, WHITE);
  } else {
    display.drawLine(x, y, x+10, y-8, WHITE);
  }
}

void drawShield(int x, int y) {
  display.drawCircle(x, y, 5, WHITE);
  display.drawLine(x-3, y+3, x+3, y+3, WHITE);
}

void drawKnight(int x, int y, bool weak, bool attackActive, int currentActFrame, bool hurtActive) {
  int head=y; int body=y+7; int leg=y+18; int ground=y+25;
  int px=x;

  // 🛠️ IMPACT ANIMATION: Hurt hone par Knight peeche jhatka khayega aur hil jayega
  if(hurtActive) {
    px -= 6; // Knockback to left
    display.drawCircle(px, head+2, 4, WHITE); // Head down
    display.drawLine(px-3, body+2, px+3, body+2, WHITE); // Shaking shoulders
    display.drawLine(px, body+3, px, leg, WHITE);
    drawHitSpark(px+8, body+2); // Hit spark on chest
    return;
  }

  if(weak) {
    display.drawCircle(px, head, 4, WHITE);
    display.drawLine(px, body, px, leg, WHITE);
    return;
  }
  
  if(!attackActive) {
    if(idle_frame==1) px-=2;
    if(idle_frame==2) px+=2;
  }

  display.drawCircle(px, head, 5, WHITE);
  display.drawLine(px-5, head, px+5, head, WHITE);
  display.drawRect(px-5, body, 10, 10, WHITE);
  display.drawLine(px-3, leg, px-5, ground, WHITE);
  display.drawLine(px+3, leg, px+6, ground, WHITE);
  drawShield(px-8, body+5);

  if(attackActive) {
    if(currentActFrame == 1) {
      drawSword(px+5, body+2, true);
    } 
    else if(currentActFrame == 2) {
      drawSword(px+12, body-2, true); // Overextended Forward Slash
      display.drawLine(px+25, body-12, px+32, body-12, WHITE);
    }
  } else {
    drawSword(px+5, body+2, false);
  }
}

// --- NINJA GRAPHICS ---
void drawNinja(int x, int y, bool weak, bool attackActive, int currentActFrame, bool hurtActive) {
  int head = y; int body = y + 6; int leg = y + 17; int ground = y + 25;
  int px = x;

  // 🛠️ IMPACT ANIMATION: Hurt hone par Ninja peeche jhatka khayega aur disbalance hoga
  if(hurtActive) {
    px += 6; // Knockback to right
    display.drawCircle(px, head+2, 3, WHITE);
    display.drawLine(px, body+2, px+4, leg, WHITE); // Slanted body
    display.drawLine(px+4, leg, px+1, ground, WHITE);
    drawHitSpark(px-8, body+2); // Hit spark on front
    return;
  }

  if(weak) {
    display.drawCircle(px, head, 3, WHITE);
    display.drawLine(px, body, px, leg, WHITE);
    display.drawLine(px, leg, x-3, ground, WHITE);
    display.drawLine(px, leg, x+3, ground, WHITE);
    return;
  }

  if(!attackActive) {
    if(idle_frame==1) px+=2;
    if(idle_frame==2) px-=2;
  }

  // Dash/Teleport Forward during attack to close the gap!
  if(attackActive && currentActFrame == 1) {
    px-=15; // Moves closer to Knight for Katana Slash!
  }
  if(attackActive && currentActFrame == 2) {
    px-=5;
    display.drawLine(px+18, body, px+8, body, WHITE);
  }

  display.drawCircle(px, head, 3, WHITE);
  display.drawLine(px-2, head, px+2, head, WHITE); 
  display.drawLine(px, body, px, leg, WHITE);
  display.drawLine(px, body+2, px-5, body+5, WHITE); 
  display.drawLine(px, body+2, px+5, body+5, WHITE); 

  display.drawLine(px, leg, px-3, ground, WHITE);
  display.drawLine(px, leg, px+3, ground, WHITE);

  if(attackActive) {
    if(currentActFrame == 1) { 
      display.drawLine(px-5, body+2, px-18, body-8, WHITE);
      display.drawLine(px-18, body-8, px-28, body-8, WHITE);
    } 
    else if(currentActFrame == 2) { 
      display.drawLine(px, body+2, px-8, body+2, WHITE); 
      int sx = px - 18; int sy = body + 2;
      display.drawPixel(sx, sy, WHITE); display.drawPixel(sx-2, sy, WHITE);
      display.drawPixel(sx+2, sy, WHITE); display.drawPixel(sx, sy-2, WHITE);
    }
  } else {
    display.drawLine(px-5, body+2, px-10, body+8, WHITE); 
  }
}

// ESP-NOW Receive (Ninja Attacks Master)
void OnDataRecv(const esp_now_recv_info *recvInfo, const uint8_t *incomingDataPtr, int len) {
  memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));
  if(incomingData.ninjaAttacked && knight_energy > 0 && ninja_energy > 0 && !ninja_isAttacking) {
    ninja_isAttacking = true;
    ninja_attackTimer = millis();
    ninja_attackFrame = random(1, 3);
    
    // 🛠️ Trigger Knight Hurt State
    knight_isHurt = true;
    knight_hurtTimer = millis();
    
    knight_energy -= 10; 
    if(knight_energy < 0) knight_energy = 0;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { for(;;); }
  
  if (!mpu.begin()) { while (1) { delay(10); } }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  display.clearDisplay();

  if (knight_energy <= 0 || ninja_energy <= 0) {
    display.setTextSize(1); display.setTextColor(WHITE); display.setCursor(30, 25);
    if(knight_energy <= 0) display.print("NINJA WINS!");
    else display.print("KNIGHT WINS!");
    display.display();
    delay(3000);
    knight_energy = 100; ninja_energy = 100;
    return;
  }

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float totalAccel = sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z);

  // Knight Attacks Ninja
  if (totalAccel > shakeThreshold && !knight_isAttacking && (millis() - lastLocalAttackTime > 600)) {
    knight_isAttacking = true;
    knight_attackTimer = millis();
    knight_attackFrame = random(1, 3);
    lastLocalAttackTime = millis();
    
    // 🛠️ Trigger Ninja Hurt State
    ninja_isHurt = true;
    ninja_hurtTimer = millis();
    
    ninja_energy -= 10; 
    if(ninja_energy < 0) ninja_energy = 0;
  }

  // Timers to reset action states
  if (knight_isAttacking && (millis() - knight_attackTimer > 350)) knight_isAttacking = false;
  if (ninja_isAttacking && (millis() - ninja_attackTimer > 350)) ninja_isAttacking = false;
  
  // 🛠️ Hurt display durations (250ms tak impact screen par dikhega)
  if (knight_isHurt && (millis() - knight_hurtTimer > 250)) knight_isHurt = false;
  if (ninja_isHurt && (millis() - ninja_hurtTimer > 250)) ninja_isHurt = false;

  // Idle cycle
  if (millis() - lastFrameTime > frameDelay) {
    idle_frame++; 
    if(idle_frame > 2) idle_frame = 0; 
    lastFrameTime = millis();
  }

  // Draw Players with new parameters
  drawKnight(30, 18, (knight_energy <= 15), knight_isAttacking, knight_attackFrame, knight_isHurt);
  drawNinja(95, 18, (ninja_energy <= 15), ninja_isAttacking, ninja_attackFrame, ninja_isHurt); 

  // UI Bars
  display.drawRect(5, 48, 45, 6, WHITE); 
  int k_fill = map(knight_energy, 0, 100, 0, 43);
  if(k_fill > 0) display.fillRect(6, 49, k_fill, 4, WHITE);

  display.drawRect(78, 48, 45, 6, WHITE); 
  int n_fill = map(ninja_energy, 0, 100, 0, 43);
  if(n_fill > 0) display.fillRect(79, 49, n_fill, 4, WHITE);

  display.setTextSize(1); display.setTextColor(WHITE);
  display.setCursor(5, 56);   display.print("KNIGHT");
  display.setCursor(92, 56);  display.print("NINJA");
  display.display();
}
