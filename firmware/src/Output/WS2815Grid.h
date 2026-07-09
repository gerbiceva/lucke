#pragma once
#include <memory>
#include "Utils/"
#include <vector>
#include "Base/LEDGridBase.h"
#include "Hardware/WS2815.h"
#include "Base/Drivers.h"
#include "Handlers/PinHandler.h"

namespace Output
{
    // Native WS2815-backed grid. Pin is runtime, so addStrip() takes it as an
    // argument. NOTE: the WS2815 driver uses a single SPI host (SPI2_HOST), so
    // in practice only one strip can be bound; multi-strip grids need a second
    // SPI host wired into Core::WS2815.
    class WS2815Grid : public LEDGridBase
    {
        std::vector<std::unique_ptr<Core::WS2815>> m_devices;

    public:
        WS2815Grid(uint16_t w, uint16_t h)
            : LEDGridBase(w, h, WS2815Driver::kind)
        {
        }

        void addStrip(int pin, uint16_t numLeds, GRange xr, GRange yr,
                      uint16_t cellW = 1, uint16_t cellH = 1, bool horizontal = true)
        {
            if (!Handler::Pin::available(pin))
            {
                Utils::Logger::printf("[GRID] pin %d already used\n", pin);
                return;
            }

            uint8_t *buf = new uint8_t[numLeds * 3]();
            auto dev = std::make_unique<Core::WS2815>();
            dev->init(numLeds, pin, buf);
            m_devices.push_back(std::move(dev));
            registerStrip(buf, xr, yr, cellW, cellH, horizontal);
        }

        void postupdate() override
        {
            for (auto &dev : m_devices)
                dev->show();
        }
    };
}
