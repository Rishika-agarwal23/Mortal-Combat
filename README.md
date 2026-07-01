Project Overview: Wearable IoT Mortal Kombat
The Wearable IoT Mortal Kombat System is a decentralized, motion-controlled physical combat game. Instead of using a traditional controller, two players wear smart gloves equipped with motion sensors and microcontrollers. The system translates real-world physical strikes into digital damage, syncing the gameplay instantly between the two players without the need for a central Wi-Fi router or external computer.

🎮 Gameplay Mechanics
Each player starts with 100 Health Points (HP), which is visually represented as an energy bar on the OLED screen mounted on their glove. Players physically spar (shadowboxing style) to trigger attacks.

The Punch (5 Damage): When a player thrusts their hand forward, the MPU6050 sensor detects a sharp linear acceleration. If it crosses the threshold, the glove registers a punch, subtracts 5 HP from the opponent, and flashes the Blue LED.

The Special Move (8 Damage): When a player sharply twists their wrist, the gyroscope detects a rapid rotational velocity. This triggers a heavy attack, subtracting 8 HP from the opponent and flashing the Red LED.

Cooldown System: To prevent players from simply shaking their hands wildly to win, the code enforces a strict 1-second cooldown between all registered attacks.

The Knockout: The first player's health to reach 0 triggers the "FATALITY" screen, ending the match.

🛠️ Hardware Architecture
This project is designed for two players, with each player wearing one smart glove. The total hardware required for the complete two-player system is:

2x ESP32 Dev Modules: The "brains" of the operation, handling sensor data, game logic, and wireless communication.

2x MPU6050 Sensors: 6-DoF IMUs used to track the acceleration and gyroscope data of the players' hands.

3x 0.96" SSD1306 OLED Displays: I2C screens that provide a real-time UI, showing player names and live health bars.

2x Blue LEDs & 2x Red LEDs: Localized visual indicators to confirm when an attack has successfully launched.

4x 220Ω - 330Ω Resistors: Crucial for protecting the ESP32 pins from LED current draw.

2x Portable Power Sources: Small LiPo batteries or USB power banks to ensure the gloves remain completely wireless and un-tethered.

📡 Software & Communication Protocol
The backbone of this project is the ESP-NOW protocol.

Because traditional Wi-Fi introduces lag and requires a router, this project bypasses it entirely. ESP-NOW allows the two ESP32 microcontrollers to speak directly to each other (Peer-to-Peer) via their MAC addresses.

State Machine: Both gloves run identical code (differentiated only by a PLAYER_NUMBER variable). Each glove acts as both a transmitter and a receiver.

Telemetry: When Player 1 lands a punch, Glove 1 instantly sends a tiny data packet (containing attackType and damage) to Glove 2.

Synchronization: Glove 2 receives the packet, drops its own health pool, flashes its LEDs so Player 2 knows they took a hit, and updates the OLED display—all in milliseconds.

🚀 Future Expansion (Optional)
Because the codebase is modular, the project can easily be upgraded. You could add a second glove to each player (requiring 4x ESP32s total) to track left and right hooks independently, or add a piezo buzzer to play sound effects when a hit lands.

Does this cover everything you need for your final project documentation, or is there a specific section you need expanded for a report or presentation?
