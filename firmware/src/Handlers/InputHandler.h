#pragma once
#include "Traits/Inputs.h"
#include <unordered_map>
#include <vector>
#include <mutex>


namespace Handler
{
    class InputHandler
    {
        static std::vector<Traits::InputInterface*> m_inputs;
        static std::mutex m_lock;
        // static std::unordered_map<uint8_t, Traits::InputInterface*> m_inputs;
        // static std::vector<std::pair<uint8_t, Traits::InputInterface*>> m_vecInputs;

        static Traits::InputInterface* find(uint8_t universe);
    public:
        static Traits::InputInterface* interface(uint8_t universe, Traits::InputInterface::InputType type = Traits::InputInterface::InputType::SACN);
        static void update();
        // static void initInputs();
        static void clearSrcBuffers();
        static JsonDocument describe();
    };
}
