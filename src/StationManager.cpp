#include "StationManager.h"

void StationManager::begin() {
    if (!LittleFS.begin(true)) {
        log_e("LittleFS mount failed");
        return;
    }
    
    if (!LittleFS.exists("/stations.txt")) {
        if (LittleFS.exists("/stations_input.txt")) {
            LittleFS.rename("/stations_input.txt", "/stations.txt");
        }
    }
}

void StationManager::loadStations() {
    File file = LittleFS.open("/stations.txt", "r");
    if (file) {
        radStat::processFile(file);
        file.close();
    }
}

void StationManager::loadSavedStation() {
    File file = LittleFS.open("/savedStation.txt", "r");
    if (file) {
        String stationName = file.readString();
        radStat::setActiveRadioStationName(stationName.c_str());
        file.close();
    }
}

void StationManager::saveCurrentStation() {
    File file = LittleFS.open("/savedStation.txt", "w", true);
    if (file) {
        file.print(radStat::activeRadioStation.Name.c_str());
        file.close();
    }
}

void StationManager::loadSavedVolume(uint8_t &volume) {
    File file = LittleFS.open("/savedVolume.txt", "r");
    if (file) {
        String volStr = file.readString();
        if (volStr.toInt() > 0) {
            volume = (uint8_t)volStr.toInt();
        }
        file.close();
    }
}

void StationManager::saveVolume(uint8_t volume) {
    File file = LittleFS.open("/savedVolume.txt", "w", true);
    if (file) {
        file.print(volume);
        file.close();
    }
}
