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
#include <Adafruit_MCP23008.h>
#include <FastLED.h>

#include "StatusClient.h"
#include "LightManager.h"
#include "SetupManager.h"
#include "MessageManager.h"
#include "DialIndicator.h"
#include "ResetButton.h"
#include "SystemStatusIndicator.h"

const char *fingerprint = "08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";

using PIN = uint8_t;
using Hz = int;

constexpr std::size_t ledCNT = 10;

const PIN NEOPIXEL_PIN = D5;
const PIN RESET = D3;

const Hz RATE = 5;
const int iterations = 1000 / RATE;
const int MINUTE = 1000 * 60;

using Tickers = std::vector<Ticker>;
Tickers tickers;

std::shared_ptr<StatusClient> client;

Adafruit_MCP23008 mcp;
auto lights = LightManager<NEOPIXEL_PIN, ledCNT>();
MessageManager<6> messageManager;
SetupManager setupManager(SPIFFS, WiFi, Log);
SystemStatusIndicator<0, 1, 2> status(mcp);

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
        reset();
    } else if (duration > secondsInMillis(3)) {
        status.setStatus(SystemStatus::resetPressedLong);
    } else {
        status.setStatus(SystemStatus::resetPressed);
    }
}

void resetButtonReleased() {
    status.setStatus(SystemStatus::idle);
}

ResetButton<RESET> resetButton(resetButtonPushed, resetButtonReleased);

void eventLoop() {
    lights.run();
    messageManager.run();
    resetButton.run();
    status.run();
}

void updateClient() {
    status.setStatus(SystemStatus::transmitting);
    auto resp = client->get();

    Log.trace("Status code %d\n", resp);
    if (resp < 200 || resp >= 400) {
        status.setStatus(SystemStatus::failed);

        if (resp != HTTPC_ERROR_READ_TIMEOUT) {
            messageManager.setMessage(client->error(resp), false);
        }
    } else if (resp != 304) {
        status.setStatus(SystemStatus::idle);
        auto update = client->parse_json();
        lights.update(update.lights);
        messageManager.setMessages(update.messages);
    }
}

void onWifiConnected() {
    client = std::make_shared<StatusClient>(Log, setupManager.getHostname(), fingerprint, setupManager.getAuthorization());
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
    status.setStatus(SystemStatus::idle);
    messageManager.setMessage("Connected", false);
    messageManager.writeOut();

    Log.notice("Connected\n");
    Log.notice("IP address: %s\n", WiFi.localIP().toString().c_str());

    WiFi.setAutoReconnect(true);
    WiFi.setAutoConnect(true);

    onWifiConnected();
}

void setupWifi() {
    if (WiFi.status() != WL_CONNECTED) {
        auto connectingMsg = "Connecting to " + WiFi.SSID();
        Log.notice("%s\n", connectingMsg.c_str());

        messageManager.setMessage(connectingMsg.c_str(), false);
        messageManager.writeOut();

        status.setStatus(SystemStatus::connecting);
    }

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
    WiFi.setAutoConnect(true);

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