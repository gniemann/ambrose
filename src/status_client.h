//
// Created by Greg Niemann on 11/4/18.
//

#ifndef BUILD_MONITOR_CLIENT_H
#define BUILD_MONITOR_CLIENT_H

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

class StatusClient {
public:
    StatusClient(const char *url, const char *fingerprint, const char *authorization): url(url), fingerprint(fingerprint), authoriation(authorization) {}

    int get();
    WiFiClient& getStream();
private:
    const char* url;
    const char* fingerprint;
    const char* authoriation;
    std::string etag;
    HTTPClient client;

};

#endif //BUILD_MONITOR_CLIENT_H
