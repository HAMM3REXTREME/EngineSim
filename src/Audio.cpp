#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "fmod/fmod_errors.h"
#include "fmod/fmod_studio.hpp"
#include "fmod/fmod_studio_common.h"

class SoundEvent {
   public:
    FMOD::Studio::EventDescription* description = nullptr;
    std::vector<FMOD::Studio::EventInstance*> instanceList;
    ~SoundEvent() { description->releaseAllInstances(); }
};

class SoundManager {
   public:
    FMOD::Studio::System* audioSystem = nullptr;
    std::unordered_map<const char*, FMOD::Studio::Bank*> bankMap;  // name of bank in filesystem --> pointer to Bank
    std::unordered_map<const char*, SoundEvent> soundEventList;

    FMOD_RESULT loadBank(const char* bankFileName) {
        FMOD_RESULT result = audioSystem->loadBankFile(bankFileName, FMOD_STUDIO_LOAD_BANK_NORMAL, &bankMap[bankFileName]);
        std::cout << "Audio: Loaded sound bank " << bankFileName << std::endl;
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
        bankMap[bankName] = nullptr;
        return result;
    }
    void debugListBanks() {
        std::cout << "Debug: List of bank names:" << std::endl;
        for (const auto& pair : bankMap) {
            std::cout << "Info - bankMap has: " << pair.first << std::endl;
        }
    }
    FMOD_RESULT getEventDescription(const char* eventName) {
        FMOD_RESULT result = audioSystem->getEvent(eventName, &soundEventList[eventName].description);
        if (result != FMOD_OK) {
            std::cerr << "Getting event description failed: " << FMOD_ErrorString(result) << std::endl;
            return result;
        }
        return result;
    }
    FMOD_RESULT releaseEventDescription(const char* eventName) {
        FMOD_RESULT result = soundEventList[eventName].description->releaseAllInstances();
        if (result != FMOD_OK) {
            std::cerr << "Getting event description failed: " << FMOD_ErrorString(result) << std::endl;
            return result;
        }
        soundEventList[eventName].description = nullptr;
        return result;
    }

    FMOD_RESULT spawnInstance(const char* eventName) {
        // Create an instance of the event
        FMOD::Studio::EventInstance* temp = nullptr;
        FMOD_RESULT result = soundEventList[eventName].description->createInstance(&temp);
        soundEventList[eventName].instanceList.push_back(temp);
        if (result != FMOD_OK) {
            std::cerr << "Creating event instance failed: " << FMOD_ErrorString(result) << std::endl;
            return result;
        }
        return result;
    }
};