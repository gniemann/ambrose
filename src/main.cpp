//
// Created by Greg Niemann on 10/20/18.
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "status_client.h"
#include "light_manager.h"
#include "setup_server.h"
#include "SettingsManager.h"
#include "MessageManager.h"
#include "DialIndicator.h"
#include "ResetButton.h"
#include <Ticker.h>
#include <Wire.h>
#include <Stepper.h>
#include <FS.h>

const char* status_url = "https://devops-status-monitor.herokuapp.com/api/status";
const char *fingerprint = "08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";

using Pin = uint8_t;
using Hz = int;

constexpr std::size_t ledCNT = 9;

const Pin DATA = D5;
const Pin CLOCK = D6;
const Pin LATCH = D7;

const Hz RATE = 5;

auto lights = LightManager<DATA, CLOCK, LATCH, ledCNT>();

std::shared_ptr<StatusClient> client;
MessageManager<6> messageManager;
SettingsManager settingsManager;

void reset() {
    Serial.println("Resetting...");
    settingsManager.reset();
    WiFi.disconnect();
    WiFi.setAutoConnect(false);
    ESP.restart();
}

void resetButtonPushed(long long duration) {
    if (duration > 6000) {
        digitalWrite(LED_BUILTIN, HIGH);
        reset();
    } else if (duration > 3000) {
        digitalWrite(LED_BUILTIN, LOW);
    }
}

void resetButtonReleased() {
    digitalWrite(LED_BUILTIN, HIGH);
}

ResetButton<D3> resetButton(resetButtonPushed, resetButtonReleased);

void eventLoop() {
    lights.setLights();
    lights.step();
    messageManager.writeOut();
    messageManager.step();
    resetButton.step();
}

void updateClient() {
    digitalWrite(LED_BUILTIN, LOW);
    auto resp = client->get();
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.println(resp);
    if (resp < 200 || resp >= 400) {
        messageManager.setMessage(client->error(resp), false);
    } else if (resp != 304) {
        auto update = client->parse_json();
        lights.update(update.lights);
        messageManager.setMessages(update.messages);
    }
}

Ticker clientTicker(updateClient, 1000 * 60, 0, MILLIS);
Ticker eventLoopTicker(eventLoop, 1000 / RATE, 0, MILLIS);

void setupWifi(const std::string &ssid, const std::string &password) {
    WiFi.setAutoConnect(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to ");
    Serial.println(ssid.c_str());

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
    WiFi.setAutoReconnect(true);
    WiFi.setAutoConnect(true);
}

// the setup function runs once when you press reset or power the board
void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    Wire.begin(SDA, SCL);
    SPIFFS.begin();

    // turn all LEDs off
    lights.off();
    messageManager.clear();

    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(115200);

    if (!settingsManager.checkForSettings()) {
        settingsManager.remoteSetup();
    }

    auto ssid = settingsManager.getSSID();
    messageManager.setMessage("Connecting to " + ssid + "...", false);
    messageManager.writeOut();

    setupWifi(ssid, settingsManager.getWiFiPassword());

    messageManager.setMessage("Contacting to service", false);
    messageManager.writeOut();

    client = std::make_shared<StatusClient>(status_url, fingerprint, settingsManager.getAuthorization());

    digitalWrite(LED_BUILTIN, LOW);
    updateClient();

    clientTicker.start();
    eventLoopTicker.start();
}

void loop() {
    clientTicker.update();
    eventLoopTicker.update();
}