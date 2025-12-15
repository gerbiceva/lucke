#include "Storage.h"

namespace Utils
{
    Storage::Storage()
    {}

    Storage::Storage(std::string partition) {
        m_Partition = partition;
        m_Preferences.begin(partition.c_str());
        m_inited = true;
    }

    void Storage::setPartition(std::string partition)
    {
        // TODO: check if open
        m_Partition = partition;
        m_Preferences.begin(partition.c_str());
        m_inited = true;
    }

    Storage::~Storage() {
        if(m_inited)
        {
            m_Preferences.end();
            m_inited = false;
        }
    }

    bool Storage::isKey(std::string key) 
    {
        if(!m_inited)
        {
            return false;
        }

        return m_Preferences.isKey(key.c_str());
    }
    
    int8_t Storage::putChar(std::string name, int8_t value) {
        if(!m_inited)
        {
            return 0;
        }

        m_Preferences.putChar(name.c_str(), value);
        return value;
    }

    uint8_t Storage::putUChar(std::string name, uint8_t value) {
        if(!m_inited)
        {
            return 0;
        }

        m_Preferences.putUChar(name.c_str(), value);
        return value;
    }

    uint16_t Storage::putUShort(std::string name, uint16_t value) {
        if(!m_inited)
        {
            return 0;
        }

        m_Preferences.putUShort(name.c_str(), value);
        return value;
    }

    std::string Storage::putString(std::string name, std::string value) {
        if(!m_inited)
        {
            return "0";
        }

        m_Preferences.putString(name.c_str(), value.c_str());
        return value;
    }

    int8_t Storage::putIfNExistChar(std::string name, int8_t value) {
        if(!m_inited)
        {
            return 0;
        }

        if(isKey(name)) 
        {
            return m_Preferences.getChar(name.c_str());
        }

        m_Preferences.putChar(name.c_str(), value);
        return value;
    }

    uint8_t Storage::putIfNExistUChar(std::string name, uint8_t value) {
        if(!m_inited)
        {
            return 0;
        }

        if(isKey(name)) 
        {
            return m_Preferences.getUChar(name.c_str());
        }

        m_Preferences.putUChar(name.c_str(), value);
        return value;
    }

    uint16_t Storage::putIfNExistUShort(std::string name, uint16_t value) {
        if(!m_inited)
        {
            return 0;
        }

        if(isKey(name)) 
        {
            return m_Preferences.getUShort(name.c_str());
        }

        m_Preferences.putUShort(name.c_str(), value);
        return value;
    }

    std::string Storage::putIfNExistString(std::string name, std::string value) {
        if(!m_inited)
        {
            return "0";
        }

        if(isKey(name))
        {
            return std::string(m_Preferences.getString(name.c_str()).c_str());
        }

        m_Preferences.putString(name.c_str(), value.c_str());
        return value;
    }

    int8_t Storage::getChar(std::string name) 
    {
        if(!m_inited)
        {
            return 0;
        }

        return m_Preferences.getChar(name.c_str());
    }

    uint8_t Storage::getUChar(std::string name) 
    {
        if(!m_inited)
        {
            return 0;
        }

        return m_Preferences.getUChar(name.c_str());
    }

    uint16_t Storage::getUShort(std::string name) 
    {
        if(!m_inited)
        {
            return 0;
        }

        return m_Preferences.getUShort(name.c_str());
    }

    std::string Storage::getString(std::string name) 
    {
        if(!m_inited)
        {
            return "0";
        }

        return m_Preferences.getString(name.c_str()).c_str();
    }

    // Optional<int8_t> Storage::getChar(std::string name) 
    // {
    //     if(!m_inited)
    //     {
    //         return Optional<int8_t>();
    //     }

    //     return m_Preferences.getChar(name.c_str());
    // }

    // Optional<uint8_t> Storage::getUChar(std::string name) 
    // {
    //     if(!m_inited)
    //     {
    //         return Optional<uint8_t>();
    //     }

    //     return m_Preferences.getUChar(name.c_str());
    // }

    // Optional<uint16_t> Storage::getUShort(std::string name) 
    // {
    //     if(!m_inited)
    //     {
    //         return Optional<uint16_t>();
    //     }

    //     return m_Preferences.getUShort(name.c_str());
    // }

    // Optional<std::string> Storage::getString(std::string name) 
    // {
    //     if(!m_inited)
    //     {
    //         return Optional<std::string>();
    //     }

    //     return m_Preferences.getString(name.c_str()).c_str();
    // }
}