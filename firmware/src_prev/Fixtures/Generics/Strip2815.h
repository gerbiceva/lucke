#pragma once
#include "Core/Fixture.h"
#include "Utils/PresetMaker.h"
#include "Output/HardwareLED1D.h"

namespace Fixtures
{

template<uint16_t N, uint8_t TPin = 5>
struct Strip2815 : public Fixture
{
    static constexpr uint16_t BYTES = N * 3;

    Strip2815()
		: Fixture("neki", "Strip",  MakeJSON<N>::make().data())
    {
        addOutput<Output::HardwareLED1D<WS2815, TPin, RGB>> (N);
    }

    Strip2815(std::string name, std::string type, std::string presetJson)
		: Fixture(name, type, presetJson)
    {
        addOutput<Output::HardwareLED1D<WS2815, TPin, RGB>> (N);
    }

    Strip2815(Config::Fixture config, std::string presetJson)
		: Fixture(config, presetJson)
    {
        addOutput<Output::HardwareLED1D<WS2815, TPin, RGB>> (N);
    }

    void wifiAnimation() override
    {
        static uint16_t off = 0;
        getOffsetSrcBuffer()[off] = 255;
        getOffsetSrcBuffer()[off == 0 ? (BYTES - 1) : off - 1] = 0;
        off = (off + 1) % BYTES;
        vTaskDelay(20);
    }

    void highlight() override
    {
        memset(getOffsetSrcBuffer(), 255, BYTES);
    }
};

}
