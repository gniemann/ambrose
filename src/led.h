//
// Created by Greg Niemann on 10/30/18.
//

#ifndef BUILD_MONITOR_LED_H
#define BUILD_MONITOR_LED_H

#include "color.h"

class LED {
public:
    LED(const Color &c) : color(c), on(true) {}
    void turnOn() { on = true; }
    void turnOff() { on = false; }
    bool isOn() const { return on; }

    Color getColor() const { return on ? color : OFF; }

    virtual void step() {}
private:
    Color color;
    bool on;
};

class BlinkingLED: public LED {
public:
    BlinkingLED(const Color &c, int onPeriod, int offPeriod): LED(c), onInterval(onPeriod), offInterval(offPeriod), intervalCounter(0) {}
    void step();
private:
    void toggle();

    int onInterval;
    int offInterval;
    int intervalCounter;
};


class InitiallyBlinkingLED: public BlinkingLED {
public:
    InitiallyBlinkingLED(const Color &c, int times, int onPeriod, int offPeriod): BlinkingLED(c, onPeriod, offPeriod), blinkTimes(times), isBlinking(true), currentTime(0) { }

    void step();
private:
    int blinkTimes;
    bool isBlinking;
    int currentTime;
};

#endif //BUILD_MONITOR_LED_H
