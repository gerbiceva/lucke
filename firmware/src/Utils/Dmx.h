#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace Dmx
{
    // A DMX universe is always exactly 512 channels. The fixed-extent span
    // types carry that size in the type, so passing a wrong-sized buffer
    // doesn't compile.
    inline constexpr size_t c_universeSize = 512;

    using Universe = std::array<uint8_t, c_universeSize>;
    using UniverseView = std::span<uint8_t, c_universeSize>;
    using UniverseCView = std::span<const uint8_t, c_universeSize>;
}
