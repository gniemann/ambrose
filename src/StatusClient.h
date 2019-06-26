//
// Created by Greg Niemann on 11/4/18.
//

#ifndef BUILD_MONITOR_CLIENT_H
#define BUILD_MONITOR_CLIENT_H

#include "led.h"

using Messages = std::vector<std::string>;

typedef struct {
    Lights lights;
    Messages messages;
} Updates;

class Logging;
class HTTPClient;

class StatusClient {
public:
    StatusClient(Logging &log, const String &hostname, String fingerprint, const String &auth);

    int get();
    std::string error(int code) const;

    Updates parse_json();
private:
    String url;
    String fingerprint;
    String authorization;
    HTTPClient client;

    Logging &log;
};

#endif //BUILD_MONITOR_CLIENT_H
