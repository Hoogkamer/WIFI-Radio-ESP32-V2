#pragma once

#include <Arduino.h>
#include "AiEsp32RotaryEncoder.h"

#ifdef HAS_REMOTE
#include <IRrecv.h>
#include <IRutils.h>
#endif

class InputManager {
public:
    enum Action {
        NONE,
        STATION_NEXT,
        STATION_PREV,
        CATEGORY_NEXT,
        CATEGORY_PREV,
        VOLUME_CHANGED,
        VOLUME_TOGGLE_MUTE,
        SCREEN_TOGGLE,
        SCREEN_ON,
        STATION_SAVE,
        SHOW_DETAILS,
        SETTINGS_ENTER,
        SETTINGS_EXIT,
        RADIO_OFF
    };

    typedef void (*ActionCallback)(Action action);

    InputManager();
    void begin();
    void update();
    void setActionCallback(ActionCallback cb) { _callback = cb; }
    
    uint8_t getVolume() { 
#ifdef HAS_ROTARIES
        return (uint8_t)_rotaryVolume.readEncoder(); 
#else
        return _volume;
#endif
    }

    void setVolume(uint8_t volume) { 
#ifdef HAS_ROTARIES
        _rotaryVolume.setEncoderValue(volume); 
#endif
        _volume = volume;
    }

private:
#ifdef HAS_ROTARIES
    AiEsp32RotaryEncoder _rotaryVolume;
    AiEsp32RotaryEncoder _rotaryTuner;
    
    int _prevRotaryTunerCode;
    void handleTunerButton();
    void handleVolumeButton();
    void loopRotaryTuner();
    void loopRotaryVolume();
    
    static void IRAM_ATTR readEncoderISR();
#endif

#ifdef HAS_REMOTE
    IRrecv _irrecv;
    void handleRemotePress(int64_t remotecode);
#endif

    ActionCallback _callback;
    void notify(Action action);
    
    uint8_t _volume;
    unsigned long _longPressMillis;
};
