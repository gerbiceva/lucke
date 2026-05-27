#pragma once
#include <unordered_map>
#include <memory>
#include "Interfaces/Serializable.h"
#include "Interfaces/Input.h"
#include "Utils/SyncBuffer.h"

namespace Handler
{
    class InputHandler : public Interface::Serializable
    {
        bool match(uint8_t uni);
    public:
        std::shared_ptr<SyncBuffer> subscribe(uint8_t uni);
        void unsubscribe(uint8_t uni);
        void update();
        void reconnect();

        void toJson(JsonObject& obj) override;
    private:
        bool m_ready = false;
        std::unordered_map<uint8_t, std::shared_ptr<Interface::Input>> m_sources;
        std::unordered_map<uint8_t, std::shared_ptr<SyncBuffer>> m_buffers;
        std::unordered_map<uint8_t, uint16_t> m_refcount;
    };
}