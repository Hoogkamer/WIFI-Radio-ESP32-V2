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
};
