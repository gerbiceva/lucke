#pragma once
#include <cstdio>
#include <memory>
#include <span>
#include "Output/Base/Base.h"
#include "Output/Driver.h"
#include "Hardware/WS2815.h"      // Core::RGB
#include "Utils/JsonUtils.h"
#include "Utils/Logger.h"

namespace Output
{
    // 1D strip: m_numGroups source colors spread over m_numLeds LEDs.
    template <typename TDriver, uint8_t Pin>
    class Strip1D final : public Base
    {
    public:
        explicit Strip1D(uint16_t numLeds)
            : Base(numLeds * 3),
              m_numLeds(numLeds),
              m_numGroups(numLeds),
              m_pixels(std::make_unique<Core::RGB[]>(numLeds)),
              m_driver(TDriver::template make<Pin>({m_pixels.get(), numLeds}))
        {
        }

        void update(std::span<const uint8_t> src) override
        {
            if (m_numGroups == 0) return;
            uint16_t groupSize = m_numLeds / m_numGroups;

            uint16_t led = 0;
            for (uint16_t g = 0; g < m_numGroups; g++)
            {
                Core::RGB c{};
                c.r = src[g * 3]; c.g = src[g * 3 + 1]; c.b = src[g * 3 + 2];
                for (uint16_t j = 0; j < groupSize; j++)
                    m_pixels[led++] = c;
            }
        }

        void postupdate() override
        {
            if (m_driver) m_driver->show();
        }

        void setPreset(const JsonDocument& doc) override
        {
            uint16_t groups = m_numGroups;
            if (!Utils::Json::updateElement<uint16_t>(doc, "num_groups", groups) ||
                groups == 0 || m_numLeds % groups != 0)
            {
                s_log.warn("invalid 'num_groups'");
                return;
            }
            m_numGroups = groups;
        }

        uint16_t footprint() const override { return m_numGroups * 3; }

        uint16_t footprintFor(const JsonDocument& setting) const override
        {
            uint16_t groups = m_numGroups;
            Utils::Json::updateElement<uint16_t>(setting, "num_groups", groups);
            return groups * 3;
        }

        const char* name() const override { return "LED1D"; }
        int hardwarePin() const override { return Pin; }

        void describe(JsonObject& desc) override
        {
            Base::describe(desc);
            desc["num_leds"] = m_numLeds;
            desc["num_groups"] = m_numGroups;
        }

        // Same fields as LED1D, so client tools see no change.
        void toJson(JsonObject& doc) override
        {
            char type[24];
            snprintf(type, sizeof(type), "%s-1D", TDriver::kind);

            doc["uuid"] = m_uuid;
            doc["type"] = type;
            doc["hardware_pin"] = Pin;
            doc["num_leds"] = m_numLeds;
            doc["num_groups"] = m_numGroups;

            JsonObject desc = doc["description"].to<JsonObject>();
            describe(desc);
        }

    private:
        static constexpr Utils::Log s_log{"STRIP"};

        uint16_t m_numLeds;
        uint16_t m_numGroups;
        // Declared before m_driver: the driver borrows this buffer.
        std::unique_ptr<Core::RGB[]> m_pixels;
        std::unique_ptr<Driver> m_driver;   // null if the pin or the hardware was unavailable
    };
}