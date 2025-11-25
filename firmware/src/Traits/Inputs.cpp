#include "Inputs.h"

namespace Traits
{
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
        // m_dmxBuffer = new uint8_t[512]();
    }
        
    InputInterface::~InputInterface() {}

    uint8_t InputInterface::getUniverse() const
    {
        return m_universe;
    }

    uint8_t* InputInterface::getBuffer()
    {
        return m_dmxBuffer;
    }
}