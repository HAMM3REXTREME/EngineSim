#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <cmath>
#include "Car.h"


int main() {
  Car car;
  car.running = true;

  // Car
  std::thread vroom(&Car::tick, std::ref(car));

  // Create a window
  sf::RenderWindow window(sf::VideoMode(1024, 768), "VROOM");

  // Guage information
  sf::Font font;
  if (!font.loadFromFile("Race-Sport.ttf")) {
    car.running = false;
    vroom.join();
    std::cerr << "Failed to load font" << std::endl;
    return EXIT_FAILURE;
  }
  sf::Text gaugeValue;
  gaugeValue.setFont(font);
  gaugeValue.setCharacterSize(16);
  gaugeValue.setFillColor(sf::Color::White);
  gaugeValue.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
  

  // Tachometer graphics
  sf::Texture texture;
  if (!texture.loadFromFile("race_tach.png")) {
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
          car.brakeFactor = 0.996;
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
        std::to_string((int)car.rpm) + " RPM\n" +
        std::to_string((int)car.horses) + " Horses\n" + 
        std::to_string(car.gear) + "\n" + 
        std::to_string((int)car.wheelRPM/100) + " kmh");


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