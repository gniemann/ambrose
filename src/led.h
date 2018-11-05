//
// Created by Greg Niemann on 10/30/18.
//

#ifndef BUILD_MONITOR_LED_H
#define BUILD_MONITOR_LED_H

#include <memory>
#include <vector>
#include "color.h"



class LED {
public:
    LED(const Color &c) : color(c), on(true) {}
    virtual ~LED() = default;
    void turnOn() { on = true; }
    void turnOff() { on = false; }
    bool isOn() const { return on; }

    virtual Color getColor() const { return on ? color : OFF; }

    virtual void step() {}
private:
    Color color;
    bool on;
};

using LEDPtr = std::shared_ptr<LED>;

class BlinkingLED: public LED {
public:
    BlinkingLED(const Color &c, int onPeriod, int offPeriod): LED(c), onInterval(onPeriod), offInterval(offPeriod), intervalCounter(0) {}
    void step() override;
private:
    void toggle();

    int onInterval;
    int offInterval;
    int intervalCounter;
};


class InitiallyBlinkingLED: public BlinkingLED {
public:
    InitiallyBlinkingLED(const Color &c, int times, int onPeriod, int offPeriod): BlinkingLED(c, onPeriod, offPeriod), blinkTimes(times), isBlinking(true), currentTime(0) { }

    void step() override;
private:
    int blinkTimes;
    bool isBlinking;
    int currentTime;
};

class MultistateLED: public LED {
public:
    using State = std::pair<LEDPtr, int>;

    MultistateLED(const std::vector<State> &states): LED(OFF), states(states), currentState(0), currentStep(0) {}
    void step() override;
    Color getColor() const override;
private:
    std::vector<State> states;
    int currentState;
    int currentStep;
};

#endif //BUILD_MONITOR_LED_H
