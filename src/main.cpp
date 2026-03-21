#include <Arduino.h>
#include <SPI.h>
#include "DisplayManager.h"
#include "AudioManager.h"
#include "InputManager.h"
#include "NetworkManager.h"
#include "StationManager.h"
#include "main1.h"

#ifndef VOLUME_DEFAULT
#define VOLUME_DEFAULT 20
#endif

// Manager instances
DisplayManager display;
InputManager input;
NetworkManager network;

// Application State
bool isRadioOn = true;
bool shouldPlay = true;

// Settings State
struct AudioSettings {
    int8_t bass = 0;
    int8_t mid = 0;
    int8_t treble = 0;
    bool mono = false;
    int8_t balance = 0;
    uint8_t brightness = 100;
} audioSettings;

int activeSettingIndex = 0; 
bool isEditingSetting = false; 

void applyAudioSettings() {
    audioManager.setTone(audioSettings.bass, audioSettings.mid, audioSettings.treble);
    audioManager.setMono(audioSettings.mono);
    audioManager.setBalance(audioSettings.balance);
    display.setBrightness(audioSettings.brightness);
}

// Action handler
void handleAction(InputManager::Action action) {
    if (!isRadioOn && action != InputManager::SCREEN_ON) return;

    // Handle Settings Mode
    if (display.isSettingsMode()) {
        int8_t dir = 0;
        switch(action) {
            case InputManager::SETTINGS_ENTER: 
                display.displayStation();
                isEditingSetting = false;
                StationManager::saveAudioSettings(audioSettings.bass, audioSettings.mid, audioSettings.treble, audioSettings.mono, audioSettings.balance, audioSettings.brightness);
                return;

            case InputManager::SCREEN_TOGGLE: 
            case InputManager::VOLUME_TOGGLE_MUTE: 
                isEditingSetting = !isEditingSetting;
                display.displaySettings(activeSettingIndex, audioSettings.bass, audioSettings.mid, audioSettings.treble, audioSettings.mono, audioSettings.balance, audioSettings.brightness, isEditingSetting);
                break;

            case InputManager::STATION_PREV:
            case InputManager::CATEGORY_PREV:
                dir = -1;
                break;
            case InputManager::STATION_NEXT:
            case InputManager::CATEGORY_NEXT:
                dir = 1;
                break;

            case InputManager::VOLUME_CHANGED:
                {
                    static uint8_t lastVol = 0;
                    uint8_t currVol = input.getVolume();
                    if (currVol > lastVol) dir = 1;
                    else if (currVol < lastVol) dir = -1;
                    lastVol = currVol;
                }
                break;

            default:
                break;
        }

        if (dir != 0) {
            if (isEditingSetting) {
                if (activeSettingIndex == 0) audioSettings.bass = constrain(audioSettings.bass + dir, -40, 6);
                else if (activeSettingIndex == 1) audioSettings.mid = constrain(audioSettings.mid + dir, -40, 6);
                else if (activeSettingIndex == 2) audioSettings.treble = constrain(audioSettings.treble + dir, -40, 6);
                else if (activeSettingIndex == 3) audioSettings.mono = !audioSettings.mono;
                else if (activeSettingIndex == 4) audioSettings.balance = constrain(audioSettings.balance + dir, -16, 16);
                else if (activeSettingIndex == 5) audioSettings.brightness = constrain(audioSettings.brightness + (dir * 5), 10, 100);
                applyAudioSettings();
            } else {
                activeSettingIndex += dir;
                if (activeSettingIndex < 0) activeSettingIndex = 5;
                if (activeSettingIndex > 5) activeSettingIndex = 0;
            }
            display.displaySettings(activeSettingIndex, audioSettings.bass, audioSettings.mid, audioSettings.treble, audioSettings.mono, audioSettings.balance, audioSettings.brightness, isEditingSetting);
        }

        if (action == InputManager::VOLUME_TOGGLE_MUTE || action == InputManager::VOLUME_CHANGED || 
            action == InputManager::STATION_NEXT || action == InputManager::STATION_PREV ||
            action == InputManager::CATEGORY_NEXT || action == InputManager::CATEGORY_PREV ||
            action == InputManager::SCREEN_TOGGLE) return;
    }

    // Normal Radio Mode Handling
    switch(action) {
        case InputManager::SCREEN_ON:
            isRadioOn = true;
            display.setScreenOn();
            break;

        case InputManager::SETTINGS_ENTER:
            activeSettingIndex = 0;
            isEditingSetting = false;
            display.displaySettings(activeSettingIndex, audioSettings.bass, audioSettings.mid, audioSettings.treble, audioSettings.mono, audioSettings.balance, audioSettings.brightness, isEditingSetting);
            break;

        case InputManager::SCREEN_TOGGLE:
            if (display.isOn()) {
                if (!display.isCategorySelection() && !display.isRadioSelection()) {
                    display.displayCategorySelection(true);
                } else if (display.isCategorySelection()) {
                    display.displayRadioSelection(true);
                } else {
                    if (radStat::activeRadioStation.Name != radStat::previousRadioStation.Name) {
                        radStat::resetPreviousRadioStation();
                        audioManager.connectToStation(radStat::activeRadioStation);
                    }
                    display.displayStation();
                }
            } else {
                display.setScreenOn();
            }
            break;

        case InputManager::STATION_NEXT:
            if (display.isCategorySelection()) {
                radStat::nextCategory();
                display.displayCategorySelection(false);
            } else {
                radStat::nextStation();
                display.displayRadioSelection(!display.isRadioSelection());
            }
            break;

        case InputManager::STATION_PREV:
            if (display.isCategorySelection()) {
                radStat::prevCategory();
                display.displayCategorySelection(false);
            } else {
                radStat::prevStation();
                display.displayRadioSelection(!display.isRadioSelection());
            }
            break;

        case InputManager::CATEGORY_NEXT:
            if (!display.isRadioSelection()) {
                radStat::nextCategory();
                display.displayCategorySelection(!display.isCategorySelection());
            } else {
                radStat::nextStation();
                display.displayRadioSelection(!display.isRadioSelection());
            }
            break;

        case InputManager::CATEGORY_PREV:
            if (!display.isRadioSelection()) {
                radStat::prevCategory();
                display.displayCategorySelection(!display.isCategorySelection());
            } else {
                radStat::prevStation();
                display.displayRadioSelection(!display.isRadioSelection());
            }
            break;

        case InputManager::VOLUME_CHANGED:
            audioManager.setVolume(input.getVolume());
            break;

        case InputManager::VOLUME_TOGGLE_MUTE:
            audioManager.toggleMute();
            display.displayMute(audioManager.isMuted());
            break;

        case InputManager::STATION_SAVE:
            StationManager::saveCurrentStation();
            StationManager::saveVolume(audioManager.getVolume());
            display.displaySaved();
            break;

        case InputManager::SHOW_DETAILS:
            display.displayDetails(network.getIP());
            break;

        case InputManager::RADIO_OFF:
            break;
            
        default:
            break;
    }
}

void audio_showstreamtitle(const char *info) {
    display.setSongInfo(info);
    if (!display.isCategorySelection() && !display.isRadioSelection() && !display.isSettingsMode()) {
        display.displayStation();
    }
}

void setup() {
    Serial.begin(115200);
    log_i("Starting WiFi Radio ESP32 V2...");

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    StationManager::begin();
    display.begin();
    display.clearAllWhite();
    display.printError("Starting...");
    
    StationManager::loadStations();
    StationManager::loadSavedStation();
    
    uint8_t savedVolume = VOLUME_DEFAULT;
    StationManager::loadSavedVolume(savedVolume);
    StationManager::loadAudioSettings(audioSettings.bass, audioSettings.mid, audioSettings.treble, audioSettings.mono, audioSettings.balance, audioSettings.brightness);

    network.begin();
    
    audioManager.begin(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audioManager.setVolume(savedVolume);
    applyAudioSettings();
    
    input.begin();
#ifdef HAS_ROTARIES
    input.setVolume(savedVolume);
#endif
    input.setActionCallback(handleAction);
    
    display.displayStation(true);
    display.displayIP(network.getIP());
    
    log_i("Radio Ready!");
}

void loop() {
    input.update();
    display.update();
    network.update();
    
    if (shouldPlay) {
        if (!audioManager.isRunning()) {
            audioManager.connectToStation(radStat::activeRadioStation);
        }
        audioManager.loop();
    }
}
