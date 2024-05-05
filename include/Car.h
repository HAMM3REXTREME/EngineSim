#ifndef CAR_H
#define CAR_H

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

// Averages float values
class Damper {
   private:
    std::queue<double> values;
    int maxSize;

   public:
    Damper(int size) : maxSize(size) {}

    void addValue(double value) {
        values.push(value);
        if (values.size() > maxSize) {
            values.pop();
        }
    }

    double getAverage() {
        if (values.empty()) {
            return 0.0;  // Return 0 if queue is empty
        }

        double sum = 0.0;
        int count = 0;
        std::queue<double> temp = values;  // Create a copy of the queue

        while (!temp.empty()) {
            sum += temp.front();
            temp.pop();
            count++;
        }

        return sum / count;
    }
};

class Car {
   public:
    bool running = false;  // Is car running - pauses tick when off
    bool ignition = false; // Ignition

    // Gearing
    float gearRatios[7] = {0, 0.8, 1.3, 1.8, 2.3, 2.7, 3.5};
    float gearLazyValues[7] = {0.99, 0.999, 0.9995, 0.9996, 0.9997, 0.9998, 0.9999};
    float gearThrottleResponses[7] = {1, 0.15, 0.08, 0.06, 0.05, 0.04, 0.035};

    // Engine
    double lazyValue = 0.99;      // Engine time for revs to settle back - values closer to one need more time to go back to idle
    double throttleResponse = 1;  // Throttle sensitivity - Should feel lower in higher gears since we reduce the value after every upshift bruh

    // Wheel
    double coastLazyValue = 0.999;  // Driving drag on wheels (and also engine if in gear)
    float brakeFactor = 1;          // Brake Drag on wheels (and also engine if in gear) - Basically the same as coastLazyValue

    float clutchKick = 0.6; // Clutch jerkiness (1 is smooth)

    std::mutex m_tick;
    void tick() {
        while (running) {
            // TODO: Fix broken multithreading
            std::lock_guard<std::mutex> lock(m_tick);

            if (rpm >= 800) {  // Idle air control valve
                idleValve = 1;
            } else if (rpm <= 700) {
                idleValve = 30;
            }
            rpm = rpm * lazyValue;
            if (rpm > 10) {
                rpm += horses / rpm;
                if (rpm <= 8000 && ignition) {  // Rev limiter thingy
                    horses = rpm * (gas + idleValve) * throttleResponse * brakeFactor;
                } else {
                    horses = 0;
                }
            }

            // Apply clutch revs in multiple smaller chunks of revs, makes a kick, like dumping the clutch in a real car
            // Idk how this works
            rpm += clutch * clutchKick;
            clutch = (1.0 - clutchKick) * clutch;

            // Wheel RPM depending on the engine rpm , current gear ratio and coasting drag
            if (gear >= 1) {
                wheelRPM = rpm * gearRatios[gear] * coastLazyValue * brakeFactor;
                rpm = rpm * brakeFactor; // * coastLazyValue; // Add this or don't doesn't really matter too much
            } else {
                wheelRPM = wheelRPM * coastLazyValue * brakeFactor;
            }

            rpmDamper.addValue(rpm);
            wheelSpeedDamper.addValue(wheelRPM);

            std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Farts per second
        }
    }

    void setGear(int newGear) {
        gear = newGear;
        lazyValue = gearLazyValues[newGear];
        throttleResponse = gearThrottleResponses[newGear];
        if (gear > 0) {
            clutch = wheelRPM / gearRatios[gear] - rpm;
            if (wheelRPM <= 0) {
                clutch = 500 - rpm;  // rpm + clutch would be 500 (since clutch takes a rev difference that it smoothly applies)
                return;
            }
        }
    }
    int getGear() { return gear; }

    void setGas(float newGas) { gas = newGas; }
    float getGas() { return gas; }

    void setRPM(float newRPM) { rpm = newRPM; }  // Sets rpm for next tick
    float getRPM() { return rpmDamper.getAverage(); }

    void setWheelSpeed(float newSpeed) { wheelRPM = newSpeed; }  // Sets wheelRPM for next tick
    float getWheelSpeed() { return wheelSpeedDamper.getAverage(); }

    float getHorses() { return horses; }

   private:
    float gas = 0;       // Throttle body
    float rpm = 0;       // Engine RPM
    float wheelRPM = 0;  // Wheel RPM or speed does not really matter

    float idleValve = 1;  // Idle valve
    float horses = 0;     // Power produced immediately
    int gear = 0;         // Gearing

    float clutch = 0;  // Difference of revs to 'smoothly' join

    Damper rpmDamper{5};
    Damper wheelSpeedDamper{10};
};

#endif