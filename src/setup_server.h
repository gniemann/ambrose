//
// Created by Greg Niemann on 2019-02-11.
//

#ifndef BUILD_MONITOR_SETUP_SERVER_H
#define BUILD_MONITOR_SETUP_SERVER_H


#include <ESP8266WebServer.h>
#include <functional>

typedef struct {
    std::string ssid;
    std::string wifiPassword;
    std::string username;
    std::string token;
} Settings;

class SetupServer {
public:
    using OnSettingsReceived = std::function<void(Settings)>;

    SetupServer(OnSettingsReceived onSettingsReceived);

    void handleClients();
private:
    void handleSettings();
    ESP8266WebServer server;
    bool hasCompleted;
    OnSettingsReceived onSettingsReceived;
};


#endif //BUILD_MONITOR_SETUP_SERVER_H
