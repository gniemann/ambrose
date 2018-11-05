//
// Created by Greg Niemann on 11/4/18.
//

#include <Arduino.h>
#include <string>
#include "status_client.h"

const char *ETAG = "ETag";
const char *IF_NONE_MATCH = "If-None-Match";

std::string StatusClient::get() {
    if (!client.begin(url, fingerprint)) {
        return "";
    }

    client.addHeader(IF_NONE_MATCH, etag.c_str());

    auto status = client.GET();
    if (status < 200 || status >= 400) {
        return "";
    }

    if (client.hasHeader(ETAG)) {
        etag = client.header(ETAG).c_str();
    }

    auto retVal = client.getString().c_str();
    client.end();
    return retVal;
}
