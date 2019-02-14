//
// Created by Greg Niemann on 10/20/18.
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "status_client.h"
#include "status.h"
#include "config.h"
#include "light_manager.h"
#include "setup_server.h"
#include "SettingsManager.h"
#include "MessageManager.h"

const char* status_url = "https://devops-status-monitor.herokuapp.com/api/status";
const char *fingerprint = "08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";

using Pin = uint8_t;
using Hz = int;

constexpr std::size_t ledCNT = 9;

const Pin DATA = D5;
const Pin CLOCK = D6;
const Pin LATCH = D7;

const Hz RATE = 5;

auto config = std::make_shared<Configuration>();
auto lights = LightManager<DATA, CLOCK, LATCH, ledCNT>();

std::shared_ptr<StatusClient> client;

void setupWifi(const std::string &ssid, const std::string &password) {
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid.c_str());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}
MessageManager<6, D2, D1> messageManager;

// the setup function runs once when you press reset or power the board
void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(DATA, OUTPUT);
    pinMode(CLOCK, OUTPUT);
    pinMode(LATCH, OUTPUT);

    // turn all LEDs off
    lights.off();
    messageManager.clear();

    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(115200);

    SettingsManager settingsManager;
    if (!settingsManager.checkForSettings()) {
        settingsManager.remoteSetup();
    }

    setupWifi(settingsManager.getSSID(), settingsManager.getWiFiPassword());

    client = std::make_shared<StatusClient>(status_url, fingerprint, settingsManager.getAuthorization());

    digitalWrite(LED_BUILTIN, LOW);
}

constexpr int iterations = RATE * 60;
// the loop function runs over and over again forever
void loop() {
    auto resp = client->get();

    Serial.println(resp);
    if (resp < 200 || resp >= 400) {
        lights.failure();
        char buffer[10];
        sprintf(buffer, "%d", resp);
        std::string message = "Request failed - " + std::string(buffer);
        messageManager.setMessage(message);
    } else if (resp != 304) {
        auto update = parse_json(client->getStream());
        config->update(update.stages);
        lights.update(config.get());
        messageManager.setMessages(update.messages);
    }

    for (int step = 0; step < iterations; ++step) {
        lights.setLights();
        lights.step();
        messageManager.writeOut();
        messageManager.step();
        delay(1000 / RATE);
    }
}