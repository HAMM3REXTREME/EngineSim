#ifndef CAR_H
#define CAR_H

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>


struct Car {
  bool running; // Is car running

  float rpm = 0; // Engine RPM
  float gas = 0; // Throttle body
  float idleValve = 1; // Idle valve

  int gear = 0; // Gearing
  float gearRatios[7] = {0,0.5, 0.7 , 0.9, 1.1, 1.4 , 1.6};
  float gearLazyValues[7] = {0.99, 0.999, 0.9995, 0.9996, 0.9997, 0.9998, 0.9999};
  float gearThrottleResponses[7] = {1, 0.1 , 0.06, 0.05, 0.04, 0.035, 0.03};
  
  
  float horses = 0; // Power produced immediately

  double lazyValue = 0.99; // Engine time for revs to settle back - values closer to one need more time to go back to idle
  double throttleResponse = 1; // Throttle sensitivity - Should feel lower in higher gears since we reduce the value after every upshift bruh
  float clutch = 0; // Difference of revs to smoothly join
  float clutchGranularity = 0.5;


  float wheelRPM = 0; // Wheel RPM or speed does not really matter

  double coastLazyValue = 0.999; // Driving drag on wheels (and also engine if in gear)
  float brakeFactor = 1; // Drag on wheels (and also engine if in gear) - Basically the same as coastLazyValue

  std::mutex m_tick;
  void tick() {
    while (running) {
      std::lock_guard<std::mutex> lock(m_tick);
      if (rpm >= 800) { // Idle air control valve
        idleValve = 1;
      } else if (rpm <= 700) {
        idleValve = 30;
      }
      rpm = rpm * lazyValue;
      if (rpm > 0) {
        rpm += horses / rpm;
        if (rpm <= 8000) { // Rev limiter thingy
          horses = rpm * (gas + idleValve) * throttleResponse * brakeFactor;
        } else {
          horses = 0;
        }
      }

      // Apply clutch revs in multiple smaller chunks of revs
      rpm += clutch * clutchGranularity;
      clutch = (1.0-clutchGranularity) * clutch;

      // Wheel RPM depending on the engine rpm , current gear ratio and coasting drag
      if (gear >= 1) {
        wheelRPM = rpm * gearRatios[gear] * coastLazyValue * brakeFactor;
        rpm = rpm * brakeFactor;
      } else {
        wheelRPM = wheelRPM * coastLazyValue * brakeFactor;
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(10)); // Farts per second
    }
  }
  void setGear(int newGear) {
    gear = newGear;
    if (gear > 0) {
      clutch = wheelRPM / gearRatios[gear] - rpm;
      if (wheelRPM <= 0) {
        clutch = 500 - rpm; // rpm + clutch would be 500 (since clutch takes a rev difference that it smoothly applies)
        return;
      }
    }
  }
};

#endif