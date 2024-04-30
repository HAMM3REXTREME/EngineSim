#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <thread>
#include <chrono>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>

class Motor{
    public:
        bool running;
        int rpm = 0;
        int gas = 0;
        int idleValve = 0;
        int horses = 0;
        std::mutex m_tick;
        void tick(){
            while (running) {
            std::lock_guard<std::mutex> lock(m_tick);
            if (rpm >= 700){ // Idle air control valve
                idleValve = 5;
            } else if (rpm <= 600) {
                idleValve = 15;
            }
            rpm = rpm * 0.98;
            if (rpm <= 8500){ // Rev limiter
            rpm += gas + idleValve;
            }
            horses = rpm * (gas + idleValve);
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Adjust sleep time as needed
               }
        }

};





int main(){
    Motor motor;
    motor.running = true;
    std::thread vroom(&Motor::tick, std::ref(motor));

   // Create a window
    sf::RenderWindow window(sf::VideoMode(800, 600), "VROOM");

    sf::Font font;
    if (!font.loadFromFile("Droid.ttf")) {
        motor.running = false;
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
    if (!texture.loadFromFile("tach.png")) {
        motor.running = false;
        vroom.join();
        return EXIT_FAILURE;
    }
    sf::Sprite sprite(texture);
    sprite.setOrigin(sprite.getLocalBounds().width / 2.f, sprite.getLocalBounds().height / 2.f);
    sprite.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 100.f);
    sprite.setScale(0.3f, 0.3f);

    sf::RectangleShape tach(sf::Vector2f(250.f, 6.f)); // Size of the tach
    tach.setFillColor(sf::Color::Red); // Color of the tach
    tach.setPosition(400.f, 300.f); // Position of the tach
    tach.setOrigin(250.f, 3.f); // Center of rotation    

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
            std::map<sf::Keyboard::Key, int> userThrottleMap;
            userThrottleMap[sf::Keyboard::Key::Q] = 50;
            userThrottleMap[sf::Keyboard::Key::W] = 100;
            userThrottleMap[sf::Keyboard::Key::E] = 150;
            userThrottleMap[sf::Keyboard::Key::R] = 215;
            userThrottleMap[sf::Keyboard::Key::T] = 300;

            if (event.type == sf::Event::KeyPressed) {
                auto it = userThrottleMap.find(event.key.code);
                 if (it != userThrottleMap.end()) {
                   std::cout << "Accelerator at " << it->second << " \n";
                  motor.gas = it->second;
             }
            if (event.key.code == sf::Keyboard::Key::Up){
                    std::cout << "Shifted up\n";
                }
                if (event.key.code == sf::Keyboard::Key::Down){
                    std::cout << "Shifted down\n";
                }

            }

            // Key release events
            if (event.type == sf::Event::KeyReleased) {
                 auto it = userThrottleMap.find(event.key.code);
                 if (it != userThrottleMap.end()) {
                    std::cout << "Accelerator released\n";
                    motor.gas = 0;
                 }
            }
            


        }

        
        float angle = motor.rpm/50; // Set the desired angle in degrees
        gaugeValue.setString("RPM: " + std::to_string(motor.rpm) + " Power: " + std::to_string(motor.horses));
        tach.setRotation(angle);

        // Clear the window with a black background
        window.clear(sf::Color::Black);

        // Draw objects, shapes, etc. here
         window.draw(sprite);
                window.draw(tach);
                window.draw(gaugeValue);


        // Display the contents of the window
        window.display();
    }


motor.running = false;
 vroom.join();
    return 0; 
}