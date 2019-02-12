//
// Created by Greg Niemann on 2019-02-07.
//

#ifndef BUILD_MONITOR_LIGHTMANAGER_H
#define BUILD_MONITOR_LIGHTMANAGER_H

#include <Arduino.h>
#include <array>
#include <algorithm>
#include "led.h"
#include "config.h"

using Pin = uint8_t;

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
class LightManager {
public:
    LightManager(): isNormalOperation(true) {}
    void off();
    void failure();
    void update(Configuration *config);
    void setLights() const;
    void step();
private:
    bool isNormalOperation;
    std::array<LEDPtr, N> lights;

    void setAll(LEDPtr led);
};

Color colorForStatus(const Status &status) {
    switch (status) {
        case Status::SUCCEEDED:
            return GREEN;
        case Status::FAILED:
            return RED;
        case Status::QUEUED:
        case Status::IN_PROGRESS:
            return BLUE;
        case Status::PENDING_APPROVAL:
            return MAGENTA;
        default:
            return OFF;
    }
}

std::shared_ptr<LED> ledForStage(const Stage &stage) {
    auto primaryColor = colorForStatus(stage.getStatus());
    switch (stage.getStatus()) {
        case Status::SUCCEEDED:
            return LEDPtr(new InitiallyBlinkingLED(primaryColor, 20, 2, 1));
        case Status::FAILED:
            return LEDPtr(new InitiallyBlinkingLED(primaryColor, 20, 1, 1));
        case Status::IN_PROGRESS:
        case Status::QUEUED:
            return LEDPtr(new AlternatingLED(primaryColor, colorForStatus(stage.getPrevStatus()), 4, 4));
        case Status::PENDING_APPROVAL:
            return LEDPtr(new LED(primaryColor));
        default:
            return LEDPtr(new LED(OFF));
    }
}

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

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::setAll(std::shared_ptr<LED> led) {
    std::fill(lights.begin(), lights.end(), led);
    setLights();
}

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::off() {
    isNormalOperation = false;
    LEDPtr off(new LED(OFF));
    setAll(off);
}

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::failure() {
    isNormalOperation = false;
    LEDPtr failure(new LED(RED));
    setAll(failure);
}

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::update(Configuration *config) {
    auto light = lights.begin();
    auto stage = config->begin();

    while (light != lights.end() && stage != config->end()) {
        if (!isNormalOperation || stage->isChanged() || *light == nullptr) {
            *light = ledForStage(*stage);
        }

        ++light;
        ++stage;
    }

    isNormalOperation = true;
}

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::setLights() const {
    uint64_t output = 0;

    // need to output in reverse order, so that the first light foes in the first position
    for (auto iter = lights.rbegin(); iter != lights.rend(); ++iter) {
        output = (output<<3) + colorToByte((*iter)->getColor());
    }

    digitalWrite(LATCH, LOW);
    for (int i = sizeof(uint64_t) * 7; i >= 0; i-=8) {
        uint8_t shift = (output >> i) & 0xFF;
        shiftOut(DATA, CLOCK, MSBFIRST, ~shift);
    }
    digitalWrite(LATCH, HIGH);
}

template <Pin DATA, Pin CLOCK, Pin LATCH, std::size_t N>
void LightManager<DATA, CLOCK, LATCH, N>::step() {
    for (auto light: lights) {
        light->step();
    }
}

#endif //BUILD_MONITOR_LIGHTMANAGER_H
