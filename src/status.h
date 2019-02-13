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
    Stage(std::string id, const Status &status): id(std::move(id)), status(status), prevStatus(Status::UNKNOWN) {}
    Stage(): id(""), status(Status::UNKNOWN), prevStatus(Status::UNKNOWN) {}

    bool operator==(const Stage &rhs) const {
        return id == rhs.id && status == rhs.status;
    }

    bool operator!=(const Stage &rhs) const {
        return !(*this == rhs);
    }

    bool isChanged() const { return status != prevStatus; }

    Status getStatus() const { return status; }
    Status getPrevStatus() const { return prevStatus; }
    std::string getId() const { return id; }

    void setPrevStatus(Status prevStatus) { this->prevStatus = prevStatus; }
private:
    std::string id;
    Status status;
    Status prevStatus;
};

using Stages = std::vector<Stage>;
using Messages = std::vector<std::string>;

typedef struct {
    Stages stages;
    Messages messages;
} Updates;


Updates parse_json(Stream& stream);
#endif //BUILD_MONITOR_STATUS_H
