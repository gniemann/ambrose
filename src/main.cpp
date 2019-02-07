//
// Created by Greg Niemann on 10/20/18.
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <vector>
#include <array>
#include <memory>
#include <unordered_map>
#include "color.h"
#include "led.h"
#include "status_client.h"
#include "status.h"
#include "config.h"

const char* ssid = "Bodhi";
const char* password = "wtaguest";
const char* status_url = "https://devops-status-monitor.herokuapp.com/api/status";
const char *fingerprint = "08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";

const char* authorization = "Basic Z3JlZy5uaWVtYW5uQHdpbGxvd3RyZWVhcHBzLmNvbTphd2Vzb21lX3Bhc3N3b3Jk";

using Pin = uint8_t;
using Hz = int;

constexpr std::size_t ledCNT = 9;
using LEDArray = std::array<LEDPtr, ledCNT>;

const Pin DATA = D5;
const Pin CLOCK = D6;
const Pin LATCH = D7;

const Hz RATE = 5;

LEDArray lights;
auto config = std::make_shared<Configuration>();
StatusClient client(status_url, fingerprint, authorization);

LEDPtr failure(new LED(RED));
LEDPtr off(new LED(OFF));

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
    // need to output in reverse order, so that the first light goes in thh first position
    for (auto iter = leds.rbegin(); iter != leds.rend(); ++iter) {
        output = (output<<3) + colorToByte((*iter)->getColor());
    }
    return output;
}

void shiftOut(uint64_t output) {
    digitalWrite(LATCH, LOW);

    for (int i = sizeof(uint64_t) * 7; i >= 0; i-=8) {
        uint8_t shift = (output >> i) & 0xFF;
//        Serial.println(shift, BIN);
        shiftOut(DATA, CLOCK, MSBFIRST, ~shift);
    }
    digitalWrite(LATCH, HIGH);
}

void setLights(const LEDArray &lights) {
    shiftOut(convertColors(lights));
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

    setupWifi();
    digitalWrite(LED_BUILTIN, LOW);
}

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
            return off;
    }
}

constexpr int iterations = RATE * 30;
// the loop function runs over and over again forever
void loop() {
    auto resp = client.get();

    Serial.println(resp);
    if (resp < 200 || resp >= 400) {
        for (auto light: lights) {
            light = failure;
        }
    } else if (resp != 304) {
        auto statuses = parse_json(client.getStream());
        config->update(statuses);

        auto light = lights.begin();
        auto stage = config->begin();

        while (light != lights.end() && stage != config->end()) {
            if (stage->isChanged() || *light == nullptr) {
                *light = ledForStage(*stage);
            }

            ++light;
            ++stage;
        }
    }

    for (int step = 0; step < iterations; ++step) {
        setLights(lights);

        for (auto light : lights) {
            light->step();
        }

        delay(1000 / RATE);
    }
}