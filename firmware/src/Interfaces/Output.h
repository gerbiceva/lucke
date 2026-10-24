#pragma once
#include <cstdint>
#include <span>
#include <vector>

#include <FastLED.h>
#include <ArduinoJson.h>

namespace Interface
{

    struct Output
    {
        virtual ~Output() = default;

        virtual uint16_t footprint() const = 0;
        // Footprint this output WOULD have under `setting`, without applying it.
        // Used to project a per-preset footprint. Defaults to the live value.
        virtual uint16_t footprintFor(const JsonDocument& setting) const { return footprint(); }
        // src holds exactly this output's slice: src.size() == footprint().
        virtual void update(std::span<const uint8_t> src) = 0;
        virtual void postupdate() = 0;
        virtual void setPreset(const JsonDocument& doc) = 0;

        virtual void toJson(JsonObject& doc) {};
        virtual void describe(JsonObject& doc) {};
    };

}
