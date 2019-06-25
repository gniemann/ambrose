//
// Created by Greg Niemann on 2019-02-11.
//

#ifndef BUILD_MONITOR_SETTINGSMANAGER_H
#define BUILD_MONITOR_SETTINGSMANAGER_H


#include "SetupServer.h"
#include "Manager.h"

namespace fs {
    class FS;
}

class Logging;

class SetupManager: Manager {
public:
    SetupManager(fs::FS &fileSystem, ESP8266WiFiClass &wifi, Logging &log);
    void init();
    bool checkForSettings();

    void remoteSetup();

    void reset();

    void run() override;

    std::string getAuthorization() const { return authorization; }
private:
    bool checkFSForSettings();
    void receiveSettings(Settings settings);
    bool hasSettings;
    const char *authFilename = "/authorization";

    std::string authorization;
    
    fs::FS &fileSystem;
    ESP8266WiFiClass &wifi;
    Logging &log;

    std::unique_ptr<SetupServer> srv;
};


#endif //BUILD_MONITOR_SETTINGSMANAGER_H
