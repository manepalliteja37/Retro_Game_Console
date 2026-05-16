# Retro Hand Game Console | Nokia 5110 & Nano

[![Arduino](https://img.shields.io/badge/Arduino-Nano-00979D?logo=arduino)](https://www.arduino.cc/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
A collection of classic retro games built for Arduino using a 0.96" SSD1306 OLED display.

Inspired by old handheld brick game consoles 🕹️ — now rebuilt with modern embedded systems.

---

## 🚀 Features

- 🎮 Multiple retro games in one device
- 🧠 Smooth gameplay using millis (no delay)
- 🎨 Custom UI for Nokia 5110
- 🔊 Sound effects using buzzer
- 🎯 Difficulty levels
- ⏸ Pause & resume system (future)
- 💤 Power-efficient design (sleep mode ready) (future)

---

## 🕹️ Games Included

- 🏓 Pong
- 🧱 Brick Breaker *(coming soon)*
- 🐍 Snake *(coming soon)*
- 🚗 Racing *(planned)*
- 👾 Space Shooter *(planned)*

---

## 🔌 Hardware Requirements
| Component | Quantity | Notes |
|-----------|----------|-------|
| Arduino Nano | 1 | ATmega328P compatible |
| Nokia 5110 LCD | 1 | 84x48 PCD8544 controller |
| Passive Buzzer | 1 | 5V magnetic/piezo |
| Tactile Buttons | 4 | Directional control |
| Resistors (10kΩ) | 4 | Nokia LCD Display |
| Resistor (1kΩ) | 2 | Buzzer current limiting and Nokia LCD Display CE Pin |
| Zero PCB / Perfboard | 1 | Prototype base |
| Jumper Wires | ~15 | Connections |
---

## 🔌 Hardware & Wiring
> **Note**: This firmware uses `INPUT_PULLUP`. Buttons must connect to GND when pressed. No external resistors required.
> ⚠️ **Update the `#define` pins in each `.ino` file to match your actual build.**

| Module | Arduino Pin | Function |
|--------|-------------|----------|
| Nokia 5110 RST | `D11` | Reset via 10kΩ |
| Nokia 5110 CE | `D10` | Chip Enable via 1kΩ |
| Nokia 5110 DC | `D9` | Data/Command via 10kΩ |
| Nokia 5110 DIN | `D8` | Serial Data |
| Nokia 5110 CLK | `D7` | Clock |
| Nokia 5110 VCC | `3.3V/5V` | Power |
| Buzzer (+) | `D6` | via 1kΩ |
| Button UP | `D2` |  GND |
| Button DOWN | `D3` |  GND |
| Button LEFT | `D4` | + 10kΩ to GND |
| Button RIGHT | `D5` | + 10kΩ to GND |

> 💡 The Nokia 5110 uses 3.3V logic. Most Nano clones include onboard level shifters. If yours doesn't, add 3.3V/5V level shifters for `D11`–`D7` or use resistors.

### 🔧 Perfboard Wiring Tips
- Connect all button GND pins together, then to Arduino `GND`.
- The Nokia 5110 requires `3.3V` logic. Most Nano clones handle 5V→3.3V shifting internally. If your LCD flickers or stays white, add a 10kΩ potentiometer to the `V0` pin for contrast tuning.
- Use `INPUT_PULLUP` mode in `setup()` to avoid floating pins and external 10kΩ resistors.

## 🚀 Getting Started
1. **Install Arduino IDE** (v2.x recommended)
2. **Install Libraries:** Sketch → Include Library → Manage Libraries → Install:
   - `Adafruit GFX Library`
   - `Adafruit PCD8544 Nokia 5110 LCD Library`
3. **Upload Firmware:**
   - Open your preferred `.ino` from `firmware/`
   - Board: `Arduino Nano`
   - Processor: `ATmega328P` (or `Old Bootloader` if upload fails)
   - Click **Upload**
---
## 🎥 Demo

- ▶ YouTube: (https://youtube.com/shorts/RQtRr-8H698?feature=share)
- 🐦 X (Twitter): (https://x.com/TejaMane37/status/2045184076940992572?s=20)

---
## Images
---

## 👨‍💻 Author

Built with ❤️ by **Telugu Mad Thinker**

---

## ⭐ Support

If you like this project, give it a ⭐ on GitHub!

## 🤝 Contributing
Found a bug or want to add features? Fork the repo, create a branch, and submit a PR. Please follow the existing code style and document any hardware changes. 
   
