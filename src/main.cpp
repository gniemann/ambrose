//
// Created by Greg Niemann on 10/20/18.
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "status_client.h"
#include "LightManager.h"
#include "setup_server.h"
#include "SetupManager.h"
#include "MessageManager.h"
#include "DialIndicator.h"
#include "ResetButton.h"
#include <Ticker.h>
#include <Wire.h>
#include <Stepper.h>
#include <FS.h>
#include <algorithm>

const char* status_url = "https://devops-status-monitor.herokuapp.com/api/status";
const char *fingerprint = "08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";

using Pin = uint8_t;
using Hz = int;

constexpr std::size_t ledCNT = 9;

const Pin DATA = D5;
const Pin CLOCK = D6;
const Pin LATCH = D7;
const Pin RESET = D3;

const Hz RATE = 5;

using Tickers = std::vector<Ticker>;
Tickers tickers;

std::shared_ptr<StatusClient> client;
auto lights = LightManager<DATA, CLOCK, LATCH, ledCNT>();
MessageManager<6> messageManager;
SetupManager setupManager(SPIFFS);

void reset() {
    Serial.println("Resetting...");
    setupManager.reset();
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

ResetButton<RESET> resetButton(resetButtonPushed, resetButtonReleased);

void eventLoop() {
    lights.run();
    messageManager.run();
    resetButton.run();
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

void setupWifi() {
    digitalWrite(LED_BUILTIN, LOW);
    auto ssid = setupManager.getSSID();
    auto password = setupManager.getWiFiPassword();
    WiFi.setAutoConnect(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    auto connectionMsg = "Connecting to " + ssid + "...";
    Serial.println(connectionMsg.c_str());

    messageManager.setMessage(connectionMsg, false);
    messageManager.writeOut();

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    messageManager.setMessage("Connected", false);
    messageManager.writeOut();

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid.c_str());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.setAutoReconnect(true);
    WiFi.setAutoConnect(true);
    digitalWrite(LED_BUILTIN, HIGH);
}

void onWifiConnected() {
    client = std::make_shared<StatusClient>(status_url, fingerprint, setupManager.getAuthorization());
    updateClient();

    tickers.emplace_back(updateClient, 1000 * 60, 0, MILLIS);
}

void checkForSettings() {
    if (!setupManager.checkForSettings()) {
        setupManager.run();
        return;
    }
    // received the settings. Stop the ticker and start the wifi
    setupWifi();
    tickers.clear();
    onWifiConnected();

    tickers.emplace_back(eventLoop, 1000/RATE, 0, MILLIS);
    std::for_each(tickers.begin(), tickers.end(), [](Ticker &t) { t.start(); });
}


// the setup function runs once when you press reset or power the board
void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    Wire.begin(SDA, SCL);
    SPIFFS.begin();
    Serial.begin(115200);
    setupManager.init();

    // turn all LEDs off
    lights.off();
    messageManager.clear();

    if (!setupManager.checkForSettings()) {
        std::vector<std::string> messages = {
                "Setup required",
                "Connect to devops_monitor_ap WiFi network",
                "Navigate to http://192.168.4.1",
                "Follow instructions to complete setup",
            };
        messageManager.setMessages(messages);
        setupManager.remoteSetup();

        tickers.emplace_back(checkForSettings, 1000 / RATE, 0, MILLIS);
    } else {
        setupWifi();
        onWifiConnected();
    }

    tickers.emplace_back(eventLoop, 1000/RATE, 0, MILLIS);
    std::for_each(tickers.begin(), tickers.end(), [](Ticker &t) { t.start(); });
}

void loop() {
    std::for_each(tickers.begin(), tickers.end(), [](Ticker &t) { t.update(); });
}