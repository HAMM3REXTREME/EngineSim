// Engine Simulator - Simplified model
#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <atomic>
#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "Car.h"
#include "fmod/fmod_errors.h"
#include "fmod/fmod_studio.hpp"
#include "fmod/fmod_studio_common.h"


// Function to convert string GUID to FMOD_GUID
FMOD_GUID StringToGUID(const char* guidString) {
    FMOD_GUID guid;
    unsigned long p0;
    unsigned int p1, p2;
    unsigned int p3[8];

    sscanf(guidString, "%8lx-%4x-%4x-%2x%2x-%2x%2x%2x%2x%2x%2x", &p0, &p1, &p2,
           &p3[0], &p3[1], &p3[2], &p3[3], &p3[4], &p3[5], &p3[6], &p3[7]);

    guid.Data1 = p0;
    guid.Data2 = p1;
    guid.Data3 = p2;
    for (int i = 0; i < 8; i++) {
        guid.Data4[i] = (unsigned char)p3[i];
    }

    return guid;
}


void manageCar(Car* car, std::atomic<bool>* run) {
    while (*run) {
        car->tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void carStarter(Car* car, bool* m_isStarting) {
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    std::cout << "Vroom!\n";
    car->setRPM(800);
    car->setGas(150);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    car->setGas(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    *m_isStarting = false;
}

void upShift(Car* car){
    int newGear = car->getGear() + 1;
    car->setGear(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    car->setGear(newGear);
}
void downShift(Car* car){
    int newGear = car->getGear() - 1;
    car->setGear(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    car->setGear(newGear);
}

int main() {
    FMOD::Studio::System* audioSystem = nullptr;
    FMOD::Studio::Bank* masterBank = nullptr;
    FMOD::Studio::Bank* masterBankStr = nullptr;
    FMOD::Studio::EventDescription* carSoundEventDescription = nullptr;
    FMOD::Studio::EventInstance* carSoundEventInstance = nullptr;
    FMOD::Studio::EventDescription* starterSoundEventDescription = nullptr;
    FMOD::Studio::EventInstance* starterSoundEventInstance = nullptr;

    FMOD_RESULT result = FMOD::Studio::System::create(&audioSystem);
    if (result != FMOD_OK) {
        std::cerr << "FMOD System creation failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }
    // Initialize the Audio system
    result = audioSystem->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) {
        std::cerr << "FMOD System initialization failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }
    // Load the sound bank
    const char* bankName = "Assets/build/soundbank/Vehicles.bank";
    result = audioSystem->loadBankFile(bankName, FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
    if (result != FMOD_OK) {
        std::cerr << "Loading sound bank failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }

    // Load the sound bank - strings
    const char* bankStrName = "Assets/build/soundbank/Vehicles.strings.bank";
    result = audioSystem->loadBankFile(bankStrName, FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBankStr);
    if (result != FMOD_OK) {
        std::cerr << "Loading sound bank failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }

    // Get the Vehicle/Car Sound event
    const char* eventName = "event:/cars/tatuusfa1/engine_ext";
    result = audioSystem->getEvent(eventName, &carSoundEventDescription);
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
    const char* starterEventName = "event:/cars/tatuusfa1/engine_ext";
    result = audioSystem->getEvent(starterEventName, &starterSoundEventDescription);
    if (result != FMOD_OK) {
        std::cerr << "Getting event description failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }
    result = starterSoundEventDescription->createInstance(&starterSoundEventInstance);
    if (result != FMOD_OK) {
        std::cerr << "Creating event instance failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }
    result = starterSoundEventDescription->createInstance(&starterSoundEventInstance);
    if (result != FMOD_OK) {
        std::cerr << "Creating event instance failed: " << FMOD_ErrorString(result) << std::endl;
        return -1;
    }

    // Create a window
    sf::RenderWindow window(sf::VideoMode(1024, 768), "Car Simulator");
    sf::Clock clock;  // For FPS display

    // Text Information
    sf::Font font;
    if (!font.loadFromFile("Assets/build/fonts/Race-Sport.ttf")) {
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
    if (!texture.loadFromFile("Assets/build/textures/race_tach.png")) {
        std::cerr << "Failed to load tachometer" << std::endl;
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

    // Speedometer needle
    sf::RectangleShape speedo(sf::Vector2f(150.f, 6.f));   // Size of the speedo
    speedo.setFillColor(sf::Color::Red);                   // Color of the speedo
    speedo.setPosition(1024.f / 2.f, 768.f / 2.f - 75.f);  // Position of the speedo
    speedo.setOrigin(150.f, 3.f);                          // Center of rotation

    // Map user keyboard input to differen levels of throttle
    std::map<sf::Keyboard::Key, int> userThrottleMap;
    userThrottleMap[sf::Keyboard::Key::Q] = 30;
    userThrottleMap[sf::Keyboard::Key::W] = 50;
    userThrottleMap[sf::Keyboard::Key::E] = 80;
    userThrottleMap[sf::Keyboard::Key::R] = 130;
    userThrottleMap[sf::Keyboard::Key::T] = 150;

    // Keyboard to gears
    std::map<sf::Keyboard::Key, int> userGearShifter;
    userGearShifter[sf::Keyboard::Key::Num0] = 0;
    userGearShifter[sf::Keyboard::Key::Num1] = 1;
    userGearShifter[sf::Keyboard::Key::Num2] = 2;
    userGearShifter[sf::Keyboard::Key::Num3] = 3;
    userGearShifter[sf::Keyboard::Key::Num4] = 4;
    userGearShifter[sf::Keyboard::Key::Num5] = 5;
    userGearShifter[sf::Keyboard::Key::Num6] = 6;
    // Numpad H-pattern
    userGearShifter[sf::Keyboard::Key::Numpad5] = 0;
    userGearShifter[sf::Keyboard::Key::Numpad7] = 1;
    userGearShifter[sf::Keyboard::Key::Numpad1] = 2;
    userGearShifter[sf::Keyboard::Key::Numpad8] = 3;
    userGearShifter[sf::Keyboard::Key::Numpad2] = 4;
    userGearShifter[sf::Keyboard::Key::Numpad9] = 5;
    userGearShifter[sf::Keyboard::Key::Numpad3] = 6;

    Car car;
    std::atomic<bool> carRunning = true;
    std::thread carThread{manageCar, &car, &carRunning};

    bool isStarting = false;  // Starter sequence thingy
    bool fakeClutched = false;
    int fakeGear = car.getGear();

    // Main loop
    while (window.isOpen()) {
        sf::Event event;

        sf::Time elapsed = clock.restart();
        float fps = 1.0f / elapsed.asSeconds();

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::Resized) {
                // update the view to the new size of the window
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
            }
            // Key press events
            if (event.type == sf::Event::KeyPressed) {
                // Accelerator options
                auto it = userThrottleMap.find(event.key.code);
                if (it != userThrottleMap.end()) {
                    std::cout << "Accelerator at " << it->second << " \n";
                    car.setGas(it->second);
                }
                // Gear shifter
                auto gearIt = userGearShifter.find(event.key.code);
                if (gearIt != userGearShifter.end()) {
                    std::cout << "Shifted to " << gearIt->second << " \n";
                    fakeGear = gearIt->second;
                    fakeClutched ? car.setGear(0) : car.setGear(gearIt->second);
                }
                // Shift to N
                if (event.key.code == sf::Keyboard::Key::LShift) {
                    std::cout << "Clutch in\n";
                    fakeClutched = true;
                    car.setGear(0);
                }
                if (event.key.code == sf::Keyboard::Key::Up) {
                    std::cout << "Sequential upshift\n";
                    fakeGear++;
                    fakeClutched ? car.setGear(0) : car.setGear(fakeGear);
                }
                if (event.key.code == sf::Keyboard::Key::Down) {
                    std::cout << "Sequential downshift\n";
                    fakeGear--;
                    fakeClutched ? car.setGear(0) : car.setGear(fakeGear);
                }
                // Brakes
                if (event.key.code == sf::Keyboard::Key::Numpad0 || event.key.code == sf::Keyboard::Key::Period) {
                    std::cout << "Brakes on\n";
                    car.linearWheelDrag = 10;
                }
                if (event.key.code == sf::Keyboard::Key::S) {
                    // Only allow starting if not already in the process of being started, otherwise we can attempt to start.
                    if (!isStarting) {
                        result = starterSoundEventInstance->start();
                        if (result != FMOD_OK) {
                            std::cerr << "Starting event instance failed: " << FMOD_ErrorString(result) << std::endl;
                            return -1;
                        }
                        // Push to start.
                        std::cout << "Starting car...\n";
                        std::thread starterThread{carStarter, &car, &isStarting};
                        starterThread.detach();
                    }
                    isStarting = true;
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
                if (event.key.code == sf::Keyboard::Key::Numpad0 || event.key.code == sf::Keyboard::Key::Period) {
                    std::cout << "Brakes released\n";
                    car.linearWheelDrag = 0;
                }
                if (event.key.code == sf::Keyboard::Key::LShift) {
                    std::cout << "Clutch out\n";
                    fakeClutched = false;
                    car.setGear(fakeGear);
                }
            }
            if (event.type == sf::Event::JoystickButtonPressed) {
                // std::cout << "Pressed controller: " << event.joystickButton.button <<"\n";
                if (event.joystickButton.button == 4) {
                    std::cout << "Sequential upshift\n";
                    fakeGear++;
                    fakeClutched ? car.setGear(0) : car.setGear(fakeGear);
                }
                if (event.joystickButton.button == 5) {
                    std::cout << "Sequential downshift\n";
                    fakeGear--;
                    fakeClutched ? car.setGear(0) : car.setGear(fakeGear);
                }
                if (event.joystickButton.button == 999) {
                    std::cout << "Clutch in\n";
                    fakeClutched = true;
                    car.setGear(0);
                }
            }

            if (event.type == sf::Event::JoystickButtonReleased) {
                if (event.joystickButton.button == 999) {
                    std::cout << "Clutch out\n";
                    fakeClutched = false;
                    car.setGear(fakeGear);
                }
            }
            if (event.type == sf::Event::JoystickMoved) {
                // std::cout << "new position on axis " << event.joystickMove.axis << " = " << event.joystickMove.position <<  "\n";
                if (event.joystickMove.axis == 2) {
                    std::cout << "Accelerator at " << -event.joystickMove.position + 100 << " \n";
                    car.setGas(-event.joystickMove.position + 100);
                }
                if (event.joystickMove.axis == 3) {
                    double brakeIntensity = (-event.joystickMove.position + 100)/10;
                    std::cout << "Brake at intensity: " << brakeIntensity << " \n";
                    car.linearWheelDrag = brakeIntensity;
                }
                if (event.joystickMove.axis == 1) {
                    if (event.joystickMove.position <= 0){
                                            std::cout << "Clutch in\n";
                    fakeClutched = true;
                    car.setGear(0);
                    } else {
                                            std::cout << "Clutch out\n";
                    fakeClutched = false;
                    car.setGear(fakeGear);
                    }
                }
            }
        }

        // Move needles
        tach.setRotation(car.getRPM() / 30 - 90);
        // Wheel speed, rpm whatever
        speedo.setRotation(car.getWheelSpeed() / 100);
        // Set gauge display
        gaugeValue.setString(std::to_string((int)car.getRPM()) + " RPM\n" + std::to_string(fakeGear) + "\n" + std::to_string((int)car.getWheelSpeed() / 100) + " kmh\n" + std::to_string((int)fps) + " FPS");

        // Set the fmod RPM parameter
        result = carSoundEventInstance->setParameterByName("rpms", car.getRPM()*1.5);
        if (result != FMOD_OK) {
            std::cerr << "Setting RPM parameter failed: " << FMOD_ErrorString(result) << std::endl;
            return -1;
        }
        result = carSoundEventInstance->setParameterByName("throttle", car.revLimitTick > 0 ? 0 : car.getGas() / 180);
        if (result != FMOD_OK) {
            std::cerr << "Setting Load parameter failed: " << FMOD_ErrorString(result) << std::endl;
            return -1;
        }
        audioSystem->update();

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

    result = audioSystem->release();
    if (result != FMOD_OK) {
        std::cerr << "Releasing FMOD System failed: " << FMOD_ErrorString(result) << std::endl;
    }

    // Car thread exit
    carRunning = false;
    carThread.join();

    return 0;
}