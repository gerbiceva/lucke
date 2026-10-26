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

    std::string Storage::putString(const std::string& name, const std::string& value) {
        if(!m_inited)
        {
            return "";
        }

        m_Preferences.putString(name.c_str(), value.c_str());
        return value;
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
