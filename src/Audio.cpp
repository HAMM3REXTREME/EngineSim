#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "fmod/fmod_errors.h"
#include "fmod/fmod_studio.hpp"
#include "fmod/fmod_studio_common.h"

class CarSound {
   public:
    // Revving sound
    FMOD::Studio::EventDescription* engineDescription = nullptr;
    FMOD::Studio::EventInstance* engineInstance = nullptr;
    // Upshift effects
    FMOD::Studio::EventDescription* upshiftDescription = nullptr;
    FMOD::Studio::EventInstance* upshiftInstance = nullptr;
    // Downshift effects
    FMOD::Studio::EventDescription* downshiftDescription = nullptr;
    FMOD::Studio::EventInstance* downshiftInstance = nullptr;
    // Road noises
    FMOD::Studio::EventDescription* roadDescription = nullptr;
    FMOD::Studio::EventInstance* roadInstance = nullptr;
    // Starter onramp
    FMOD::Studio::EventDescription* starterStartDescription = nullptr;
    FMOD::Studio::EventInstance* startStartInstance = nullptr;
    // Starter middle
    FMOD::Studio::EventDescription* starterMiddleDescription = nullptr;
    FMOD::Studio::EventInstance* startMiddleInstance = nullptr;
    // Starter done
    FMOD::Studio::EventDescription* starterDoneDescription = nullptr;
    FMOD::Studio::EventInstance* startDoneInstance = nullptr;
};

class SoundManager {
   public:
    FMOD::Studio::System* audioSystem = nullptr;
    CarSound mainCar;
    std::vector<FMOD::Studio::Bank*> loadedSoundBanks;

    FMOD_RESULT createSystem() {
        FMOD_RESULT result = FMOD::Studio::System::create(&audioSystem);
        if (result != FMOD_OK) {
            std::cerr << "FMOD Error - Could not create system"
                      << " Error code: " << result << ", " << FMOD_ErrorString(result) << std::endl;
        }
        return result;
    }

    FMOD_RESULT initAudio(int maxChannels) {
        FMOD_RESULT result = audioSystem->initialize(maxChannels, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK) {
            std::cerr << "FMOD Error - Could not initialize audio"
                      << " Error code: " << result << ", " << FMOD_ErrorString(result) << std::endl;
            return result;
        }
        return result;
    }
    FMOD_RESULT loadSoundBank(const std::string& name, const std::string& filePath) {
        FMOD::Studio::Bank* bank = nullptr;
        FMOD_RESULT result = audioSystem->loadBankFile(filePath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
        if (result != FMOD_OK) {
            std::cerr << "FMOD Error - Could not load Bank"
                      << " Error code: " << result << ", " << FMOD_ErrorString(result) << std::endl;
            return result;
        }
        loadedSoundBanks.emplace_back(bank);
        return result;
    }

    FMOD_RESULT getEventDescription(const char* eventName) {
        // FMOD_RESULT result = audioSystem->getEvent(eventName, mainCar);
        return FMOD_OK;
    }
};