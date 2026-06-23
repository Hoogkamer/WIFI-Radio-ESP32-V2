#include "NetworkManager.h"
#include "DisplayManager.h"

extern DisplayManager display;

NetworkManager::NetworkManager() : _server(80) {
}

void NetworkManager::begin() {
    _wifiManager.setAPCallback(warnNotConnected);
    _wifiManager.setConfigPortalTimeout(120); // 2 minutes config portal timeout

    // Try direct connection with stored credentials first.
    // This bypasses WiFiManager's internal credential check (esp_wifi_get_config)
    // which can fail on alternating boots due to NVS WiFi mode state issues.
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    delay(500);

    log_i("Attempting direct WiFi connection with stored credentials...");
    display.printError("Connecting...");
    WiFi.begin(); // uses NVS-stored SSID/password

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        wl_status_t s = WiFi.status();
        // Exit early on terminal failures (no stored creds, wrong password, etc.)
        if (s == WL_NO_SSID_AVAIL || s == WL_CONNECT_FAILED) {
            log_w("WiFi.begin() failed immediately (status=%d)", s);
            break;
        }
        delay(250);
    }

    if (WiFi.status() == WL_CONNECTED) {
        log_i("WiFi Connected directly. IP: %s", getIP().c_str());
        startWebServer();
        return;
    }

    // Direct connection failed — fall back to WiFiManager config portal
    log_w("Direct WiFi connection failed (status=%d), starting config portal...", WiFi.status());
    WiFi.disconnect(true);
    delay(500);

    if (!_wifiManager.autoConnect("WIFI_RADIO")) {
        log_w("Config portal timed out, restarting...");
        display.printError("Restarting...");
        delay(3000);
        ESP.restart();
    }

    log_i("WiFi Connected via portal. IP: %s", getIP().c_str());
    startWebServer();
}

void NetworkManager::update() {
    // WiFiManager and AsyncWebServer are mostly event-driven
}

void NetworkManager::warnNotConnected(WiFiManager *myWiFiManager) {
    log_i("Could not connect. Connect your computer/phone to 'WIFI_RADIO' to configure wifi.");
    display.displayWiFiInstructions(myWiFiManager->getConfigPortalSSID());
}

void NetworkManager::startWebServer() {
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });
    
    _server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/stations.txt", "text/plain");
    });

    _server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            static File stationsFile;
            if (index == 0) {
                stationsFile = LittleFS.open("/stations.txt", "w");
            }
            if (stationsFile) {
                stationsFile.write(data, len);
            }
            if ((index + len) == total) {
                if (stationsFile) stationsFile.close();
                request->send(200);
                delay(100);
                ESP.restart();
            }
        }
    );

    _server.serveStatic("/", LittleFS, "/");

#ifdef CORS_DEBUG
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
#endif

    _server.begin();
}
