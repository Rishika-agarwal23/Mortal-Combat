#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- GAME VARIABLES ----------------
float playerX = 30;
float velocity = 1.5;

int energy = 100;
int frame = 0;

// Shifted base down slightly so the taller sword doesn't hit the top of the screen
const int baseY = 22; 

// ---------------- IMPROVED SWORD ----------------
// hx and hy are the exact coordinates of the character's hand
void drawSword(int hx, int hy, int mode)
{
  if (mode == 0) {
    // MODE 0: IDLE / WALK (Sword held pointing up)
    // Handle
    display.drawLine(hx, hy, hx, hy + 3, WHITE);
    // Crossguard
    display.drawLine(hx - 3, hy - 1, hx + 3, hy - 1, WHITE);
    // Thick Blade (3 lines wide)
    display.drawLine(hx - 1, hy - 2, hx - 1, hy - 12, WHITE);
    display.drawLine(hx,     hy - 2, hx,     hy - 14, WHITE); // Tip reaches highest
    display.drawLine(hx + 1, hy - 2, hx + 1, hy - 12, WHITE);
    
  } else if (mode == 1) {
    // MODE 1: ATTACK (Sword swung forward)
    // Handle
    display.drawLine(hx, hy, hx - 2, hy + 2, WHITE);
    // Crossguard
    display.drawLine(hx - 1, hy - 2, hx + 2, hy + 1, WHITE);
    // Thick Blade pointing forward
    display.drawLine(hx + 2, hy - 1, hx + 12, hy - 4, WHITE);
    display.drawLine(hx + 2, hy,     hx + 14, hy - 3, WHITE); // Tip reaches furthest
    display.drawLine(hx + 1, hy + 1, hx + 12, hy - 2, WHITE);
    
  } else if (mode == 2) {
    // MODE 2: EXHAUSTED (Sword planted downward into the ground)
    // Handle (above the hand)
    display.drawLine(hx, hy, hx, hy - 3, WHITE);
    // Crossguard
    display.drawLine(hx - 3, hy + 1, hx + 3, hy + 1, WHITE);
    // Thick Blade pointing down to the floor
    display.drawLine(hx - 1, hy + 2, hx - 1, hy + 10, WHITE);
    display.drawLine(hx,     hy + 2, hx,     hy + 12, WHITE); // Tip in the dirt
    display.drawLine(hx + 1, hy + 2, hx + 1, hy + 10, WHITE);
  }
}

// ---------------- CHARACTER ----------------
void drawCharacter(int x, int y, bool weak)
{
  int headY = y;
  int bodyY = y + 3;
  int legY  = y + 12;
  int groundY = y + 20;

  if (weak) {
    // ---------------- WEAK / KNEELING STATE ----------------
    // Head slouched forward and down
    display.drawCircle(x + 2, headY + 5, 3, WHITE); 
    
    // Curved back
    display.drawLine(x + 2, headY + 8, x - 2, legY + 2, WHITE); 
    
    // Left leg kneeling (knee on the ground)
    display.drawLine(x - 2, legY + 2, x + 3, groundY, WHITE); 
    display.drawLine(x + 3, groundY, x + 8, groundY, WHITE); 
    
    // Right leg bent upward slightly
    display.drawLine(x - 2, legY + 2, x - 5, groundY, WHITE); 
    
    // Arm reaching down to plant the sword
    int hx = x + 8;
    int hy = legY + 1;
    display.drawLine(x + 2, headY + 9, hx, hy, WHITE);
    
    // Draw sword in Mode 2 (Planted)
    drawSword(hx, hy, 2);
    
  } else {
    // ---------------- STANDING / ACTIVE STATE ----------------
    // Head and straight spine
    display.drawCircle(x, headY, 3, WHITE);
    display.drawLine(x, bodyY, x, legY, WHITE);

    int hx, hy, swordMode;

    switch (frame)
    {
      case 0: // IDLE
        // Legs normal stance
        display.drawLine(x, legY, x - 3, groundY, WHITE);
        display.drawLine(x, legY, x + 3, groundY, WHITE);
        
        // Arm bent slightly holding sword up
        hx = x + 5; 
        hy = bodyY + 4;
        display.drawLine(x, bodyY + 2, hx, hy, WHITE);
        
        swordMode = 0;
        break;

      case 1: // WALK
        // Legs wider stance
        display.drawLine(x, legY, x - 5, groundY, WHITE);
        display.drawLine(x, legY, x + 5, groundY, WHITE);
        
        // Arm same as idle
        hx = x + 5; 
        hy = bodyY + 4;
        display.drawLine(x, bodyY + 2, hx, hy, WHITE);
        
        swordMode = 0;
        break;

      case 2: // ATTACK
        // Legs in a lunge
        display.drawLine(x, legY, x - 4, groundY, WHITE);
        display.drawLine(x, legY, x + 6, groundY, WHITE);
        
        // Arm stretched forward
        hx = x + 8; 
        hy = bodyY + 2;
        display.drawLine(x, bodyY + 2, hx, hy, WHITE);
        
        swordMode = 1;
        break;
    }
    
    // Draw sword perfectly attached to the hand coordinates
    drawSword(hx, hy, swordMode);
  }
}

// ---------------- SETUP ----------------
void setup()
{
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    while (1);
  }
}

// ---------------- LOOP ----------------
void loop()
{
  display.clearDisplay();

  // Character feels weak when energy is low
  bool weak = (energy <= 15);

  if (weak) {
    frame = 0; 
  } else {
    // Movement only happens if the character isn't weak/kneeling
    playerX += velocity;
    if (playerX > 110) velocity = -1.5;
    if (playerX < 10)  velocity = 1.5;
  }

  drawCharacter((int)playerX, baseY, weak);

  // ---------------- ENERGY BAR ----------------
  display.drawRect(30, 48, 70, 8, WHITE);
  int fill = map(energy, 0, 100, 0, 68);
  
  // Prevent drawing a negative fill width when energy is 0
  if (fill > 0) {
    display.fillRect(31, 49, fill, 6, WHITE);
  }

  display.setCursor(45, 57);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("ENERGY");

  display.display();

  // Animation cycle
  if (!weak)
  {
    frame++;
    if (frame > 2) frame = 0;
  }

  // Energy drain system
  energy--;
  if (energy < 0) {
    energy = 100; // Reset energy once it bottoms out
  }

  delay(120);
}