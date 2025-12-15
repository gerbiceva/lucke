#pragma once
#include <Preferences.h>
#include <string>
#include "Optional.h"

namespace Utils 
{

    class Storage {
    private:
        std::string m_Partition;
        Preferences m_Preferences;
        bool m_inited = false;
    
    public:
        Storage();
        Storage(std::string partition);

        ~Storage();

        void setPartition(std::string partition);
        
        bool isKey(std::string key);

        int8_t putChar(std::string name, int8_t value);
        uint8_t putUChar(std::string name, uint8_t value);
        uint16_t putUShort(std::string name, uint16_t value);
        std::string putString(std::string name, std::string value);
    
        int8_t putIfNExistChar(std::string name, int8_t value);
        uint8_t putIfNExistUChar(std::string name, uint8_t value);
        uint16_t putIfNExistUShort(std::string name, uint16_t value);
        std::string putIfNExistString(std::string name, std::string value);
    
        // Optional<int8_t> getChar(std::string name);
        // Optional<uint8_t> getUChar(std::string name);
        // Optional<uint16_t> getUShort(std::string name);
        // Optional<std::string> getString(std::string name);

        int8_t getChar(std::string name);
        uint8_t getUChar(std::string name);
        uint16_t getUShort(std::string name);
        std::string getString(std::string name);
        
    };
}
