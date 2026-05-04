#pragma once
#include <cstdint>
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
        uint8_t* m_dmxBuffer;
        uint8_t m_universe;
        InputType m_type;
        uint8_t m_ID;

        std::string typeToString() const;
    public:
        InputInterface(uint8_t universe);
        virtual ~InputInterface();

        uint8_t* getBuffer();
        uint8_t getUniverse() const;

        void clearSrcBuffer();
        virtual void setUniverse(uint8_t universe) { m_universe = universe; }
        virtual void update() = 0;
    private:
        static uint8_t s_ID;
    };
}
