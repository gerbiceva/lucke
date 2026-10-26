#pragma once
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include <FastLED.h>
#include <ArduinoJson.h>

namespace Interface
{

    // One option of an output (invert, LED count, ...). Outputs list theirs;
    // the API validates a request against the list, then calls applySetting.
    struct SettingDesc
    {
        enum class Type : uint8_t { Bool, Int, Enum };

        std::string_view key;
        Type type = Type::Bool;
        int32_t min = 0;                                  // Int
        int32_t max = 0;                                  // Int
        std::span<const std::string_view> options = {};   // Enum
        // False: read-only (e.g. num_groups, which only a preset changes).
        bool writable = true;
        // True: saved, and applied after a restart (the output is built with it).
        bool requiresReboot = false;
    };

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

        // What the output is: driver-independent name ("LED1D", ...), the
        // concrete type ("WS2815-1D"), and the pin it drives or -1.
        virtual const char* name() const { return "Output"; }
        virtual const char* typeName() const { return "output"; }
        virtual int hardwarePin() const { return -1; }

        virtual std::span<const SettingDesc> settings() const { return {}; }
        // Writes the live value of `key` to out. False if there's no such key.
        virtual bool getSetting(std::string_view key, JsonVariant out) const { return false; }
        // `value` already passed the checks for its SettingDesc. Not called for
        // requiresReboot settings: the output reads those when it is built.
        virtual void applySetting(std::string_view key, JsonVariantConst value) {}
    };

}
