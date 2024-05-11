#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "fmod/fmod_errors.h"
#include "fmod/fmod_studio.hpp"
#include "fmod/fmod_studio_common.h"

enum class CarSoundDescriptions {
    // Engine revs
    engineDescription
    FMOD::Studio::EventInstance* engineInstance = nullptr;
    // Upshift effects
    FMOD::Studio::EventDescription* upshiftDescription = nullptr;
    FMOD::Studio::EventInstance* upshiftInstance = nullptr;
    // Downshift effects
    FMOD::Studio::EventDescription* downshiftDescription = nullptr;
    FMOD::Studio::EventInstance* downshiftInstance = nullptr;
    // Road noises + transmission whining
    FMOD::Studio::EventDescription* speedDescription = nullptr;
    FMOD::Studio::EventInstance* speedInstance = nullptr;
    // Starter
    FMOD::Studio::EventDescription* starterDescription = nullptr;
    FMOD::Studio::EventInstance* starterInstance = nullptr;
    // High load fx like turbo
    FMOD::Studio::EventDescription* turboDescription = nullptr;
    FMOD::Studio::EventInstance* turboInstance = nullptr;
};

class SoundManager {
   public:
    CarSound mainCar;
    FMOD::Studio::System* audioSystem = nullptr;
    std::map<const char*, FMOD::Studio::Bank*> bankMap;  // name of bank in filesystem --> pointer to Bank

    FMOD_RESULT loadBank(const char* bankName) {
        FMOD_RESULT result = audioSystem->loadBankFile(bankName, FMOD_STUDIO_LOAD_BANK_NORMAL, &bankMap[bankName]);
        std::cout << "Audio: Loaded sound bank " << bankName << std::endl;
        if (result != FMOD_OK) {
            std::cerr << "Loading sound bank failed: " << FMOD_ErrorString(result) << std::endl;
            return result;
        }
        return result;
    }
    FMOD_RESULT unloadBank(const char* bankName) {
        FMOD_RESULT result = bankMap[bankName]->unload();
        std::cout << "Audio: Unloaded sound bank " << bankName << std::endl;
        if (result != FMOD_OK) {
            std::cerr << "Unloading sound bank failed: " << FMOD_ErrorString(result) << std::endl;
            return result;
        }
        return result;
    }
    void debugListBanks() {
        std::cout << "Debug: List of bank names:" << std::endl;
        for (const auto& pair : bankMap) {
            std::cout << "Info - bankMap has: " << pair.first << std::endl;
        }
    }
    FMOD_RESULT getEvent(){
            const char* eventName = "event:/Vehicles/Sport Sedan";  // Replace with your event path
    FMOD_RESULT result = audioSystem->getEvent(eventName, &carSoundEventDescription);
    if (result != FMOD_OK) {
        std::cerr << "Getting event description failed: " << FMOD_ErrorString(result) << std::endl;
        return result;
    }
    return result;
    }

};