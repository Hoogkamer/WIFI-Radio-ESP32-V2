#include "main1.h"
#include <vector>
#include <string>

#include "Roboto.h"

long screenSwitchOnMillis = 0;
long radioSwitchMillis = 0;

int screenTimeoutSec = 60;
int autoSwitchSec = 3;

File stationsFile;

String _sspw = "";
String _ssid = "";

WiFiManager manager;
TFT_eSPI tft = TFT_eSPI();

Audio audio;
#ifdef HAS_REMOTE
IRrecv irrecv(kRecvPin);
decode_results results;
#endif

bool radioIsOn = true;
bool radioIsMuted = false;
bool tftIsOn = true;
bool playRadio = true;
bool isCategorySelection = false;
bool isRadioSelection = false;
const char *songInfo = "";
const char *stationInfo = "";

#if TFT_ROTATION == 1 || TFT_ROTATION == 3
const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;
const boolean SCREEN_LANDSCAPE = true;
#else
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 320;
const boolean SCREEN_LANDSCAPE = false;
#endif

unsigned long shortPressAfterMiliseconds = 50;  // how long short press shoud be. Do not set too low to avoid bouncing (false press events).
unsigned long longPressAfterMiliseconds = 1000; // how long čong press shoud be.

int prevRotaryTunerCode = 500;

AsyncWebServer server(80);
#define FONT_INFO 0
#define FONT_CATEGORY 1
#define FONT_STATION 2
#define FONT_SONG 3

#define LEFT_MARGIN 5
/***********************************************************************************************************************
 *                                                 I M A G E                                                           *
 ***********************************************************************************************************************/

void setTFTbrightness(uint8_t duty)
{ // duty 0...100 (min...max)
  if (TFT_BL == -1)
    return;
  ledcSetup(0, 1200, 8);                  // 1200 Hz PWM and 8 bit resolution
  ledcAttachPin(TFT_BL, 0);               // Configure variable led, TFT_BL pin to channel 1
  uint8_t d = round((double)duty * 2.55); // #186
  ledcWrite(0, d);
}

inline void clearTFTAllBlack() { tft.fillScreen(TFT_BLACK); } // y   0...239
inline void clearTFTAllWhite() { tft.fillScreen(TFT_WHITE); } // y   0...239

/***********************************************************************************************************************
 *                                                  P R O G R A M                                                      *
 ***********************************************************************************************************************/

void printSplitString(String text)
{
  int wordStart = 0;
  int wordEnd = 0;
  if ((text.indexOf(' ', wordStart) < 0))
  {
    tft.println(text);
    return;
  }
  while ((text.indexOf(' ', wordStart) >= 0) && (wordStart <= text.length()))
  {
    wordEnd = text.indexOf(' ', wordStart + 1);
    uint16_t len = tft.textWidth(text.substring(wordStart, wordEnd));
    if (tft.getCursorX() + len >= tft.width())
    {
      tft.println();
      wordStart++;
      tft.setCursor(LEFT_MARGIN, tft.getCursorY());
    }
    tft.print(text.substring(wordStart, wordEnd));
    wordStart = wordEnd;
  }

  tft.println();
}

std::vector<std::string> splitString(const std::string &str, const std::string &delimiter)
{
  std::vector<std::string> tokens;
  size_t start = 0, end = 0;

  while ((end = str.find(delimiter, start)) != std::string::npos)
  {
    tokens.push_back(str.substr(start, end - start));
    start = end + delimiter.length();
  }

  // Add the last token (or the only token if delimiter not found)
  tokens.push_back(str.substr(start));

  return tokens;
}

void loadStations()
{
  File configurations1 = SPIFFS.open("/stations.txt", "r");
  radStat::processFile(configurations1);
  
  configurations1.close();
}

void startRadioStream()
{
  if (isCategorySelection || isRadioSelection)
  {
    return;
  }
  String url4 = radStat::activeRadioStation.URL;
  const char *url1 = url4.c_str();
  log_i("trying url:>>>%s<<<", url1);
  audio.connecttohost(url1);
}

void clearScreenStation()
{

  clearTFTAllWhite();
}
void clearScreenSong()
{
  if (TFT_ROTATION) // reset station info
  {
    tft.fillRect(0, 150, 320, 240, TFT_WHITE);
  }
  else
  {
    clearTFTAllWhite();
  }
}
void setScreenOn()
{
  setTFTbrightness(100);
  tftIsOn = true;
  screenSwitchOnMillis = millis();
}
void setScreenOff()
{

  setTFTbrightness(10);
  tftIsOn = false;
  screenSwitchOnMillis = 0;
}
void displayMenuHeader(String header)
{
  tft.fillRect(0, 0, 320, 30, TFT_ORANGE);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(&Roboto_24);
  tft.setCursor(LEFT_MARGIN, 23);
  tft.print(header);
}
void displayCategory()
{
  tft.fillRect(0, 0, SCREEN_WIDTH, 35, TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&Roboto_24);
  tft.setCursor(LEFT_MARGIN, 25);
  tft.print(radStat::activeRadioStation.Category.c_str());
}
void displayMute()
{
  if (radioIsMuted)
  {
    tft.fillRect(SCREEN_WIDTH - 60, 0, 60, 35, TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.setFreeFont(FF18);
    tft.setCursor(SCREEN_WIDTH - 60, 25);
    tft.print("Mute");
  }
  else
  {
    displayCategory();
  }
}
void displayStationName()
{
  int sectionHeight = 40;
  if (!SCREEN_LANDSCAPE)
  {
    sectionHeight = 40;
  }
  tft.fillRect(0, 35, SCREEN_WIDTH, sectionHeight, TFT_WHITE);
  tft.setTextColor(TFT_BLUE);

  tft.setFreeFont(&Orbitron_Light_24);
  tft.setCursor(LEFT_MARGIN, 65);

  if (strcmp(stationInfo, "") != 0)
  {
    tft.setTextWrap(false, false);
    tft.print(stationInfo);
    tft.setTextWrap(true, false);
  }
  else
  {
    String nameReplaceUnderscroresBySpaces = radStat::activeRadioStation.Name;
    nameReplaceUnderscroresBySpaces.replace("_", " ");
    tft.print(nameReplaceUnderscroresBySpaces.c_str());
  }
}
void displaySongInfo()
{
  if (isCategorySelection || isRadioSelection)
  {
    return;
  }
  int artistLines = 1;
  int fromPos = 75;
  int sectionHeight = 40;
  if (!SCREEN_LANDSCAPE)
  {
    sectionHeight = 80;
  }

  tft.fillRect(0, fromPos, SCREEN_WIDTH, sectionHeight + 90, TFT_WHITE);
  tft.drawLine(0, fromPos, SCREEN_WIDTH, fromPos, TFT_BLUE);

  if (strcmp(songInfo, "") == 0)
    return;

  std::vector<std::string> tokens = splitString(songInfo, " - ");
  String artistName = tokens[0].c_str();
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(&Roboto_24);
  tft.setCursor(LEFT_MARGIN, fromPos + 30);
  // tft.println(artistName);
  printSplitString(artistName);

  if (tokens.size() >= 2)
  {
    String songName = tokens[1].c_str();
    tft.setFreeFont(&Roboto_Thin_24);
    tft.setCursor(LEFT_MARGIN, tft.getCursorY());
    // tft.println(songName);
    printSplitString(songName);
  }
}

void displayIP()
{
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(FF1);
  tft.fillRect(0, SCREEN_HEIGHT - 35, SCREEN_WIDTH, 35, TFT_WHITE);
  tft.drawLine(0, SCREEN_HEIGHT - 35, SCREEN_WIDTH, SCREEN_HEIGHT - 35, TFT_BLUE);
  tft.setCursor(LEFT_MARGIN, SCREEN_HEIGHT - 12);
  tft.print("IP:");
  tft.setCursor(LEFT_MARGIN + 40, SCREEN_HEIGHT - 12);
  tft.print(WiFi.localIP().toString());
}
void displaySaved()
{
  tft.fillRect(0, SCREEN_HEIGHT - 35, SCREEN_WIDTH, 35, TFT_WHITE);
  tft.drawLine(0, SCREEN_HEIGHT - 35, SCREEN_WIDTH, SCREEN_HEIGHT - 35, TFT_BLUE);
  tft.setFreeFont(&Roboto_Thin_24);
  tft.setTextColor(TFT_BLUE);
  tft.setCursor(LEFT_MARGIN, SCREEN_HEIGHT - 12);
  tft.print("Station Saved");
}

void displayStation()
{
  isCategorySelection = false;
  isRadioSelection = false;
  radStat::activeRadioStation.printDetails();
  radioSwitchMillis = millis();
  setScreenOn();
  tft.fillScreen(TFT_WHITE);
  displayCategory();
  displayStationName();
  displaySongInfo();
  displayIP();
}
void displayCategorySelection(boolean clearScreen)
{
  static int previousCategoryNr = 0;
  String *categories = radStat::getRadioCategories();
  int nrOfCategories = radStat::getNrOfRadioCategories();
  int activeCategoryNr = radStat::getActiveCategoryNr();
  if (clearScreen)
  {
    tft.fillScreen(TFT_WHITE);
    displayMenuHeader("Categories");
    previousCategoryNr = activeCategoryNr;
  }
  setScreenOn();
  const int MARGIN_TOP = 30;
  isCategorySelection = true;
  tft.setFreeFont(&Roboto_Thin_24);
  for (int i = 0; i < nrOfCategories; ++i)
  {

    tft.setCursor(LEFT_MARGIN, MARGIN_TOP + i * 22 + 20);
    if (i != activeCategoryNr && i == previousCategoryNr)
    {
      tft.fillRect(0, MARGIN_TOP + 22 * i + 3, 320, 21, TFT_WHITE);
      tft.setTextColor(TFT_BLACK);
      tft.print(categories[i]);
    }
    else if (i == activeCategoryNr)
    {
      tft.fillRect(0, MARGIN_TOP + 22 * i + 3, 320, 21, TFT_BLUE);
      tft.setTextColor(TFT_WHITE);
      tft.print(categories[i]);
    }
    else if (clearScreen)
    {
      tft.setTextColor(TFT_BLACK);
      tft.print(categories[i]);
    }
  }
  previousCategoryNr = activeCategoryNr;
}

void printStationName(String stationName)
{
  String nameReplaceUnderscroresBySpaces = stationName;
  nameReplaceUnderscroresBySpaces.replace("_", " ");
  tft.print(nameReplaceUnderscroresBySpaces.c_str());
}
void displayRadioSelection(bool clearScreen)
{
    static int previousStationIndex = -1;
    static int previousScrollOffset = -1;

    const int MARGIN_TOP = 30;
    const int LINE_HEIGHT = 24;
    const int MAX_VISIBLE = 8;

    setScreenOn();
    isCategorySelection = false;
    isRadioSelection = true;

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

    // Scroll management
    static int scrollOffset = 0;
    if (activeIndex < scrollOffset)
        scrollOffset = activeIndex;
    else if (activeIndex >= scrollOffset + MAX_VISIBLE)
        scrollOffset = activeIndex - MAX_VISIBLE + 1;

    // Clamp scroll
    if (scrollOffset < 0) scrollOffset = 0;
    if (scrollOffset > radioCount - MAX_VISIBLE)
        scrollOffset = max(0, radioCount - MAX_VISIBLE);

    // Determine if we must fully redraw
    bool fullRedraw = clearScreen || (scrollOffset != previousScrollOffset);

    if (clearScreen) {
        tft.fillScreen(TFT_WHITE);
        displayMenuHeader("Stations");
    }
    

    tft.setFreeFont(&Roboto_Thin_24);

    for (int i = 0; i < MAX_VISIBLE; ++i) {
        int idx = scrollOffset + i;
        if (idx >= radioCount) break;

        const auto &station = radioStations[idx];

        int y = MARGIN_TOP + i * LINE_HEIGHT;
        bool isActive = (idx == activeIndex);
        bool wasActive = (idx == previousStationIndex);

        if (fullRedraw || isActive || wasActive) {
            uint16_t bgColor = isActive ? TFT_BLUE : (i % 2 == 0 ? TFT_WHITE : 0xE71C);
            uint16_t textColor = isActive ? TFT_WHITE : TFT_BLACK;

            tft.fillRect(0, y, 320, LINE_HEIGHT, bgColor);
            tft.setCursor(LEFT_MARGIN, y + 18);
            tft.setTextColor(textColor);
            printStationName(station.Name);
        }
    }

    previousStationIndex = activeIndex;
    previousScrollOffset = scrollOffset;
}




void setStation()
{
  stationInfo = "";
  songInfo = "";
  displayStation();
}
void displayDetails()
{
  setScreenOn();
  tft.setFreeFont(&Roboto_Thin_24);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(2, 40);
  tft.print("IP:");
  tft.setCursor(2, 70);
  tft.print(WiFi.localIP().toString());
  tft.setCursor(2, 120);
  tft.print("Connect with your pc/phone to this IP address to configure the stations.");
}
void switchRadioOn()
{
  connectToWIFI();
  clearTFTAllWhite();
  displayStation();
  radioIsOn = true;
}
void switchRadioOff()
{
  radioIsOn = false;
  setScreenOn();
  printError("Going in standby");
  log_i("Powering off");
  delay(5000);
  setScreenOff();
  // esp_deep_sleep_start();
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}
void audio_showstreamtitle(const char *info)
{
  songInfo = info;
  displaySongInfo();
}

void audio_showstation(const char *info)
{
  // keep using provided names, instead of steam names

  // stationInfo = info;
  // displayStationName();
}

void toggleMute()
{
  radioIsMuted = !radioIsMuted;
  if (radioIsMuted)
  {
    audio.setVolume(0);
  }
  else
  {
#ifdef HAS_ROTARIES
    audio.setVolume(VOLUME_MAX - rotaryVolume.readEncoder());
#endif
  }
  displayMute();
}

#ifdef HAS_REMOTE
void handleRemotePress(int64_t remotecode)
{

  Serial.print("Remote code is:");
  Serial.println(remotecode);
  // remote codes

  if (!radioIsOn)
  {
    if (remotecode == RC_RADIO_ON)
    {
      switchRadioOn();
    }
    return;
  }
  if (remotecode == RC_STANDBY)
  {
    playRadio = false;
  }
  if (remotecode == RC_PREVIOUS_STATION)
  {
    radStat::prevStation();
    displayRadioSelection(!isRadioSelection);
  }
  if (remotecode == RC_NEXT_STATION)
  {
    radStat::nextStation();
    displayRadioSelection(!isRadioSelection);
  }
  if (remotecode == RC_NEXT_CATEGORY)
  {
    if (!isRadioSelection)
    {
      radStat::nextCategory();
      displayCategorySelection(!isCategorySelection);
    }
    else
    {
      radStat::nextStation();
      displayRadioSelection(!isRadioSelection);
    }
  }
  if (remotecode == RC_PREVIOUS_CATEGORY)
  {
    if (!isRadioSelection)
    {
      radStat::prevCategory();
      displayCategorySelection(!isCategorySelection);
    }
    else
    {
      radStat::prevStation();
      displayRadioSelection(!isRadioSelection);
    }
  }
  if (remotecode == RC_TURN_ON_SCREEN)
  {
    onTunerShortClick();
  }
  if (remotecode == RC_SHOW_DETAILS)
  {
    displayDetails();
  }

  if (remotecode == RC_SAVE_STATION)
  {
    saveTheStation();
  }

  if (remotecode == RC_RADIO_OFF1 || remotecode == RC_RADIO_OFF2 || remotecode == RC_RADIO_OFF3)
  {
    // switchRadioOff();
    toggleMute();
  }
}
#endif
void saveTheVolume()
{
  File file1 = SPIFFS.open("/savedVolume.txt", "w", true);
  if (file1)
  {
#ifdef HAS_ROTARIES
    file1.print(rotaryVolume.readEncoder());
#endif
  }
  file1.close();
}

void saveTheStation()
{
  File file1 = SPIFFS.open("/savedStation.txt", "w", true);
  if (file1)
  {
    file1.print(radStat::activeRadioStation.Name);
  }
  file1.close();
  saveTheVolume();
  displaySaved();
}
void loadSavedVolume()
{
  File file1 = SPIFFS.open("/savedVolume.txt", "r", false);
  if (file1)
  {
    String volume = file1.readString();
    log_i("volume = %s", volume);
#ifdef HAS_ROTARIES
    if (volume.toInt())
    {
      rotaryVolume.setEncoderValue(volume.toInt());
    }
#endif
  }

  file1.close();
}

void loadSavedStation()
{
  File file1 = SPIFFS.open("/savedStation.txt", "r", false);
  if (file1)
  {
    String stationName = file1.readString();
    radStat::setActiveRadioStationName(stationName);
  }

  file1.close();
}
void loadSettings()
{
  // todo:
  // screenTimoutSec
  // autoSwitchSec
}

void saveSettings()
{
}

void startWebServer()
{

  //  Route for root / web page https://raphaelpralat.medium.com/example-of-json-rest-api-for-esp32-4a5f64774a05
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });
  server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/stations.txt", "text/plain"); });

  server.on(
      "/post",
      HTTP_POST,
      [](AsyncWebServerRequest *request) {},
      NULL,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
      {
        if (index == 0)
        {
          stationsFile = SPIFFS.open("/stations.txt", "w");
        }

        stationsFile.write(data, len); // Write data in chunks

        if ((index + len) == total)
        {
          stationsFile.close();
          request->send(200);
          ESP.restart();
        }
      });

  server.serveStatic("/", SPIFFS, "/");

#ifdef CORS_DEBUG
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
#endif
  server.begin();
}

#ifdef HAS_ROTARIES
void IRAM_ATTR readEncoderISR()
{
  rotaryTuner.readEncoder_ISR();
  rotaryVolume.readEncoder_ISR();
}
#endif

void setup()
{

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  delay(500);
  log_i("Starting");
  tft.begin();
  tft.setTextWrap(true, false);
  tft.setTextPadding(15);
  tft.setRotation(TFT_ROTATION);

#ifdef HAS_ROTARIES
  pinMode(ROTARY_ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ROTARY_ENCODER_B_PIN, INPUT_PULLUP);

  pinMode(ROTARY_ENCODER2_A_PIN, INPUT_PULLUP);
  pinMode(ROTARY_ENCODER2_B_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  rotaryTuner.begin();
  rotaryTuner.disableAcceleration();
  rotaryTuner.setup(readEncoderISR);
  rotaryTuner.setBoundaries(0, 1000, true); // minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryTuner.setEncoderValue(500);
  prevRotaryTunerCode = 500;
  // rotaryTuner.setAcceleration(250);

  rotaryVolume.begin();
  rotaryVolume.disableAcceleration();
  rotaryVolume.setup(readEncoderISR);
  rotaryVolume.setBoundaries(0, VOLUME_MAX, false); // minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryVolume.setEncoderValue(VOLUME_MAX - VOLUME_DEFAULT);
  // rotaryVolume.setAcceleration(250);
#endif

  // tft.begin(TFT_CS, TFT_DC, VSPI, TFT_MOSI, TFT_MISO, TFT_SCK);
  setScreenOn();
  // tft.setFrequency(TFT_FREQUENCY);
  tft.setFreeFont(&Roboto_24);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(25, 100);
  tft.print("Starting...");

#ifdef HAS_SDCARD
  if (!SD.begin(5))
  {
    log_w("SD Card Mount Failed. It is not mandatory, program will continue.");
  }
#endif
  if (!SPIFFS.begin(true))
  {
    log_e("An Error has occurred while mounting SPIFFS");
    return;
  }

  // this is the file where the stations are stored
  if (!SPIFFS.exists("/stations.txt"))
  {
    SPIFFS.rename("/stations_input.txt", "/stations.txt");
  }

  loadSettings();
  loadStations();
  loadSavedStation();
  loadSavedVolume();
  connectToWIFI();
  displayStation();
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolumeSteps(VOLUME_STEPS);
#ifdef HAS_ROTARIES
  audio.setVolume(VOLUME_MAX - rotaryVolume.readEncoder());
#else
  audio.setVolume(VOLUME_DEFAULT);
#endif

#ifdef MONO_OUTPUT
  audio.forceMono(true);
#endif
#ifdef HAS_REMOTE
  irrecv.enableIRIn();
#endif

  startWebServer();
}

void onTunerShortClick()
{
  if (!tftIsOn)
  {
    setScreenOn();
    return;
  }
  {
    if (!isCategorySelection && !isRadioSelection)
    {
      displayCategorySelection(true);
    }
    else if (isCategorySelection)
    {
      isCategorySelection = false;
      isRadioSelection = true;
      displayRadioSelection(true);
    }
    else
    {
      if (radStat::activeRadioStation.Name != radStat::previousRadioStation.Name)
      {
        radStat::resetPreviousRadioStation();
        isRadioSelection = false;
        startRadioStream();
      }
      setStation();
    }
  }
}

#ifdef HAS_ROTARIES
void onTunerLongClick()
{
  saveTheStation();
}
void handleTunerButton()
{
  static unsigned long buttonPressedTime = 0;
  static bool isLongpress = false;
  bool isEncoderButtonDown = rotaryTuner.isEncoderButtonDown();
  if (isEncoderButtonDown)
  {
    if (!buttonPressedTime)
    {
      buttonPressedTime = millis();
    }

    if (!isLongpress && (millis() - buttonPressedTime >= longPressAfterMiliseconds))
    {
      onTunerLongClick();
      isLongpress = true;
    }
  }
  else
  {
    if (buttonPressedTime && !isLongpress)
    {
      onTunerShortClick();
    }
    buttonPressedTime = 0;
    isLongpress = false;
  }
}

void onVolumeShortClick()
{

  // switchRadioOff();
  toggleMute();
}
void onVolumeLongClick()
{
  displayDetails();
}

void handleVolumeButton()
{
  static unsigned long buttonPressedTime = 0;
  static bool isLongpress = false;
  bool isEncoderButtonDown = rotaryVolume.isEncoderButtonDown();
  if (isEncoderButtonDown)
  {
    if (!buttonPressedTime)
    {
      buttonPressedTime = millis();
    }

    if (!isLongpress && (millis() - buttonPressedTime >= longPressAfterMiliseconds))
    {
      onVolumeLongClick();
      isLongpress = true;
    }
  }
  else
  {
    if (buttonPressedTime && !isLongpress)
    {
      onVolumeShortClick();
    }
    buttonPressedTime = 0;
    isLongpress = false;
  }
}

void loopRotaryTuner()
{
  if (rotaryTuner.encoderChanged())
  {
    int currentRotaryTunerCode = rotaryTuner.readEncoder();
    Serial.println(rotaryTuner.readEncoder());
    if (currentRotaryTunerCode > prevRotaryTunerCode)
    {
      if (isCategorySelection)
      {
        radStat::prevCategory();
        displayCategorySelection(false);
      }
      else
      {
        radStat::prevStation();
        displayRadioSelection(!isRadioSelection);
      }
      if (currentRotaryTunerCode > 900)
      {
        currentRotaryTunerCode = 500;
        prevRotaryTunerCode = 499;
      }
      else
      {
        prevRotaryTunerCode = currentRotaryTunerCode;
      }
    }
    else if (currentRotaryTunerCode < prevRotaryTunerCode)
    {
      if (isCategorySelection)
      {
        radStat::nextCategory();
        displayCategorySelection(false);
      }
      else
      {
        radStat::nextStation();
        displayRadioSelection(!isRadioSelection);
      }
      if (currentRotaryTunerCode < 100)
      {
        currentRotaryTunerCode = 500;
        prevRotaryTunerCode = 501;
      }
      else
      {
        prevRotaryTunerCode = currentRotaryTunerCode;
      }
    }
  }

  handleTunerButton();
}
void loopRotaryVolume()
{
  if (rotaryVolume.encoderChanged())
  {
    if (radioIsMuted)
    {
      toggleMute();
    }
    audio.setVolume(VOLUME_MAX - rotaryVolume.readEncoder());
  }
  handleVolumeButton();
}
#endif

void loop()
{

#ifdef HAS_REMOTE
  if (irrecv.decode(&results))
  {
    handleRemotePress(results.value);
    irrecv.resume(); // Receive the next value
  }

#endif

  if (!radioIsOn)
  {
#ifdef HAS_ROTARIES
    if (rotaryVolume.isEncoderButtonClicked())
    {
      switchRadioOn();
    }
#endif
    return;
  }
#ifdef HAS_ROTARIES
  loopRotaryTuner();
  loopRotaryVolume();
#endif
  // if ((autoSwitchSec > -1) && (radioSwitchMillis > 0) && (millis() - radioSwitchMillis > autoSwitchSec * 1000))
  // {
  //   radioSwitchMillis = 0;
  //   if (radStat::activeRadioStation.Name != radStat::previousRadioStation.Name)
  //   {
  //     radStat::resetPreviousRadioStation();
  //     startRadioStream();
  //   }
  // }
  if ((screenTimeoutSec > 1) && (screenSwitchOnMillis > 0) && (millis() - screenSwitchOnMillis > screenTimeoutSec * 1000))
  {
    setScreenOff();
  }
  if (playRadio)
  {
    if (!audio.isRunning())
    {
      startRadioStream();
    }
    audio.loop();
  }
}
void printError(const char *error)
{
  clearTFTAllWhite();
  tft.setTextColor(TFT_RED);
  tft.setCursor(25, 100);
  tft.print(error);
}
void warnNotConnected(WiFiManager *myWiFiManager)
{
  printError("Could not connect. Connect your computer/phone to 'WIFI_RADIO' to configure wifi.");
  log_i("Could not connect. Connect your computer/phone to 'WIFI_RADIO' to configure wifi.");
}
void connectToWIFI()
{
  // manager.resetSettings();
  manager.setAPCallback(warnNotConnected);
  bool success = manager.autoConnect("WIFI_RADIO");
  if (!success)
  {
    log_w("Failed to connect");
  }
  else
  {
    log_i("Connected");
  }
}