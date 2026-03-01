#pragma once
#include "Traits/Inputs.h"
#include "Traits/Serializable.h"
#include "Traits/Deserializable.h"
#include <unordered_map>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>

#include "Utils/Optional.h"


namespace Handler
{
    class InputHandler : public Traits::Serializable, public Traits::Deserializable
    {
        struct Counter {
            uint8_t count = 0;
            std::shared_ptr<Traits::InputInterface> interface;
        };

        Utils::Optional<Counter*> find(uint8_t universe);
        void iterateInputs(std::function<void(std::shared_ptr<Traits::InputInterface>)> callback);

        void unsubscribe(uint8_t universe);
        std::shared_ptr<Traits::InputInterface> subscribe(uint8_t universe);
    public:

        std::shared_ptr<Traits::InputInterface> interface(
            uint8_t universe,
            uint8_t old_universe,
            Traits::InputInterface::InputType type= Traits::InputInterface::InputType::SACN);

        void update();
        void canUpdate(bool b);
        // void initInputs();
        void clearSrcBuffers();
        
        void fromJson(std::string json) override;
        void toJson(JsonObject& doc) override;

    private:
        std::unordered_map<uint8_t, Counter> m_inputs;
        TaskHandle_t m_handle;
    };
}
