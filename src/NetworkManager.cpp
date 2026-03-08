#include "NetworkManager.h"

NetworkManager::NetworkManager() : _server(80) {
}

void NetworkManager::begin() {
    _wifiManager.setAPCallback(warnNotConnected);
    if (!_wifiManager.autoConnect("WIFI_RADIO")) {
        log_w("Failed to connect to WiFi");
    } else {
        log_i("WiFi Connected. IP: %s", getIP().c_str());
    }
    startWebServer();
}

void NetworkManager::update() {
    // WiFiManager and AsyncWebServer are mostly event-driven
}

void NetworkManager::warnNotConnected(WiFiManager *myWiFiManager) {
    log_i("Could not connect. Connect your computer/phone to 'WIFI_RADIO' to configure wifi.");
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
