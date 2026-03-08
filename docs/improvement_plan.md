# Project Improvement Roadmap: WIFI Radio ESP32 V2

This document outlines the progress and future plans for the WIFI Radio project.

## 1. Build System & Configuration (COMPLETED ✅)
- [x] **Fix TFT_eSPI Configuration Anti-Pattern**: Now uses PlatformIO `build_flags` to inject `User_Setup.h`.
- [x] **Multi-Environment PlatformIO Setup**: Created `[env:ipod]`, `[env:portable]`, and `[env:native]`.
- [x] **Portable Logic Library**: Refactored `radStat` to use standard C++ for cross-platform compatibility.
- [x] **Native Unit Testing**: Established a test suite in `test/` that runs on Linux.

## 2. Library & Core Modernization (COMPLETED ✅)
- [x] **Upgrade to Arduino-ESP32 Core 3.x**: Now using the latest ESP-IDF 5.1 underlying framework.
- [x] **Refactor PWM**: Updated brightness logic to use the new `ledcAttach()` API.
- [x] **Migrate SPIFFS to LittleFS**: Filesystem is now faster and more resilient.
- [x] **Update Libraries**: WiFiManager, Audio, and WebServer are now on modern, maintained versions.

## 3. UI/UX Enhancements (Next Phase 🚀)
- [ ] **Web Remote Control Dashboard**:
    - [ ] Update `index.html` with a "Now Playing" view (Volume slider, Play/Stop, Station/Song info).
- [ ] **Advanced Display Logic**:
    - [ ] **Marquee Text**: Implement smooth scrolling for long song titles.
    - [ ] **Buffering Animation**: Show visual feedback while the stream is loading.
    - [ ] **Logarithmic Volume Scaling**: Adjust volume curve to match human hearing.
- [ ] **"Recents" List (Flash-Safe)**:
    - [ ] Implement a "Recent Stations" menu using **RTC Memory** to prevent Flash wear.

## 4. System Architecture & Stability (COMPLETED ✅)
- [x] **Refactor `main.cpp`**: Decoupled the monolithic code into modular classes (`DisplayManager`, `InputManager`, `AudioManager`, `NetworkManager`, `StationManager`).
- [x] **Memory Optimization**: Reduced heap fragmentation by minimizing Arduino `String` usage and moving to modular structure.

## 5. Power & Deployment (Next Phase 🚀)
- [ ] **True Deep Sleep**: Implement `esp_deep_sleep_start()` with wake-up triggers.
- [ ] **OTA (Over-The-Air) Updates**: Enable wireless firmware updates.
