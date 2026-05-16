# 🐍 Arduino Snake Game | Nokia 5110 & Nano

[![Arduino](https://img.shields.io/badge/Arduino-Nano-00979D?logo=arduino)](https://www.arduino.cc/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> A retro handheld Snake game built entirely on perfboard (zero PCB) using an Arduino Nano, Nokia 5110 LCD, passive buzzer, and tactile buttons. Optimized, modular, and shipped with 4 distinct firmware versions.

## 📦 Features
- ✅ Classic Snake mechanics with collision & boundary detection
- 🔊 Sound effects (eat, game over, startup) via passive buzzer
- 🎮 4-button D-pad control (Up/Down/Left/Right)
- 💾 Lightweight firmware variants for tight memory constraints
- 🎬 Dedicated boot animation sequence
- 🛠️ Fully documented wiring & perfboard layout

## Doc Structure
arduino-snake-nokia5110/
├── README.md
├── LICENSE
├── .gitignore
├── firmware/
│   ├── v1_basic/
│   │   └── v1_basic.ino
│   ├── v2_lighter/
│   │   └── v2_lighter.ino
│   ├── v3_full/
│   │   └── v3_full.ino
│   └── startup_animation/
│       └── startup_animation.ino
├── hardware/
│   ├── schematic.png
│   
|__ Images
|
└── docs/
    └── Hardware
    |__ Learning
    |__ Code_Readme

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

## 📜 Firmware Versions
| Version | Size (Approx.) | Features | Best For |
|---------|----------------|----------|----------|
| `v1_basic` | ~6KB Flash / 1.2KB RAM | Core movement, no sound, basic collision | Learning & debugging |
| `v2_simple` | ~5KB Flash / 0.9KB RAM | Stripped UI, optimized loops,no tail and head | Low-memory Nano clones |
| `v3_lighter` | ~9KB Flash / 1.8KB RAM | tail and head,no score, speed progression, buzzer SFX, boot anim | Final product |
| `v4_full` | ~9KB Flash / 1.8KB RAM | Scoring, speed progression, buzzer SFX, boot anim | Final product |
| `startup_anim` | ~2KB Flash / 0.4KB RAM | LCD test & logo animation | Branding and playing withit |

## 🔧 Customization
- Adjust game speed: Modify `GAME_SPEED_MS` or `FRAME_DELAY` at the top of the sketch
- Change pinout: Edit the `#define PIN_...` block
- Tweak buzzer tones: Modify `TONE_EAT`, `TONE_GAME_OVER`, etc.
- LCD contrast: Add a 10kΩ potentiometer to `V0` pin if text is too faint

## Images
Retro_Snake_game/Images/wiring-nokia.png
## 🤝 Contributing
Found a bug or want to add features? Fork the repo, create a branch, and submit a PR. Please follow the existing code style and document any hardware changes.

## 📄 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 💡 Acknowledgments
- Nokia 5110 (PCD8544) datasheet
- Arduino community & Adafruit library maintainers
- Retro gaming inspiration & open-source hardware builders
