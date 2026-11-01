#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Utils
{
    // The last log lines (Info and up), kept in RTC memory. That survives a
    // crash, a watchdog or a brownout reset (not a power cut), so the next
    // boot can see what led up to it.
    //
    // The first call to any of these takes over what the previous boot left
    // and starts an empty trail for this one; previous() returns the old one.
    namespace CrashTrail
    {
        inline constexpr size_t Capacity = 1536;

        struct Previous
        {
            bool valid = false;        // false after a power-on: RTC memory held garbage
            std::string bootId;        // as given to setBoot(); empty if it never ran
            std::string version;
            uint32_t uptimeMs = 0;     // millis() at the last line
            std::vector<std::string> lines;   // oldest first
        };

        // Names this boot in the trail, for whoever reads it after a reset.
        void setBoot(std::string_view bootId, std::string_view version);

        // Called by Log for every line it keeps in its history.
        void append(std::string_view line);

        [[nodiscard]] Previous previous();
    }
}
