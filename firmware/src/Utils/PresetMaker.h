#pragma once
#include <array>
#include <cstddef>
#include <string_view>

namespace Presets
{
    namespace detail
    {
        // Appends to out, or only counts when out is null. Running the same
        // code twice (count, then write) sizes the buffer exactly, so no
        // strip length can overflow it.
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

    // Null-terminated preset JSON for an N-LED strip, built at compile time
    // and stored in flash.
    template <int N>
    inline constexpr auto c_groupPresets = [] {
        std::array<char, detail::writeGroupPresets(N, nullptr) + 1> json{};
        detail::writeGroupPresets(N, json.data());
        return json;
    }();
}

// Old runtime builder: wrote into a fixed 1024-byte buffer with no bounds
// check, so strips with many divisors (e.g. 300 LEDs) overflowed it.
//
// template<int N, int I = 1>
// struct Divisors {
//     static constexpr int value = (N % I == 0 ? 1 : 0) + Divisors<N, I + 1>::value;
// };
//
// template<int N>
// struct Divisors<N, N> {
//     static constexpr int value = 1;
// };
//
// template<int N, int I = 1, int IDX = 0>
// struct FillDivisors {
//     template<typename Arr>
//     static void fill(Arr& arr) {
//         FillDivisors<N, I + 1, IDX + (N % I == 0 ? 1 : 0)>::fill(arr);
//         if (N % I == 0) arr[IDX] = I;
//     }
// };
//
// template<int N, int IDX>
// struct FillDivisors<N, N, IDX> {
//     template<typename Arr>
//     static void fill(Arr& arr) {
//         arr[IDX] = N;
//     }
// };
//
// template<size_t N>
// struct ConstStr {
//     std::array<char, N> data{};
//     size_t pos = 0;
//
//     void push(char c) {
//         data[pos++] = c;
//     }
//
//     void push_str(const char* s) {
//         while (*s)
//             data[pos++] = *s++;
//     }
//
//     void push_int(int value) {
//         char tmpbuf[20]{};
//         int idx = 0;
//         int div = 1;
//         while (value / div >= 10) div *= 10;
//         while (div > 0) {
//             tmpbuf[idx++] = '0' + value / div;
//             value %= div;
//             div /= 10;
//         }
//         for (int i = 0; i < idx; ++i)
//             push(tmpbuf[i]);
//     }
// };
//
// template<int N>
// struct MakeJSON {
//     static constexpr size_t max_size = 1024;
//
//     static std::array<char, max_size> make() {
//         ConstStr<max_size> s{};
//         s.push_str("{ \"groups\": [\n");
//
//         constexpr int count = Divisors<N>::value;
//         std::array<int, count> divs{};
//         FillDivisors<N>::fill(divs);
//
//         for (int i = 0; i < count; ++i) {
//             s.push_str("  { \"name\": \"group by ");
//             s.push_int(divs[i]);
//             s.push_str("\", \"settings\": [ { \"num_groups\": ");
//             s.push_int(N / divs[i]);
//             s.push_str(" } ] }");
//             if (i + 1 < count)
//                 s.push_str(",\n");
//         }
//
//         s.push_str("\n] }\0");
//         return s.data;
//     }
// };
