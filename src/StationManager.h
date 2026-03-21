#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include "radStat.h"

class StationManager {
public:
    static void begin();
    static void loadStations();
    static void loadSavedStation();
    static void saveCurrentStation();
    static void loadSavedVolume(uint8_t &volume);
    static void saveVolume(uint8_t volume);
    static void loadAudioSettings(int8_t &bass, int8_t &mid, int8_t &treble, bool &mono, int8_t &balance, uint8_t &brightness);
    static void saveAudioSettings(int8_t bass, int8_t mid, int8_t treble, bool mono, int8_t balance, uint8_t brightness);
};
