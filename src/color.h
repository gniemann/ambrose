//
// Created by Greg Niemann on 10/30/18.
//

#ifndef BUILD_MONITOR_COLOR_H
#define BUILD_MONITOR_COLOR_H

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

#endif //BUILD_MONITOR_COLOR_H
