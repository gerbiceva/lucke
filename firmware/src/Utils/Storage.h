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
        // Removes every key in this partition.
        void clear();

        std::string putString(const std::string& name, const std::string& value);
        std::string getString(const std::string& name);
    };
}
