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

Color MultistateLED::getColor() const {
    return states[currentState].first->getColor();
}

void MultistateLED::step() {
    if (++currentStep >= states[currentState].second) {
        currentState = (currentState + 1) % states.size();
        currentStep = 0;
    } else {
        states[currentState].first->step();
    }
}