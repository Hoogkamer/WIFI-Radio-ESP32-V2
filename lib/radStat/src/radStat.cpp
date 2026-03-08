#include "radStat.h"

namespace radStat {
    int nrOfStations = 0;
    int nrOfCategories = 0;
    int activeCategory = 0;
    int radioStation = 0;
    int stationsInCategoryCount = 0;

    RadioStation activeRadioStation;
    RadioStation previousRadioStation;
    RadioStation radioStations[MAX_STATIONS];
    std::string radioCategories[MAX_CATEGORIES];

    void RadioStation::printDetails() {
#ifdef ARDUINO
        Serial.print("name:");
        Serial.print(Name.c_str());
        Serial.print(", category:");
        Serial.print(Category.c_str());
        Serial.print(", url:");
        Serial.print(URL.c_str());
        Serial.print(", id:");
        Serial.print(ID);
        Serial.println();
#else
        printf("name:%s, category:%s, url:%s, id:%d\n", Name.c_str(), Category.c_str(), URL.c_str(), ID);
#endif
    }

#ifndef ARDUINO
    MockSerial Serial;
#endif
}
