//
// Created by Greg Niemann on 10/20/18.
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <vector>
#include "color.h"
#include "led.h"

const char* ssid = "Bodhi";
const char* password = "wtaguest";

using Pin = uint8_t;
using Hz = int;
using LEDPtr = std::shared_ptr<LED>;

const Pin DATA = D5;
const Pin CLOCK = D6;
const Pin LATCH = D7;

const Hz RATE = 5;

std::vector<LEDPtr> lights;

void setupWifi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting:");
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
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

// this function converts up to 21 RGB LEDs into a 64-bit integer value
//template<template<class ...> class Container, class ... Args>
//uint64_t convertColors(const Container<LEDPtr, Args...> &leds) {
//    uint64_t output = 0;
//    for (auto&& light : leds) {
//        output+=colorToByte(light->getColor());
//        output = output<<3;
//    }
//
//    return output;
//}

uint64_t convertColors(const std::vector<LEDPtr> &leds) {
    if (leds.empty()) {
        return 0;
    }

    auto it = leds.begin();
    uint64_t output = colorToByte((*it)->getColor());
    while (++it != leds.end()) {
        output = (output<<3) + colorToByte((*it)->getColor());
    }

    return output;
}

void shiftOut(uint64_t output) {
    digitalWrite(LATCH, LOW);

    for (int i = sizeof(uint64_t); i >= 0; i-=8) {
        uint8_t shift = (output >> i) & 0xFF;
        shiftOut(DATA, CLOCK, MSBFIRST, ~shift);
    }

    digitalWrite(LATCH, HIGH);
}

// the setup function runs once when you press reset or power the board
void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(DATA, OUTPUT);
    pinMode(CLOCK, OUTPUT);
    pinMode(LATCH, OUTPUT);

    // turn all LEDs off
    shiftOut(UINT64_MAX);

    // set up lights
    lights.push_back(std::make_shared<LED>(RED));
    lights.push_back(std::make_shared<LED>(GREEN));
    lights.push_back(std::make_shared<LED>(BLUE));
//    auto output = convertColors(lights);
//    shiftOut(output);

    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(115200);

//    setupWifi();

    digitalWrite(LED_BUILTIN, LOW);
}

// the loop function runs over and over again forever
void loop() {
    std::vector<LEDPtr> lightOneStates {
        LEDPtr(new InitiallyBlinkingLED(RED, 15, 1, 1)),
        LEDPtr(new LED(GREEN)),
        LEDPtr(new BlinkingLED(BLUE, 4, 1))
    };

    std::vector<LEDPtr> lightTwoStates {
        LEDPtr(new BlinkingLED(GREEN, 2, 2)),
        LEDPtr(new InitiallyBlinkingLED(BLUE, 20, 2, 1)),
        LEDPtr(new BlinkingLED(RED, 3, 1))
    };

    std::vector<LEDPtr> lightThreeStates {
        LEDPtr(new LED(BLUE)),
        LEDPtr(new BlinkingLED(RED, 4, 4)),
        LEDPtr(new BlinkingLED(GREEN, 1, 1))
    };

    auto iterations = min({lightOneStates.size(), lightTwoStates.size(), lightThreeStates.size()});
    for (uint i = 0; i < iterations; i++) {
        lights[0] = lightOneStates[i];
        lights[1] = lightTwoStates[i];
        lights[2] = lightThreeStates[i];

        for (int step = 0; step < 50; ++step) {
            auto output = convertColors(lights);
            shiftOut(output);

            for (auto light : lights) {
                light->step();
            }

            delay(1000 / RATE);
        }
    }
}