//
// Created by Greg Niemann on 2019-03-01.
//

#ifndef BUILD_MONITOR_SYSTEMSTATUSINDICATOR_H
#define BUILD_MONITOR_SYSTEMSTATUSINDICATOR_H

enum class SystemStatus {
    idle,
    transmitting,
    connecting,
    notConnected,
    failed
};

template <uint8_t redPin, uint8_t greenPin, uint8_t bluePin>
class SystemStatusIndicator: Manager {
public:
    SystemStatusIndicator();
    void setStatus(SystemStatus newStatus);
    void run() override;
private:
    void display();
    SystemStatus status;
    LEDPtr led;
};

template<uint8_t redPin, uint8_t greenPin, uint8_t bluePin>
SystemStatusIndicator<redPin, greenPin, bluePin>::SystemStatusIndicator() {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    setStatus(SystemStatus::notConnected);
}

template<uint8_t redPin, uint8_t greenPin, uint8_t bluePin>
void SystemStatusIndicator<redPin, greenPin, bluePin>::setStatus(SystemStatus newStatus) {
    if (newStatus == status) {
        return;
    }
    status = newStatus;

    switch (status) {
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
    digitalWrite(redPin, col.red == 0 ? HIGH : LOW);
    digitalWrite(greenPin, col.green == 0 ? HIGH : LOW);
    digitalWrite(bluePin, col.blue == 0 ? HIGH : LOW);
}


#endif //BUILD_MONITOR_SYSTEMSTATUSINDICATOR_H
