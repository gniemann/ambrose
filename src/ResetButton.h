//
// Created by Greg Niemann on 2019-02-26.
//

#ifndef BUILD_MONITOR_RESETBUTTON_H
#define BUILD_MONITOR_RESETBUTTON_H

#include <Arduino.h>
#include <functional>

template <uint8_t pin>
class ResetButton: Manager {
public:
    using PushedFunction = std::function<void(long long)>;
    using ReleasedFunction = std::function<void()>;
    ResetButton(PushedFunction pushed, ReleasedFunction released);

    void run() override;
private:
    bool isDepressed = false;
    long long startMillis = 0;
    PushedFunction onPush;
    ReleasedFunction onRelease;
};

template<uint8_t pin>
ResetButton<pin>::ResetButton(PushedFunction pushed, ReleasedFunction released): onPush(pushed), onRelease(released) {
    pinMode(pin, INPUT_PULLUP);
}

template<uint8_t pin>
void ResetButton<pin>::run() {
    if (digitalRead(pin) == LOW) {
        auto current = millis();
        if (!isDepressed) {
            isDepressed = true;
            startMillis = current;
        }
        onPush(current - startMillis);
    } else if (isDepressed) {
        isDepressed = false;
        onRelease();
    }
}


#endif //BUILD_MONITOR_RESETBUTTON_H
