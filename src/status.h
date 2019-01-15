//
// Created by Greg Niemann on 11/4/18.
//

#ifndef BUILD_MONITOR_STATUS_H
#define BUILD_MONITOR_STATUS_H

#include <string>
#include <memory>
#include <vector>
#include <Stream.h>

enum class Status {
    UNKNOWN = -1,
    NOT_STARTED = 0,
    QUEUED = 1,
    IN_PROGRESS = 2,
    SUCCEEDED = 3,
    PENDING_APPROVAL = 4,
    FAILED = 5,
};

Status status_factory(const std::string &status);

class Stage {
public:
    Stage(std::string id, const Status &status): id(std::move(id)), status(status) {}
    Stage(): id(""), status(Status::UNKNOWN) {}

    bool operator==(const Stage &rhs) const {
        return id == rhs.id && status == rhs.status;
    }

    bool operator!=(const Stage &rhs) const {
        return !(*this == rhs);
    }

    Status getStatus() const { return status; }
    std::string getId() const { return id; }
private:
    std::string id;
    Status status;
};

using Stages = std::vector<Stage>;

Stages parse_json(Stream& stream);
#endif //BUILD_MONITOR_STATUS_H
