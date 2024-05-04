#include "include/fmod_studio.hpp"
#include "include/fmod_errors.h"
#include <iostream>
#include <vector>

#include <chrono>
#include <thread>




int main(){

// Initialize FMOD Studio system
FMOD::Studio::System* system = nullptr;
FMOD::Studio::Bank* masterBank = nullptr;
FMOD::Studio::EventDescription* carSoundEventDescription = nullptr;
FMOD::Studio::EventInstance* carSoundEventInstance = nullptr;

FMOD_RESULT result = FMOD::Studio::System::create(&system);
if (result != FMOD_OK) {
    std::cerr << "FMOD System creation failed: " << FMOD_ErrorString(result) << std::endl;
    return -1;
}

// Initialize the system
result = system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
if (result != FMOD_OK) {
    std::cerr << "FMOD System initialization failed: " << FMOD_ErrorString(result) << std::endl;
    return -1;
}

// Load the sound bank
const char* bankName = "assets/Vehicles.bank"; // Replace with your sound bank path
result = system->loadBankFile(bankName, FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
if (result != FMOD_OK) {
    std::cerr << "Loading sound bank failed: " << FMOD_ErrorString(result) << std::endl;
    return -1;
}

// Load the sound bank
const char* bankStrName = "assets/Vehicles.strings.bank"; // Replace with your sound bank path
result = system->loadBankFile(bankStrName, FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
if (result != FMOD_OK) {
    std::cerr << "Loading sound bank failed: " << FMOD_ErrorString(result) << std::endl;
    return -1;
}


// Get the Vehicle/Car Sound event
const char* eventName = "event:/V8"; // Replace with your event path
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

// Set the RPM parameter
float rpmValue = 5000.0f; // RPM value
result = carSoundEventInstance->setParameterByName("RPM", rpmValue);
if (result != FMOD_OK) {
    std::cerr << "Setting RPM parameter failed: " << FMOD_ErrorString(result) << std::endl;
    return -1;
}

// Update the system
system->update();


std::this_thread::sleep_for(std::chrono::seconds(10));

// Clean up
result = carSoundEventInstance->release();
if (result != FMOD_OK) {
    std::cerr << "Releasing event instance failed: " << FMOD_ErrorString(result) << std::endl;
}

result = system->release();
if (result != FMOD_OK) {
    std::cerr << "Releasing FMOD System failed: " << FMOD_ErrorString(result) << std::endl;
}

return 0;

}