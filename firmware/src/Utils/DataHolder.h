#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace Utils
{
    class Data
    {
        std::vector<std::string> m_content;
        std::vector<std::pair<std::string, Data>> m_vecObjects;
        std::unordered_map<std::string, uint16_t> m_mapObjects;

    public:
        void setString(std::string str, uint16_t index = 0);
        std::string getString(uint16_t index);

        void setShort(uint16_t number, uint16_t index = 0);
        uint16_t getShort(uint16_t index);

        void setShort(std::vector<Data> data, uint16_t index = 0);
        // void addArray(std::string, )

        Data& operator[](std::string name);
    };
}