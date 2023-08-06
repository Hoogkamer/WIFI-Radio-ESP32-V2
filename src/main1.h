

#pragma once
#pragma GCC optimize("Os") // optimize for code size

#include "Arduino.h"
#include <FS.h>
#include "Audio.h"
#include "SPI.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SD.h>
#include "tft.h"
#include <WiFiManager.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "radStat.h"

// USER CHANGEABLE -------------

// enable if you have a remote
// #define HAS_REMOTE

// enable if you have rotary knobs
#define HAS_ROTARIES

// #define TFT_ROTATION 0 // 1 or 3 (landscape)
#define TFT_ROTATION 1 // 1 or 3 (landscape)

// enable if you need mono output on both channels (when you have 1 speaker)
#define MONO_OUTPUT

// -----------------------

#define CORS_DEBUG

#define LOG_DEFAULT_LEVEL_INFO

#ifdef HAS_REMOTE
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#endif

#ifdef HAS_ROTARIES
#include "AiEsp32RotaryEncoder.h"

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
#define SD_CS 5xx
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

#define TFT_SCK 18
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_CS 22
#define TFT_DC 21
#define TFT_RESET 17
#define TFT_BL 32

#define IR_RECEIVE_PIN 15

// #define TFT_ROTATION 0 // 1 or 3 (landscape)

#define DISPLAY_INVERSION 0    // (0) off (1) on
#define TFT_FREQUENCY 40000000 // 27000000, 40000000, 80000000

const uint16_t kRecvPin = 15; // IR receiver

void connectToWIFI();
void setTFTbrightness(uint8_t duty);
boolean drawImage(const char *path, uint16_t posX, uint16_t posY, uint16_t maxWidth = 0, uint16_t maxHeigth = 0);
void printError(const char *error);
void loadSavedStation();
void saveTheStation();
