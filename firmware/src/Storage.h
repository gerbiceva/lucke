#pragma once
#include <Preferences.h>
#include <string>

class Storage {
private:
    std::string m_Partition;
    Preferences m_Preferences;

public:
    Storage(std::string partition) {
        m_Partition = partition;
        m_Preferences.begin(partition.c_str());
    }

    ~Storage() {
        m_Preferences.end();
    }

    bool isKey(std::string key) {
        return m_Preferences.isKey(key.c_str());
    }
    
    int8_t putChar(std::string name, int8_t value) {
        m_Preferences.putChar(name.c_str(), value);
        return value;
    }

    uint8_t putUChar(std::string name, uint8_t value) {
        m_Preferences.putUChar(name.c_str(), value);
        return value;
    }

    uint16_t putUShort(std::string name, uint16_t value) {
        m_Preferences.putUShort(name.c_str(), value);
        return value;
    }

    std::string putString(std::string name, std::string value) {
        m_Preferences.putString(name.c_str(), value.c_str());
        return value;
    }

    int8_t putIfNExistChar(std::string name, int8_t value) {
        if(m_Preferences.isKey(name.c_str())) 
            return m_Preferences.getChar(name.c_str());
        m_Preferences.putChar(name.c_str(), value);
        return value;
    }

    uint8_t putIfNExistUChar(std::string name, uint8_t value) {
        if(m_Preferences.isKey(name.c_str()))
            return m_Preferences.getUChar(name.c_str());
        m_Preferences.putUChar(name.c_str(), value);
        return value;
    }

    uint16_t putIfNExistUShort(std::string name, uint16_t value) {
        if(m_Preferences.isKey(name.c_str()))
            return m_Preferences.getUShort(name.c_str());
        m_Preferences.putUShort(name.c_str(), value);
        return value;
    }

    std::string putIfNExistString(std::string name, std::string value) {
        if(m_Preferences.isKey(name.c_str()))
            return std::string(m_Preferences.getString(name.c_str()).c_str());
        m_Preferences.putString(name.c_str(), value.c_str());
        return value;
    }

    int8_t getChar(std::string name) {
        return m_Preferences.getChar(name.c_str());
    }

    uint8_t getUChar(std::string name) {
        return m_Preferences.getUChar(name.c_str());
    }

    uint16_t getUShort(std::string name) {
        return m_Preferences.getUShort(name.c_str());
    }

    std::string getString(std::string name) {
        return m_Preferences.getString(name.c_str()).c_str();
    }
    
};
