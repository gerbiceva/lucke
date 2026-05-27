#pragma once
#include "Interfaces/Input.h"
#include "Utils/SacnParser.h"

namespace Input
{
    class SacnSource : public Interface::Input
    {
    public:
        ~SacnSource();
        void begin(uint8_t universe) override;
        void stop() override;
        bool poll(uint8_t* buff) override;
        void toJson(JsonObject& obj) override;
    private:
        uint8_t m_universe;
        Utils::SacnParser m_sacn;

    };
}