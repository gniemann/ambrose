//
// Created by Greg Niemann on 2019-02-11.
//

#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <base64.h>
#include <ESP8266mDNS.h>
#include "SetupManager.h"

const char *cert_fingerprint = "08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";

const size_t capacity = 2 * JSON_OBJECT_SIZE(1) + 1000;

bool SetupManager::checkFSForSettings() {
    if (fileSystem.exists(authFilename) && fileSystem.exists(hostnameFilename)) {
        auto authFile = fileSystem.open(authFilename, "r");
        if (!authFile) {
            return false;
        }
        authorization = authFile.readString();
        authFile.close();

        auto hostFile = fileSystem.open(hostnameFilename, "r");
        if (!hostFile) {
            return false;
        }

        hostname = hostFile.readString();
        hostFile.close();

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
    log.notice("IP address: %s\n", WiFi.softAPIP().toString().c_str());

    server.serveStatic("/bootstrap.min.css", SPIFFS, "/bootstrap.min.css", "max-age=31536000");
    server.serveStatic("/", SPIFFS, "/wifi.html");
    server.on("/wifi", HTTP_POST, [this]() { this->postWifi(); } );
    server.on("/settings", HTTP_POST, [this]() { this->postSettings(); } );

    server.begin();

    log.trace("Waiting for settings");

    // set up mDNS

    if (!MDNS.begin("ambrose")) {
        log.error("Error setting up mDNS");
    }
    MDNS.addService("http", "tcp", 80);
}

void SetupManager::reset() {
    if (!fileSystem.remove(authFilename)) {
        log.fatal("Reset failed!");
    } else {
        log.notice("Reset successful");
    }
}

void SetupManager::postWifi() {
    auto ssid = server.arg("ssid");
    auto wifiPassword = server.arg("wifi-password");

    log.notice("Received wifi settings. Connecting...");

    WiFi.begin(ssid, wifiPassword);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
    }

    File file = SPIFFS.open("/device.html", "r");
    server.streamFile(file, "text/html");
    file.close();
}

void SetupManager::postSettings() {
    hostname = server.arg("hostname");
    auto username = server.arg("username");
    auto password = server.arg("password");
    auto deviceName = server.arg("devicename");

    auto auth = username + ":" + password;
    auto authHeader = "Basic " + base64::encode(auth);

    // get the device token
    HTTPClient client;
    auto registration_url = hostname + "/api/devices/register";
    if (!client.begin(registration_url, cert_fingerprint)) {
        log.notice("Could not begin session");
        // TODO: Return failure page
    }

    client.addHeader("Authorization", authHeader);
    client.addHeader("Content-Type", "application/json");

    std::array<String, 4> keys = {
            R"("name": ")" + deviceName + "\"",
            R"("lights": 10)",
            R"("gagues": 0)",
            R"("messages": true)"
    };
    auto key = keys.cbegin();
    String payload = "{" + *key;

    while (++key != keys.end()) {
        payload += ", " + *key;
    }

    payload += "}";

    auto status = client.POST(payload);
    if (status < 200 || status >= 400) {
        log.notice("Received non-200 status for device registration");
        // TODO: Return failure page
    }

    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject& root = jsonBuffer.parseObject(client.getStream());

    authorization = root["access_token"].as<char*>();

    auto authFile = fileSystem.open(authFilename, "w");
    authFile.write((const uint8_t*)authorization.c_str(), authorization.length());
    authFile.close();

    auto hostFile = fileSystem.open(hostnameFilename, "w");
    hostFile.write((const uint8_t*)hostname.c_str(), hostname.length());
    hostFile.close();

    hasSettings = true;

    File file = SPIFFS.open("/success.html", "r");
    server.streamFile(file, "text/html");
    file.close();



    wifi.softAPdisconnect(true);
    MDNS.close();
    server.close();
}

void SetupManager::run() {
    MDNS.update();
    server.handleClient();
}

void SetupManager::init() {
    hasSettings = checkFSForSettings();
}

SetupManager::SetupManager(fs::FS &fileSystem, ESP8266WiFiClass &wifi, Logging &log) : fileSystem(fileSystem), wifi(wifi), log(log) {}
