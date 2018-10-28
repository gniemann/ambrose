//
// Created by Greg Niemann on 10/20/18.
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <vector>

const char* ssid = "bodhi";
const char* password = "wtaguest";

ESP8266WebServer server(80);

int rgbLED[] = {D5, D6, D7};

class Color {
public:
    int red;
    int green;
    int blue;

    Color(int r, int g, int b) : red(r), green(g), blue(b) {}
};

const Color RED = Color(255, 0, 0);
const Color GREEN = Color(0, 255, 0);
const Color BLUE = Color(0, 0, 255);
const Color YELLOW = Color(255, 255, 0);
const Color CYAN = Color(0, 255, 255);
const Color MAGENTA = Color(255, 0, 255);
const Color SILVER = Color(192, 192, 192);
const Color GRAY = Color(128, 128, 128);
const Color MAROON = Color(128, 0, 0);
const Color OLIVE = Color(128, 128, 0);
const Color PURPLE = Color(128, 0, 128);
const Color TEAL = Color(0, 128, 128);
const Color NAVY = Color(0, 0, 128);
const Color WHITE = Color(255, 255, 255);
const Color OFF = Color(0, 0, 0);

const std::vector<Color> COLORS = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, SILVER, GRAY, MAROON, OLIVE, PURPLE, TEAL, NAVY, WHITE};

class LED {
public:
    LED(const Color &c) : color(c), on(true) {}
    void turnOn() { on = true; }
    void turnOff() { on = false; }
    bool isOn() const { return on; }

    Color getColor() const { return on ? color : OFF; }

    virtual void step() {}
private:
    Color color;
    bool on;
};

class BlinkingLED: public LED {
public:
    BlinkingLED(const Color &c, int onPeriod, int offPeriod): LED(c), onInterval(onPeriod), offInterval(offPeriod), intervalCounter(0) {}
    void step();
private:
    void toggle();

    int onInterval;
    int offInterval;
    int intervalCounter;
};

void BlinkingLED::step() {
    if (++intervalCounter >= (isOn() ? onInterval : offInterval)) {
        toggle();
    }
}

void BlinkingLED::toggle() {
    if (isOn()) {
        turnOff();
    } else {
        turnOn();
    }
    intervalCounter = 0;
}

class InitiallyBlinkingLED: public BlinkingLED {
public:
    InitiallyBlinkingLED(const Color &c, int times, int onPeriod, int offPeriod): BlinkingLED(c, onPeriod, offPeriod), blinkTimes(times), isBlinking(true), currentTime(0) { }

    void step();

private:
    int blinkTimes;
    bool isBlinking;
    int currentTime;
};

void InitiallyBlinkingLED::step() {
    if (isBlinking) {
        auto wasOn = isOn();
        BlinkingLED::step();
        if (!wasOn && isOn() && ++currentTime >= blinkTimes) {
            isBlinking = false;
        }
    }
}

void handleRoot() {
    digitalWrite(LED_BUILTIN, LOW);
    server.send(200, "text/plain", "hello from esp8266!");
}

void setupWifi() {
    WiFi.begin(ssid, password);
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setupServer() {
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");
}

void setColor(int *led, const bool* color) {
    for (int i = 0; i < 3; i++) {
        digitalWrite(led[i], color[i]);
    }
}

void setColor(int *led, const Color &color) {
    analogWrite(led[0], 255 - color.red);
    analogWrite(led[1], 255 - color.green);
    analogWrite(led[2], 255 - color.blue);
}

Color randomColor() {
    return COLORS[random(0, COLORS.size())];
}

void blink(int *led, const Color &color, int times, int onTime, int offTime) {

    for (int i = 0; i < times; i++) {
        setColor(led, color);
        delay(onTime);
        setColor(led, OFF);
        delay(offTime);
    }
}

// the setup function runs once when you press reset or power the board
void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    for (int i = 0; i < 3; i++) {
        pinMode(rgbLED[i], OUTPUT);
    }


    digitalWrite(LED_BUILTIN, HIGH);
//    Serial.begin(115200);
//    Serial.print("Connecting:");

//    setupWifi();
//    setupServer();

    digitalWrite(LED_BUILTIN, LOW);

    analogWriteRange(255);
}

// the loop function runs over and over again forever
void loop() {
    auto fastBlink = InitiallyBlinkingLED(RED, 5, 1, 1);
    auto medBlink = InitiallyBlinkingLED(GREEN, 5, 2, 1);
    auto slowBlink = InitiallyBlinkingLED(BLUE, 5, 4, 1);

    for (int i = 0; i < 50; i++) {
        setColor(rgbLED, fastBlink.getColor());
        fastBlink.step();
        delay(200);
    }

    for (int i = 0; i < 50; i++) {
        setColor(rgbLED, medBlink.getColor());
        medBlink.step();
        delay(200);
    }

    for (int i = 0; i < 50; ++i) {
        setColor(rgbLED, slowBlink.getColor());
        slowBlink.step();
        delay(200);
    }

//    server.handleClient();
}