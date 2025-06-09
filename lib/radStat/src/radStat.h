#pragma once

#include <iostream>
#include <string>
#include <vector>

const int MAX_CATEGORIES = 10;
const int MAX_STATIONS = 100;

namespace radStat
{
    class RadioStation
    {
    public:
        String Name;
        String Category;
        String URL;
        int ID;

        RadioStation() {}

        RadioStation(string c, string n, string u, int id)
        {
            Name = String(n.c_str());
            Name.trim();
            Category = String(c.c_str());
            Category.trim();
            URL = String(u.c_str());
            URL.trim();
            ID = id;
        }

        void printDetails()
        {
            Serial.print("name:");
            Serial.print(Name.c_str());
            Serial.print(", category:");
            Serial.print(Category.c_str());
            Serial.print(", url:");
            Serial.print(URL.c_str());
            Serial.print(", id:");
            Serial.print(ID);
        }
    };

    int nrOfStations = 0;
    int nrOfCategories = 0;
    int activeCategory = 0;

    int radioStation = 0;
    int stationsInCategoryCount = 0;
    RadioStation activeRadioStation;
    RadioStation previousRadioStation;

    RadioStation radioStations[100];
    String radioCategories[30];

    int findStringIndex(String arr[], int size, String target)
    {
        for (int i = 0; i < size; ++i)
        {
            if (arr[i] == target)
            {
                return i; // Return the index when a match is found
            }
        }

        return -1; // Return -1 if the string is not found
    }


    String *getRadioCategories()
    {
        return radioCategories;
    }
    int getNrOfRadioCategories()
    {
        return nrOfCategories;
    }
    int getActiveCategoryNr()
    {
        return activeCategory;
    }
    std::vector<RadioStation> getRadioStationsOfActiveCategory()
    {

        std::vector<RadioStation> Stations;

        int i;
        int count = 0;
        for (i = 0; i < nrOfStations; i++)
        {
            const RadioStation &thisStation = radioStations[i];
            if (thisStation.Category == radioCategories[activeCategory])
            {
                Stations.push_back(thisStation);
                count++;
            }
        }
        stationsInCategoryCount = count;
        return Stations;
    }
    int getRadioCountOfActiveCategory()
    {
        return stationsInCategoryCount;
    }
    int getActiveRadioStation()
    {
        return radioStation;
    }
    void setActiveRadioStation(int nr)
    {

        activeRadioStation = radioStations[nr];
        activeCategory = findStringIndex(radioCategories, nrOfCategories, activeRadioStation.Category);
        if (activeCategory < 0)
        {
            activeCategory = 0;
        }
    }
    void resetPreviousRadioStation()
    {
        previousRadioStation = activeRadioStation;
    }
    void setActiveRadioStationName(String stationName)
    {
        int i = 0;
        bool found = false;
        for (i = 0; i < nrOfStations; i++)
        {
            if (radioStations[i].Name == stationName)
            {
                radioStation = i;
                found = true;
                break;
            }
        }
        if (!found)
        {
            i = 0;
        }
        setActiveRadioStation(i);
        previousRadioStation = activeRadioStation;
        // activeRadioStation.printDetails();
    }
    void nextStation()
    {

        radioStation++;
        if (radioStation > nrOfStations)
        {
            radioStation = 0;
        }
        RadioStation thisStation = radioStations[radioStation];
        if (thisStation.Category != radioCategories[activeCategory])
        {
            nextStation();
        }
        else
        {
            setActiveRadioStation(radioStation);
        }
    }
    void prevStation()
    {
        radioStation--;
        if (radioStation < 0)
        {
            radioStation = nrOfStations - 1;
        }
        RadioStation thisStation = radioStations[radioStation];
        if (thisStation.Category != radioCategories[activeCategory])
        {
            prevStation();
        }
        else
        {
            setActiveRadioStation(radioStation);
        }
    }
    bool findStationCat()
    {
        bool found = false;
        for (int i = 0; i < nrOfStations; i++)
        {
            if (radioStations[i].Category == radioCategories[activeCategory])
            {
                radioStation = i;
                found = true;
                break;
            }
        }
        if (found)
        {
            setActiveRadioStation(radioStation);
        }
        return found;
    }
    void nextCategory()
    {

        activeCategory++;
        if (activeCategory > nrOfCategories)
        {
            activeCategory = 0;
        }
        if (!findStationCat())
        {
            nextCategory();
        }
    }
    void prevCategory()
    {
        activeCategory--;
        if (activeCategory < 0)
        {
            activeCategory = nrOfCategories - 1;
        }
        if (!findStationCat())
        {
            prevCategory();
        }
    }
    void handleStation(std::string category, string name, string url)
    {
        if (nrOfStations >= MAX_STATIONS) { // define MAX_STATIONS somewhere
            Serial.println("Max stations reached, ignoring station");
            return;
        }
        radioStations[nrOfStations] = RadioStation(category, name, url, nrOfStations);
        nrOfStations++;
    }

    void handleCategory(std::string category)
    {
        if (nrOfCategories >= MAX_CATEGORIES) {
            Serial.println("Max categories reached, ignoring category");
            return;
        }

        radioCategories[nrOfCategories] = String(category.c_str());
        nrOfCategories++;
    }
  

    void processLine(const char* line, int length, std::string& currentCategory) {
        if (length <= 0) return;

        if (line[0] == '[' && line[length - 1] == ']') {
            // Extract category name without the brackets
            currentCategory = std::string(line + 1, length - 2);
            handleCategory(currentCategory);
        } else {
            std::string strLine(line, length);
            size_t eqPos = strLine.find('=');
            if (eqPos != std::string::npos) {
                std::string name = strLine.substr(0, eqPos);
                std::string url = strLine.substr(eqPos + 1);
                handleStation(currentCategory, name, url);
            } else {
                Serial.printf("Invalid station line (no '='): %.*s\n", length, line);
            }
        }
    }

    void processFile(File file) {
        const int maxLineLength = 256;
        char line[maxLineLength];
        int linePos = 0;
        std::string currentCategory;

        while (file.available()) {
            char ch = file.read();
            if (ch == '\r') continue;

            if (ch == '\n' || linePos >= maxLineLength - 1) {
                // Process the full line when newline or buffer full
                processLine(line, linePos, currentCategory);
                linePos = 0;  // Reset line position for next line
            } else {
                line[linePos++] = ch;
            }
        }

        // Process last line if file doesn't end with newline
        if (linePos > 0) {
            processLine(line, linePos, currentCategory);
        }
    }

}


