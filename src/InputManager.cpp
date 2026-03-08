#include "InputManager.h"

#ifdef HAS_ROTARIES
#define ROTARY_ENCODER_A_PIN 13
#define ROTARY_ENCODER_B_PIN 17
#define ROTARY_ENCODER_BUTTON_PIN 35
#define ROTARY_ENCODER_STEPS 4

#define ROTARY_ENCODER2_A_PIN 14
#define ROTARY_ENCODER2_B_PIN 34
#define ROTARY_ENCODER2_BUTTON_PIN 33
#define ROTARY_ENCODER2_STEPS 4

#ifndef VOLUME_MAX
#define VOLUME_MAX 100
#endif

#ifndef VOLUME_DEFAULT
#define VOLUME_DEFAULT 20
#endif

// Static pointer for ISR
static AiEsp32RotaryEncoder *s_rotaryTuner = nullptr;
static AiEsp32RotaryEncoder *s_rotaryVolume = nullptr;

void IRAM_ATTR InputManager::readEncoderISR() {
    if (s_rotaryTuner) s_rotaryTuner->readEncoder_ISR();
    if (s_rotaryVolume) s_rotaryVolume->readEncoder_ISR();
}
#endif

#ifdef HAS_REMOTE
#include "ipodradio.h"
#endif

InputManager::InputManager() : 
#ifdef HAS_ROTARIES
#ifdef REVERSE_VOLUME_ENCODER
    _rotaryVolume(ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS),
#else
    _rotaryVolume(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS),
#endif
#ifdef REVERSE_TUNER_ENCODER
    _rotaryTuner(ROTARY_ENCODER2_B_PIN, ROTARY_ENCODER2_A_PIN, ROTARY_ENCODER2_BUTTON_PIN, -1, ROTARY_ENCODER2_STEPS),
#else
    _rotaryTuner(ROTARY_ENCODER2_A_PIN, ROTARY_ENCODER2_B_PIN, ROTARY_ENCODER2_BUTTON_PIN, -1, ROTARY_ENCODER2_STEPS),
#endif
    _prevRotaryTunerCode(500),
#endif
#ifdef HAS_REMOTE
    _irrecv(15), // kRecvPin
#endif
    _callback(nullptr),
    _longPressMillis(1000) {
    
#ifdef HAS_ROTARIES
    s_rotaryVolume = &_rotaryVolume;
    s_rotaryTuner = &_rotaryTuner;
#endif
}

void InputManager::begin() {
#ifdef HAS_ROTARIES
    pinMode(ROTARY_ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(ROTARY_ENCODER_B_PIN, INPUT_PULLUP);
    pinMode(ROTARY_ENCODER2_A_PIN, INPUT_PULLUP);
    pinMode(ROTARY_ENCODER2_B_PIN, INPUT_PULLUP);

    _rotaryTuner.begin();
    _rotaryTuner.disableAcceleration();
    _rotaryTuner.setup(readEncoderISR);
    _rotaryTuner.setBoundaries(0, 1000, true);
    
    _rotaryVolume.begin();
    _rotaryVolume.disableAcceleration();
    _rotaryVolume.setup(readEncoderISR);
    _rotaryVolume.setBoundaries(0, VOLUME_MAX, false);
#endif

#ifdef HAS_REMOTE
    _irrecv.enableIRIn();
#endif
}

void InputManager::update() {
#ifdef HAS_REMOTE
    decode_results results;
    if (_irrecv.decode(&results)) {
        handleRemotePress(results.value);
        _irrecv.resume();
    }
#endif

#ifdef HAS_ROTARIES
    loopRotaryTuner();
    loopRotaryVolume();
#endif
}

void InputManager::notify(Action action) {
    if (_callback) {
        _callback(action);
    }
}

#ifdef HAS_REMOTE
void InputManager::handleRemotePress(int64_t remotecode) {
    switch(remotecode) {
        case RC_RADIO_ON: notify(SCREEN_ON); break;
        case RC_STANDBY: notify(RADIO_OFF); break;
        case RC_PREVIOUS_STATION: notify(STATION_PREV); break;
        case RC_NEXT_STATION: notify(STATION_NEXT); break;
        case RC_NEXT_CATEGORY: notify(CATEGORY_NEXT); break;
        case RC_PREVIOUS_CATEGORY: notify(CATEGORY_PREV); break;
        case RC_TURN_ON_SCREEN: notify(SCREEN_TOGGLE); break;
        case RC_SHOW_DETAILS: notify(SHOW_DETAILS); break;
        case RC_SAVE_STATION: notify(STATION_SAVE); break;
        case RC_RADIO_OFF1:
        case RC_RADIO_OFF2:
        case RC_RADIO_OFF3: notify(VOLUME_TOGGLE_MUTE); break;
    }
}
#endif

#ifdef HAS_ROTARIES
void InputManager::loopRotaryTuner() {
    if (_rotaryTuner.encoderChanged()) {
        int currentCode = _rotaryTuner.readEncoder();
        if (currentCode > _prevRotaryTunerCode) {
            notify(STATION_PREV);
            if (currentCode > 900) {
                _rotaryTuner.setEncoderValue(500);
                _prevRotaryTunerCode = 499;
            } else {
                _prevRotaryTunerCode = currentCode;
            }
        } else if (currentCode < _prevRotaryTunerCode) {
            notify(STATION_NEXT);
            if (currentCode < 100) {
                _rotaryTuner.setEncoderValue(500);
                _prevRotaryTunerCode = 501;
            } else {
                _prevRotaryTunerCode = currentCode;
            }
        }
    }
    handleTunerButton();
}

void InputManager::loopRotaryVolume() {
    if (_rotaryVolume.encoderChanged()) {
        notify(VOLUME_CHANGED); 
    }
    handleVolumeButton();
}

void InputManager::handleTunerButton() {
    static unsigned long buttonPressedTime = 0;
    static bool isLongpress = false;
    bool isDown = _rotaryTuner.isEncoderButtonDown();
    if (isDown) {
        if (!buttonPressedTime) buttonPressedTime = millis();
        if (!isLongpress && (millis() - buttonPressedTime >= _longPressMillis)) {
            // Changed from STATION_SAVE to SETTINGS_EXIT or keep SAVE if preferred.
            // Let's use Long Tuner for SAVE and Long Volume for SETTINGS.
            notify(STATION_SAVE); 
            isLongpress = true;
        }
    } else {
        if (buttonPressedTime && !isLongpress) notify(SCREEN_TOGGLE);
        buttonPressedTime = 0;
        isLongpress = false;
    }
}

void InputManager::handleVolumeButton() {
    static unsigned long buttonPressedTime = 0;
    static bool isLongpress = false;
    bool isDown = _rotaryVolume.isEncoderButtonDown();
    if (isDown) {
        if (!buttonPressedTime) buttonPressedTime = millis();
        if (!isLongpress && (millis() - buttonPressedTime >= _longPressMillis)) {
            notify(SETTINGS_ENTER);
            isLongpress = true;
        }
    } else {
        if (buttonPressedTime && !isLongpress) notify(VOLUME_TOGGLE_MUTE);
        buttonPressedTime = 0;
        isLongpress = false;
    }
}
#endif
