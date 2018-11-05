//
// Created by Greg Niemann on 11/4/18.
//

#include <ArduinoJson.h>
#include <memory>
#include <vector>
#include <locale>
#include "status.h"

const int capacity = JSON_ARRAY_SIZE(4) + 4*JSON_OBJECT_SIZE(3) + 300;

Status status_factory(const std::string &status) {
    std::locale loc;
    auto raised = std::toupper(status, loc).c_str();

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

Stages parse_json(const std::string &json) {
    Stages stages;
    StaticJsonBuffer<capacity> jsonBuffer;
    auto& arr = jsonBuffer.parseArray(json.c_str());

    if (!arr.success()) {
        return stages;
    }

    for (JsonObject& arrayVal: arr) {
        auto id = arrayVal.get<char*>("id");
        auto status = status_factory(arrayVal.get<char*>("status"));

        if (strlen(id) == 0 || status == Status::UNKNOWN) {
            continue;
        }

        stages.push_back(std::make_shared<Stage>(id, status));
    }

    return stages;
}