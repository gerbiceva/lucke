#include "DataHolder.h"

namespace Utils
{
    void Data::setString(std::string str, uint16_t index)
    {
        if(index > m_content.size())
        {
            m_content.resize(index + 1);
        }

        m_content[index] = str;
    }

    std::string Data::getString(uint16_t index)
    {
        if(index >= m_content.size())
        {
            return "";
        }
        else 
        {
            return m_content[index];
        }
    }

    void Data::setShort(uint16_t number, uint16_t index)
    {
        setString(std::to_string(number), index);
    }

    uint16_t Data::getShort(uint16_t index)
    {
        return std::atoi(getString(index).c_str());
    }

    void setArray(std::vector<Data> data, uint16_t index)
    {
        
    }

    Data& Data::operator[](std::string name)
    {
        if(m_mapObjects.count(name) == 0)
        {
            m_mapObjects[name] = m_vecObjects.size();

            m_vecObjects.push_back({name, Data()});
        }

        return m_vecObjects[m_mapObjects[name]].second;
    }
}