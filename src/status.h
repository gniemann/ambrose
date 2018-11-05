//
// Created by Greg Niemann on 11/4/18.
//

#ifndef BUILD_MONITOR_STATUS_H
#define BUILD_MONITOR_STATUS_H

#include <string>
#include <memory>
#include <vector>

enum class Status {
    UNKNOWN = -1,
    NOT_STARTED = 0,
    QUEUED = 1,
    IN_PROGRESS = 2,
    SUCCEEDED = 3,
    FAILED = 4
};

Status status_factory(const std::string &status);

class Stage {
public:
    Stage(std::string id, const Status &status): id(std::move(id)), status(status) {}

    bool operator==(const Stage &rhs) const {
        return id == rhs.id && status == rhs.status;
    }
private:
    std::string id;
    Status status;
};

using Stages = std::vector<std::shared_ptr<Stage>>;

Stages parse_json(const std::string &json);
#endif //BUILD_MONITOR_STATUS_H
