#include "Outputs.h"

namespace Output
{
    std::set<uint8_t> Outputs::m_usedPins;

    bool Outputs::isPinUnused(uint8_t pin) const
    {
        if(m_usedPins.find(pin) == m_usedPins.end())
        {
            m_usedPins.emplace(pin);
            return true;
        }
        else 
        {
            return false;
        }
    }

    Outputs::Outputs(uint16_t buffer_size)
    {
        m_size = buffer_size;
        // m_usedPins.emplace(pin);
        m_dstBuffer = new uint8_t[m_size]();
        // memset(m_dstBuffer, 0, buffer_size);
        // m_dstBuffer.resize(buffer_size);
    }

    void Outputs::setSrcBuffer(uint8_t* src_buffer)
    {
        m_srcBuffer = src_buffer;
    }

    // std::vector<uint8_t>& Outputs::getBuffer()
    // {
    //     return m_dstBuffer;
    // }

    uint8_t* Outputs::getBufferPtr()
    {
        return m_dstBuffer;
    }

    uint8_t* Outputs::getSrcBufferPtr()
    {
        return m_srcBuffer;
    }

    uint16_t Outputs::getSize() const
    {
        return m_size;
    }
}