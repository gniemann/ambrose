//
// Created by Greg Niemann on 10/20/18.
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <memory>
#include "color.h"
#include "led.h"
#include "status_client.h"
#include "status.h"
#include "config.h"
#include "light_manager.h"

const char* ssid = "Bodhi";
const char* password = "wtaguest";
const char* status_url = "https://devops-status-monitor.herokuapp.com/api/status";
const char *fingerprint = "08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";

const char* authorization = "Basic Z3JlZy5uaWVtYW5uQHdpbGxvd3RyZWVhcHBzLmNvbTphd2Vzb21lX3Bhc3N3b3Jk";

using Pin = uint8_t;
using Hz = int;

constexpr std::size_t ledCNT = 9;

const Pin DATA = D5;
const Pin CLOCK = D6;
const Pin LATCH = D7;

const Hz RATE = 5;

auto config = std::make_shared<Configuration>();
auto lights = LightManager<DATA, CLOCK, LATCH, ledCNT>();
StatusClient client(status_url, fingerprint, authorization);

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

    // turn all LEDs off
    lights.off();

    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(115200);

    setupWifi();
    digitalWrite(LED_BUILTIN, LOW);
}

constexpr int iterations = RATE * 30;
// the loop function runs over and over again forever
void loop() {
    auto resp = client.get();

    Serial.println(resp);
    if (resp < 200 || resp >= 400) {
        lights.failure();
    } else if (resp != 304) {
        auto statuses = parse_json(client.getStream());
        config->update(statuses);
        lights.update(config.get());
    }

    for (int step = 0; step < iterations; ++step) {
        lights.setLights();
        lights.step();

        delay(1000 / RATE);
    }
}