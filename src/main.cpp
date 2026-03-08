#include <Arduino.h>
#include <SPI.h>
#include "DisplayManager.h"
#include "AudioManager.h"
#include "InputManager.h"
#include "NetworkManager.h"
#include "StationManager.h"
#include "main1.h"

// Manager instances
DisplayManager display;
InputManager input;
NetworkManager network;

// Global constants
const int SCREEN_TIMEOUT_SEC = 60;

// Application State
bool isRadioOn = true;
bool shouldPlay = true;

// Action handler
void handleAction(InputManager::Action action) {
    if (!isRadioOn && action != InputManager::SCREEN_ON) return;

    switch(action) {
        case InputManager::SCREEN_ON:
            isRadioOn = true;
            display.setScreenOn();
            break;

        case InputManager::SCREEN_TOGGLE:
            if (display.isOn()) {
                if (!display.isCategorySelection() && !display.isRadioSelection()) {
                    display.displayCategorySelection(true);
                } else if (display.isCategorySelection()) {
                    display.displayRadioSelection(true);
                } else {
                    // Back to main view
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
#ifdef HAS_ROTARIES
            audioManager.setVolume(input.getVolume());
#endif
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
            // Placeholder for sleep logic
            break;
            
        default:
            break;
    }
}

// Global callback for Audio library (mandatory)
void audio_showstreamtitle(const char *info) {
    display.setSongInfo(info);
    display.displayStation();
}

void setup() {
    Serial.begin(115200);
    log_i("Starting WiFi Radio ESP32 V2...");

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    // Initialize Managers
    StationManager::begin();
    display.begin();
    display.clearAllWhite();
    display.printError("Starting..."); // Reuse printError for boot message
    
    StationManager::loadStations();
    StationManager::loadSavedStation();
    
    uint8_t savedVolume = VOLUME_DEFAULT;
    StationManager::loadSavedVolume(savedVolume);

    network.begin();
    
    audioManager.begin(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audioManager.setVolume(savedVolume);
    
    input.begin();
#ifdef HAS_ROTARIES
    input.setVolume(savedVolume);
#endif
    input.setActionCallback(handleAction);
    
    display.displayStation();
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
