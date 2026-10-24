#pragma once
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace Presets
{
    namespace detail
    {
        // Appends to out, or only counts when out is null (to size the buffer).
        struct Writer
        {
            char* out;
            size_t pos = 0;

            constexpr void putChar(char c)
            {
                if (out) out[pos] = c;
                ++pos;
            }

            constexpr void putStr(std::string_view s)
            {
                for (char c : s) putChar(c);
            }

            constexpr void putInt(int value)
            {
                char digits[10]{};
                int n = 0;
                do
                {
                    digits[n++] = static_cast<char>('0' + value % 10);
                    value /= 10;
                } while (value > 0);
                while (n > 0) putChar(digits[--n]);
            }
        };

        // One "group by d" preset per divisor d of numLeds, ascending.
        // Returns the length written (without the terminator).
        constexpr size_t writeGroupPresets(int numLeds, char* out)
        {
            Writer w{out};
            w.putStr("{ \"groups\": [\n");
            bool first = true;
            for (int d = 1; d <= numLeds; ++d)
            {
                if (numLeds % d != 0) continue;
                if (!first) w.putStr(",\n");
                first = false;

                w.putStr("  { \"name\": \"group by ");
                w.putInt(d);
                w.putStr("\", \"settings\": [ { \"num_groups\": ");
                w.putInt(numLeds / d);
                w.putStr(" } ] }");
            }
            w.putStr("\n] }");
            return w.pos;
        }
    }

    // Preset JSON for a strip of numLeds LEDs.
    inline std::string groupPresets(int numLeds)
    {
        std::string json(detail::writeGroupPresets(numLeds, nullptr), '\0');
        detail::writeGroupPresets(numLeds, json.data());
        return json;
    }
}
