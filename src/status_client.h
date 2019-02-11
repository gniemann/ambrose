//
// Created by Greg Niemann on 11/4/18.
//

#ifndef BUILD_MONITOR_CLIENT_H
#define BUILD_MONITOR_CLIENT_H

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

class StatusClient {
public:
    StatusClient(std::string url, std::string fingerprint, std::string authorization): url(std::move(url)), fingerprint(std::move(fingerprint)), authorization(std::move(authorization)) {}

    int get();
    WiFiClient& getStream();
private:
    std::string url;
    std::string fingerprint;
    std::string authorization;
    std::string etag;
    HTTPClient client;

};

#endif //BUILD_MONITOR_CLIENT_H
