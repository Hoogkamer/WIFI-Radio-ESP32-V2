#pragma once

#include <Arduino.h>
#include "Audio.h"
#include "radStat.h"

class AudioManager {
public:
    AudioManager();
    void begin(int bclk, int lrc, int dout);
    void update();
    
    void connectToHost(const char* url);
    void connectToStation(const radStat::RadioStation& station);
    
    void setVolume(uint8_t volume);
    uint8_t getVolume() const { return _volume; }
    
    void toggleMute();
    bool isMuted() const { return _isMuted; }
    
    bool isRunning() { return _audio.isRunning(); }
    void loop() { _audio.loop(); }
    
    void stop();

    // Callback helpers
    void setStreamTitleCallback(void (*cb)(const char*)) { _streamTitleCallback = cb; }

private:
    Audio _audio;
    uint8_t _volume;
    uint8_t _preMuteVolume;
    bool _isMuted;
    
    void (*_streamTitleCallback)(const char*);
};

// Global instance or managed by main
extern AudioManager audioManager;
