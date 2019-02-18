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

Color color_factory(JsonObject &obj) {
    auto red = obj["red"].as<int>();
    auto green = obj["green"].as<int>();
    auto blue = obj["blue"].as<int>();

    return Color(red, green, blue);
}

LEDPtr lightFromJSONObject(JsonObject &obj) {
    auto type = obj["type"].as<char*>();
    auto primaryColor = color_factory(obj["primary_color"]);

    if (strcmp(type, "steady") == 0) {
        return std::make_shared<LED>(primaryColor);
    } else if (strcmp(type, "blinking") == 0) {
        auto primaryPeriod = obj["primary_period"].as<int>();
        auto secondaryPeriod = obj["secondary_period"].as<int>();
        auto secondaryColor = color_factory(obj["secondary_color"]);

        return LEDPtr(new AlternatingLED(primaryColor, secondaryColor, primaryPeriod, secondaryPeriod));
    } else if (strcmp(type, "initially_blinking") == 0) {
        auto primaryPeriod = obj["primary_period"].as<int>();
        auto secondaryPeriod = obj["secondary_period"].as<int>();
        auto repeat = obj["repeat"].as<int>();

        return LEDPtr(new InitiallyBlinkingLED(primaryColor, repeat, primaryPeriod, secondaryPeriod));
    }

    return std::make_shared<LED>(OFF);
}

Lights lightsFromJSONArray(JsonArray &array) {
    Lights lights;
    for (JsonObject& light: array) {
        lights.push_back(lightFromJSONObject(light));
    }
    return lights;
}