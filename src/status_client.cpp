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

const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(9) + 10*JSON_OBJECT_SIZE(2) + 9*JSON_OBJECT_SIZE(3) + 1000;

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

