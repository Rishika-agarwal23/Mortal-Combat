# IoT Mortal Kombat Wearable Gloves 🥊

Welcome to the **IoT Mortal Kombat** project! This repository contains the source code for a pair of wearable, motion-controlled smart gloves. Using ESP32 microcontrollers and MPU6050 motion sensors, physical punches and wrist twists are translated into digital attacks, with real-time health synchronization handled wirelessly via the ESP-NOW protocol.

## Features
* **Motion-Controlled Combat:** Translates real physical strikes (forward acceleration and rotational twists) into distinct in-game attacks.
* **Lag-Free Wireless Play:** Utilizes the decentralized ESP-NOW protocol for direct, router-free communication between the two gloves.
* **Real-Time UI:** Synchronizes and displays dynamic health bars and player stats on I2C OLED screens.
* **Haptic/Visual Feedback:** Triggers localized LED flashes based on the type of attack landed.

## 🛠️ Hardware Requirements
To build this project, you will need the following for **each glove** (double this list for two players):
* 2x ESP32 Dev Module
  2x MPU6050 (6-DoF Accelerometer & Gyroscope)
* 2x 0.96" SSD1306 OLED Display (128x64)
* 2x Blue LED (Punch Indicator)
* 1x Red LED (Special Move Indicator)
* 2x 220Ω - 330Ω Resistors (for LEDs)
* Jumper wires & Breadboard/Perfboard
* Portable power source (e.g., small LiPo battery or USB power bank)

## 🔌 Wiring Guide
Both the OLED display and the MPU6050 communicate via the I2C bus.

| Component   | Pin Name | ESP32 Pin |
| :---        | :---     | :---      |
| **OLED / MPU** | SDA      | GPIO 21   |
| **OLED / MPU** | SCL      | GPIO 22   |
| **OLED / MPU** | VCC      | 3.3V      |
| **OLED / MPU** | GND      | GND       |
| **Blue LED** | Anode (+) | GPIO 18   |
| **Red LED** | Anode (+) | GPIO 19   |

*(Note: Ensure your LEDs are connected in series with a resistor to ground to protect the ESP32 pins).*

## 💻 Software Setup
### 1. Install Dependencies
Ensure you have the Arduino IDE installed along with the ESP32 board manager. Install the following libraries via the Library Manager:
* `Adafruit MPU6050`
* `Adafruit SSD1306`
* `Adafruit GFX Library`
* `Adafruit Unified Sensor`

### 2. Find Your MAC Addresses
ESP-NOW requires the boards to know each other's hardware MAC addresses.
1. Upload the provided `MAC_Address_Scanner.ino` sketch to **ESP32 #1**.
2. Open the Serial Monitor (115200 baud) and copy the printed MAC address.
3. Repeat the process for **ESP32 #2**.

### 3. Configure the Main Game Code
1. Open the main `Mortal_Kombat_ESP32_Glove.ino` sketch.
2. In the `CONFIGURATION` section at the top of the code, paste the opponent's MAC address (replace `:` with `0x` and commas). 
   * *Example: `uint8_t opponentAddress[] = {0x24, 0x6F, 0x28, 0xAE, 0x9C, 0x14};`*
3. Set `#define PLAYER_NUMBER 1`, compile, and upload to the first glove.
4. Change it to `#define PLAYER_NUMBER 2`, swap the MAC address for the other board, and upload to the second glove.

## 🎮 How to Play
Once both gloves are powered on, the OLEDs will display **"FIGHT!"** * **The Punch (5 Damage):** Thrust your hand sharply forward. The MPU6050 detects a high linear acceleration (`X > 15.0`) and flashes the Blue LED.
* **The Special (8 Damage):** Twist your wrist sharply. The gyroscope detects a high rotational velocity (`Z > 4.0`) and flashes the Red LED.
* **Cooldowns:** There is a 1-second cooldown between attacks to prevent spamming. 

The OLED screens will automatically sync. The first player to drop the opponent's HP to 0 wins the match!

---
*Developed for embedded systems and IoT exploration.*
