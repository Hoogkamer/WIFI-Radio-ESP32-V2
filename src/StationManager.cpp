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

void StationManager::loadAudioSettings(int8_t &bass, int8_t &mid, int8_t &treble, bool &mono, int8_t &balance, uint8_t &brightness) {
    File file = LittleFS.open("/audioSettings.txt", "r");
    if (file) {
        bass = (int8_t)file.readStringUntil('\n').toInt();
        mid = (int8_t)file.readStringUntil('\n').toInt();
        treble = (int8_t)file.readStringUntil('\n').toInt();
        mono = (bool)file.readStringUntil('\n').toInt();
        balance = (int8_t)file.readStringUntil('\n').toInt();
        brightness = (uint8_t)file.readStringUntil('\n').toInt();
        file.close();
    }
}

void StationManager::saveAudioSettings(int8_t bass, int8_t mid, int8_t treble, bool mono, int8_t balance, uint8_t brightness) {
    File file = LittleFS.open("/audioSettings.txt", "w", true);
    if (file) {
        file.println(bass);
        file.println(mid);
        file.println(treble);
        file.println(mono ? 1 : 0);
        file.println(balance);
        file.println(brightness);
        file.close();
    }
}
