//
// Created by Greg Niemann on 2019-02-11.
//

#include "setup_server.h"
#include <ArduinoJson.h>
#include <FS.h>

SetupServer::SetupServer(): hasCompleted(false) {

    server.serveStatic("/bootstrap.min.css", SPIFFS, "/bootstrap.min.css", "maxage=86400");
    server.serveStatic("/", SPIFFS, "/setup.html");
    server.on("/settings", HTTP_POST, [this]() { this->handleSettings(); } );
}

void SetupServer::waitForSettings() {
    server.begin();

    while (!hasCompleted) {
        server.handleClient();
        delay(200);
    }

    server.close();
}

void SetupServer::handleSettings() {
    Serial.println("Received settings POST");
    settings.ssid = server.arg("ssid").c_str();
    settings.wifiPassword = server.arg("wifi-password").c_str();
    settings.username = server.arg("username").c_str();
    settings.token = server.arg("password").c_str();

    File file = SPIFFS.open("/success.html", "r");
    server.streamFile(file, "text/html");
    file.close();
    hasCompleted = true;
}
