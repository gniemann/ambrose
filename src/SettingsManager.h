//
// Created by Greg Niemann on 2019-02-11.
//

#ifndef BUILD_MONITOR_SETTINGSMANAGER_H
#define BUILD_MONITOR_SETTINGSMANAGER_H

namespace fs {
    class FS;
}

class SettingsManager {
public:
    SettingsManager(fs::FS &fileSystem): fileSystem(fileSystem) {}
    bool checkForSettings();

    void remoteSetup();

    void reset();

    std::string getSSID() const { return ssid; }
    std::string getWiFiPassword() const { return wifiPassword; }
    std::string getAuthorization() const { return authorization; }
private:
    const char *ssidFilename = "/ssid";
    const char *wifiPasswordFilename = "/wifiPassword";
    const char *authFilename = "/authorization";

    std::string ssid;
    std::string wifiPassword;
    std::string authorization;
    
    fs::FS &fileSystem;
};


#endif //BUILD_MONITOR_SETTINGSMANAGER_H
