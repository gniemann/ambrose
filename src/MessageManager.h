//
// Created by Greg Niemann on 2019-02-12.
//

#ifndef BUILD_MONITOR_MESSAGEMANAGER_H
#define BUILD_MONITOR_MESSAGEMANAGER_H

#include <string>
#include <array>
#include <deque>
#include <Adafruit_LEDBackpack.h>
#include <algorithm>
#include "Manager.h"

enum class MessageState {
    Stopped,
    Scrolling,
    Paused,
    ScrollOut
};

template <size_t NumSegments>
class MessageManager: Manager {
public:
    MessageManager();

    void setMessage(const std::string msg, bool scrollIn = true);

    void setMessages(const Messages &newMessages);

    void writeOut();
    void run() override;
    void clear();
private:
    void next();
    void setCurrentMessage(const std::string &message, bool scrollIn);

    static constexpr size_t numCharacters = NumSegments * 4;
    std::array<Adafruit_AlphaNum4, NumSegments> segments;
    std::array<char, numCharacters> buffer;
    std::string currentMessage;
    std::deque<std::string> messages;
    size_t position;
    int steps;
    int pauseSteps;
    MessageState state;
};

std::string uppercase(const std::string &str) {
    std::string retVal(str.length(), ' ');

    std::transform(str.cbegin(), str.cend(), retVal.begin(), [](char c) { return toupper(c); });
    return retVal;
}

template <size_t NumSegments>
MessageManager<NumSegments>::MessageManager(): position(0), steps(0), pauseSteps(0), state(MessageState::Stopped) {
    for (int i = 0; i < NumSegments; i++) {
        segments[i] = Adafruit_AlphaNum4();
        segments[i].begin(0x70 + i);
    }

    std::fill(buffer.begin(), buffer.end(), ' ');
}

template <size_t NumSegments>
void MessageManager<NumSegments>::setMessage(const std::string msg, bool scrollIn) {
    auto newMsg = uppercase(msg);

    messages.clear();
    messages.push_back(newMsg);

    setCurrentMessage(newMsg, scrollIn);
}

template<size_t NumSegments>
void MessageManager<NumSegments>::setMessages(const Messages &newMessages) {
    messages.clear();
    for (const auto& msg: newMessages) {
        messages.push_back(uppercase(msg));
    }
}

template <size_t NumSegments>
void MessageManager<NumSegments>::writeOut() {
    int index = 0;
    for (auto& segment: segments) {
        for (int j = 0; j < 4; j++) {
            segment.writeDigitAscii(j, buffer[index++]);
        }
        segment.writeDisplay();
    }
}

template <size_t NumSegments>
void MessageManager<NumSegments>::clear() {
    std::fill(buffer.begin(), buffer.end(), ' ');
    writeOut();
}

template <size_t NumSegments>
void MessageManager<NumSegments>::run() {
    switch (state) {
        case MessageState::Paused:
            if (++pauseSteps > 10) {
                pauseSteps = 0;
                if (position >= currentMessage.size()) {
                    next();
                } else {
                    state = MessageState::Scrolling;
                }
            }
            break;

        case MessageState::ScrollOut:
            if (std::all_of(buffer.begin(), buffer.end(), [](char c) { return c == ' '; })) {
                state = MessageState::Scrolling;
            } else {
                std::rotate(buffer.begin(), buffer.begin()+1, buffer.end());
                buffer[numCharacters - 1] = ' ';
            }
            break;

        case MessageState::Scrolling:
            if (++steps > numCharacters) {
                steps = 0;
                pauseSteps = 0;
                state = MessageState::Paused;
            } else {
                std::rotate(buffer.begin(), buffer.begin()+1, buffer.end());
                buffer[numCharacters - 1] = position < currentMessage.size() ? currentMessage[position++] : ' ';

                if (position >= currentMessage.size() && currentMessage.size() > numCharacters) {
                    state = MessageState::Paused;
                }
            }
            break;

        default:
            break;
    }
    writeOut();
}

template <size_t NumSegments>
void MessageManager<NumSegments>::next() {
    if (messages.size() == 0) {
        currentMessage = "";
        position = 0;
        return;
    }

    auto msg = messages.front();
    messages.pop_front();
    messages.push_back(msg);

    setCurrentMessage(msg, true);
}

template<size_t NumSegments>
void MessageManager<NumSegments>::setCurrentMessage(const std::string &message, bool scrollIn) {
    currentMessage = message;

    if (scrollIn) {
        state = MessageState::ScrollOut;
        position = 0;
    } else {
        state = MessageState::Paused;
        for (int i = 0; i < numCharacters; i++) {
            buffer[i] = i < message.size() ? message[i] : ' ';
        }
        position = message.size() > numCharacters ? numCharacters : message.size();
    }

    steps = 0;
    pauseSteps = 0;
}


#endif //BUILD_MONITOR_MESSAGEMANAGER_H
