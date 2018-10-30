//
// Created by Greg Niemann on 10/30/18.
//

#include "led.h"

void BlinkingLED::step() {
    if (++intervalCounter >= (isOn() ? onInterval : offInterval)) {
        toggle();
    }
}

void BlinkingLED::toggle() {
    if (isOn()) {
        turnOff();
    } else {
        turnOn();
    }
    intervalCounter = 0;
}

void InitiallyBlinkingLED::step() {
    if (isBlinking) {
        auto wasOn = isOn();
        BlinkingLED::step();
        if (!wasOn && isOn() && ++currentTime >= blinkTimes) {
            isBlinking = false;
        }
    }
}