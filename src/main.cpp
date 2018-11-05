//
// Created by Greg Niemann on 10/20/18.
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <vector>
#include <array>
#include "color.h"
#include "led.h"
#include "status_client.h"

const char* ssid = "Bodhi";
const char* password = "wtaguest";
const char* status_url = "https://buildstatus2.herokuapp.com/stages";
const char *fingerprint = "08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";

using Pin = uint8_t;
using Hz = int;

constexpr std::size_t ledCNT = 4;
using LEDArray = std::array<LEDPtr, ledCNT>;

const Pin DATA = D5;
const Pin CLOCK = D6;
const Pin LATCH = D7;

const Hz RATE = 5;

LEDArray lights;

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



template <std::size_t N>
uint64_t convertColors(const std::array<LEDPtr, N> &leds) {
    uint64_t output = 0;

    for (auto light: leds) {
        output = (output<<3) + colorToByte(light->getColor());
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
    shiftOut(0);

    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(115200);

//    setupWifi();

    digitalWrite(LED_BUILTIN, LOW);
}

LEDPtr make_multistate() {
    std::vector<MultistateLED::State> states {
        std::make_pair(LEDPtr(new LED(RED)), 5),
        std::make_pair(LEDPtr(new LED(GREEN)), 5),
        std::make_pair(LEDPtr(new LED(BLUE)), 5),
        std::make_pair(LEDPtr(new LED(OFF)), 5)
    };

    return LEDPtr(new MultistateLED(states));
}

std::vector<LEDPtr> states {
        make_multistate(),
        LEDPtr(new InitiallyBlinkingLED(RED, 15, 1, 1)),
        LEDPtr(new LED(GREEN)),
        LEDPtr(new BlinkingLED(BLUE, 4, 1)),
        LEDPtr(new BlinkingLED(GREEN, 2, 2)),
        LEDPtr(new InitiallyBlinkingLED(BLUE, 20, 2, 1)),
        LEDPtr(new BlinkingLED(RED, 3, 1)),
        LEDPtr(new LED(BLUE)),
        LEDPtr(new BlinkingLED(RED, 4, 4)),
        LEDPtr(new BlinkingLED(GREEN, 1, 1)),
        LEDPtr(new InitiallyBlinkingLED(GREEN, 10, 2, 1)),
        LEDPtr(new LED(BLUE)),
        LEDPtr(new InitiallyBlinkingLED(RED, 10, 2, 1))
};

// the loop function runs over and over again forever
void loop() {
    for (auto& light: lights) {
        light = states[random(0, states.size())];
    }

    for (int step = 0; step < 50; ++step) {
        auto output = convertColors(lights);
        shiftOut(output);

        for (auto light : lights) {
            light->step();
        }

        delay(1000 / RATE);
    }
}