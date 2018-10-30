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

const int DATA = D5;
const int CLOCK = D6;
const int LATCH = D7;


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

// the setup function runs once when you press reset or power the board
void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(DATA, OUTPUT);
    pinMode(CLOCK, OUTPUT);
    pinMode(LATCH, OUTPUT);

    digitalWrite(LATCH, LOW);
    shiftOut(DATA, CLOCK, MSBFIRST, 0xFF);
    digitalWrite(LATCH, HIGH);

    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(115200);

//    setupWifi();

    digitalWrite(LED_BUILTIN, LOW);
}

uint8_t colorToByte(const Color &c) {
    uint8_t output = 0;

    if (c.blue == 0) {
        output++;
    }
    output = output<<1;
    if (c.green == 0) {
        output++;
    }
    output = output<<1;
    if (c.red == 0) {
        output = output + 1;
    }

    return output;
}

using LEDPtr = std::shared_ptr<LED>;

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

    auto lightOneIt = lightOneStates.begin();
    auto lightTwoIt = lightTwoStates.begin();

    while (lightOneIt != lightOneStates.end() && lightTwoIt != lightOneStates.end()) {
        auto light1 = *lightOneIt;
        auto light2 = *lightTwoIt;
        for (int i = 0; i < 50; i++) {
            auto c1 = light1->getColor();
            uint8_t output = colorToByte(c1);

            output = output<<3;
            auto c2 = light2->getColor();
            output += colorToByte(c2);

            digitalWrite(LATCH, LOW);
            shiftOut(DATA, CLOCK, MSBFIRST, output);
            digitalWrite(LATCH, HIGH);

            light1->step();
            light2->step();
            delay(200);
        }

        ++lightOneIt;
        ++lightTwoIt;
    }

}