//
// Created by Greg Niemann on 2019-02-11.
//

#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <base64.h>

#include "SetupManager.h"
#include "SetupServer.h"

const char* registration_url = "https://devops-status-monitor.herokuapp.com/api/devices/register";
const char *cert_fingerprint = "08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";

const size_t capacity = 2 * JSON_OBJECT_SIZE(1) + 1000;

bool SetupManager::checkFSForSettings() {
    if (fileSystem.exists(authFilename)) {
        auto authFile = fileSystem.open(authFilename, "r");
        if (!authFile) {
            return false;
        }
        authorization = authFile.readString().c_str();
        authFile.close();

        return true;
    }

    return false;
}

bool SetupManager::checkForSettings() {
    return hasSettings;
}

void SetupManager::remoteSetup() {
    wifi.mode(WIFI_AP);
    wifi.softAP("devops_monitor_ap");
    log.notice("IP address: %s", WiFi.softAPIP().toString().c_str());
    srv = std::unique_ptr<SetupServer>(new SetupServer([this](Settings s) { this->receiveSettings(s); }));
    log.trace("Waiting for settings");
}

void SetupManager::reset() {
    if (!fileSystem.remove(authFilename)) {
        log.fatal("Reset failed!");
    } else {
        log.notice("Reset successful");
    }
}

void SetupManager::receiveSettings(Settings settings) {
    wifi.softAPdisconnect(true);
    log.notice("Received settings. Connecting...");

    WiFi.begin(settings.ssid.c_str(), settings.wifiPassword.c_str());
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
    }

    std::string auth = settings.username + ":" + settings.token;

    auto authHeader = "Basic " + base64::encode(auth.c_str());

    // get the device token
    HTTPClient client;
    if (!client.begin(registration_url, cert_fingerprint)) {
        log.notice("Could not begin session");
    }

    client.addHeader("Authorization", authHeader.c_str());
    client.addHeader("Content-Type", "application/json");
    auto payload = "{\"name\": \"ambrose X\"}";
    auto status = client.POST(payload);

    if (status < 200 || status >= 400) {
        log.notice("Received non-200 status for device registration");
    }

    DynamicJsonBuffer jsonBuffer(capacity);

    JsonObject& root = jsonBuffer.parseObject(client.getStream());

    authorization = std::string(root["access_token"].as<char*>());

    auto authFile = fileSystem.open(authFilename, "w");
    authFile.write((const uint8_t*)authorization.c_str(), authorization.size());
    authFile.close();

    hasSettings = true;
}

void SetupManager::run() {
    srv->handleClients();
}

void SetupManager::init() {
    hasSettings = checkFSForSettings();
}

SetupManager::SetupManager(fs::FS &fileSystem, ESP8266WiFiClass &wifi, Logging &log) : fileSystem(fileSystem), wifi(wifi), log(log) {}
