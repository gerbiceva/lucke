#pragma once
#include "Interfaces/Input.h"
#include "Utils/SacnParser.h"

namespace Input
{
    class SacnSource : public Interface::Input
    {
    public:
        SacnSource() = default;
        // Pre-seed the universe so toJson() reports the correct value even
        // before begin() runs (begin() is deferred until wifi connects).
        explicit SacnSource(uint8_t universe) : m_universe(universe) {}
        ~SacnSource();
        void begin(uint8_t universe) override;
        void stop() override;
        bool poll(Dmx::UniverseView out) override;
        void toJson(JsonObject& obj) override;
    private:
        uint8_t m_universe = 0;
        Utils::SacnParser m_sacn;

    };
}