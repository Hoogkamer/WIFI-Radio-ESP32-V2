#define VOLUME 100 // volume level 0-100
#define EEPROM_SIZE 2

#include "main1.h"

long previousMillis = 0;
long radioSwitchMillis = 0;

int radioStation = 0;
int previousRadioStation = 0;
int screenTimeoutSec = 60;
int autoSwitchSec = 3;

File stationsFile;

String _sspw = "";
String _ssid = "";

RadioStation radioStations[100];
String radioCategories[30];
int nrOfCategories = 0;
int nrOfStations = 0;
int activeCategory = 0;

TFT tft(0, 0);
WiFiManager manager;

Audio audio;
IRrecv irrecv(kRecvPin);
decode_results results;

bool radioIsOn = true;
bool playRadio = true;

AsyncWebServer server(80);
// AsyncWebSocket ws("/ws");

/***********************************************************************************************************************
 *                                                 I M A G E                                                           *
 ***********************************************************************************************************************/
const unsigned short *_fonts[1] = {
    // Times_New_Roman15x14,
    // Times_New_Roman21x17,
    // Times_New_Roman27x21,
    // Times_New_Roman34x27,
    Times_New_Roman38x31,
    // Times_New_Roman43x35,
};

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
 *                                                  A U D I O                                                          *
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 *                                                  C O M M O N                                                         *
 ***********************************************************************************************************************/

bool startsWith(const char *base, const char *searchString)
{
  char c;
  while ((c = *searchString++) != '\0')
    if (c != *base++)
      return false;
  return true;
}

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

/***********************************************************************************************************************
 *                                                  P R O G R A M                                                      *
 ***********************************************************************************************************************/

void loadStations()
{
  Serial.println("START Loading stations>>");
  DynamicJsonDocument schedulesjson(20000);
  schedulesjson = getStationData();

  JsonArray array = schedulesjson["stations"].as<JsonArray>();
  int cnt = 0;
  for (JsonVariant v : array)
  {
    JsonArray stat = v.as<JsonArray>();

    radioStations[cnt] = RadioStation(stat[2].as<String>(), stat[0].as<String>(), stat[1].as<String>());
    Serial.print("Station:");
    radioStations[cnt].printDetails();
    cnt++;
  }
  nrOfStations = cnt;

  int ccnt = 0;
  JsonArray array1 = schedulesjson["categories"].as<JsonArray>();
  for (JsonVariant v : array1)
  {

    radioCategories[ccnt] = v.as<String>();
    Serial.print("Categorie:");
    Serial.println(radioCategories[ccnt]);
    ccnt++;
  }
  nrOfCategories = ccnt;
  Serial.println("<< END Loading stations");
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

void showStationImage(string name, string type, int position)
{
  string imgAdr0 = "/wifiradio/img/" + type + "/" + name + ".jpg";
  const char *imgAdr = imgAdr0.c_str();
  int maxHeight = 80;
  if (type == "radio")
  {
    maxHeight = 240;
  }

  Serial.println(imgAdr);
  if (SD.exists(imgAdr))
  {
    drawImage(imgAdr, 0, position, 240, maxHeight);
  }
  else
  {
    tft.setCursor(25, position);
    tft.print(name.c_str());
  }
}
void startRadioStream()
{
  RadioStation thisStation = radioStations[radioStation];
  String url4 = thisStation.URL;
  const char *url1 = url4.c_str();
  Serial.println("trying url:>>>" + String(url1) + "<<<");
  audio.connecttohost(url1);
  activeCategory = findStringIndex(radioCategories, nrOfCategories, thisStation.Category);
  if (activeCategory < 0)
  {
    activeCategory = 0;
  }
}
void displayStation()
{
  RadioStation thisStation = radioStations[radioStation];
  thisStation.printDetails();
  previousMillis = millis();
  radioSwitchMillis = millis();
  setTFTbrightness(100);
  clearTFTAllWhite();
  showStationImage(thisStation.Category.c_str(), "category", 0);
  showStationImage(thisStation.Name.c_str(), "radio", 80);
}
void setStation()
{
  displayStation();
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
    prevStation();
    setStation();
  }
  if (remotecode == 70386011624047) // right: next station
  {
    nextStation();
    setStation();
  }
  if (remotecode == 70386013224938) // down: next category
  {
    nextCategory();
    setStation();
  }
  if (remotecode == 70386013192042) // up: previous category
  {
    prevCategory();
    setStation();
  }
  if (remotecode == 70386011660258) // ok: Wakeup screen and show station and set active station
  {
    playRadio = true;
    displayStation();
    if (radioStation != previousRadioStation)
    {
      previousRadioStation = radioStation;
      startRadioStream();
    }
  }
  if (remotecode == 70386011657704) // Ipod menu: Show details
  {
    previousMillis = millis();
    setTFTbrightness(100);
    clearTFTAllWhite();
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(25, 80);
    tft.print("IP:");
    tft.setCursor(25, 110);
    tft.print(WiFi.localIP().toString());
  }

  if (remotecode == 70386010088896) // auto preset
  {
    previousMillis = millis();
    setTFTbrightness(100);
    if (SD.exists("/wifiradio/mp3/command/StationSaved.mp3"))
    {
      audio.connecttoFS(SD, "/wifiradio/mp3/command/StationSaved.mp3");
    }
    saveTheStation();
  }

  if (remotecode == 70386011651201 || remotecode == 70386013196293 || remotecode == 70386010039552) // power off: OFF/CD/FM buttons
  {
    radioIsOn = false;
    setTFTbrightness(100);
    printError("Powering off");
    Serial.print("Powering off");
    drawImage("/wifiradio/img/shutdown.jpg", 0, 0);
    delay(5000);
    setTFTbrightness(0);
    // esp_deep_sleep_start();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
  }

  Serial.print("radio number is:");
  Serial.println(radioStation);
  Serial.print("Category is:");
  Serial.println(radioCategories[activeCategory]);
}

void saveTheStation()
{
  File file1 = SPIFFS.open("/savedStation.txt", "w", true);
  if (file1)
  {
    file1.print(radioStations[radioStation].Name);
  }
  file1.close();
}
void loadSavedStation()
{
  File file1 = SPIFFS.open("/savedStation.txt", "r", false);
  radioStation = 0;
  if (file1)
  {
    String stationName = file1.readString();
    for (int i = 0; i < nrOfStations; i++)
    {
      if (radioStations[i].Name == stationName)
      {
        radioStation = i;
        break;
      }
    }
  }

  previousRadioStation = radioStation;
  file1.close();
  Serial.println("Loaded station" + String(radioStation));
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
// void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
// {
//   AwsFrameInfo *info = (AwsFrameInfo *)arg;
//   if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
//   {
//     data[len] = 0;
//     // message = (char*)data;
//     // if (message.indexOf("1s") >= 0) {
//     //   sliderValue1 = message.substring(2);
//     //   dutyCycle1 = map(sliderValue1.toInt(), 0, 100, 0, 255);
//     //   Serial.println(dutyCycle1);
//     //   Serial.print(getSliderValues());
//     //   notifyClients(getSliderValues());
//     // }
//     // if (message.indexOf("2s") >= 0) {
//     //   sliderValue2 = message.substring(2);
//     //   dutyCycle2 = map(sliderValue2.toInt(), 0, 100, 0, 255);
//     //   Serial.println(dutyCycle2);
//     //   Serial.print(getSliderValues());
//     //   notifyClients(getSliderValues());
//     // }
//     // if (message.indexOf("3s") >= 0) {
//     //   sliderValue3 = message.substring(2);
//     //   dutyCycle3 = map(sliderValue3.toInt(), 0, 100, 0, 255);
//     //   Serial.println(dutyCycle3);
//     //   Serial.print(getSliderValues());
//     //   notifyClients(getSliderValues());
//     // }
//     // if (strcmp((char*)data, "getValues") == 0) {
//     //   notifyClients(getSliderValues());
//     // }
//   }
// }

// void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
// {
//   switch (type)
//   {
//   case WS_EVT_CONNECT:
//     Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
//     break;
//   case WS_EVT_DISCONNECT:
//     Serial.printf("WebSocket client #%u disconnected\n", client->id());
//     break;
//   case WS_EVT_DATA:
//     handleWebSocketMessage(arg, data, len);
//     break;
//   case WS_EVT_PONG:
//   case WS_EVT_ERROR:
//     break;
//   }
// }

// void initWebSocket()
// {
//   // ws.onEvent(onEvent);
//   // server.addHandler(&ws);
// }

DynamicJsonDocument getStationData()
{
  // print content of stations data file to see what is in it
  File configurations1 = SPIFFS.open("/stations.json", "r");
  Serial.println("stations.json >>>");
  while (configurations1.available())
  {
    Serial.write(configurations1.read());
  }
  Serial.println("<<<<");
  configurations1.close();

  File configurations = SPIFFS.open("/stations.json", "r");
  DynamicJsonDocument stationsjson(20000);

  DeserializationError err = deserializeJson(stationsjson, configurations);
  Serial.println(err.c_str());
  if (err)
  {
    Serial.print(F("deserializeJson() failed with code, restarting "));
    Serial.println(err.c_str());
    delay(5000);
    printError("Memory problem, will restart");

    ESP.restart(); // try to free up memory
  }
  else
  {
  }

  Serial.println("Found!");

  configurations.close();

  return stationsjson;
}

void startWebServer()
{

  // initWebSocket();
  //  Route for root / web page https://raphaelpralat.medium.com/example-of-json-rest-api-for-esp32-4a5f64774a05
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });
  // server.on("/get-data1", HTTP_GET, [](AsyncWebServerRequest *request)
  //           { request->send(200, "application/json", getStationData()); });
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
          Serial.println("opening file");
        }
        for (size_t i = 0; i < len; i++)
        {
          Serial.write(data[i]);
          stationsFile.write(data[i]);
        }
        if ((len + index) == total)
        {
          stationsFile.close();
          Serial.println("closing file");
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

void setup()
{

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  delay(500);
  Serial.println("----Start");

  tft.begin(TFT_CS, TFT_DC, VSPI, TFT_MOSI, TFT_MISO, TFT_SCK);
  setTFTbrightness(100);
  tft.setFrequency(TFT_FREQUENCY);
  tft.setRotation(TFT_ROTATION);
  Serial.println("----Start1");

  tft.setFont(_fonts[0]);
  clearTFTAllBlack();
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(25, 80);
  tft.print("Starting...");
  tft.setTextColor(TFT_BLACK);
  Serial.println("----Start2");
  if (!SD.begin(5))
  {
    Serial.println("SD Card Mount Failed. It is not mandatory, program will continue.");
  }
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
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
  audio.setVolume(12);
  irrecv.enableIRIn();
  startWebServer();
}

void loop()
{
  if (irrecv.decode(&results))
  {
    handleRemotePress(results.value);
    irrecv.resume(); // Receive the next value
  }
  if (!radioIsOn)
  {
    return;
  }
  if ((autoSwitchSec > -1) && (radioSwitchMillis > 0) && (millis() - radioSwitchMillis > autoSwitchSec * 1000))
  {
    radioSwitchMillis = 0;
    Serial.println("auto switching??");
    if (radioStation != previousRadioStation)
    {
      previousRadioStation = radioStation;
      startRadioStream();
      Serial.println("auto switching!!");
    }
  }
  if ((screenTimeoutSec > 1) && (previousMillis > 0) && (millis() - previousMillis > screenTimeoutSec * 1000))
  {
    previousMillis = 0;
    setTFTbrightness(0);
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
void connectToWIFI()
{
  // manager.resetSettings();
  bool success = manager.autoConnect("WIFI_RADIO");
  if (!success)
  {
    Serial.println("Failed to connect");
  }
  else
  {
    Serial.println("Connected");
  }
}