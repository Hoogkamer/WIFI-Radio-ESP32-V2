#pragma once
#pragma GCC optimize("Os") // optimize for code size

//-----------------------------------------
// select radio config here

#include "portableradio.h"
// #include "ipodradio.h"

//-----------------------------------------

#include "Arduino.h"
#include <FS.h>
#include "Audio.h"
#include "SPI.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <EEPROM.h>

#include "TFT_eSPI.h"
#include "Free_Fonts.h"

#ifdef HAS_SDCARD
#include <SD.h>
#endif

#include <WiFiManager.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "radStat.h"
#ifdef HAS_REMOTE
// #include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#endif
#ifdef HAS_ROTARIES
#include "AiEsp32RotaryEncoder.h"
#endif

#define CORS_DEBUG
#define LOG_DEFAULT_LEVEL_INFO

#ifdef HAS_ROTARIES
#define ROTARY_ENCODER_A_PIN 13
#define ROTARY_ENCODER_B_PIN 17
#define ROTARY_ENCODER_BUTTON_PIN 35
#define ROTARY_ENCODER_STEPS 4
AiEsp32RotaryEncoder rotaryVolume = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS);

#define ROTARY_ENCODER2_A_PIN 14
#define ROTARY_ENCODER2_B_PIN 34
#define ROTARY_ENCODER2_BUTTON_PIN 33
#define ROTARY_ENCODER2_STEPS 4
AiEsp32RotaryEncoder rotaryTuner = AiEsp32RotaryEncoder(ROTARY_ENCODER2_A_PIN, ROTARY_ENCODER2_B_PIN, ROTARY_ENCODER2_BUTTON_PIN, -1, ROTARY_ENCODER2_STEPS);
#endif

// Digital I/O used
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

#define SD_CS 5
#define SPI_SCK 18
#define SPI_MOSI 23
#define SPI_MISO 19

#define IR_RECEIVE_PIN 15

#define DISPLAY_INVERSION 0 // (0) off (1) on
// #define SPI_FREQUENCY 40000000 // 27000000, 40000000, 80000000

#define LOAD_GLCD  // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2 // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4 // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6 // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7 // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8 // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
// #define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// Comment out the #define below to stop the SPIFFS filing system and smooth font code being loaded
// this will save ~20kbytes of FLASH
#define SMOOTH_FONT

const uint16_t kRecvPin = 15; // IR receiver

void connectToWIFI();
void setTFTbrightness(uint8_t duty);
boolean drawImage(const char *path, uint16_t posX, uint16_t posY, uint16_t maxWidth = 0, uint16_t maxHeigth = 0);
void printError(const char *error);
void loadSavedStation();
void saveTheStation();
void displayCategory();
void displayStation();
void onTunerShortClick();
