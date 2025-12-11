#pragma once
#include "Traits/Inputs.h"
#include "Traits/Serializable.h"
#include "Traits/Deserializable.h"
#include <unordered_map>
#include <vector>
#include <mutex>


namespace Handler
{
    class InputHandler : public Traits::Serializable, public Traits::Deserializable
    {
        std::vector<Traits::InputInterface*> m_inputs;
        TaskHandle_t m_handle;
        // std::unordered_map<uint8_t, Traits::InputInterface*> m_inputs;
        // std::vector<std::pair<uint8_t, Traits::InputInterface*>> m_vecInputs;

        Traits::InputInterface* find(uint8_t universe);
    public:
        void fromJson(std::string json) override;

        Traits::InputInterface* interface(uint8_t universe, Traits::InputInterface::InputType type = Traits::InputInterface::InputType::SACN);
        void update();
        void updateTask(void*);
        void canUpdate(bool b);
        // void initInputs();
        void clearSrcBuffers();
        JsonDocument describe() override;
    };
}
