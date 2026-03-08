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
        RADIO_OFF
    };

    typedef void (*ActionCallback)(Action action);

    InputManager();
    void begin();
    void update();
    void setActionCallback(ActionCallback cb) { _callback = cb; }
    
#ifdef HAS_ROTARIES
    uint8_t getVolume() { return (uint8_t)_rotaryVolume.readEncoder(); }
    void setVolume(uint8_t volume) { _rotaryVolume.setEncoderValue(volume); }
#endif

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
    
    unsigned long _longPressMillis;
};
