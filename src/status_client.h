//
// Created by Greg Niemann on 11/4/18.
//

#ifndef BUILD_MONITOR_CLIENT_H
#define BUILD_MONITOR_CLIENT_H

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

class StatusClient {
public:
    StatusClient(const char *url, const char *fingerprint): url(url), fingerprint(fingerprint) {}

    int get();
    WiFiClient& getStream();
private:
    const char* url;
    const char* fingerprint;
    std::string etag;
    HTTPClient client;

};

#endif //BUILD_MONITOR_CLIENT_H
