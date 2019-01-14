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
    class ConfigIterator {
    private:
        int pos;
        Configuration *config;
    public:
        ConfigIterator(Configuration *c, int p = 0): pos(p), config(c) {}

        std::pair<Status, bool> operator*() const { return config->status(pos); }
        bool operator==(const ConfigIterator &rhs) const { return pos == rhs.pos; }
        bool operator!=(const ConfigIterator &rhs) const { return !(*this == rhs); }
        ConfigIterator& operator++() { ++pos; return *this; }

        ConfigIterator& operator++(int) {
            ConfigIterator clone(*this);
            ++pos;
            return clone;
        }
    };

    Stage operator[](std::size_t index) const { return stages[index]; }
    std::size_t size() const { return stages.size(); }

    void update(std::vector<Stage> newStages);

    std::pair<Status, bool> status(int pos) const { return std::make_pair(stages[pos].getStatus(), hasChanged[pos]); }

    typedef ConfigIterator iterator;
    iterator begin() { return ConfigIterator(this); }
    iterator end() { return ConfigIterator(this, stages.size()); }
private:
    std::vector<Stage> stages;
    std::vector<bool> hasChanged;
};

#endif //BUILD_MONITOR_CONFIGURATION_H
