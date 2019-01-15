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

void AlternatingLED::step() {
    intervalCounter = (intervalCounter) % (period1 + period2) + 1;
}

Color AlternatingLED::getColor() const {
    return (intervalCounter <= period1) ? color1 : color2;
}