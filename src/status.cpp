//
// Created by Greg Niemann on 11/4/18.
//

#include <ArduinoJson.h>
#include <memory>
#include <vector>
#include <locale>
#include <WiFiClient.h>
#include <algorithm>
#include <Arduino.h>
#include "status.h"

const int capacity = JSON_ARRAY_SIZE(4) + 4*JSON_OBJECT_SIZE(3) + 300;

std::string uppercase(const std::string &str) {
    std::string retVal(str.length(), ' ');

    std::transform(str.cbegin(), str.cend(), retVal.begin(), [](char c) { return toupper(c); });
    return retVal;
}

Status status_factory(const std::string &status) {
    auto raised = uppercase(status).c_str();
    Serial.println(raised);

    // using strcmp here to avoid the cost of allocating and copying the strings
    if (strcmp(raised, "NOT_STARTED") == 0) {
        return Status::NOT_STARTED;
    } else if (strcmp(raised, "QUEUED") == 0) {
        return Status::QUEUED;
    } else if (strcmp(raised, "IN_PROGRESS") == 0) {
        return Status::IN_PROGRESS;
    } else if (strcmp(raised, "SUCCEEDED") == 0) {
        return Status::SUCCEEDED;
    } else if (strcmp(raised, "FAILED") == 0) {
        return Status::FAILED;
    }

    return Status::UNKNOWN;
}

Stages parse_json(Stream& stream) {
    Stages stages;
    StaticJsonBuffer<capacity> jsonBuffer;
    auto& arr = jsonBuffer.parseArray(stream);

    if (!arr.success()) {
        return stages;
    }

    for (JsonObject& arrayVal: arr) {
        std::string id = arrayVal["id"].as<char*>();
        auto status = status_factory(arrayVal["status"].as<char*>());

        if (id.empty()) {
            continue;
        }

        stages.push_back(std::make_shared<Stage>(id, status));
    }

    return stages;
}