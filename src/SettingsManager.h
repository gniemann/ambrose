//
// Created by Greg Niemann on 2019-02-11.
//

#ifndef BUILD_MONITOR_SETTINGSMANAGER_H
#define BUILD_MONITOR_SETTINGSMANAGER_H


class SettingsManager {
public:

    bool checkForSettings() { return false; }

    void remoteSetup();

    std::string getSSID() const { return ssid; }
    std::string getWiFiPassword() const { return wifiPassword; }
    std::string getAuthorization() const;
private:
    std::string ssid;
    std::string wifiPassword;
    std::string username;
    std::string password;
};


#endif //BUILD_MONITOR_SETTINGSMANAGER_H
