#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include "Roboto.h"
#include <FS.h>
#include "radStat.h"
#include <vector>
#include <string>

class DisplayManager {
public:
    DisplayManager();
    void begin();
    void setBrightness(uint8_t duty);
    void clearAllBlack();
    void clearAllWhite();
    void setScreenOn();
    void setScreenOff();
    bool isOn() const { return _isOn; }
    void update();

    void displayStation(bool forceRedraw = false);
    void displayCategorySelection(bool clearScreen);
    void displayRadioSelection(bool clearScreen);
    void displayDetails(const String& ip);
    void displayMute(bool muted);
    void displaySaved();
    void displayIP(const String& ip = "");
    void displaySettings(int activeIndex, int8_t bass, int8_t mid, int8_t treble, bool mono, int8_t balance, uint8_t brightness, bool isEditing = false);
    
    void setSongInfo(const String& info) { _songInfo = info; }
    void setStationInfo(const String& info) { _stationInfo = info; }

    void printError(const char* error);
    
    void setCategorySelection(bool active) { _isCategorySelection = active; _isRadioSelection = false; _isSettingsMode = false; }
    void setRadioSelection(bool active) { _isRadioSelection = active; _isCategorySelection = false; _isSettingsMode = false; }
    void setSettingsMode(bool active) { _isSettingsMode = active; _isCategorySelection = false; _isRadioSelection = false; }
    
    bool isCategorySelection() const { return _isCategorySelection; }
    bool isRadioSelection() const { return _isRadioSelection; }
    bool isSettingsMode() const { return _isSettingsMode; }

private:
    TFT_eSPI _tft;
    bool _isOn;
    unsigned long _screenOnMillis;
    int _screenTimeoutSec;

    void displayMenuHeader(const String& header);
    void displayCategory();
    void displayStationName();
    void displaySongInfo();
    void printSplitString(const String& text, uint16_t bgColor);
    void printStationName(const String& stationName);
    std::vector<std::string> splitString(const std::string &str, const std::string &delimiter);

    String _songInfo;
    String _lastSongInfo;
    String _stationInfo;
    String _lastStationInfo;
    String _lastStationName;
    String _ip;
    
    bool _isCategorySelection;
    bool _isRadioSelection;
    bool _isSettingsMode;

    // Settings tracking
    int _lastActiveIndex = -1;
    int8_t _lastBass = -99, _lastMid = -99, _lastTreble = -99;
    bool _lastMono = false;
    int8_t _lastBalance = -99;
    uint8_t _lastBrightness = 255;
    bool _lastIsEditing = false;

    static const int LEFT_MARGIN = 5;
    int _screenWidth;
    int _screenHeight;
    bool _screenLandscape;
};
