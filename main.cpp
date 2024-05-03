#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct Car {
  bool running; // Is car running

  float rpm = 0; // Engine RPM
  float gas = 0; // Throttle body
  float idleValve = 1; // Idle valve

  int gear = 0; // Gearing
  float gearRatios[7] = {0,0.5, 0.7 , 0.9, 1.1, 1.4 , 1.6};
  float gearLazyValues[7] = {0.99, 0.999, 0.9995, 0.9996, 0.9997, 0.9998, 0.999};
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
        wheelRPM = rpm * gear * 0.6 * coastLazyValue * brakeFactor;
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
    if (newGear > 0) {
      clutch = wheelRPM / (newGear * 0.6) - rpm;
      if (wheelRPM <= 0) {
        clutch = 500 - rpm; // rpm + clutch would be 500 (since clutch takes a rev difference that it smoothly applies)
        return;
      }
    }
  }
};

int main() {
  Car car;
  car.running = true;

  // Car
  std::thread vroom(&Car::tick, std::ref(car));

  // Create a window
  sf::RenderWindow window(sf::VideoMode(1024, 768), "VROOM");

  // Guage information
  sf::Font font;
  if (!font.loadFromFile("Droid.ttf")) {
    car.running = false;
    vroom.join();
    std::cerr << "Failed to load font" << std::endl;
    return EXIT_FAILURE;
  }
  sf::Text gaugeValue;
  gaugeValue.setFont(font);
  gaugeValue.setCharacterSize(24);
  gaugeValue.setFillColor(sf::Color::White);
  gaugeValue.setPosition(10, 10);

  // Tachometer graphics
  sf::Texture texture;
  if (!texture.loadFromFile("lfa.png")) {
    car.running = false;
    vroom.join();
    return EXIT_FAILURE;
  }
  sf::Sprite sprite(texture);
  sprite.setOrigin(sprite.getLocalBounds().width / 2.f,
                   sprite.getLocalBounds().height / 2.f);
  sprite.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
  sprite.setScale(0.3f, 0.3f);

  // Tachometer needle
  sf::RectangleShape tach(sf::Vector2f(250.f, 6.f)); // Size of the tach
  tach.setFillColor(sf::Color::Red);                 // Color of the tach
  tach.setPosition(1024.f / 2.f, 768.f / 2.f);       // Position of the tach
  tach.setOrigin(250.f, 3.f);                        // Center of rotation

  // Speedometer meter
  sf::RectangleShape speedo(sf::Vector2f(150.f, 6.f));  // Size of the speedo
  speedo.setFillColor(sf::Color::Red);                  // Color of the speedo
  speedo.setPosition(1024.f / 2.f, 768.f / 2.f - 75.f); // Position of the speedo
  speedo.setOrigin(150.f, 3.f);                         // Center of rotation

  // Map user keyboard input into differen levels of throttle 
  std::map<sf::Keyboard::Key, int> userThrottleMap;
  userThrottleMap[sf::Keyboard::Key::Q] = 50;
  userThrottleMap[sf::Keyboard::Key::W] = 100;
  userThrottleMap[sf::Keyboard::Key::E] = 150;
  userThrottleMap[sf::Keyboard::Key::R] = 215;
  userThrottleMap[sf::Keyboard::Key::T] = 300;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
      // Key press events
      if (event.type == sf::Event::KeyPressed) {
        auto it = userThrottleMap.find(event.key.code);
        if (it != userThrottleMap.end()) {
          std::cout << "Accelerator at " << it->second << " \n";
          car.gas = it->second;
        }
        if (event.key.code == sf::Keyboard::Key::Up) {
          std::cout << "Shifted Up\n";
          car.setGear(car.gear + 1);
          car.lazyValue = car.gearLazyValues[car.gear];
          car.throttleResponse = car.gearThrottleResponses[car.gear];
        }
        if (event.key.code == sf::Keyboard::Key::Down) {
          std::cout << "Shifted Down\n";
          car.setGear(car.gear - 1);
          car.lazyValue = car.gearLazyValues[car.gear];
          car.throttleResponse = car.gearThrottleResponses[car.gear];
        }
        if (event.key.code == sf::Keyboard::Key::LShift) {
          std::cout << "To neutral\n";
          car.setGear(0);
          car.lazyValue = car.gearLazyValues[car.gear];
          car.throttleResponse = car.gearThrottleResponses[car.gear];
        }
        if (event.key.code == sf::Keyboard::Key::Period) {
          std::cout << "Brakes on\n";
          car.brakeFactor = 0.99;
        }

        if (event.key.code == sf::Keyboard::Key::S) {
          // One strong starter
          car.rpm = 500;
          std::cout << "Starter\n";
        }
      }
      // Key release events
      if (event.type == sf::Event::KeyReleased) {
        // If one of the keys in out throttle map is released, release the throttle
        auto it = userThrottleMap.find(event.key.code);
        if (it != userThrottleMap.end()) {
          std::cout << "Accelerator released\n";
          car.gas = 0;
        }
        if (event.key.code == sf::Keyboard::Key::Period) {
          std::cout << "Brakes off\n";
          car.brakeFactor = 1;
        }
      }
    }

    tach.setRotation(car.rpm / 30 - 90);
    speedo.setRotation(car.wheelRPM / 100);

    gaugeValue.setString(
        "RPM: " + std::to_string(car.rpm) + "    Current power output: " +
        std::to_string(car.horses) + "    Gear: " + std::to_string(car.gear) +
        "    Wheel RPM: " + std::to_string(car.wheelRPM));


    window.clear(sf::Color::Black);

    window.draw(sprite);
    window.draw(speedo);
    window.draw(tach);
    window.draw(gaugeValue);

    window.display();
  }

  car.running = false;
  vroom.join();
  return 0;
}