#ifndef CAR_H
#define CAR_H

#include <mutex>
#include <queue>

// Averages float values
class Damper {
   private:
    std::queue<double> values;
    int maxSize;

   public:
    Damper(int size) : maxSize(size) {}

    void addValue(double value);

    double getAverage();
};

class Car {
   public:
    bool ignition = false;    // Ignition

    // Rev limiter
    int defaultRevLimitTick = 5;  // Cuts off gas for n ticks if rev limit is reached.
    int revLimitTick = 0;         // no-gas ticks remaining
    int revLimit = 8000;          // Gas will be cut when rev limit is reached

    float gearRatios[7] = {0, 0.8, 1.3, 1.8, 2.3, 2.7, 3.5};                          // Gearing ratios - Used to match engine rpm to wheel rpm
    float gearLazyValues[7] = {0.99, 0.999, 0.9994, 0.9995, 0.9996, 0.9997, 0.9998};  // Engine time for revs to settle back - values closer to one need more time to go back to idle cause less resistance (exponential decay)
    float gearThrottleResponses[7] = {1, 0.3, 0.18, 0.15, 0.13, 0.11, 0.09};        // Throttle sensitivity - Should feel lower in higher gears since the high gears is hard on the engine.

    // Wheel resistances
    double coastLazyValue = 0.999;  // Driving drag on wheels (and also engine if in gear)
    float brakeFactor = 1;          // Brake Drag on wheels (and also engine if in gear) - Basically the same as coastLazyValue

    float clutchKick = 0.6;  // Clutch jerkiness (1 is smooth)

    std::mutex m_tick;

    void tick();

    void setGear(int newGear);
    int getGear();

    void setGas(float newGas);
    float getGas();

    void setRPM(float newRPM);  // Sets rpm for next tick
    float getRPM();

    void setWheelSpeed(float newSpeed);  // Sets wheelRPM for next tick
    float getWheelSpeed();

    float getHorses();

   private:
    float gas = 0;       // Throttle body
    float rpm = 0;       // Engine RPM
    float wheelRPM = 0;  // Wheel RPM or speed does not really matter

    float idleValve = 1;  // Idle valve
    float horses = 0;     // Power produced immediately
    int gear = 0;         // Current Gear

    float clutch = 0;  // Difference of revs to 'smoothly' join

    Damper rpmDamper{5};
    Damper wheelSpeedDamper{10};

    void controlIdle();
    void addEnergy();
};

#endif