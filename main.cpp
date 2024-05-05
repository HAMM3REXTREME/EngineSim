#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "include/Car.h"
#include "include/fmod_errors.h"
#include "include/fmod_studio.hpp"
#include "include/fmod_studio_common.h"

int main() {
    FMOD::Studio::System* system = nullptr;
    FMOD::Studio::Bank* masterBank = nullptr;
    FMOD::Studio::EventDescription* carSoundEventDescription = nullptr;
    FMOD::Studio::EventInstance* carSoundEventInstance = nullptr;

    FMOD_RESULT result = FMOD::Studio::System::create(&system);
    if (result != FMOD_OK) {
        std::cerr << "FMOD System creation failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }

    // Initialize the Audio system
    result = system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) {
        std::cerr << "FMOD System initialization failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }

    // Load the sound bank
    const char* bankName = "assets/Vehicles.bank";  // Replace with your sound bank path
    result = system->loadBankFile(bankName, FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
    if (result != FMOD_OK) {
        std::cerr << "Loading sound bank failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }

    // Load the sound bank
    const char* bankStrName = "assets/Vehicles.strings.bank";  // Replace with your sound bank path
    result = system->loadBankFile(bankStrName, FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
    if (result != FMOD_OK) {
        std::cerr << "Loading sound bank failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }

    // Get the Vehicle/Car Sound event
    const char* eventName = "event:/Vehicles/Car Engine";  // Replace with your event path
    result = system->getEvent(eventName, &carSoundEventDescription);
    if (result != FMOD_OK) {
        std::cerr << "Getting event description failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }

    // Create an instance of the event
    result = carSoundEventDescription->createInstance(&carSoundEventInstance);
    if (result != FMOD_OK) {
        std::cerr << "Creating event instance failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }

    // Start the event
    result = carSoundEventInstance->start();
    if (result != FMOD_OK) {
        std::cerr << "Starting event instance failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }

    Car car;
    car.running = true;

    // Car
    std::thread vroom(&Car::tick, std::ref(car));

    // Create a window
    sf::RenderWindow window(sf::VideoMode(1024, 768), "VROOM");
    sf::Clock clock;  // For FPS display

    // Text Information
    sf::Font font;
    if (!font.loadFromFile("assets/Race-Sport.ttf")) {
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
    if (!texture.loadFromFile("assets/race_tach.png")) {
        car.running = false;
        vroom.join();
        return EXIT_FAILURE;
    }
    sf::Sprite sprite(texture);
    sprite.setOrigin(sprite.getLocalBounds().width / 2.f, sprite.getLocalBounds().height / 2.f);
    sprite.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
    sprite.setScale(0.3f, 0.3f);

    // Tachometer needle
    sf::RectangleShape tach(sf::Vector2f(250.f, 6.f));  // Size of the tach
    tach.setFillColor(sf::Color::Red);                  // Color of the tach
    tach.setPosition(1024.f / 2.f, 768.f / 2.f);        // Position of the tach
    tach.setOrigin(250.f, 3.f);                         // Center of rotation

    // Speedometer meter
    sf::RectangleShape speedo(sf::Vector2f(150.f, 6.f));   // Size of the speedo
    speedo.setFillColor(sf::Color::Red);                   // Color of the speedo
    speedo.setPosition(1024.f / 2.f, 768.f / 2.f - 75.f);  // Position of the speedo
    speedo.setOrigin(150.f, 3.f);                          // Center of rotation

    // Map user keyboard input into differen levels of throttle
    std::map<sf::Keyboard::Key, int> userThrottleMap;
    userThrottleMap[sf::Keyboard::Key::Q] = 30;
    userThrottleMap[sf::Keyboard::Key::W] = 50;
    userThrottleMap[sf::Keyboard::Key::E] = 80;
    userThrottleMap[sf::Keyboard::Key::R] = 130;
    userThrottleMap[sf::Keyboard::Key::T] = 150;

    // Main loop
    while (window.isOpen()) {
        sf::Event event;

        sf::Time elapsed = clock.restart();
        float fps = 1.0f / elapsed.asSeconds();

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            // Key press events
            if (event.type == sf::Event::KeyPressed) {
                // Accelerator options
                auto it = userThrottleMap.find(event.key.code);
                if (it != userThrottleMap.end()) {
                    std::cout << "Accelerator at " << it->second << " \n";
                    car.setGas(it->second);
                }
                // Upshift
                if (event.key.code == sf::Keyboard::Key::Up) {
                    std::cout << "Shifted Up\n";
                    car.setGear(car.getGear() + 1);
                }
                // Downshift
                if (event.key.code == sf::Keyboard::Key::Down) {
                    std::cout << "Shifted Down\n";
                    car.setGear(car.getGear() - 1);
                }
                // Shift to N
                if (event.key.code == sf::Keyboard::Key::LShift) {
                    std::cout << "To neutral\n";
                    car.setGear(0);
                }
                // Brakes
                if (event.key.code == sf::Keyboard::Key::Period) {
                    std::cout << "Brakes on\n";
                    car.brakeFactor = 0.996;
                }
                // Crude Starter
                if (event.key.code == sf::Keyboard::Key::S) {
                    // One strong starter
                    car.setRPM(500);
                    std::cout << "Starter\n";
                }
                if (event.key.code == sf::Keyboard::Key::A) {
                    car.ignition = !car.ignition;
                    std::cout << "Set ignition to " << car.ignition << "\n";
                }
            }
            // Key release events
            if (event.type == sf::Event::KeyReleased) {
                // If one of the keys in our throttle map is released, release the throttle
                auto it = userThrottleMap.find(event.key.code);
                if (it != userThrottleMap.end()) {
                    std::cout << "Accelerator released\n";
                    car.setGas(0);
                }
                // Disengage brakes
                if (event.key.code == sf::Keyboard::Key::Period) {
                    std::cout << "Brakes off\n";
                    car.brakeFactor = 1;
                }
            }
        }

        // Move needles
        tach.setRotation(car.getRPM() / 30 - 90);
        speedo.setRotation(car.getWheelSpeed() / 100);
        // Set gauge display
        gaugeValue.setString(std::to_string((int)car.getRPM()) + " RPM\n" + std::to_string((int)car.getHorses()) + " Horses\n" + std::to_string(car.getGear()) + "\n" + std::to_string((int)car.getWheelSpeed() / 100) + " kmh\n" + std::to_string((int)fps) +
                             " FPS");

        // Set the fmod RPM parameter
        result = carSoundEventInstance->setParameterByName("RPM", car.getRPM());
        if (result != FMOD_OK) {
            std::cerr << "Setting RPM parameter failed: " << FMOD_ErrorString(result) << std::endl;
            return -1;
        }
        result = carSoundEventInstance->setParameterByName("Load", car.getGas()/80);
        if (result != FMOD_OK) {
            std::cerr << "Setting Load parameter failed: " << FMOD_ErrorString(result) << std::endl;
            return -1;
        }
        system->update();

        window.clear(sf::Color::Black);

        window.draw(sprite);
        window.draw(speedo);
        window.draw(tach);
        window.draw(gaugeValue);

        window.display();

        // TODO: Add a proper fps limiter
        sf::Time sleepTime = sf::seconds(0.01);
        sf::sleep(sleepTime);
    }

    // FMOD exit
    result = carSoundEventInstance->release();
    if (result != FMOD_OK) {
        std::cerr << "Releasing event instance failed: " << FMOD_ErrorString(result) << std::endl;
    }

    result = system->release();
    if (result != FMOD_OK) {
        std::cerr << "Releasing FMOD System failed: " << FMOD_ErrorString(result) << std::endl;
    }

    car.running = false;
    vroom.join();

    return 0;
}