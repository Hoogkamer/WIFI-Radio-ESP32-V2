#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace radStat
{
    class RadioStation
    {
    public:
        String Name;
        String Category;
        String URL;

        RadioStation() {}

        RadioStation(string c, string n, string u)
        {
            Name = String(n.c_str());
            Name.trim();
            Category = String(c.c_str());
            Category.trim();
            URL = String(u.c_str());
            URL.trim();
        }

        void printDetails()
        {
            Serial.print("name:");
            Serial.print(Name.c_str());
            Serial.print(", category:");
            Serial.print(Category.c_str());
            Serial.print(", url:");
            Serial.print(URL.c_str());
        }
    };

    int nrOfStations = 0;
    int nrOfCategories = 0;
    int activeCategory = 0;
    int radioStation = 0;
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

    std::vector<std::string> stringToArray(const std::string &input)
    {
        std::vector<std::string> dataArray;
        size_t startPos = input.find('[') + 1; // Start position after the opening square bracket
        size_t endPos;

        while (true)
        {
            // Find the next comma or closing square bracket
            endPos = input.find(',', startPos);
            if (endPos == std::string::npos)
            {
                endPos = input.find(']', startPos);
            }

            // Extract the element from the substring
            std::string element = input.substr(startPos, endPos - startPos);

            // Remove surrounding quotes, if any
            if (element.front() == '"' && element.back() == '"')
            {
                element.pop_back();
                element.erase(0, 1);
            }

            dataArray.push_back(element);

            // Move to the next position for the next iteration
            if (input[endPos] == ']')
            {
                break;
            }
            startPos = endPos + 1;
        }

        return dataArray;
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
    void setActiveRadioStation(String stationName)
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
        activeRadioStation = radioStations[i];
        previousRadioStation = radioStations[i];
        Serial.println("nrofstation, stationname, i");
        Serial.print(nrOfStations);
        Serial.print(stationName);
        Serial.println(i);
        Serial.println("Selected station:");
        activeRadioStation.printDetails();
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
    void findStationCat()
    {
        for (int i = 0; i < nrOfStations; i++)
        {
            if (radioStations[i].Category == radioCategories[activeCategory])
            {
                radioStation = i;
                break;
            }
        }
        setActiveRadioStation(radioStation);
    }
    void nextCategory()
    {
        activeCategory++;
        if (activeCategory > nrOfCategories - 1)
        {
            activeCategory = 0;
        }
        findStationCat();
    }
    void prevCategory()
    {
        activeCategory--;
        if (activeCategory < 0)
        {
            activeCategory = nrOfCategories - 1;
        }
        findStationCat();
    }
    void handleStation(std::string station)
    {
        if (station[0] == ']')
        {
            return;
        }

        if (!station.empty() && station[0] == ',')
            station.erase(0, 1); // Remove the first character (the comma)

        std::vector<std::string> result = stringToArray(station);

        radioStations[nrOfStations] = RadioStation(result[2], result[0], result[1]);

        Serial.print("Station>>>>:");
        radioStations[nrOfStations].printDetails();
        nrOfStations++;
    }
    void handleCategory(std::string category)
    {
        std::vector<std::string> result = stringToArray(category);
        for (const std::string &str : result)
        {
            radioCategories[nrOfCategories] = String(str.c_str());
            Serial.print("Categorie:");
            Serial.println(radioCategories[nrOfCategories]);
            nrOfCategories++;
        }
    }
    void processJSON(File configurations1)
    {
        // File configurations1 = SPIFFS.open("/stations.json", "r");

        std::string station;
        std::string category;

        bool stations = false;
        bool categories = false;
        char prevch = '*';
        char ch = ' ';
        while (configurations1.available())

        {
            ch = configurations1.read();

            if (ch == ' ' || ch < 32 || ch == '\n' || ch == '\r' || ch == '\t')
            {
                continue;
            }
            // std::cout << ch;

            if (ch == '[' && prevch == ':')
            {
                categories = true;
                stations = false;
                category = "";
                station = "";
            }
            if (ch == '[' && prevch == '[')
            {
                stations = true;
                categories = false;
                category = "";
                station = "";
            }
            prevch = ch;
            if (categories)
            {

                category += ch;
                if (ch == ']')
                {
                    handleCategory(category);
                    category = "";
                }
            }

            if (stations)
            {

                station += ch;
                if (ch == ']')
                {
                    handleStation(station);
                    station = "";
                }
            }
        }
    }
}