#define VOLUME 100 // volume level 0-100
#define EEPROM_SIZE 2

#include "main1.h"

long previousMillis = 0;
long radioSwitchMillis = 0;

int radioStation = 0;
int previousRadioStation = 0;
int screenTimeoutSec = 10;
int autoSwitchSec = 3;

String _sspw = "ho03840384";
String _ssid = "Pixel_5047";

String ftp_username = "";
String ftp_pw = "";

RadioStation radioStations[99];
String radioCategories[99];
int nrOfCategories = 0;
int nrOfStations = 0;
int activeCategory = 0;

TFT tft(0, 0);
WiFiManager manager;

FtpServer ftpSrv;
Audio audio;
IRrecv irrecv(kRecvPin);
decode_results results;

bool radioIsOn = true;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

/***********************************************************************************************************************
 *                                                 I M A G E                                                           *
 ***********************************************************************************************************************/
const unsigned short *_fonts[7] = {
    Times_New_Roman15x14,
    Times_New_Roman21x17,
    Times_New_Roman27x21,
    Times_New_Roman34x27,
    Times_New_Roman38x31,
    Times_New_Roman43x35,
    Big_Numbers133x156 // ASCII 32...64 only
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

boolean loadStationsCSV()
{
  String Category = "", Name = "", URL = "";
  String currentLine = "", tmp = "";

  uint16_t cnt = 0;
  // StationList
  if (!SD.exists("/wifiradio/stations.csv"))
  {
    Serial.println("stations not found");
    return false;
  }

  File file = SD.open("/wifiradio/stations.csv");

  if (!file)
  {
    Serial.println("cannot open stations");
    return false;
  }

  while (file.available())
  {
    currentLine = file.readStringUntil('\n');
    int rfound = currentLine.indexOf('\r');
    Serial.println(">>" + currentLine + "<<");
    if (rfound != -1)
    {
      currentLine.remove(rfound);
    }
    Serial.println("!>>" + currentLine + "<<");
    uint p = 0, q = 0;
    Category = "";
    Name = "";
    URL = "";
    for (int i = 0; i < currentLine.length() + 1; i++)
    {
      if (currentLine[i] == '\t' || i == currentLine.length())
      {
        if (p == 0)
          Category = currentLine.substring(q, i);
        if (p == 1)
          Name = currentLine.substring(q, i);
        if (p == 2)
          URL = currentLine.substring(q, i);

        p++;
        i++;
        q = i;
      }
    }
    if (Name == "")
      continue; // is empty
    if (startsWith(Category.c_str(), "*"))
      continue;
    if (URL == "")
      continue; // is empty
    radioStations[cnt] = RadioStation(Category, Name, URL);
    radioStations[cnt].printDetails();
    cnt++;
    // Check if the category is already present
    bool categoryExists = false;
    for (int i = 0; i < nrOfCategories; ++i)
    {
      if (radioCategories[i] == Category)
      {
        categoryExists = true;
        break;
      }
    }
    if (!categoryExists)
    {
      radioCategories[nrOfCategories] = Category;
      nrOfCategories++;
    }
  }
  nrOfStations = cnt;
  file.close();
  Serial.println("stationlist internally loaded");
  Serial.println("number of stations: " + String(nrOfStations));

  Serial.println("number of categories: " + String(nrOfCategories));
  loadSavedStation();

  return true;
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
    tft.setCursor(25, 140);
    tft.print("ID:");
    tft.setCursor(100, 140);
    tft.print(ftp_username);
    tft.setCursor(25, 170);
    tft.print("PW:");
    tft.setCursor(100, 170);
    tft.print(ftp_pw);
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
  File file1 = SD.open("/wifiradio/savedStation.txt", "w", true);
  if (file1)
  {
    file1.print(radioStation);
  }
}
void loadSavedStation()
{
  File file1 = SD.open("/wifiradio/savedStation.txt", "r", false);
  if (file1)
  {
    String line = file1.readString();
    radioStation = stoi(line.c_str());
    Serial.print("1.1 Radiostation read:");
    Serial.println(line);
    Serial.println(radioStation);
  }
  Serial.println("2");
  if (radioStation < 0)
  {
    radioStation = 0;
  }
  if (radioStation > nrOfStations)
  {
    radioStation = 0;
  }
  previousRadioStation = radioStation;

  file1.close();

  Serial.println("Loaded station" + String(radioStation));
}
void loadSettings()
{
  if (!SD.begin(5))
  {
    Serial.println("SD Card Mount Failed");
    printError("SD Card Mount Failed");
    return;
  }
  delay(1500);
  File file = SD.open("/wifiradio/settings.json", "r", false);
  delay(1500);
  if (!file)
  {
    Serial.println("problem opening: /wifiradio/settings.json");
    printError("problem opening: /wifiradio/settings.json");
  }
  String jO = file.readString();
  file.close();

  // JSONVar jV = JSON.parse(jO);
  // screenTimeoutSec = (uint8_t)jV["screenTimoutSec"];
  // autoSwitchSec = (uint8_t)jV["autoSwitchSec"];

  // _ssid = (const char *)jV["ssid"];
  // _sspw = (const char *)jV["sspw"];
  // ftp_username = (const char *)jV["ftpid"];
  // ftp_pw = (const char *)jV["ftppw"];
  // Serial.println("1");
}

void saveSettings()
{
}
String processor(const String &var)
{
  Serial.println(var);
  String ledState;
  if (var == "STATE")
  {
    ledState = "test";
    return ledState;
  }
  return String();
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    // message = (char*)data;
    // if (message.indexOf("1s") >= 0) {
    //   sliderValue1 = message.substring(2);
    //   dutyCycle1 = map(sliderValue1.toInt(), 0, 100, 0, 255);
    //   Serial.println(dutyCycle1);
    //   Serial.print(getSliderValues());
    //   notifyClients(getSliderValues());
    // }
    // if (message.indexOf("2s") >= 0) {
    //   sliderValue2 = message.substring(2);
    //   dutyCycle2 = map(sliderValue2.toInt(), 0, 100, 0, 255);
    //   Serial.println(dutyCycle2);
    //   Serial.print(getSliderValues());
    //   notifyClients(getSliderValues());
    // }
    // if (message.indexOf("3s") >= 0) {
    //   sliderValue3 = message.substring(2);
    //   dutyCycle3 = map(sliderValue3.toInt(), 0, 100, 0, 255);
    //   Serial.println(dutyCycle3);
    //   Serial.print(getSliderValues());
    //   notifyClients(getSliderValues());
    // }
    // if (strcmp((char*)data, "getValues") == 0) {
    //   notifyClients(getSliderValues());
    // }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  // ws.onEvent(onEvent);
  // server.addHandler(&ws);
}

String getStationData()
{
  String result;
  result = "";

  File configurations = SPIFFS.open("/stations.json", "r"); // Enter the file name
  if (!configurations || !configurations.size())
  {
    Serial.println("json stations not found");
    DynamicJsonDocument data(2048);
    JsonArray categories = data.createNestedArray("categories");
    categories.add("Pop");
    categories.add("Chill");
    categories.add("Retro");
    categories.add("Jazz");

    JsonArray stations = data.createNestedArray("stations");
    JsonArray station1 = stations.createNestedArray();
    station1.add("Veronica");
    station1.add("https://playerservices.streamtheworld.com/api/livestream-redirect/VERONICA.mp3");
    station1.add("Pop");

    serializeJson(data, result);
  }
  else
  {
    DynamicJsonDocument schedulesjson(20000);
    DeserializationError err = deserializeJson(schedulesjson, configurations);
    Serial.println(err.c_str());
    if (err)
    {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
    }
    else
    {
      serializeJson(schedulesjson, result);
    }

    Serial.println("Found!");
    Serial.println(result);
  }
  configurations.close();

  return result;
}
void startWebServer()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  initWebSocket();
  // Route for root / web page https://raphaelpralat.medium.com/example-of-json-rest-api-for-esp32-4a5f64774a05
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });
  server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "application/json", getStationData()); });
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "Post route"); });
  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/post-message", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                                         {
      Serial.println("post start");
      DynamicJsonDocument data(20000);
      if (json.is<JsonArray>())
      {
        data = json.as<JsonArray>();
      }
      else if (json.is<JsonObject>())
      {
        data = json.as<JsonObject>();
      }
      String response;
      serializeJson(data, response);
      request->send(200, "application/json", response);
      Serial.println(response); 
  File configurations = SPIFFS.open("/stations.json", "w");

  if(configurations.print(response)) {
    Serial.println("config written");
  } else {
    Serial.println("config NOT written");
  } });
  server.addHandler(handler);

  // Route to load style.css file
  server.serveStatic("/", SPIFFS, "/");

#ifdef CORS_DEBUG
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With");

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

  tft.setFont(_fonts[5]);
  clearTFTAllBlack();
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(25, 80);
  tft.print("Starting...");
  tft.setTextColor(TFT_BLACK);
  Serial.println("----Start2");
  // loadSettings();

  // loadStationsCSV();

  connectToWIFI();
  startWebServer();
  // displayStation();
  // audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  // audio.setVolume(12);
  // ftpSrv.begin(SD, ftp_username, ftp_pw); // username, password for ftp.

  // irrecv.enableIRIn();
}

void loop()
{
  // if (irrecv.decode(&results))
  // {
  //   handleRemotePress(results.value);
  //   irrecv.resume(); // Receive the next value
  // }
  // if (!radioIsOn)
  // {
  //   return;
  // }
  // if ((autoSwitchSec > -1) && (radioSwitchMillis > 0) && (millis() - radioSwitchMillis > autoSwitchSec * 1000))
  // {
  //   radioSwitchMillis = 0;
  //   Serial.println("auto switching??");
  //   if (radioStation != previousRadioStation)
  //   {
  //     previousRadioStation = radioStation;
  //     startRadioStream();
  //     Serial.println("auto switching!!");
  //   }
  // }
  // if ((screenTimeoutSec > 1) && (previousMillis > 0) && (millis() - previousMillis > screenTimeoutSec * 1000))
  // {
  //   previousMillis = 0;
  //   setTFTbrightness(0);
  // }
  // if (!audio.isRunning())
  // {
  //   startRadioStream();
  // }
  // audio.loop();
  // ftpSrv.handleFTP();
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
void connectToWIFI1()
{
  WiFi.mode(WIFI_MODE_STA);
  setTFTbrightness(100);
  drawImage("/wifiradio/img/start.jpg", 0, 0);
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++)
  {
    Serial.println(WiFi.SSID(i));
  }
  Serial.println("Connecting to ssd:" + _ssid + ", pw:" + _sspw);
  WiFi.disconnect();
  delay(500);
  WiFi.begin(_ssid, _sspw);

  int tries = 0;

  while (WiFi.status() != WL_CONNECTED)
  {
    tries++;
    delay(500);
    Serial.print(".");
    if (tries > 20)
    {
      WiFi.disconnect();
      delay(500);
      WiFi.begin(_ssid, _sspw);
      tries = 0;
      Serial.println("Retrying");
    }
  }
  Serial.println("WiFi connected");
}