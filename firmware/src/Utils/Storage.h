#pragma once
#include <Preferences.h>
#include <string>

namespace Utils
{

    class Storage {
    private:
        std::string m_Partition;
        Preferences m_Preferences;
        bool m_inited = false;

    public:
        Storage();
        Storage(const std::string& partition);

        ~Storage();

        void setPartition(const std::string& partition);

        bool isKey(const std::string& key);

        int8_t putChar(const std::string& name, int8_t value);
        uint8_t putUChar(const std::string& name, uint8_t value);
        uint16_t putUShort(const std::string& name, uint16_t value);
        std::string putString(const std::string& name, const std::string& value);

        int8_t putIfNExistChar(const std::string& name, int8_t value);
        uint8_t putIfNExistUChar(const std::string& name, uint8_t value);
        uint16_t putIfNExistUShort(const std::string& name, uint16_t value);
        std::string putIfNExistString(const std::string& name, const std::string& value);

        int8_t getChar(const std::string& name);
        uint8_t getUChar(const std::string& name);
        uint16_t getUShort(const std::string& name);
        std::string getString(const std::string& name);
    };
}
