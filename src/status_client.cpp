//
// Created by Greg Niemann on 11/4/18.
//

#include <Arduino.h>
#include <string>
#include "status_client.h"

const char *ETAG = "ETag";
const char *IF_NONE_MATCH = "If-None-Match";
const char *AUTHORIZATION = "Authorization";

int StatusClient::get() {
    client.end();

    if (!client.begin(url.c_str(), fingerprint.c_str())) {
        return 0;
    }

    client.addHeader(IF_NONE_MATCH, etag.c_str());
    client.addHeader(AUTHORIZATION, authorization.c_str());

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