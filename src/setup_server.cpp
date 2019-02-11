//
// Created by Greg Niemann on 2019-02-11.
//

#include "setup_server.h"
#include <ArduinoJson.h>

SetupServer::SetupServer(): hasCompleted(false) {
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
    settings.ssid = server.arg("ssid").c_str();
    settings.wifiPassword = server.arg("password").c_str();
    settings.username = server.arg("username").c_str();
    settings.token = server.arg("token").c_str();

    server.send(200, "plain/text", "OK");
    hasCompleted = true;
}