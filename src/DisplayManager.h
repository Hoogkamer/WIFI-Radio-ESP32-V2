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

    void displayStation();
    void displayCategorySelection(bool clearScreen);
    void displayRadioSelection(bool clearScreen);
    void displayDetails(const String& ip);
    void displayMute(bool muted);
    void displaySaved();
    void displayIP(const String& ip);
    
    void setSongInfo(const String& info) { _songInfo = info; }
    void setStationInfo(const String& info) { _stationInfo = info; }

    void printError(const char* error);
    
    void setCategorySelection(bool active) { _isCategorySelection = active; }
    void setRadioSelection(bool active) { _isRadioSelection = active; }
    bool isCategorySelection() const { return _isCategorySelection; }
    bool isRadioSelection() const { return _isRadioSelection; }

private:
    TFT_eSPI _tft;
    bool _isOn;
    unsigned long _screenOnMillis;
    int _screenTimeoutSec;

    void displayMenuHeader(const String& header);
    void displayCategory();
    void displayStationName();
    void displaySongInfo();
    void printSplitString(const String& text);
    void printStationName(const String& stationName);
    std::vector<std::string> splitString(const std::string &str, const std::string &delimiter);

    String _songInfo;
    String _stationInfo;
    
    bool _isCategorySelection;
    bool _isRadioSelection;

    static const int LEFT_MARGIN = 5;
    int _screenWidth;
    int _screenHeight;
    bool _screenLandscape;
};
