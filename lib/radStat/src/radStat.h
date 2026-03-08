#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#ifdef ARDUINO
#include <Arduino.h>
#include <FS.h>
#else
// Mock some Arduino things for native testing
#include <stdarg.h>
#include <stdio.h>

typedef std::string String;

class MockSerial {
public:
    void print(const char* s) { printf("%s", s); }
    void print(int i) { printf("%d", i); }
    void println(const char* s = "") { printf("%s\n", s); }
    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
};

// Mock File for native
class File {
public:
    virtual bool available() = 0;
    virtual char read() = 0;
};
#endif

namespace radStat {
    class RadioStation {
    public:
        std::string Name;
        std::string Category;
        std::string URL;
        int ID;

        RadioStation() : ID(-1) {}

        RadioStation(const std::string& c, const std::string& n, const std::string& u, int id)
            : Name(n), Category(c), URL(u), ID(id) {
            trim(Name);
            trim(Category);
            trim(URL);
        }

        void printDetails();

    private:
        void trim(std::string& s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());
        }
    };

    const int MAX_STATIONS = 100;
    const int MAX_CATEGORIES = 30;

    extern int nrOfStations;
    extern int nrOfCategories;
    extern int activeCategory;
    extern int radioStation;
    extern int stationsInCategoryCount;

    extern RadioStation activeRadioStation;
    extern RadioStation previousRadioStation;
    extern RadioStation radioStations[MAX_STATIONS];
    extern std::string radioCategories[MAX_CATEGORIES];

#ifndef ARDUINO
    extern MockSerial Serial;
#endif

    inline int findCategoryIndex(const std::string& target) {
        for (int i = 0; i < nrOfCategories; ++i) {
            if (radioCategories[i] == target) return i;
        }
        return -1;
    }

    inline void sortRadioStationsByName() {
        std::vector<RadioStation> tempStations;
        for (int i = 0; i < nrOfStations; ++i) tempStations.push_back(radioStations[i]);
        
        std::sort(tempStations.begin(), tempStations.end(), [](const RadioStation& a, const RadioStation& b) {
            return a.Name < b.Name;
        });

        for (int i = 0; i < (int)tempStations.size(); ++i) {
            tempStations[i].ID = i;
            radioStations[i] = tempStations[i];
        }
    }

    inline void sortRadioCategories() {
        std::sort(radioCategories, radioCategories + nrOfCategories);
    }

    inline std::string* getRadioCategories() {
        return radioCategories;
    }

    inline int getNrOfRadioCategories() { return nrOfCategories; }
    inline int getActiveCategoryNr() { return activeCategory; }

    inline std::vector<RadioStation> getRadioStationsOfActiveCategory() {
        std::vector<RadioStation> Stations;
        if (activeCategory < 0 || activeCategory >= nrOfCategories) return Stations;

        for (int i = 0; i < nrOfStations; i++) {
            if (radioStations[i].Category == radioCategories[activeCategory]) {
                Stations.push_back(radioStations[i]);
            }
        }
        stationsInCategoryCount = Stations.size();
        return Stations;
    }

    inline int getRadioCountOfActiveCategory() { return stationsInCategoryCount; }
    inline int getActiveRadioStation() { return radioStation; }

    inline void setActiveRadioStation(int nr) {
        if (nr < 0 || nr >= nrOfStations) return;
        activeRadioStation = radioStations[nr];
        radioStation = nr;
        activeCategory = findCategoryIndex(activeRadioStation.Category);
        if (activeCategory < 0) activeCategory = 0;
    }

    inline void resetPreviousRadioStation() { previousRadioStation = activeRadioStation; }

    inline void setActiveRadioStationName(const std::string& stationName) {
        int foundIdx = 0;
        for (int i = 0; i < nrOfStations; i++) {
            if (radioStations[i].Name == stationName) {
                foundIdx = i;
                break;
            }
        }
        setActiveRadioStation(foundIdx);
        previousRadioStation = activeRadioStation;
    }

    inline bool findStationInActiveCategory() {
        for (int i = 0; i < nrOfStations; i++) {
            if (radioStations[i].Category == radioCategories[activeCategory]) {
                setActiveRadioStation(i);
                return true;
            }
        }
        return false;
    }

    inline void nextStation() {
        int start = radioStation;
        do {
            radioStation++;
            if (radioStation >= nrOfStations) radioStation = 0;
            if (radioStations[radioStation].Category == radioCategories[activeCategory]) {
                setActiveRadioStation(radioStation);
                return;
            }
        } while (radioStation != start);
    }

    inline void prevStation() {
        int start = radioStation;
        do {
            radioStation--;
            if (radioStation < 0) radioStation = nrOfStations - 1;
            if (radioStations[radioStation].Category == radioCategories[activeCategory]) {
                setActiveRadioStation(radioStation);
                return;
            }
        } while (radioStation != start);
    }

    inline void nextCategory() {
        if (nrOfCategories == 0) return;
        activeCategory = (activeCategory + 1) % nrOfCategories;
        if (!findStationInActiveCategory()) nextCategory();
    }

    inline void prevCategory() {
        if (nrOfCategories == 0) return;
        activeCategory = (activeCategory - 1 + nrOfCategories) % nrOfCategories;
        if (!findStationInActiveCategory()) prevCategory();
    }

    inline void handleStation(const std::string& category, const std::string& name, const std::string& url) {
        if (nrOfStations >= MAX_STATIONS) return;
        radioStations[nrOfStations] = RadioStation(category, name, url, nrOfStations);
        nrOfStations++;
    }

    inline void handleCategory(const std::string& category) {
        if (nrOfCategories >= MAX_CATEGORIES) return;
        // Avoid duplicates
        for(int i=0; i<nrOfCategories; ++i) {
            if (radioCategories[i] == category) return;
        }
        radioCategories[nrOfCategories] = category;
        nrOfCategories++;
    }

    inline void processLine(const std::string& line, std::string& currentCategory) {
        if (line.empty()) return;
        if (line[0] == '[' && line.back() == ']') {
            currentCategory = line.substr(1, line.size() - 2);
            handleCategory(currentCategory);
        } else {
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string name = line.substr(0, eqPos);
                std::string url = line.substr(eqPos + 1);
                handleStation(currentCategory, name, url);
            }
        }
    }

    // Platform-independent logic for processing string content
    inline void processContent(const std::string& content) {
        std::string line;
        std::string currentCategory;
        for (char ch : content) {
            if (ch == '\r') continue;
            if (ch == '\n') {
                processLine(line, currentCategory);
                line.clear();
            } else {
                line += ch;
            }
        }
        if (!line.empty()) processLine(line, currentCategory);
        sortRadioCategories();
        sortRadioStationsByName();
    }

#ifdef ARDUINO
    inline void processFile(fs::File file) {
        std::string content;
        while (file.available()) content += (char)file.read();
        processContent(content);
    }
#endif
}
