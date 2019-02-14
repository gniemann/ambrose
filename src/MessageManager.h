//
// Created by Greg Niemann on 2019-02-12.
//

#ifndef BUILD_MONITOR_MESSAGEMANAGER_H
#define BUILD_MONITOR_MESSAGEMANAGER_H

#include <string>
#include <array>
#include <deque>
#include <Adafruit_LEDBackpack.h>
#include <Wire.h>

template <int NumSegments, uint8_t SDA, uint8_t SCL>
class MessageManager {
public:
    MessageManager();

    void setMessage(const std::string msg);

    void setMessages(const Messages &newMessages);

    void writeOut();
    void step();
    void clear();
private:
    void next();

    static constexpr size_t numCharacters = NumSegments * 4;
    std::array<Adafruit_AlphaNum4, NumSegments> segments;
    std::string currentMessage;
    std::deque<std::string> messages;
    int startPosition;
    int pauseSteps;
};

template <int NumSegments, uint8_t SDA, uint8_t SCL>
MessageManager<NumSegments, SDA, SCL>::MessageManager() {
    Wire.begin(SDA, SCL);
    for (int i = 0; i < NumSegments; i++) {
        segments[i] = Adafruit_AlphaNum4();
        segments[i].begin(0x70 + i);
    }
}

template <int NumSegments, uint8_t SDA, uint8_t SCL>
void MessageManager<NumSegments, SDA, SCL>::setMessage(const std::string msg) {
    messages.clear();
    messages.push_back(msg);
}

template<int NumSegments, uint8_t SDA, uint8_t SCL>
void MessageManager<NumSegments, SDA, SCL>::setMessages(const Messages &newMessages) {
    messages.clear();
    for (auto msg: newMessages) {
        messages.push_back(msg);
    }
}

template <int NumSegments, uint8_t SDA, uint8_t SCL>
void MessageManager<NumSegments, SDA, SCL>::writeOut() {
    for (auto& segment: segments) {
        segment.writeDisplay();
    }
}

template <int NumSegments, uint8_t SDA, uint8_t SCL>
void MessageManager<NumSegments, SDA, SCL>::clear() {
    for (auto& segment: segments) {
        segment.clear();
        segment.writeDisplay();
    }
}

template <int NumSegments, uint8_t SDA, uint8_t SCL>
void MessageManager<NumSegments, SDA, SCL>::step() {
    if (currentMessage.empty()) {
        next();
    }

    if (startPosition > currentMessage.size() - numCharacters - 1) {
        if (++pauseSteps > 10) {
            pauseSteps = 0;
            next();
        } else {
            return;
        }
    }

    if (startPosition % numCharacters == 0) {
        if (++pauseSteps > 10) {
            pauseSteps = 0;
        } else {
            return;
        }
    }

    int index = ++startPosition;
    for (auto& segment: segments) {
        for (int j = 0; j < 4; j++) {
            segment.writeDigitAscii(j, currentMessage[index++]);
        }
    }
}

template <int NumSegments, uint8_t SDA, uint8_t SCL>
void MessageManager<NumSegments, SDA, SCL>::next() {
    if (messages.size() == 0) {
        return;
    }

    auto msg = messages.front();
    messages.pop_front();
    messages.push_back(msg);

    if (msg.size() < numCharacters) {
        msg += std::string(numCharacters - msg.size(), ' ');
    }

    msg = std::string(numCharacters, ' ') + msg;

    if (currentMessage.size() > 0) {
        auto trail = currentMessage.substr(currentMessage.size() - numCharacters, numCharacters);
        msg = trail + msg;
    }

    currentMessage = msg;
    startPosition = 0;
}


#endif //BUILD_MONITOR_MESSAGEMANAGER_H
