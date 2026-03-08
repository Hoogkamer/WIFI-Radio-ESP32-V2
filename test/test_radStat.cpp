#include <unity.h>
#include "radStat.h"

void setUp(void) {
    // Reset radStat state before each test
    radStat::nrOfStations = 0;
    radStat::nrOfCategories = 0;
    radStat::activeCategory = 0;
    radStat::radioStation = 0;
}

void tearDown(void) {}

void test_parsing_logic(void) {
    std::string content = "[Jazz]\nStation A=http://jazz.com/a\nStation B=http://jazz.com/b\n[News]\nWorld News=http://news.com/world\n";
    radStat::processContent(content);

    TEST_ASSERT_EQUAL(2, radStat::getNrOfRadioCategories());
    TEST_ASSERT_EQUAL(3, radStat::nrOfStations);
}

void test_navigation_logic(void) {
    std::string content = "[Jazz]\nStation A=urlA\nStation B=urlB\n[Pop]\nStation C=urlC\n";
    radStat::processContent(content);

    // Start at Jazz -> Station A
    radStat::setActiveRadioStationName("Station A");
    TEST_ASSERT_EQUAL_STRING("Jazz", radStat::activeRadioStation.Category.c_str());
    TEST_ASSERT_EQUAL_STRING("Station A", radStat::activeRadioStation.Name.c_str());

    // Next station in Jazz
    radStat::nextStation();
    TEST_ASSERT_EQUAL_STRING("Station B", radStat::activeRadioStation.Name.c_str());

    // Next category (Pop)
    radStat::nextCategory();
    TEST_ASSERT_EQUAL_STRING("Pop", radStat::radioCategories[radStat::activeCategory].c_str());
    TEST_ASSERT_EQUAL_STRING("Station C", radStat::activeRadioStation.Name.c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_parsing_logic);
    RUN_TEST(test_navigation_logic);
    return UNITY_END();
}
