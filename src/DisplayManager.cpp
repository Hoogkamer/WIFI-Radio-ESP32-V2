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
    _lastSongInfo("---"),
    _stationInfo(""),
    _lastStationInfo("---"),
    _lastStationName("---"),
    _ip(""),
    _isCategorySelection(false),
    _isRadioSelection(false),
    _isSettingsMode(false) {
    
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
    _tft.setRotation(TFT_ROTATION);
    setBrightness(100);
}

void DisplayManager::setBrightness(uint8_t duty) {
#ifdef TFT_BL
    if (TFT_BL == -1) return;

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    static bool ledcConfigured = false;
    if (!ledcConfigured) {
        ledcAttach(TFT_BL, 1200, 8);
        ledcConfigured = true;
    }
    uint8_t d = round((double)duty * 2.55);
    ledcWrite(TFT_BL, d);
#else
    static bool ledcConfigured = false;
    if (!ledcConfigured) {
        ledcSetup(0, 1200, 8);
        ledcAttachPin(TFT_BL, 0);
        ledcConfigured = true;
    }
    uint8_t d = round((double)duty * 2.55);
    ledcWrite(0, d);
#endif
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
    String currentName = radStat::activeRadioStation.Name.c_str();
    if (currentName == _lastStationName && _stationInfo == _lastStationInfo) return;

    // Clear station name area
    _tft.fillRect(0, 35, _screenWidth, 40, TFT_WHITE);
    
    _tft.setTextColor(TFT_BLUE);
    _tft.setFreeFont(&Orbitron_Light_24);
    _tft.setCursor(LEFT_MARGIN, 65);

    if (_stationInfo.length() > 0) {
        _tft.print(_stationInfo);
    } else {
        String name = currentName;
        name.replace("_", " ");
        _tft.print(name.c_str());
    }
    _lastStationName = currentName;
    _lastStationInfo = _stationInfo;
}

void DisplayManager::displaySongInfo() {
    if (_isCategorySelection || _isRadioSelection || _isSettingsMode) return;
    if (_songInfo == _lastSongInfo) return;

    int fromPos = 75;
    // Clear song info area (between the lines)
    _tft.fillRect(0, fromPos + 1, _screenWidth, (_screenHeight - 35) - fromPos - 1, TFT_WHITE);

    if (_songInfo.length() == 0) {
        _lastSongInfo = _songInfo;
        return;
    }

    std::vector<std::string> tokens = splitString(_songInfo.c_str(), " - ");
    String artistName = tokens[0].c_str();
    
    _tft.setTextColor(TFT_BLACK);
    _tft.setFreeFont(&Roboto_24);
    _tft.setCursor(LEFT_MARGIN, fromPos + 30);
    printSplitString(artistName, TFT_WHITE);

    if (tokens.size() >= 2) {
        String songName = tokens[1].c_str();
        _tft.setFreeFont(&Roboto_Thin_24);
        _tft.setCursor(LEFT_MARGIN, _tft.getCursorY());
        printSplitString(songName, TFT_WHITE);
    }
    _lastSongInfo = _songInfo;
}

void DisplayManager::displayIP(const String& ip) {
    if (ip != "") _ip = ip;
    if (_ip == "") return;

    // Clear IP area
    _tft.fillRect(0, _screenHeight - 34, _screenWidth, 34, TFT_WHITE);
    _tft.setTextColor(TFT_BLACK);
    _tft.setFreeFont(FF1);
    _tft.setCursor(LEFT_MARGIN, _screenHeight - 12);
    _tft.print("IP: " + _ip);
}

void DisplayManager::displaySaved() {
    _tft.fillRect(0, _screenHeight - 34, _screenWidth, 34, TFT_WHITE);
    _tft.setFreeFont(&Roboto_Thin_24);
    _tft.setTextColor(TFT_BLUE);
    _tft.setCursor(LEFT_MARGIN, _screenHeight - 12);
    _tft.print("Station Saved");
}

void DisplayManager::displayStation(bool forceRedraw) {
    bool wasOtherMode = _isSettingsMode || _isCategorySelection || _isRadioSelection;
    _isCategorySelection = false;
    _isRadioSelection = false;
    _isSettingsMode = false;

    if (forceRedraw || wasOtherMode) {
        _tft.fillScreen(TFT_WHITE);
        displayCategory();
        _tft.drawLine(0, 75, _screenWidth, 75, TFT_BLUE);
        _tft.drawLine(0, _screenHeight - 35, _screenWidth, _screenHeight - 35, TFT_BLUE);
        _lastSongInfo = "---"; 
        _lastStationName = "---";
        displayIP();
    }
    
    displayStationName();
    displaySongInfo();
}

void DisplayManager::displayCategorySelection(bool clearScreen) {
    static int previousCategoryNr = -1;
    std::string *categories = radStat::getRadioCategories();
    int nrOfCategories = radStat::getNrOfRadioCategories();
    int activeCategoryNr = radStat::getActiveCategoryNr();
    
    if (clearScreen || !_isCategorySelection) {
        _tft.fillScreen(TFT_WHITE);
        displayMenuHeader("Categories");
        previousCategoryNr = -1;
    }
    
    _isCategorySelection = true;
    _isRadioSelection = false;
    _isSettingsMode = false;
    setScreenOn();

    const int MARGIN_TOP = 30;
    _tft.setFreeFont(&Roboto_Thin_24);
    
    for (int i = 0; i < nrOfCategories; ++i) {
        if (i == activeCategoryNr || i == previousCategoryNr || clearScreen) {
            uint16_t bgColor = (i == activeCategoryNr) ? TFT_BLUE : TFT_WHITE;
            uint16_t textColor = (i == activeCategoryNr) ? TFT_WHITE : TFT_BLACK;
            
            _tft.fillRect(0, MARGIN_TOP + 22 * i + 3, _screenWidth, 21, bgColor);
            _tft.setTextColor(textColor);
            _tft.setCursor(LEFT_MARGIN, MARGIN_TOP + i * 22 + 20);
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
    
    bool initialEntry = !_isRadioSelection;
    _isCategorySelection = false;
    _isRadioSelection = true;
    _isSettingsMode = false;
    setScreenOn();

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

    bool fullRedraw = clearScreen || initialEntry || (scrollOffset != previousScrollOffset);

    if (fullRedraw) {
        _tft.fillScreen(TFT_WHITE);
        displayMenuHeader("Stations");
    }

    _tft.setFreeFont(&Roboto_Thin_24);

    for (int i = 0; i < maxVisible; ++i) {
        int idx = scrollOffset + i;
        if (idx >= radioCount) break;

        if (fullRedraw || idx == activeIndex || idx == previousStationIndex) {
            const auto &station = radioStations[idx];
            int y = MARGIN_TOP + i * LINE_HEIGHT;
            bool isActive = (idx == activeIndex);
            uint16_t bgColor = isActive ? TFT_BLUE : (i % 2 == 0 ? TFT_WHITE : 0xE71C);
            uint16_t textColor = isActive ? TFT_WHITE : TFT_BLACK;

            _tft.fillRect(0, y, _screenWidth, LINE_HEIGHT, bgColor);
            _tft.setTextColor(textColor);
            _tft.setCursor(LEFT_MARGIN, y + 18);
            printStationName(station.Name.c_str());
        }
    }

    previousStationIndex = activeIndex;
    previousScrollOffset = scrollOffset;
}

void DisplayManager::displayDetails(const String& ip) {
    _tft.fillScreen(TFT_WHITE);
    displayMenuHeader("System Details");
    _tft.setTextColor(TFT_BLACK);
    _tft.setFreeFont(&Roboto_Thin_24);
    _tft.setCursor(LEFT_MARGIN, 60);
    _tft.print("IP: " + ip);
    _tft.setCursor(LEFT_MARGIN, 100);
    _tft.print("Web config enabled.");
    setScreenOn();
}

void DisplayManager::displaySettings(int activeIndex, int8_t bass, int8_t mid, int8_t treble, bool mono, int8_t balance, uint8_t brightness, bool isEditing) {
    bool initialEntry = !_isSettingsMode;
    _isSettingsMode = true;
    _isCategorySelection = false;
    _isRadioSelection = false;
    
    if (initialEntry) {
        _tft.fillScreen(TFT_WHITE);
        displayMenuHeader("Audio Settings");
        _lastActiveIndex = -1; 
    }
    
    const int MARGIN_TOP = 40;
    const int LINE_HEIGHT = 30; // Slightly smaller to fit 6
    const char* labels[] = {"Bass", "Mid", "Treble", "Mode", "Bal", "Light"};
    String values[6] = {
        String(bass), String(mid), String(treble), 
        mono ? "Mono" : "Stereo", String(balance), String(brightness)
    };
    
    _tft.setFreeFont(&Roboto_24);
    
    for (int i = 0; i < 6; i++) {
        bool needsRedraw = (i == activeIndex) || (i == _lastActiveIndex) || initialEntry;
        if (i == 0 && bass != _lastBass) needsRedraw = true;
        if (i == 1 && mid != _lastMid) needsRedraw = true;
        if (i == 2 && treble != _lastTreble) needsRedraw = true;
        if (i == 3 && mono != _lastMono) needsRedraw = true;
        if (i == 4 && balance != _lastBalance) needsRedraw = true;
        if (i == 5 && brightness != _lastBrightness) needsRedraw = true;
        if (i == activeIndex && isEditing != _lastIsEditing) needsRedraw = true;

        if (needsRedraw) {
            int y = MARGIN_TOP + i * LINE_HEIGHT;
            uint16_t bgColor = TFT_WHITE;
            uint16_t textColor = TFT_BLACK;
            
            if (i == activeIndex) {
                bgColor = isEditing ? 0xFDA0 : TFT_BLUE; 
                textColor = TFT_WHITE;
            }
            
            _tft.fillRect(0, y, _screenWidth, LINE_HEIGHT - 2, bgColor);
            _tft.setTextColor(textColor);
            _tft.setCursor(LEFT_MARGIN, y + 22);
            _tft.print(labels[i]);
            _tft.setCursor(_screenWidth - 80, y + 22);
            _tft.print(values[i]);
        }
    }
    
    _lastActiveIndex = activeIndex;
    _lastBass = bass; _lastMid = mid; _lastTreble = treble; _lastMono = mono;
    _lastBalance = balance; _lastBrightness = brightness;
    _lastIsEditing = isEditing;
    setScreenOn();
}

void DisplayManager::displayWiFiInstructions(const String& apName) {
    _tft.fillScreen(TFT_WHITE);
    displayMenuHeader("WiFi Config");
    _tft.setTextColor(TFT_BLACK);
    _tft.setFreeFont(&Roboto_24);
    _tft.setCursor(LEFT_MARGIN, 60);
    _tft.print("No WiFi connected!");
    _tft.setFreeFont(&Roboto_Thin_24);
    _tft.setCursor(LEFT_MARGIN, 100);
    _tft.print("1. Connect your device to:");
    _tft.setCursor(LEFT_MARGIN, 130);
    _tft.setTextColor(TFT_BLUE);
    _tft.print(apName);
    _tft.setTextColor(TFT_BLACK);
    _tft.setCursor(LEFT_MARGIN, 170);
    _tft.print("2. Go to 192.168.4.1");
    _tft.setCursor(LEFT_MARGIN, 200);
    _tft.print("3. Enter your WiFi credentials");
}

void DisplayManager::printError(const char *error) {
    clearAllWhite();
    _tft.setTextColor(TFT_RED);
    _tft.setFreeFont(&Roboto_24);
    _tft.setCursor(25, 100);
    _tft.print(error);
}

void DisplayManager::printSplitString(const String& text, uint16_t bgColor) {
    int wordStart = 0;
    int wordEnd = 0;
    if (text.indexOf(' ') < 0) {
        _tft.println(text);
        return;
    }
    while (wordStart < (int)text.length()) {
        wordEnd = text.indexOf(' ', wordStart + 1);
        if (wordEnd < 0) wordEnd = text.length();
        
        uint16_t len = _tft.textWidth(text.substring(wordStart, wordEnd));
        if (_tft.getCursorX() + len >= _screenWidth) {
            _tft.println();
            _tft.setCursor(LEFT_MARGIN, _tft.getCursorY());
            if (text[wordStart] == ' ') wordStart++;
        }
        _tft.print(text.substring(wordStart, wordEnd));
        wordStart = wordEnd;
    }
    _tft.println();
}

void DisplayManager::printStationName(const String& stationName) {
    String name = stationName;
    name.replace("_", " ");
    _tft.print(name);
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
