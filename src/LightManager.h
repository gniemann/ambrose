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
#include <Adafruit_MCP23008.h>

using Pin = uint8_t;

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
class LightManager: Manager {
public:
    LightManager(Adafruit_MCP23008 &mcp);
    void off();
    void update(const Lights &newLights);
    void setLights() const;
    void run() override;
private:
    Adafruit_MCP23008 &mcp;
    bool isNormalOperation;
    std::array<LEDPtr, N> lights;

    void setAll(LEDPtr led);
};

uint8_t colorToByte(const Color &c) {
    uint8_t output = 0;

    if (c.blue != 0) {
        output++;
    }
    output = output<<1;
    if (c.green != 0) {
        output++;
    }
    output = output<<1;
    if (c.red != 0) {
        output++;
    }

    return output;
}

template<Pin DATA, Pin CLOCK, Pin LATCH, size_t N>
LightManager<DATA, CLOCK, LATCH, N>::LightManager(Adafruit_MCP23008 &mcp): mcp(mcp), isNormalOperation(true) {
    mcp.begin();
    mcp.pinMode(DATA, OUTPUT);
    mcp.pinMode(CLOCK, OUTPUT);
    mcp.pinMode(LATCH, OUTPUT);
}

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::setAll(std::shared_ptr<LED> led) {
    std::fill(lights.begin(), lights.end(), led);
    setLights();
}

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::off() {
    LEDPtr off(new LED(OFF));
    setAll(off);
}

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::update(const Lights &newLights) {
    auto light = lights.begin();
    auto newLight = newLights.begin();
    
    while (light != lights.end() && newLight != newLights.end()) {
        *light = *newLight;
        
        ++light;
        ++newLight;
    }
    
    // set the rest of the lights off, if we didn't receive all lights
    while (light != lights.end()) {
        *light = std::make_shared<LED>(OFF);
        ++light;
    }
}

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::setLights() const {
    uint64_t output = 0;

    // need to output in reverse order, so that the first light foes in the first position
    for (auto iter = lights.rbegin(); iter != lights.rend(); ++iter) {
        output = (output<<3) + colorToByte((*iter)->getColor());
    }

    mcp.digitalWrite(LATCH, LOW);

    for (int i = sizeof(uint64_t) * 7; i >= 0; i-=8) {
        uint8_t shift = (output >> i) & 0xFF;
        shift = ~shift;

        for (uint8_t j = 0; j < 8; j++) {
            // this is copied from the core_esp8266_wiring_shift.c impl of shiftOut
            mcp.digitalWrite(DATA, !!(shift & (1 << (7 - j))));
            mcp.digitalWrite(CLOCK, HIGH);
            mcp.digitalWrite(CLOCK, LOW);
        }
    }

    mcp.digitalWrite(LATCH, HIGH);
}

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::run() {
    for (auto light: lights) {
        light->step();
    }
    setLights();
}

#endif //BUILD_MONITOR_LIGHTMANAGER_H
