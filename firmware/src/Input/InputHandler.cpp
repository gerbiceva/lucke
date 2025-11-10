#include "InputHandler.h"

namespace Input
{
    std::unordered_map<uint8_t, InputInterface*> Handler::m_inputs;
    std::vector<std::pair<uint8_t, InputInterface*>> Handler::m_vecInputs;


}