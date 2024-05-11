// Simple car simulator - Not realistic at all, only 'feels' real.

#include "Car.h"

#include <mutex>
#include <queue>

// Averages float values
void Damper::addValue(double value) {
    values.push(value);
    if (values.size() > maxSize) {
        values.pop();
    }
}

double Damper::getAverage() {
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

void Car::tick() {
    std::lock_guard<std::mutex> lock(m_tick);
    controlIdle();
    rpm = rpm * gearLazyValues[gear];  // Apply engine internal drag
    addEnergy();

    // Apply clutch revs in multiple smaller chunks of revs, makes a kick, like dumping the clutch in a real car
    rpm += clutch * clutchKick;
    clutch = (1.0 - clutchKick) * clutch;

    // Set Wheel RPM depending on the engine rpm, current gear ratio and coasting drag
    if (gear >= 1) {
        wheelRPM = rpm * gearRatios[gear] * coastLazyValue * brakeFactor;
        rpm = rpm * brakeFactor * coastLazyValue;
    } else {
        // Just apply the rolling and brake resistance if in neutral
        wheelRPM = wheelRPM * coastLazyValue * brakeFactor;
    }

    rpmDamper.addValue(rpm);
    wheelSpeedDamper.addValue(wheelRPM);
}

void Car::setGear(int newGear) {
    gear = newGear;
    if (gear > 0) {
        clutch = wheelRPM / gearRatios[gear] - rpm;
        if (wheelRPM <= 0 && rpm >= 700) {  // 'Dumping' the clutch won't stall
            clutch = 500 - rpm;             // rpm + clutch would be 500 (since clutch takes a rev difference that it 'smoothly' applies)
            return;
        }
    }
}

void Car::controlIdle() {
    if (rpm >= 800) {  // Idle air control valve
        idleValve = 5;
    } else if (rpm <= 700) {
        idleValve = 10;
    }
}

void Car::addEnergy() {
    if (rpm > 50) {
        rpm += horses / rpm;  // Don't divide by zero
        if (ignition) {
            if (rpm <= revLimit) {  // Rev limiter thingy
                if (revLimitTick <= 0) {
                    horses = rpm * (gas + idleValve) * gearThrottleResponses[gear] * brakeFactor;
                } else {
                    revLimitTick--;
                }
            } else {
                revLimitTick = defaultRevLimitTick;
                horses = 0;
            }

        } else {
            horses = 0;
        }
    }
}

int Car::getGear() { return gear; }

void Car::setGas(float newGas) { gas = newGas; }
float Car::getGas() { return gas; }

void Car::setRPM(float newRPM) { rpm = newRPM; }  // Sets rpm for next tick
float Car::getRPM() { return rpmDamper.getAverage(); }

void Car::setWheelSpeed(float newSpeed) { wheelRPM = newSpeed; }  // Sets wheelRPM for next tick
float Car::getWheelSpeed() { return wheelSpeedDamper.getAverage(); }

float Car::getHorses() { return horses; }
