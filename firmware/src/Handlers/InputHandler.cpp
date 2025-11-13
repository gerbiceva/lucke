#include "InputHandler.h"

std::unordered_map<uint8_t, Traits::InputInterface*> InputHandler::m_inputs;
std::vector<std::pair<uint8_t, Traits::InputInterface*>> InputHandler::m_vecInputs;

