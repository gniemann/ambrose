//
// Created by Greg Niemann on 11/4/18.
//

#ifndef BUILD_MONITOR_CLIENT_H
#define BUILD_MONITOR_CLIENT_H

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "led.h"

using Messages = std::vector<std::string>;

typedef struct {
    Lights lights;
    Messages messages;
} Updates;

class StatusClient {
public:
    StatusClient(std::string url, std::string fingerprint, std::string authorization): url(std::move(url)), fingerprint(std::move(fingerprint)), authorization(std::move(authorization)) {}

    int get();
    std::string error(int code) const;

    Updates parse_json();
private:
    std::string url;
    std::string fingerprint;
    std::string authorization;
    std::string etag;
    HTTPClient client;

};

#endif //BUILD_MONITOR_CLIENT_H
