//
// Created by Greg Niemann on 2019-02-11.
//

#ifndef BUILD_MONITOR_SETUP_SERVER_H
#define BUILD_MONITOR_SETUP_SERVER_H


#include <ESP8266WebServer.h>

typedef struct {
    std::string ssid;
    std::string wifiPassword;
    std::string username;
    std::string token;
} Settings;

class SetupServer {
public:
    SetupServer();
    void waitForSettings();

    Settings getSettings() const { return settings; }
private:
    void handleSettings();
    ESP8266WebServer server;

    Settings settings;
    bool hasCompleted;
};


#endif //BUILD_MONITOR_SETUP_SERVER_H
