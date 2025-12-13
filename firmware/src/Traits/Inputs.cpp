#include "Inputs.h"

namespace Traits
{
    uint8_t InputInterface::s_ID = 0;
    std::string InputInterface::typeToString() const
    {
        switch(m_type)
        {
            case (InputType::SACN):
                return "SACN";
            case (InputType::DMX):
                return "DMX";
        }

        return "Unknown";
    }

    InputInterface::InputInterface(uint8_t universe)
        : m_universe(universe) 
    {
        m_ID = s_ID++;

        m_dmxBuffer = new uint8_t[512];
        memset(m_dmxBuffer, 0, 512);
    }
        
    InputInterface::~InputInterface() {}


    void InputInterface::clearSrcBuffer()
    {
        memset(m_dmxBuffer, 0, sizeof(uint8_t));
    }

    uint8_t InputInterface::getUniverse() const
    {
        return m_universe;
    }

    uint8_t* InputInterface::getBuffer()
    {
        return m_dmxBuffer;
    }
}