//
// Created by Greg Niemann on 11/4/18.
//

#include <Arduino.h>
#include <string>
#include "status_client.h"

const char *ETAG = "Etag";
const char *IF_NONE_MATCH = "If-None-Match";

int StatusClient::get() {
    client.end();

    if (!client.begin(url, fingerprint)) {
        return 0;
    }

    client.addHeader(IF_NONE_MATCH, etag.c_str());

    auto status = client.GET();
    if (status < 200 || status >= 400) {
        return status;
    }

    if (client.hasHeader(ETAG)) {
        etag = client.header(ETAG).c_str();
    }

    return status;
}

WiFiClient& StatusClient::getStream() {
    return client.getStream();
}