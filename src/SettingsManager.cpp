//
// Created by Greg Niemann on 2019-02-11.
//

#include <ESP8266WiFi.h>
#include "SettingsManager.h"
#include "setup_server.h"
#include <FS.h>

const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

std::string base64_encode(char const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }

    return ret;

}

bool SettingsManager::checkForSettings() {
    if (SPIFFS.exists(ssidFilename) && SPIFFS.exists(wifiPasswordFilename) && SPIFFS.exists(authFilename)) {
        auto SSIDFile = SPIFFS.open(ssidFilename, "r");
        if (!SSIDFile) {
            return false;
        }
        ssid = SSIDFile.readString().c_str();
        SSIDFile.close();

        auto passwordFile = SPIFFS.open(wifiPasswordFilename, "r");
        if (!passwordFile) {
            return false;
        }
        wifiPassword = passwordFile.readString().c_str();
        passwordFile.close();

        auto authFile = SPIFFS.open(authFilename, "r");
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

    std::string authString = settings.username + ":" + settings.token;
    authorization = "Basic " + base64_encode(authString.c_str(), authString.size());

    auto ssidFile = SPIFFS.open(ssidFilename, "w");
    ssidFile.write((const uint8_t*)ssid.c_str(), ssid.size());
    ssidFile.close();

    auto passwordFile = SPIFFS.open(wifiPasswordFilename, "w");
    passwordFile.write((const uint8_t*)wifiPassword.c_str(), wifiPassword.size());
    passwordFile.close();

    auto authFile = SPIFFS.open(authFilename, "w");
    authFile.write((const uint8_t*)authorization.c_str(), authorization.size());
    authFile.close();
}

void SettingsManager::reset() {
    if (!SPIFFS.remove(ssidFilename) ||
        !SPIFFS.remove(wifiPasswordFilename) ||
        !SPIFFS.remove(authFilename)) {
        Serial.println("Reset failed!");
    } else {
        Serial.println("Reset successful.");
    }
}
