#pragma once
#include <cstdint>
#include <array>
#include <ArduinoJson.h>
#include "Traits/Deserializable.h"

namespace Traits
{

    class InputInterface : public Deserializable
    {
    public:
        enum class InputType
        {
            None,
            SACN,
            DMX
        };
    protected:
        std::array<uint8_t, 512> m_dmxBuffer;
        uint8_t m_universe;
        InputType m_type;
        uint8_t m_ID;

        std::string typeToString() const;
    public:
        InputInterface(uint8_t universe);
        virtual void reinit() = 0;
        virtual ~InputInterface();

        uint8_t* getBuffer();
        uint8_t getUniverse() const;

        void clearSrcBuffer();
        virtual void setUniverse(uint8_t universe) { m_universe = universe; }
        virtual void update() = 0;

        std::string inputTypeStr() const
        {
            switch(m_type)
            {
                case InputType::SACN:
                    return "SACN";
                case InputType::DMX:
                    return "DMX";
            }

            return "None";
        }

        void bytesJson(JsonArray& obj) const
        {
            for(int i = 0; i < 512; i++)
            {
                obj.add((uint8_t)m_dmxBuffer[i]);
            }
        }
    private:
        static uint8_t s_ID;
    };
}
