#include "Storage.h"

namespace Utils
{
    Storage::Storage()
    {}

    Storage::Storage(const std::string& partition) {
        m_Partition = partition;
        m_Preferences.begin(partition.c_str());
        m_inited = true;
    }

    void Storage::setPartition(const std::string& partition)
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

    bool Storage::isKey(const std::string& key)
    {
        if(!m_inited)
        {
            return false;
        }

        return m_Preferences.isKey(key.c_str());
    }

    void Storage::clear()
    {
        if(m_inited)
        {
            m_Preferences.clear();
        }
    }

    int8_t Storage::putChar(const std::string& name, int8_t value) {
        if(!m_inited)
        {
            return 0;
        }

        m_Preferences.putChar(name.c_str(), value);
        return value;
    }

    uint8_t Storage::putUChar(const std::string& name, uint8_t value) {
        if(!m_inited)
        {
            return 0;
        }

        m_Preferences.putUChar(name.c_str(), value);
        return value;
    }

    uint16_t Storage::putUShort(const std::string& name, uint16_t value) {
        if(!m_inited)
        {
            return 0;
        }

        m_Preferences.putUShort(name.c_str(), value);
        return value;
    }

    std::string Storage::putString(const std::string& name, const std::string& value) {
        if(!m_inited)
        {
            return "";
        }

        m_Preferences.putString(name.c_str(), value.c_str());
        return value;
    }

    int8_t Storage::putIfNExistChar(const std::string& name, int8_t value) {
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

    uint8_t Storage::putIfNExistUChar(const std::string& name, uint8_t value) {
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

    uint16_t Storage::putIfNExistUShort(const std::string& name, uint16_t value) {
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

    std::string Storage::putIfNExistString(const std::string& name, const std::string& value) {
        if(!m_inited)
        {
            return "";
        }

        if(isKey(name))
        {
            return std::string(m_Preferences.getString(name.c_str()).c_str());
        }

        m_Preferences.putString(name.c_str(), value.c_str());
        return value;
    }

    int8_t Storage::getChar(const std::string& name)
    {
        if(!m_inited)
        {
            return 0;
        }

        return m_Preferences.getChar(name.c_str());
    }

    uint8_t Storage::getUChar(const std::string& name)
    {
        if(!m_inited)
        {
            return 0;
        }

        return m_Preferences.getUChar(name.c_str());
    }

    uint16_t Storage::getUShort(const std::string& name)
    {
        if(!m_inited)
        {
            return 0;
        }

        return m_Preferences.getUShort(name.c_str());
    }

    std::string Storage::getString(const std::string& name)
    {
        if(!m_inited)
        {
            return "";
        }

        return m_Preferences.getString(name.c_str()).c_str();
    }
}
