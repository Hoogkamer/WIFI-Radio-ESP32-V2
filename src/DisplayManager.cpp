#include "DisplayManager.h"

#ifndef TFT_ROTATION
#define TFT_ROTATION 1
#endif

#ifndef TFT_BL
#define TFT_BL -1
#endif

DisplayManager::DisplayManager() : 
    _isOn(true), 
    _screenOnMillis(0), 
    _screenTimeoutSec(60),
    _songInfo(""),
    _stationInfo(""),
    _isCategorySelection(false),
    _isRadioSelection(false) {
    
#if TFT_ROTATION == 1 || TFT_ROTATION == 3
    _screenWidth = 320;
    _screenHeight = 240;
    _screenLandscape = true;
#else
    _screenWidth = 240;
    _screenHeight = 320;
    _screenLandscape = false;
#endif
}

void DisplayManager::begin() {
    _tft.begin();
    _tft.setTextWrap(true, false);
    _tft.setTextPadding(15);
    _tft.setRotation(TFT_ROTATION);
    setBrightness(100);
}

void DisplayManager::setBrightness(uint8_t duty) {
    if (TFT_BL == -1) return;

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttach(TFT_BL, 1200, 8);
    uint8_t d = round((double)duty * 2.55);
    ledcWrite(TFT_BL, d);
#else
    ledcSetup(0, 1200, 8);
    ledcAttachPin(TFT_BL, 0);
    uint8_t d = round((double)duty * 2.55);
    ledcWrite(0, d);
#endif
}

void DisplayManager::clearAllBlack() { _tft.fillScreen(TFT_BLACK); }
void DisplayManager::clearAllWhite() { _tft.fillScreen(TFT_WHITE); }

void DisplayManager::setScreenOn() {
    setBrightness(100);
    _isOn = true;
    _screenOnMillis = millis();
}

void DisplayManager::setScreenOff() {
    setBrightness(10);
    _isOn = false;
    _screenOnMillis = 0;
}

void DisplayManager::update() {
    if (_screenTimeoutSec > 1 && _screenOnMillis > 0 && (millis() - _screenOnMillis > (unsigned long)_screenTimeoutSec * 1000)) {
        setScreenOff();
    }
}

void DisplayManager::displayMenuHeader(const String& header) {
    _tft.fillRect(0, 0, _screenWidth, 30, TFT_ORANGE);
    _tft.setTextColor(TFT_BLACK);
    _tft.setFreeFont(&Roboto_24);
    _tft.setCursor(LEFT_MARGIN, 23);
    _tft.print(header);
}

void DisplayManager::displayCategory() {
    _tft.fillRect(0, 0, _screenWidth, 35, TFT_BLUE);
    _tft.setTextColor(TFT_WHITE);
    _tft.setFreeFont(&Roboto_24);
    _tft.setCursor(LEFT_MARGIN, 25);
    _tft.print(radStat::activeRadioStation.Category.c_str());
}

void DisplayManager::displayMute(bool muted) {
    if (muted) {
        _tft.fillRect(_screenWidth - 60, 0, 60, 35, TFT_RED);
        _tft.setTextColor(TFT_WHITE);
        _tft.setFreeFont(FF18);
        _tft.setCursor(_screenWidth - 60, 25);
        _tft.print("Mute");
    } else {
        displayCategory();
    }
}

void DisplayManager::displayStationName() {
    int sectionHeight = 40;
    _tft.fillRect(0, 35, _screenWidth, sectionHeight, TFT_WHITE);
    _tft.setTextColor(TFT_BLUE);
    _tft.setFreeFont(&Orbitron_Light_24);
    _tft.setCursor(LEFT_MARGIN, 65);

    if (_stationInfo.length() > 0) {
        _tft.setTextWrap(false, false);
        _tft.print(_stationInfo);
        _tft.setTextWrap(true, false);
    } else {
        String name = radStat::activeRadioStation.Name.c_str();
        name.replace("_", " ");
        _tft.print(name.c_str());
    }
}

void DisplayManager::displaySongInfo() {
    if (_isCategorySelection || _isRadioSelection) return;

    int fromPos = 75;
    int sectionHeight = _screenLandscape ? 40 : 80;

    _tft.fillRect(0, fromPos, _screenWidth, sectionHeight + 90, TFT_WHITE);
    _tft.drawLine(0, fromPos, _screenWidth, fromPos, TFT_BLUE);

    if (_songInfo.length() == 0) return;

    std::vector<std::string> tokens = splitString(_songInfo.c_str(), " - ");
    String artistName = tokens[0].c_str();
    _tft.setTextColor(TFT_BLACK);
    _tft.setFreeFont(&Roboto_24);
    _tft.setCursor(LEFT_MARGIN, fromPos + 30);
    printSplitString(artistName);

    if (tokens.size() >= 2) {
        String songName = tokens[1].c_str();
        _tft.setFreeFont(&Roboto_Thin_24);
        _tft.setCursor(LEFT_MARGIN, _tft.getCursorY());
        printSplitString(songName);
    }
}

void DisplayManager::displayIP(const String& ip) {
    _tft.setTextColor(TFT_BLACK);
    _tft.setFreeFont(FF1);
    _tft.fillRect(0, _screenHeight - 35, _screenWidth, 35, TFT_WHITE);
    _tft.drawLine(0, _screenHeight - 35, _screenWidth, _screenHeight - 35, TFT_BLUE);
    _tft.setCursor(LEFT_MARGIN, _screenHeight - 12);
    _tft.print("IP:");
    _tft.setCursor(LEFT_MARGIN + 40, _screenHeight - 12);
    _tft.print(ip);
}

void DisplayManager::displaySaved() {
    _tft.fillRect(0, _screenHeight - 35, _screenWidth, 35, TFT_WHITE);
    _tft.drawLine(0, _screenHeight - 35, _screenWidth, _screenHeight - 35, TFT_BLUE);
    _tft.setFreeFont(&Roboto_Thin_24);
    _tft.setTextColor(TFT_BLUE);
    _tft.setCursor(LEFT_MARGIN, _screenHeight - 12);
    _tft.print("Station Saved");
}

void DisplayManager::displayStation() {
    _isCategorySelection = false;
    _isRadioSelection = false;
    _tft.fillScreen(TFT_WHITE);
    displayCategory();
    displayStationName();
    displaySongInfo();
}

void DisplayManager::displayCategorySelection(bool clearScreen) {
    static int previousCategoryNr = 0;
    std::string *categories = radStat::getRadioCategories();
    int nrOfCategories = radStat::getNrOfRadioCategories();
    int activeCategoryNr = radStat::getActiveCategoryNr();
    
    if (clearScreen) {
        _tft.fillScreen(TFT_WHITE);
        displayMenuHeader("Categories");
        previousCategoryNr = activeCategoryNr;
    }
    
    setScreenOn();
    const int MARGIN_TOP = 30;
    _isCategorySelection = true;
    _isRadioSelection = false;
    _tft.setFreeFont(&Roboto_Thin_24);
    
    for (int i = 0; i < nrOfCategories; ++i) {
        _tft.setCursor(LEFT_MARGIN, MARGIN_TOP + i * 22 + 20);
        if (i != activeCategoryNr && i == previousCategoryNr) {
            _tft.fillRect(0, MARGIN_TOP + 22 * i + 3, _screenWidth, 21, TFT_WHITE);
            _tft.setTextColor(TFT_BLACK);
            _tft.print(categories[i].c_str());
        } else if (i == activeCategoryNr) {
            _tft.fillRect(0, MARGIN_TOP + 22 * i + 3, _screenWidth, 21, TFT_BLUE);
            _tft.setTextColor(TFT_WHITE);
            _tft.print(categories[i].c_str());
        } else if (clearScreen) {
            _tft.setTextColor(TFT_BLACK);
            _tft.print(categories[i].c_str());
        }
    }
    previousCategoryNr = activeCategoryNr;
}

void DisplayManager::displayRadioSelection(bool clearScreen) {
    static int previousStationIndex = -1;
    static int previousScrollOffset = -1;

    const int MARGIN_TOP = 34;
    const int LINE_HEIGHT = 24;
    int maxVisible = _screenLandscape ? 11 : 8;
    
    setScreenOn();
    _isCategorySelection = false;
    _isRadioSelection = true;

    std::vector<radStat::RadioStation> radioStations = radStat::getRadioStationsOfActiveCategory();
    int radioCount = radioStations.size();
    int activeStationID = radStat::getActiveRadioStation();
    int activeIndex = 0;
    
    for (int i = 0; i < radioCount; ++i) {
        if (radioStations[i].ID == activeStationID) {
            activeIndex = i;
            break;
        }
    }

    static int scrollOffset = 0;
    if (activeIndex < scrollOffset) scrollOffset = activeIndex;
    else if (activeIndex >= scrollOffset + maxVisible) scrollOffset = activeIndex - maxVisible + 1;

    if (scrollOffset < 0) scrollOffset = 0;
    if (scrollOffset > radioCount - maxVisible) scrollOffset = max(0, radioCount - maxVisible);

    bool fullRedraw = clearScreen || (scrollOffset != previousScrollOffset);

    if (clearScreen) {
        _tft.fillScreen(TFT_WHITE);
        displayMenuHeader("Stations");
    }

    _tft.setFreeFont(&Roboto_Thin_24);

    for (int i = 0; i < maxVisible; ++i) {
        int idx = scrollOffset + i;
        if (idx >= radioCount) break;

        const auto &station = radioStations[idx];
        int y = MARGIN_TOP + i * LINE_HEIGHT;
        bool isActive = (idx == activeIndex);
        bool wasActive = (idx == previousStationIndex);

        if (fullRedraw || isActive || wasActive) {
            uint16_t bgColor = isActive ? TFT_BLUE : (i % 2 == 0 ? TFT_WHITE : 0xE71C);
            uint16_t textColor = isActive ? TFT_WHITE : TFT_BLACK;

            _tft.fillRect(0, y, _screenWidth, LINE_HEIGHT, bgColor);
            _tft.setCursor(LEFT_MARGIN, y + 18);
            _tft.setTextColor(textColor);
            printStationName(station.Name.c_str());
        }
    }

    previousStationIndex = activeIndex;
    previousScrollOffset = scrollOffset;
}

void DisplayManager::displayDetails(const String& ip) {
    setScreenOn();
    _tft.setFreeFont(&Roboto_Thin_24);
    _tft.fillScreen(TFT_WHITE);
    _tft.setTextColor(TFT_BLACK);
    _tft.setCursor(2, 40);
    _tft.print("IP:");
    _tft.setCursor(2, 70);
    _tft.print(ip);
    _tft.setCursor(2, 120);
    _tft.print("Connect with your pc/phone to this IP address to configure the stations.");
}

void DisplayManager::printError(const char *error) {
    clearAllWhite();
    _tft.setTextColor(TFT_RED);
    _tft.setFreeFont(&Roboto_24);
    _tft.setCursor(25, 100);
    _tft.print(error);
}

void DisplayManager::printSplitString(const String& text) {
    int wordStart = 0;
    int wordEnd = 0;
    if ((text.indexOf(' ', wordStart) < 0)) {
        _tft.println(text);
        return;
    }
    while ((text.indexOf(' ', wordStart) >= 0) && (wordStart <= (int)text.length())) {
        wordEnd = text.indexOf(' ', wordStart + 1);
        uint16_t len = _tft.textWidth(text.substring(wordStart, wordEnd));
        if (_tft.getCursorX() + len >= _tft.width()) {
            _tft.println();
            wordStart++;
            _tft.setCursor(LEFT_MARGIN, _tft.getCursorY());
        }
        _tft.print(text.substring(wordStart, wordEnd));
        wordStart = wordEnd;
    }
    _tft.println();
}

void DisplayManager::printStationName(const String& stationName) {
    String name = stationName;
    name.replace("_", " ");
    _tft.print(name.c_str());
}

std::vector<std::string> DisplayManager::splitString(const std::string &str, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(str.substr(start));
    return tokens;
}
