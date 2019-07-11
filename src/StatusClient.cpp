#include <utility>

//
// Created by Greg Niemann on 11/4/18.
//

#include <string>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <ArduinoLog.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "StatusClient.h"

const char *AUTHORIZATION = "Authorization";

const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(9) + 6*JSON_OBJECT_SIZE(2) + 13*JSON_OBJECT_SIZE(3) + 4*JSON_OBJECT_SIZE(6) + 1000;

int StatusClient::get() {
    client.end();

    if (!client.begin(wifiClient, url)) {
        return 0;
    }

    client.addHeader(AUTHORIZATION, authorization);

    auto status = client.GET();
    if (status < 200 || status >= 400) {
        return status;
    }

    return status;
}

Updates StatusClient::parse_json() {
    Updates updates;
    auto& stream = client.getStream();

    DynamicJsonBuffer jsonBuffer(capacity);

    JsonObject& root = jsonBuffer.parseObject(stream);
    if (!root.success()) {
        log.error("Parsing json failed");
        return updates;
    }

    JsonArray& lights = root["lights"];

    if (lights.success()) {
        updates.lights = lightsFromJSONArray(lights);
    } else {
        log.error("Parse json lights failed");
    }

    JsonArray& msgArr = root["messages"];
    if (msgArr.success()) {
        for (const char* msg: msgArr) {
            updates.messages.emplace_back(msg);
        }
    } else {
        log.error("Parse json messages failed");
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

StatusClient::StatusClient(Logging &log, const String &hostname, const String& pem, const String &auth) : log(log) {
    authorization = "Bearer " + auth;
    url = hostname + "/api/status";

    cert = std::unique_ptr<BearSSL::X509List>(new BearSSL::X509List(pem.c_str()));
    wifiClient.setTrustAnchors(cert.get());
}

