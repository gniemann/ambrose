//
// Created by Greg Niemann on 2019-02-11.
//

#ifndef BUILD_MONITOR_SETTINGSMANAGER_H
#define BUILD_MONITOR_SETTINGSMANAGER_H


class SettingsManager {
public:
    SettingsManager();
    ~SettingsManager();

    bool checkForSettings();

    void remoteSetup();

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
};


#endif //BUILD_MONITOR_SETTINGSMANAGER_H
