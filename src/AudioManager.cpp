#include "AudioManager.h"

#ifndef VOLUME_STEPS
#define VOLUME_STEPS 64
#endif

#ifndef VOLUME_DEFAULT
#define VOLUME_DEFAULT 20
#endif

AudioManager::AudioManager() : 
    _volume(VOLUME_DEFAULT), 
    _preMuteVolume(VOLUME_DEFAULT),
    _isMuted(false),
    _streamTitleCallback(nullptr) {
}

void AudioManager::begin(int bclk, int lrc, int dout) {
    _audio.setPinout(bclk, lrc, dout);
    _audio.setVolumeSteps(VOLUME_STEPS);
    setVolume(_volume);
    
#ifdef MONO_OUTPUT
    _audio.forceMono(true);
#endif
}

void AudioManager::connectToHost(const char* url) {
    log_i("Connecting to host: %s", url);
    _audio.connecttohost(url);
}

void AudioManager::connectToStation(const radStat::RadioStation& station) {
    connectToHost(station.URL.c_str());
}

void AudioManager::setVolume(uint8_t volume) {
    _volume = volume;
    if (!_isMuted) {
        _audio.setVolume(_volume);
    }
}

void AudioManager::toggleMute() {
    _isMuted = !_isMuted;
    if (_isMuted) {
        _preMuteVolume = _volume;
        _audio.setVolume(0);
    } else {
        _audio.setVolume(_volume);
    }
}

void AudioManager::stop() {
    _audio.stopSong();
}

void AudioManager::update() {
    _audio.loop();
}

// Singleton or global instance
AudioManager audioManager;
