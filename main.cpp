#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Car {
public:
  bool running;
  float rpm = 0;
  float gas = 0;
  int gear = 0;
  float idleValve = 1;
  float horses = 0;
  double lazyValue = 0.99;
  double throttleResponse = 1;
  double coastLazyValue = 0.999999999;
  float clutch = 0;
  float wheelRPM = 0;
  float brakeFactor = 1;
  std::mutex m_tick;
  void tick() {
    while (running) {
      std::lock_guard<std::mutex> lock(m_tick);
      if (rpm >= 800) { // Idle air control valve
        idleValve = 1;
      } else if (rpm <= 700) {
        idleValve = 30;
      }
      rpm = rpm * lazyValue; // Drag
      if (rpm > 0) {
        rpm += horses / rpm;
        if (rpm <= 8000) {
          horses = rpm * (gas + idleValve) * throttleResponse * brakeFactor;
        } else {
          horses = 0;
        }
      }
      rpm += clutch * 0.1;
      clutch = 0.9 * clutch;
      if (gear >= 1) {
        wheelRPM = rpm * gear * 0.6 * brakeFactor;
        rpm = rpm * brakeFactor;
      } else {
        wheelRPM = wheelRPM * coastLazyValue * brakeFactor;
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(10)); // Adjust sleep time as needed
    }
  }
  void setGear(int newGear) {
    gear = newGear;

    if (newGear > 0) {
      clutch = wheelRPM / (newGear * 0.6) - rpm;
      if (wheelRPM <= 0) {
        clutch = 500 - rpm;
        return;
      }
    }
  }
  void setGas(float userGas){
    gas = userGas;
  }
};

int main() {
  Car car;
  car.running = true;
  std::thread vroom(&Car::tick, std::ref(car));

  // Create a window
  sf::RenderWindow window(sf::VideoMode(1024, 768), "VROOM");

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

  sf::RectangleShape tach(sf::Vector2f(250.f, 6.f)); // Size of the tach
  tach.setFillColor(sf::Color::Red);                 // Color of the tach
  tach.setPosition(1024.f / 2.f, 768.f / 2.f);       // Position of the tach
  tach.setOrigin(250.f, 3.f);                        // Center of rotation

  sf::RectangleShape speed(sf::Vector2f(150.f, 6.f));  // Size of the tach
  speed.setFillColor(sf::Color::Red);                  // Color of the tach
  speed.setPosition(1024.f / 2.f, 768.f / 2.f - 75.f); // Position of the tach
  speed.setOrigin(150.f, 3.f);                         // Center of rotation

  std::map<sf::Keyboard::Key, int> userThrottleMap;
  userThrottleMap[sf::Keyboard::Key::Q] = 50;
  userThrottleMap[sf::Keyboard::Key::W] = 100;
  userThrottleMap[sf::Keyboard::Key::E] = 150;
  userThrottleMap[sf::Keyboard::Key::R] = 215;
  userThrottleMap[sf::Keyboard::Key::T] = 300;

  std::vector<std::pair<float, float>> gearMap(6);
  int currentGear = 0;
  gearMap[0] = {0.99, 1};
  gearMap[1] = {0.999, 0.1};
  gearMap[2] = {0.9995, 0.06};
  gearMap[3] = {0.9996, 0.05};
  gearMap[4] = {0.9997, 0.04};

  // Main loop - keep the window open until it's closed
  while (window.isOpen()) {
    // Process events
    sf::Event event;

    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {

        // Close the window if the close button is clicked
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
          car.lazyValue = gearMap[car.gear].first;
          car.throttleResponse = gearMap[car.gear].second;
        }
        if (event.key.code == sf::Keyboard::Key::Down) {
          std::cout << "Shifted Down\n";
          car.setGear(car.gear - 1);
          car.lazyValue = gearMap[car.gear].first;
          car.throttleResponse = gearMap[car.gear].second;
        }
        if (event.key.code == sf::Keyboard::Key::LShift) {
          std::cout << "To neutral\n";
          car.setGear(0);
          car.lazyValue = gearMap[car.gear].first;
          car.throttleResponse = gearMap[car.gear].second;
        }
        if (event.key.code == sf::Keyboard::Key::Period) {
          std::cout << "Brakes on\n";
          car.brakeFactor = 0.98;
        }

        if (event.key.code == sf::Keyboard::Key::S) {
          car.rpm = 500;
          std::cout << "Starter\n";
        }
      }

      // Key release events
      if (event.type == sf::Event::KeyReleased) {
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

    float angle = car.rpm / 30 - 90; // Set the desired angle in degrees
    gaugeValue.setString(
        "RPM: " + std::to_string(car.rpm) + "    Current power output: " +
        std::to_string(car.horses) + "    Gear: " + std::to_string(car.gear) +
        "    Wheel RPM: " + std::to_string(car.wheelRPM));
    tach.setRotation(angle);
    speed.setRotation(car.wheelRPM / 100);

    // Clear the window with a black background
    window.clear(sf::Color::Black);

    // Draw objects, shapes, etc. here
    window.draw(sprite);
    window.draw(speed);
    window.draw(tach);
    window.draw(gaugeValue);

    // Display the contents of the window
    window.display();
  }

  car.running = false;
  vroom.join();
  return 0;
}