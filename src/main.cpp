#include "main1.h"

long previousMillis = 0;
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

#define TUNE_CATEGORY 1
#define TUNE_STATION 2
#define MAX_VOL 15

int rotaryTuneMode = TUNE_STATION;
int prevRotaryTunerCode = 500;

AsyncWebServer server(80);

/***********************************************************************************************************************
 *                                                 I M A G E                                                           *
 ***********************************************************************************************************************/
const unsigned short *_fonts[1] = {
    Courier_New16x30};
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

  if (endsWith(scImg, "bmp"))
  {
    // log_i("drawImage %s, x=%i, y=%i, mayWidth=%i, maxHeight=%i", scImg, posX, posY, maxWidth, maxHeigth);
    return tft.drawBmpFile(SD, scImg, posX, posY, maxWidth, maxHeigth);
  }
  if (endsWith(scImg, "jpg"))
  {
    return tft.drawJpgFile(SD, scImg, posX, posY, maxWidth, maxHeigth);
  }

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
    tft.setCursor(25, position);
    tft.setTextColor(TFT_BLACK);
    tft.print(name.c_str());
  }
}
void startRadioStream()
{
  String url4 = radStat::activeRadioStation.URL;
  const char *url1 = url4.c_str();
  log_i("trying url:>>>%s<<<", url1);
  audio.connecttohost(url1);
}
void setScreenOn()
{
  setTFTbrightness(100);
  tftIsOn = true;
  previousMillis = millis();
}
void setScreenOff()
{
  setTFTbrightness(0);
  tftIsOn = false;
  previousMillis = 0;
}
void displayStation()
{
  radStat::activeRadioStation.printDetails();
  radioSwitchMillis = millis();
  setScreenOn();
  clearTFTAllWhite();
  showStationImage(radStat::activeRadioStation.Category.c_str(), "category", 0);
  showStationImage(radStat::activeRadioStation.Name.c_str(), "radio", 80);
#ifdef HAS_ROTARIES
  displayTunerMode();
#endif
}

void displayTunerMode()
{
  if (rotaryTuneMode == TUNE_STATION)
  {
    tft.setTextColor(TFT_BLUE);
    Serial.println("Tunermode:TUNER");
    log_i("Tunermode:TUNER");
  }
  else
  {
    tft.setTextColor(TFT_WHITE);
  }
  tft.setCursor(25, 200);
  tft.print("STATION");
  if (rotaryTuneMode == TUNE_CATEGORY)
  {
    tft.setTextColor(TFT_BLUE);
    Serial.println("Tunermode:CATEGORY");
  }
  else
  {
    tft.setTextColor(TFT_WHITE);
  }
  tft.setCursor(125, 200);
  tft.print("CATEGORY");
}
void setStation()
{
  displayStation();
}
void displayDetails()
{
  setScreenOn();
  clearTFTAllWhite();
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(2, 20);
  tft.print("IP:");
  tft.setCursor(2, 50);
  tft.print(WiFi.localIP().toString());
  tft.setCursor(2, 100);
  tft.print("Connect with your pc/phone to this IP address to configure the stations.");
}
void handleRemotePress(int64_t remotecode)
{

  Serial.print("Remote code is:");
  Serial.println(remotecode);
  // remote codes

  if (!radioIsOn)
  {
    if (remotecode == 70386007955804) // AUX: switch on again
    {
      connectToWIFI();
      displayStation();
      radioIsOn = true;
    }
    return;
  }
  if (remotecode == 70386011603005) // STOP stream
  {
    playRadio = false;
  }
  if (remotecode == 70386011640495) // left: previous station
  {
    radStat::prevStation();
    setStation();
  }
  if (remotecode == 70386011624047) // right: next station
  {
    radStat::nextStation();
    setStation();
  }
  if (remotecode == 70386013224938) // down: next category
  {
    radStat::nextCategory();
    setStation();
  }
  if (remotecode == 70386013192042) // up: previous category
  {
    radStat::prevCategory();
    setStation();
  }
  if (remotecode == 70386011660258) // ok: Wakeup screen and show station and set active station
  {
    playRadio = true;
    displayStation();
    if (radStat::activeRadioStation.Name != radStat::previousRadioStation.Name)
    {
      radStat::resetPreviousRadioStation();

      startRadioStream();
    }
  }
  if (remotecode == 70386011657704) // Ipod menu: Show details
  {

    displayDetails();
  }

  if (remotecode == 70386010088896) // auto preset
  {
    setScreenOn();
    if (SD.exists("/wifiradio/mp3/command/StationSaved.mp3"))
    {
      audio.connecttoFS(SD, "/wifiradio/mp3/command/StationSaved.mp3");
    }
    saveTheStation();
  }

  if (remotecode == 70386011651201 || remotecode == 70386013196293 || remotecode == 70386010039552) // power off: OFF/CD/FM buttons
  {
    radioIsOn = false;
    setScreenOn();
    printError("Powering off");
    log_i("Powering off");
    drawImage("/wifiradio/img/shutdown.jpg", 0, 0);
    delay(5000);
    setScreenOff();
    // esp_deep_sleep_start();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
  }
}

void saveTheStation()
{
  File file1 = SPIFFS.open("/savedStation.txt", "w", true);
  if (file1)
  {
    file1.print(radStat::activeRadioStation.Name);
  }
  file1.close();
}
void loadSavedStation()
{
  File file1 = SPIFFS.open("/savedStation.txt", "r", false);
  if (file1)
  {
    String stationName = file1.readString();
    radStat::setActiveRadioStation(stationName);
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
          loadStations();
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
  rotaryVolume.setBoundaries(0, MAX_VOL, false); // minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryVolume.setEncoderValue(MAX_VOL - 3);
  // rotaryVolume.setAcceleration(250);
#endif

  tft.begin(TFT_CS, TFT_DC, VSPI, TFT_MOSI, TFT_MISO, TFT_SCK);
  setScreenOn();
  tft.setFrequency(TFT_FREQUENCY);
  tft.setRotation(TFT_ROTATION);

  tft.setFont(_fonts[0]);
  clearTFTAllBlack();
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(25, 80);
  tft.print("Starting...");

  if (!SD.begin(5))
  {
    log_w("SD Card Mount Failed. It is not mandatory, program will continue.");
  }
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
  connectToWIFI();
  displayStation();
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolumeSteps(100);
  audio.setVolume(MAX_VOL - rotaryVolume.readEncoder());
#ifdef MONO_OUTPUT
  audio.forceMono(true);
#endif
#ifdef HAS_REMOTE
  irrecv.enableIRIn();
#endif

  startWebServer();
}
#ifdef HAS_ROTARIES
void loopRotaryTuner()
{
  if (rotaryTuner.encoderChanged())
  {
    int currentRotaryTunerCode = rotaryTuner.readEncoder();
    Serial.println(rotaryTuner.readEncoder());
    if (currentRotaryTunerCode > prevRotaryTunerCode)
    {
      if (rotaryTuneMode == TUNE_STATION)
      {
        radStat::nextStation();
      }
      else
      { // TUNE_CATEGORY
        radStat::nextCategory();
      }
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
      if (rotaryTuneMode == TUNE_STATION)
      {
        radStat::prevStation();
      }
      else
      { // TUNE_CATEGORY
        radStat::prevCategory();
      }
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

  if (rotaryTuner.isEncoderButtonClicked())
  {
    if (tftIsOn)
    {
      if (rotaryTuneMode == TUNE_STATION)
      {
        rotaryTuneMode = TUNE_CATEGORY;
      }
      else
      {
        rotaryTuneMode = TUNE_STATION;
      }
    }
    displayStation();
  }
}
void loopRotaryVolume()
{
  if (rotaryVolume.encoderChanged())
  {
    Serial.println(rotaryVolume.readEncoder());
    audio.setVolume(MAX_VOL - rotaryVolume.readEncoder());
  }
  if (rotaryVolume.isEncoderButtonClicked())
  {
    Serial.println("VOL button pressed");
    displayDetails();
  }
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
#ifdef HAS_ROTARIES
  loopRotaryTuner();
  loopRotaryVolume();
#endif
  if (!radioIsOn)
  {
    return;
  }
  if ((autoSwitchSec > -1) && (radioSwitchMillis > 0) && (millis() - radioSwitchMillis > autoSwitchSec * 1000))
  {
    radioSwitchMillis = 0;
    if (radStat::activeRadioStation.Name != radStat::previousRadioStation.Name)
    {
      radStat::resetPreviousRadioStation();
      startRadioStream();
    }
  }
  if ((screenTimeoutSec > 1) && (previousMillis > 0) && (millis() - previousMillis > screenTimeoutSec * 1000))
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