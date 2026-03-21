# WIFI Radio ESP32 V2

A fully functional, professional-grade internet radio built with ESP32. This project focuses on simplicity, high-quality audio, and a polished user interface.

## New Features (V2)

- **Modular Architecture**: Decoupled monolithic code into dedicated Managers (`Display`, `Audio`, `Input`, `Network`, `Station`).
- **Flicker-Free UI**: Implemented partial screen updates and state tracking for smooth transitions.
- **Audio Settings Menu**: Intuitive "Point & Click" menu to adjust audio parameters, which are now **saved to permanent storage** (LittleFS) automatically upon exit.
  - **Bass / Mid / Treble** (-40dB to +6dB)
  - **Mono / Stereo** Toggle
  - **Balance** (Left/Right)
  - **Screen Brightness** (Backlight PWM control)
- **Enhanced Input Support**: Full navigation of all menus (including audio settings) via both **Rotary Encoders** and **IR Remote Control** (dedicated settings button).
- **Memory Optimized**: Significantly reduced heap fragmentation by minimizing `String` usage.
- **Fast Compilation**: Conditional compilation of IR protocols and library management via PlatformIO.

---

### Radio 1: Drop-in for an IPOD with remote
![RadioRemote](/readme_info/images/ipod1.jpg)

### Radio 2: Portable radio with rotary buttons
![RadioRotaries](/readme_info/images/radio3.jpg)
![RadioRotaries2](/readme_info/images/radio4.jpg)

## User Documentation

For detailed instructions on how to use the radio, setup WiFi, and navigate the menus, please refer to the:

👉 **[USER_MANUAL.md](./USER_MANUAL.md)**

---

## Hardware and Pinouts

Total cost remains under 30 EURO.

### Used pinouts (Detailed)

| ESP32 | LCD         | PCM             | IR        |         | wire colors | Rotary1 | Rotary2 |
| ----- | ----------- | --------------- | --------- | ------- | ----------- | ------- | ------- |
| v5    | 1- VCC      | 6- VIN          | 3- VCC    |         | red         |         |         |
| GND   | 2- GND      | 5- GND + 1- SCK | 2- GND    |         | black       |         |         |
| 22    | 3- CS       |                 |           |         | blue        |         |         |
| EN    | 4- RESET    |                 |           |         | yellow      |         |         |
| 21    | 5- DC       |                 |           |         | green       |         |         |
| 23    | 6- SDI/MOSI |                 |           |         | brown       |         |         |
| 18    | 7-SCK       |                 |           |         | grey        |         |         |
| 32    | 8-LED       |                 |           |         | purple      |         |         |
| 19    | 9-SDO/MISO  |                 |           |         | white       |         |         |
| 5     |             |                 |           |         | orange      |         |         |
| 26    |             | 4- LCK          |           |         |             |         |         |
| 25    |             | 3- DIN          |           |         |             |         |         |
| 27    |             | 2- BCK          |           |         |             |         |         |
| 15    |             |                 | 1- SIGNAL |         |             |         |         |
| 13    |             |                 |           |         |             | 1-CLK A |         |
| 17    |             |                 |           |         |             | 2-DT B  |         |
| 35    |             |                 |           |         |             | 3-SW    |         |
| v3    |             |                 |           |         |             | 4-+     | 4-+     |
| GND   |             |                 |           |         |             | 5-GND   | 5-GND   |
| 14    |             |                 |           |         |             |         | 1-CLK A |
| 34    |             |                 |           |         |             |         | 2-DT B  |
| 33    |             |                 |           |         |             |         | 3-SW    |

*Note: Don't use pins 0, 2, 12, 15 for general I/O if possible as they are strapping pins.*

---

## Installation & Setup

1.  **Clone the Repo**: `git clone https://github.com/Hoogkamer/wifiradio.git`
2.  **PlatformIO**: Use VS Code with the PlatformIO extension.
3.  **Config**: Edit `src/User_Setup.h` if your display pins differ.
4.  **Filesystem**: 
    - Build and Upload the **LittleFS** filesystem image via PlatformIO (contains the web dashboard).
5.  **Build**:
    - `pio run -e ipod -t upload` (for IR version)
    - `pio run -e portable -t upload` (for Rotary version)

## Web Dashboard
Navigate to the radio's IP address in your browser to:
- **Search & Add**: Integrated search for thousands of stations via `radio-browser.info`.
- **Test Before Saving**: Live "Play" button to verify stream URLs in your browser.
- **Easy Reordering**: Move stations up/down within categories with a single click.
- **Modern UI**: Fully responsive, mobile-friendly interface for easy management on any device.
- **Backup & Restore**: Export your entire station list as text for safe keeping.

![Edit radio](/readme_info/images/edit%20radio.png)
