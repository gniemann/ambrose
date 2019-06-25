//
// Created by Greg Niemann on 2019-02-11.
//

#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoLog.h>

#include "SetupManager.h"
#include "SetupServer.h"

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

    //
    authorization = settings.username + ":" + settings.token;

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
