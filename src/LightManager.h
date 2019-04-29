//
// Created by Greg Niemann on 2019-02-07.
//

#ifndef BUILD_MONITOR_LIGHTMANAGER_H
#define BUILD_MONITOR_LIGHTMANAGER_H

#include <Arduino.h>
#include <array>
#include <algorithm>
#include "led.h"
#include "Manager.h"
#include <FastLED.h>

using PIN = uint8_t;

template <PIN DATA, std::size_t N>
class LightManager: Manager {
public:
    LightManager();
    void off();
    void update(const Lights &newLights);
    void setLights();
    void run() override;
private:
    bool isNormalOperation;
    std::array<LEDPtr, N> ledConfig;
    std::array<CRGB, N> leds;

    void setAll(LEDPtr led);
};

template<PIN DATA, size_t N>
LightManager<DATA, N>::LightManager(): isNormalOperation(true) {
    pinMode(DATA, OUTPUT);
    FastLED.addLeds<WS2812B, DATA>(leds.data(), N);
}

template <PIN DATA, std::size_t N>
void LightManager<DATA, N>::setAll(std::shared_ptr<LED> led) {
    std::fill(ledConfig.begin(), ledConfig.end(), led);
    setLights();
}

template <PIN DATA, std::size_t N>
void LightManager<DATA, N>::off() {
    LEDPtr off(new LED(OFF));
    setAll(off);
}

template <PIN DATA, std::size_t N>
void LightManager<DATA, N>::update(const Lights &newLights) {
    auto light = ledConfig.begin();
    auto newLight = newLights.begin();
    
    while (light != ledConfig.end() && newLight != newLights.end()) {
        *light = *newLight;
        
        ++light;
        ++newLight;
    }
    
    // set the rest of the lights off, if we didn't receive all lights
    while (light != ledConfig.end()) {
        *light = std::make_shared<LED>(OFF);
        ++light;
    }
}

template <PIN DATA, std::size_t N>
void LightManager<DATA, N>::setLights() {
    for (size_t i = 0; i < N; i++) {
        auto color = ledConfig[i]->getColor();
        leds[i] = CRGB(color.red, color.green, color.blue);
    }

    FastLED.show();
}

template <PIN DATA, std::size_t N>
void LightManager<DATA, N>::run() {
    for (auto light: ledConfig) {
        light->step();
    }
    setLights();
}

#endif //BUILD_MONITOR_LIGHTMANAGER_H
