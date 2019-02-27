//
// Created by Greg Niemann on 2019-02-11.
//

#include <ESP8266WiFi.h>
#include "SettingsManager.h"
#include "setup_server.h"
#include <FS.h>

bool SettingsManager::checkForSettings() {
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

void SettingsManager::remoteSetup() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("devops_monitor_ap");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    SetupServer srv;
    Serial.println("Waiting for settings");
    srv.waitForSettings();

    WiFi.softAPdisconnect(true);
    auto settings = srv.getSettings();
    Serial.println("Received settings");

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
}

void SettingsManager::reset() {
    if (!fileSystem.remove(ssidFilename) ||
        !fileSystem.remove(wifiPasswordFilename) ||
        !fileSystem.remove(authFilename)) {
        Serial.println("Reset failed!");
    } else {
        Serial.println("Reset successful.");
    }
}
