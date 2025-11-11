#include "Outputs.h"

namespace Traits
{
    std::set<uint8_t> OutputInterface::m_usedPins;

    bool OutputInterface::isPinUnused(uint8_t pin) const
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

    OutputInterface::OutputInterface(uint16_t buffer_size)
    {
        m_size = buffer_size;
        // m_usedPins.emplace(pin);
        m_dstBuffer = new uint8_t[m_size]();
        // memset(m_dstBuffer, 0, buffer_size);
        // m_dstBuffer.resize(buffer_size);
    }

    void OutputInterface::setSrcBuffer(uint8_t* src_buffer)
    {
        m_srcBuffer = src_buffer;
    }

    // std::vector<uint8_t>& Outputs::getBuffer()
    // {
    //     return m_dstBuffer;
    // }

    uint8_t* OutputInterface::getBufferPtr()
    {
        return m_dstBuffer;
    }

    uint8_t* OutputInterface::getSrcBufferPtr()
    {
        return m_srcBuffer;
    }

    uint16_t OutputInterface::getSize() const
    {
        return m_size;
    }
}