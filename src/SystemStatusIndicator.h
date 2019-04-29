//
// Created by Greg Niemann on 2019-03-01.
//

#ifndef BUILD_MONITOR_SYSTEMSTATUSINDICATOR_H
#define BUILD_MONITOR_SYSTEMSTATUSINDICATOR_H

#include <Adafruit_MCP23008.h>

enum class SystemStatus {
    off,
    idle,
    transmitting,
    connecting,
    notConnected,
    failed,
    resetPressed,
    resetPressedLong,
};

template <uint8_t redPin, uint8_t greenPin, uint8_t bluePin>
class SystemStatusIndicator: Manager {
public:
    SystemStatusIndicator(Adafruit_MCP23008 &mcp);
    void setStatus(SystemStatus newStatus);
    void run() override;
private:
    void display();
    Adafruit_MCP23008 &mcp;
    SystemStatus status;
    LEDPtr led;
};

template<uint8_t redPin, uint8_t greenPin, uint8_t bluePin>
SystemStatusIndicator<redPin, greenPin, bluePin>::SystemStatusIndicator(Adafruit_MCP23008 &mcp): mcp(mcp) {
    mcp.begin();
    mcp.pinMode(redPin, OUTPUT);
    mcp.pinMode(greenPin, OUTPUT);
    mcp.pinMode(bluePin, OUTPUT);
    setStatus(SystemStatus::off);
}

template<uint8_t redPin, uint8_t greenPin, uint8_t bluePin>
void SystemStatusIndicator<redPin, greenPin, bluePin>::setStatus(SystemStatus newStatus) {
    if (newStatus == status && led != nullptr) {
        return;
    }
    status = newStatus;

    switch (status) {
        case SystemStatus::off:
            led = LEDPtr(new LED(OFF));
            break;
        case SystemStatus::idle:
            led = LEDPtr(new LED(GREEN));
            break;
        case SystemStatus::transmitting:
            led = LEDPtr(new LED(BLUE));
            break;
        case SystemStatus::connecting:
            led = LEDPtr(new BlinkingLED(GREEN, 2, 2));
            break;
        case SystemStatus::notConnected:
            led = LEDPtr(new BlinkingLED(RED, 2, 2));
            break;
        case SystemStatus::failed:
            led = LEDPtr(new LED(RED));
            break;
        case SystemStatus::resetPressed:
            led = LEDPtr(new BlinkingLED(RED, 2, 2));
            break;
        case SystemStatus::resetPressedLong:
            led = LEDPtr(new BlinkingLED(RED, 1, 1));
            break;
    }

    display();
}

template<uint8_t redPin, uint8_t greenPin, uint8_t bluePin>
void SystemStatusIndicator<redPin, greenPin, bluePin>::run() {
    led->step();
    display();
}

template<uint8_t redPin, uint8_t greenPin, uint8_t bluePin>
void SystemStatusIndicator<redPin, greenPin, bluePin>::display() {
    Color col = led->getColor();
    mcp.digitalWrite(redPin, col.red == 0 ? HIGH : LOW);
    mcp.digitalWrite(greenPin, col.green == 0 ? HIGH : LOW);
    mcp.digitalWrite(bluePin, col.blue == 0 ? HIGH : LOW);
}


#endif //BUILD_MONITOR_SYSTEMSTATUSINDICATOR_H
