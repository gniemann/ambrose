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

template <std::size_t N>
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

    void init(Stream &stream);
    
    Stage operator[](std::size_t index) const { return stages[index]; }
    std::size_t size() const { return stages.size(); }

    void update(const Stage &stage) {
        if (idToIndexMap.find(stage.getId()) == idToIndexMap.end()) {
            return;
        }

        auto index = idToIndexMap[stage.getId()];
        if (stages[index] != stage) {
            stages[index] = stage;
            hasChanged[index] = true;
        } else {
            hasChanged[index] = false;
        }
    }

    std::pair<Status, bool> status(int pos) const { return std::make_pair(stages[pos].getStatus(), hasChanged[pos]); }

    typedef ConfigIterator iterator;
    iterator begin() { return ConfigIterator(this); }
    iterator end() { return ConfigIterator(this, N); }
private:

    static constexpr int capacity = JSON_ARRAY_SIZE(N) + N*JSON_OBJECT_SIZE(1) + 300;

    std::array<Stage, N> stages;
    std::unordered_map<std::string, int> idToIndexMap;
    std::array<bool, N> hasChanged;
};

template <std::size_t N>
void Configuration<N>::init(Stream &stream) {
    StaticJsonBuffer<capacity> jsonBuffer;
    auto& arr = jsonBuffer.parseArray(stream);

    if (!arr.success()) {
        return;
    }

    auto stage = stages.begin();
    auto index = 0;
    for (JsonObject& arrayVal: arr) {
        *stage = Stage(arrayVal["id"].as<char*>(), Status::UNKNOWN);
        idToIndexMap[stage->getId()] = index++;

        if (++stage == stages.end()) {
            break;
        }
    }
}


#endif //BUILD_MONITOR_CONFIGURATION_H
