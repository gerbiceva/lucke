#pragma once
#include "Traits/Inputs.h"
#include "Utils/SacnParser.h"

namespace Input
{
    class Sacn : public Traits::InputInterface
    {
        Utils::SacnParser m_sacn;
    public:
        Sacn(uint8_t universe);
        ~Sacn();

        void update() override;

        void toJson(JsonObject& doc) override;
    };
}