//
// Created by Greg Niemann on 2019-02-11.
//

#ifndef BUILD_MONITOR_SETUPMANAGER_H
#define BUILD_MONITOR_SETUPMANAGER_H

#include <ESP8266WebServer.h>
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

    String getAuthorization() const { return authorization; }
    String getHostname() const { return hostname; }
    String getCertificate() const { return cert; }
private:
    bool checkFSForSettings();
    void postWifi();
    void postSettings();
    void errorPage();
    bool hasSettings;

    const char *authFilename = "/authorization";
    const char *hostnameFilename = "/hostname";
    const char *certFilename = "/certificate";

    String authorization;
    String hostname;
    String cert;
    
    fs::FS &fileSystem;
    ESP8266WiFiClass &wifi;
    ESP8266WebServer server;
    Logging &log;
};


#endif //BUILD_MONITOR_SETUPMANAGER_H
