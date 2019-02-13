//
// Created by Greg Niemann on 2019-02-12.
//

#ifndef BUILD_MONITOR_MESSAGEMANAGER_H
#define BUILD_MONITOR_MESSAGEMANAGER_H

#include <string>
#include <array>
#include <Adafruit_LEDBackpack.h>
#include <Wire.h>

template <int NumSegments, uint8_t SDA, uint8_t SCL>
class MessageManager {
public:
    MessageManager();

    void setMessage(const std::string msg);
    void writeOut();
    void step();
private:
    static constexpr size_t numCharacters = NumSegments * 4;
    std::array<Adafruit_AlphaNum4, NumSegments> segments;
    std::string currentMessage;
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
    for (auto segment: segments) {
        segment.clear();
    }
    currentMessage = std::string(numCharacters, ' ') + msg;
    startPosition = 0;
}

template <int NumSegments, uint8_t SDA, uint8_t SCL>
void MessageManager<NumSegments, SDA, SCL>::writeOut() {
    for (auto& segment: segments) {
        segment.writeDisplay();
    }
}

template <int NumSegments, uint8_t SDA, uint8_t SCL>
void MessageManager<NumSegments, SDA, SCL>::step() {
    if (startPosition > currentMessage.size() - numCharacters - 1) {
        return;
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


#endif //BUILD_MONITOR_MESSAGEMANAGER_H
