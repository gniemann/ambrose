//
// Created by Greg Niemann on 11/4/18.
//

#include <Arduino.h>
#include <string>
#include "status_client.h"
#include <ArduinoJson.h>

const char *ETAG = "ETag";
const char *IF_NONE_MATCH = "If-None-Match";
const char *AUTHORIZATION = "Authorization";

const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(9) + 6*JSON_OBJECT_SIZE(2) + 13*JSON_OBJECT_SIZE(3) + 4*JSON_OBJECT_SIZE(6) + 1000;

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

Updates StatusClient::parse_json() {
    Updates updates;
    auto& stream = client.getStream();

    DynamicJsonBuffer jsonBuffer(capacity);
    Serial.print("Buffer is ");
    Serial.print(jsonBuffer.size());
    Serial.println(" bytes");

    JsonObject& root = jsonBuffer.parseObject(stream);
    if (!root.success()) {
        Serial.println("Parsing json failed");
        return updates;
    }

    JsonArray& lights = root["lights"];

    if (lights.success()) {
        updates.lights = lightsFromJSONArray(lights);
    } else {
        Serial.println("Parse json lights failed");
    }

    JsonArray& msgArr = root["messages"];
    if (msgArr.success()) {
        for (const char* msg: msgArr) {
            updates.messages.emplace_back(msg);
        }
    } else {
        Serial.println("Parse json messages failed");
    }

    return updates;
}

std::string StatusClient::error(int code) const {
    switch (code) {
        case HTTPC_ERROR_READ_TIMEOUT:
            return "Request timed out";
        case HTTP_CODE_BAD_REQUEST:
            return "Request failed due to Bad Request error";
        case HTTP_CODE_UNAUTHORIZED:
            return "Request returned Unauthorized - check credentials";
        case HTTP_CODE_FORBIDDEN:
            return "Request returned Forbidden - check credentials";
        case HTTP_CODE_INTERNAL_SERVER_ERROR:
            return "Request failed due to server error";

        default:
            char buffer[10];
            sprintf(buffer, "%d", code);
            return "Request failed - " + std::string(buffer);
    }
}

