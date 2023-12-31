#include "main1.h"

long screenSwitchOnMillis = 0;
long radioSwitchMillis = 0;

int screenTimeoutSec = 60;
int autoSwitchSec = 3;

File stationsFile;

String _sspw = "";
String _ssid = "";

TFT tft(0, 0);
WiFiManager manager;

Audio audio;
#ifdef HAS_REMOTE
IRrecv irrecv(kRecvPin);
decode_results results;
#endif

bool radioIsOn = true;
bool tftIsOn = true;
bool playRadio = true;

unsigned long shortPressAfterMiliseconds = 50;  // how long short press shoud be. Do not set too low to avoid bouncing (false press events).
unsigned long longPressAfterMiliseconds = 1000; // how long čong press shoud be.

int prevRotaryTunerCode = 500;

AsyncWebServer server(80);
#define FONT_INFO 0
#define FONT_CATEGORY 1
#define FONT_STATION 2

/***********************************************************************************************************************
 *                                                 I M A G E                                                           *
 ***********************************************************************************************************************/
const unsigned short *_fonts[3] = {
    Courier_New16x30, Courier_New19x36, Courier_New22x39};
bool startsWith(const char *base, const char *searchString)
{
  char c;
  while ((c = *searchString++) != '\0')
    if (c != *base++)
      return false;
  return true;
}

bool endsWith(const char *base, const char *searchString)
{
  int slen = strlen(searchString) - 1;
  const char *p = base + strlen(base) - 1;
  while (p > base && isspace(*p))
    p--; // rtrim
  p -= slen;
  if (p < base)
    return false;
  return (strncmp(p, searchString, slen) == 0);
}

boolean drawImage(const char *scImg, uint16_t posX, uint16_t posY, uint16_t maxWidth, uint16_t maxHeigth)
{
#ifdef HAS_SDCARD
  if (endsWith(scImg, "bmp"))
  {
    return tft.drawBmpFile(SD, scImg, posX, posY, maxWidth, maxHeigth);
  }
  if (endsWith(scImg, "jpg"))
  {
    return tft.drawJpgFile(SD, scImg, posX, posY, maxWidth, maxHeigth);
  }
#endif
  return false; // neither jpg nor bmp
}

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

void loadStations()
{
  File configurations1 = SPIFFS.open("/stations.json", "r");
  radStat::processJSON(configurations1);
  configurations1.close();
}
#ifdef HAS_SDCARD
void showStationImage(string name, string type, int position)
{

  string imgAdr0 = "/wifiradio/img/" + type + "/" + name + ".jpg";
  const char *imgAdr = imgAdr0.c_str();
  int maxHeight = 80;
  if (type == "radio")
  {
    maxHeight = 240;
  }

  log_i("Getting image from SD:%s", imgAdr);
  if (SD.exists(imgAdr))
  {
    drawImage(imgAdr, 0, position, 240, maxHeight);
  }
  else
  {
    tft.setCursor(25, position + 20);
    tft.setTextColor(TFT_BLACK);
    tft.print(name.c_str());
  }
}
#endif
void startRadioStream()
{
  String url4 = radStat::activeRadioStation.URL;
  const char *url1 = url4.c_str();
  log_i("trying url:>>>%s<<<", url1);
  audio.connecttohost(url1);
}
void setScreenOn()
{
  clearTFTAllWhite();
  setTFTbrightness(100);
  tftIsOn = true;
  screenSwitchOnMillis = millis();
}
void setScreenOff()
{
  clearTFTAllWhite();
  setTFTbrightness(0);
  tftIsOn = false;
  screenSwitchOnMillis = 0;
}

void displayStation()
{
  radStat::activeRadioStation.printDetails();
  radioSwitchMillis = millis();
  setScreenOn();
#ifdef SHOW_IMAGES
  showStationImage(radStat::activeRadioStation.Category.c_str(), "category", 0);
  showStationImage(radStat::activeRadioStation.Name.c_str(), "radio", 80);
#else
  tft.setTextColor(TFT_BLUE);
  tft.setFont(_fonts[FONT_CATEGORY]);
  tft.setCursor(25, 30);
  tft.print(radStat::activeRadioStation.Category.c_str());
  tft.setTextColor(TFT_BLACK);
  tft.setFont(_fonts[FONT_STATION]);
  tft.setCursor(25, 70);
  String nameReplaceUnderscroresBySpaces = radStat::activeRadioStation.Name;

  nameReplaceUnderscroresBySpaces.replace("_", " ");
  // Copy non-space characters to the result string

  tft.print(nameReplaceUnderscroresBySpaces.c_str());
  tft.setFont(_fonts[FONT_INFO]);

#endif
}

void setStation()
{
  displayStation();
}
void displayDetails()
{
  setScreenOn();
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(2, 20);
  tft.print("IP:");
  tft.setCursor(2, 50);
  tft.print(WiFi.localIP().toString());
  tft.setCursor(2, 100);
  tft.print("Connect with your pc/phone to this IP address to configure the stations.");
}
void switchRadioOn()
{
  connectToWIFI();
  displayStation();
  radioIsOn = true;
}
void switchRadioOff()
{
  radioIsOn = false;
  setScreenOn();
  printError("Going in standby");
  log_i("Powering off");
  drawImage("/wifiradio/img/shutdown.jpg", 0, 0);
  delay(5000);
  setScreenOff();
  // esp_deep_sleep_start();
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
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
    displayStation();
  }
  if (remotecode == RC_NEXT_STATION)
  {
    radStat::nextStation();
    displayStation();
  }
  if (remotecode == RC_NEXT_CATEGORY)
  {
    radStat::nextCategory();
    displayStation();
  }
  if (remotecode == RC_PREVIOUS_CATEGORY)
  {
    radStat::prevCategory();
    displayStation();
  }
  if (remotecode == RC_TURN_ON_SCREEN)
  {
    playRadio = true;
    displayStation();
    if (radStat::activeRadioStation.Name != radStat::previousRadioStation.Name)
    {
      radStat::resetPreviousRadioStation();

      startRadioStream();
    }
  }
  if (remotecode == RC_SHOW_DETAILS)
  {
    displayDetails();
  }

  if (remotecode == RC_SAVE_STATION)
  {
    saveTheStation();
#ifdef HAS_SDCARD
    if (SD.exists("/wifiradio/mp3/command/StationSaved.mp3"))
    {
      audio.connecttoFS(SD, "/wifiradio/mp3/command/StationSaved.mp3");
    }
#endif
  }

  if (remotecode == RC_RADIO_OFF1 || remotecode == RC_RADIO_OFF2 || remotecode == RC_RADIO_OFF3)
  {
    switchRadioOff();
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
  displayStation();
  saveTheVolume();

  tft.setTextColor(TFT_BLUE);
  tft.setCursor(5, 160);
  tft.print("Station Saved");
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
            { request->send(SPIFFS, "/stations.json", "application/json"); });

  server.on(
      "/post",
      HTTP_POST,
      [](AsyncWebServerRequest *request) {},
      NULL,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
      {
        if (index == 0)
        {
          stationsFile = SPIFFS.open("/stations.json", "w");
        }
        for (size_t i = 0; i < len; i++)
        {
          stationsFile.write(data[i]);
        }
        if ((len + index) == total)
        {
          stationsFile.close();
          ESP.restart();
        }

        request->send(200);
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

  tft.begin(TFT_CS, TFT_DC, VSPI, TFT_MOSI, TFT_MISO, TFT_SCK);
  setScreenOn();
  tft.setFrequency(TFT_FREQUENCY);
  tft.setRotation(TFT_ROTATION);
  tft.setFont(_fonts[FONT_INFO]);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(25, 80);
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
  if (!SPIFFS.exists("/stations.json"))
  {
    SPIFFS.rename("/stations_input.json", "/stations.json");
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
#ifdef HAS_ROTARIES
void onTunerShortClick()
{
  if (tftIsOn)
  {
    radStat::nextCategory();
    setStation();
  }
  else
  {
    displayStation();
  }
}
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

  switchRadioOff();
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
      radStat::prevStation();
      setStation();
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

      radStat::nextStation();

      setStation();

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
    Serial.println(rotaryVolume.readEncoder());
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
  if ((autoSwitchSec > -1) && (radioSwitchMillis > 0) && (millis() - radioSwitchMillis > autoSwitchSec * 1000))
  {
    radioSwitchMillis = 0;
    if (radStat::activeRadioStation.Name != radStat::previousRadioStation.Name)
    {
      radStat::resetPreviousRadioStation();
      startRadioStream();
    }
  }
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
  tft.setCursor(25, 80);
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