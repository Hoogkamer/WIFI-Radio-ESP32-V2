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
        nrOfStations++;
    }
    void handleCategory(std::string category)
    {
        std::vector<std::string> result = stringToArray(category);
        for (const std::string &str : result)
        {
            radioCategories[nrOfCategories] = String(str.c_str());
            nrOfCategories++;
        }
    }
    void processJSON(File configurations1)
    {
        // This should be the file structure:
        //       {
        //   "categories": ["NL", "Jazz", "Local", "Retro", "Chill", "News"],
        //   "stations": [
        //     ["Veronica", "http://22343.live.streamtheworld.com/VERONICA.mp3", "NL"],
        //     [
        //       "SmoothJazz247",
        //       "http://www.smoothjazz247.com/smoothjazz24-7.m3up",
        //       "Jazz"
        //     ],
        //     [
        //       "Radio10",
        //       "http://playerservices.streamtheworld.com/api/livestream-redirect/RADIO10.mp3",
        //       "NL"
        //     ]]}

        std::string station;
        std::string category;

        bool stations = false;
        bool categories = false;
        char prevch = '*';
        char ch = ' ';
        while (configurations1.available())

        {
            ch = configurations1.read();
            // skip spaces, except when it is in a station name, and skip other non printable characters
            if ((ch == ' ' && station == "") || ch < 32 || ch == '\n' || ch == '\r' || ch == '\t')
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