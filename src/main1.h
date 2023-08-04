

#pragma once
#pragma GCC optimize("Os") // optimize for code size

// #include "Arduino_JSON.h"
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
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "SPIFFS.h"

#include "radStat.h"

// enable if you have a remote
#define HAS_REMOTE

#define CORS_DEBUG

#define LOG_DEFAULT_LEVEL_INFO

#ifdef HAS_REMOTE
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#endif

// Digital I/O used
#define SD_CS 5
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

#define TFT_ROTATION 0         // 1 or 3 (landscape)
#define DISPLAY_INVERSION 0    // (0) off (1) on
#define TFT_FREQUENCY 40000000 // 27000000, 40000000, 80000000

const uint16_t kRecvPin = 15; // IR receiver

class RadioStation
{
public:
    String Name;
    String Category;
    String URL;

    RadioStation() {}

    RadioStation(String c, String n, String u)
    {
        Name = n;
        Name.trim();
        Category = c;
        Category.trim();
        URL = u;
        URL.trim();
    }

    void printDetails()
    {
        Serial.print("name:");
        Serial.print(Name.c_str());
        Serial.print(", category:");
        Serial.print(Category.c_str());
        Serial.print(", url:");
        Serial.print(URL.c_str());
    }
};

void connectToWIFI();
void setTFTbrightness(uint8_t duty);
boolean drawImage(const char *path, uint16_t posX, uint16_t posY, uint16_t maxWidth = 0, uint16_t maxHeigth = 0);
void findStationCat();
void printError(const char *error);
void loadSavedStation();
void saveTheStation();
DynamicJsonDocument getStationData();