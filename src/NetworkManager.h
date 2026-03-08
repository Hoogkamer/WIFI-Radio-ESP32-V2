#pragma once

#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

class NetworkManager {
public:
    NetworkManager();
    void begin();
    void update();
    
    String getIP() const { return WiFi.localIP().toString(); }
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    
    void startWebServer();

private:
    AsyncWebServer _server;
    WiFiManager _wifiManager;
    
    static void warnNotConnected(WiFiManager *myWiFiManager);
};
