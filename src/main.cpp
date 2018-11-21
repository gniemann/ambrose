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
const char* status_url = "https://buildstatus2.herokuapp.com/stages";
const char* config_url = "https://buildstatus2.herokuapp.com/config";
const char *fingerprint = "08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";

using Pin = uint8_t;
using Hz = int;

constexpr std::size_t ledCNT = 4;
using LEDArray = std::array<LEDPtr, ledCNT>;
using LEDConfig = Configuration<ledCNT>;

const Pin DATA = D5;
const Pin CLOCK = D6;
const Pin LATCH = D7;

const Hz RATE = 5;

LEDArray lights;
auto config = std::make_shared<LEDConfig>();
StatusClient client(status_url, fingerprint);

LEDPtr success(new LED(GREEN));
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

void setLights(const LEDArray &lights) {
    shiftOut(convertColors(lights));
}

void getConfig() {
    StatusClient configClient(config_url, fingerprint);
    auto isComplete = false;

    while (!isComplete) {
        auto resp = configClient.get();

        if (resp < 200 || resp >= 400) {
            for (auto &light : lights) {
                light = failure;
            }

            setLights(lights);
            delay(60000);
        } else {
            isComplete = true;
            config->init(configClient.getStream());
        }
    }
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

    getConfig();
}

constexpr int iterations = RATE * 30;
// the loop function runs over and over again forever
void loop() {
    auto resp = client.get();

    Serial.println(resp);
    LEDPtr toSet;


    if (resp < 200 || resp >= 400) {
        for (auto light: lights) {
            light = failure;
        }
    } else {
        auto statuses = parse_json(client.getStream());

        for (auto s: statuses) {
            config->update(*s);
        }

        auto light = lights.begin();
        auto status = config->begin();

        while (light != lights.end() && status != config->end()) {
            if ((*status).second) {
                switch ((*status).first) {
                    case Status::SUCCEEDED:
                        toSet = LEDPtr(new InitiallyBlinkingLED(GREEN, 20, 2, 1));
                        break;
                    case Status::FAILED:
                        toSet = LEDPtr(new InitiallyBlinkingLED(RED, 20, 1, 1));
                        break;
                    case Status::IN_PROGRESS:
                        toSet = LEDPtr(new BlinkingLED(BLUE, 4, 4));
                        break;
                    case Status::QUEUED:
                        toSet = LEDPtr(new LED(BLUE));
                    default:
                        toSet = off;
                }
                *light = toSet;
            }

            ++light;
            ++status;
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