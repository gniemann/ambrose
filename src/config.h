//
// Created by Greg Niemann on 11/21/18.
//

#ifndef BUILD_MONITOR_CONFIGURATION_H
#define BUILD_MONITOR_CONFIGURATION_H

#include <array>
#include <Stream.h>
#include <ArduinoJson.h>
#include <unordered_map>
#include "status.h"

class Configuration {
public:
    Stage operator[](std::size_t index) const { return stages[index]; }
    std::size_t size() const { return stages.size(); }

    void update(std::vector<Stage> newStages);

    typedef std::vector<Stage>::iterator iterator;
    iterator begin() { return stages.begin(); }
    iterator end() { return stages.end(); }
private:
    std::vector<Stage> stages;
};

#endif //BUILD_MONITOR_CONFIGURATION_H
