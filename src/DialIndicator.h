//
// Created by Greg Niemann on 2019-02-26.
//

#ifndef BUILD_MONITOR_DIALINDICATOR_H
#define BUILD_MONITOR_DIALINDICATOR_H

#include <Stepper.h>

template <int p1, int p2, int p3, int p4>
class DialIndicator {
public:
    DialIndicator();

    void setPercent(double percent);
private:
    static constexpr int steps = 360;
    Stepper motor;
    int position = 0;
};

template <int p1, int p2, int p3, int p4>
DialIndicator<p1, p2, p3, p4>::DialIndicator(): motor(steps, p1, p2, p3, p4) {
    motor.setSpeed(60);
}

template <int p1, int p2, int p3, int p4>
void DialIndicator<p1, p2, p3, p4>::setPercent(double percent) {
    int newPos = percent * steps;

    motor.step(newPos - position);
    position = newPos;
}


#endif //BUILD_MONITOR_DIALINDICATOR_H
