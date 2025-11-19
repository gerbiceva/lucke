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
        : m_universe(universe) {}
        
    InputInterface::~InputInterface() {}

    uint8_t* InputInterface::getBuffer()
    {
        return m_dmxBuffer;
    }
}