//
// Created by Greg Niemann on 2019-02-11.
//

#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoLog.h>

#include "SetupManager.h"
#include "SetupServer.h"

bool SetupManager::checkFSForSettings() {
    if (fileSystem.exists(ssidFilename) && fileSystem.exists(wifiPasswordFilename) && fileSystem.exists(authFilename)) {
        auto SSIDFile = fileSystem.open(ssidFilename, "r");
        if (!SSIDFile) {
            return false;
        }
        ssid = SSIDFile.readString().c_str();
        SSIDFile.close();

        auto passwordFile = fileSystem.open(wifiPasswordFilename, "r");
        if (!passwordFile) {
            return false;
        }
        wifiPassword = passwordFile.readString().c_str();
        passwordFile.close();

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
    WiFi.mode(WIFI_AP);
    WiFi.softAP("devops_monitor_ap");
    log.notice("IP address: %s", WiFi.softAPIP().toString().c_str());
    srv = std::unique_ptr<SetupServer>(new SetupServer([this](Settings s) { this->receiveSettings(s); }));
    log.trace("Waiting for settings");
}

void SetupManager::reset() {
    if (!fileSystem.remove(ssidFilename) ||
        !fileSystem.remove(wifiPasswordFilename) ||
        !fileSystem.remove(authFilename)) {
        log.fatal("Reset failed!");
    } else {
        log.notice("Reset successful");
    }
}

void SetupManager::receiveSettings(Settings settings) {
    WiFi.softAPdisconnect(true);
    log.notice("Received settings");

    ssid = settings.ssid;
    wifiPassword = settings.wifiPassword;
    authorization = settings.username + ":" + settings.token;

    auto ssidFile = fileSystem.open(ssidFilename, "w");
    ssidFile.write((const uint8_t*)ssid.c_str(), ssid.size());
    ssidFile.close();

    auto passwordFile = fileSystem.open(wifiPasswordFilename, "w");
    passwordFile.write((const uint8_t*)wifiPassword.c_str(), wifiPassword.size());
    passwordFile.close();

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

SetupManager::SetupManager(fs::FS &fileSystem, Logging &log) : fileSystem(fileSystem), log(log) {}
