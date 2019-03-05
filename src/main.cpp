//
// Created by Greg Niemann on 10/20/18.
//

#include <algorithm>

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>
#include <Wire.h>
#include <Stepper.h>
#include <FS.h>
#include <ArduinoLog.h>

#include "StatusClient.h"
#include "LightManager.h"
#include "SetupServer.h"
#include "SetupManager.h"
#include "MessageManager.h"
#include "DialIndicator.h"
#include "ResetButton.h"
#include "SystemStatusIndicator.h"

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
const int iterations = 1000 / RATE;
const int MINUTE = 1000 * 60;

using Tickers = std::vector<Ticker>;
Tickers tickers;

std::shared_ptr<StatusClient> client;
auto lights = LightManager<DATA, CLOCK, LATCH, ledCNT>();
MessageManager<6> messageManager;
SetupManager setupManager(SPIFFS, WiFi, Log);
//SystemStatusIndicator<D4, D0, D8> status;

constexpr int secondsInMillis(int sec) {
    return sec * 1000;
}

void reset() {
    Log.notice("Resetting\n");
    setupManager.reset();
    WiFi.disconnect();
    WiFi.setAutoConnect(false);
    ESP.restart();
}

void resetButtonPushed(long long duration) {
    if (duration > secondsInMillis(6)) {
        digitalWrite(LED_BUILTIN, HIGH);
        reset();
    } else if (duration > secondsInMillis(3)) {
        digitalWrite(LED_BUILTIN, LOW);
    }
}

void resetButtonReleased() {
    digitalWrite(LED_BUILTIN, HIGH);
}

ResetButton<RESET> resetButton(resetButtonPushed, resetButtonReleased);

void eventLoop() {
    Log.trace("Event loop\n");
    lights.run();
    messageManager.run();
    resetButton.run();
}

void updateClient() {
//    status.setStatus(SystemStatus::transmitting);
    auto resp = client->get();

    Log.trace("Status code %d\n", resp);
    if (resp < 200 || resp >= 400) {
//        status.setStatus(SystemStatus::failed);
        messageManager.setMessage(client->error(resp), false);
    } else if (resp != 304) {
//        status.setStatus(SystemStatus::idle);
        auto update = client->parse_json();
        lights.update(update.lights);
        messageManager.setMessages(update.messages);
    }
}

void onWifiConnected() {
    client = std::make_shared<StatusClient>(Log, status_url, fingerprint, setupManager.getAuthorization());
    updateClient();

    Ticker tick(updateClient, MINUTE, 0, MILLIS);
    tick.start();
    tickers.push_back(tick);
}

void checkWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Ticker tick(checkWiFi, 500, 1, MILLIS);
        tick.start();
        tickers.push_back(tick);
        return;
    }

    // connected
//    status.setStatus(SystemStatus::idle);
    messageManager.setMessage("Connected", false);
    messageManager.writeOut();

    Log.notice("Connected\n");
    Log.notice("IP address: %s\n", WiFi.localIP().toString().c_str());

    WiFi.setAutoReconnect(true);
    WiFi.setAutoConnect(true);

    onWifiConnected();
}

void setupWifi() {
//    status.setStatus(SystemStatus::connecting);
    auto ssid = setupManager.getSSID();

    WiFi.setAutoConnect(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), setupManager.getWiFiPassword().c_str());

    auto connectionMsg = "Connecting to " + ssid;
    Log.notice("%s\n", connectionMsg.c_str());

    messageManager.setMessage(connectionMsg, false);
    messageManager.writeOut();

    // Wait for connection
    checkWiFi();
}

void checkForSettings() {
    if (!setupManager.checkForSettings()) {
        setupManager.run();
        return;
    }
    // received the settings. Stop the ticker and start the wifi
    tickers.clear();
    setupWifi();

    tickers.emplace_back(eventLoop, iterations, 0, MILLIS);
    std::for_each(tickers.begin(), tickers.end(), [](Ticker &t) { t.start(); });
}


// the setup function runs once when you press reset or power the board
void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    Wire.begin(SDA, SCL);
    SPIFFS.begin();
    Serial.begin(115200);
    Log.begin(LOG_LEVEL_VERBOSE, &Serial, true);
    setupManager.init();

    // turn all LEDs off
    lights.off();
    messageManager.clear();

    // If there are no settings, begin setup flow, which starts the setup server and sets the setup ticker event
    // If there are settings, bypass and connect to wifi, which will create the event loop for us
    if (!setupManager.checkForSettings()) {
        std::vector<std::string> messages = {
                "Setup required",
                "Connect to devops_monitor_ap WiFi network",
                "Navigate to http://192.168.4.1",
                "Follow instructions to complete setup",
            };
        messageManager.setMessages(messages);
        setupManager.remoteSetup();

        tickers.emplace_back(checkForSettings, iterations, 0, MILLIS);
    } else {
        setupWifi();
    }

    tickers.emplace_back(eventLoop, iterations, 0, MILLIS);
    std::for_each(tickers.begin(), tickers.end(), [](Ticker &t) { t.start(); });
}

void loop() {
    std::for_each(tickers.begin(), tickers.end(), [](Ticker &t) { t.update(); });
}