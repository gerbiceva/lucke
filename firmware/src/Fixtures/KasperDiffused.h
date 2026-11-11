#pragma once
#include "Fixtures/Generics/Strip.h"

namespace Fixtures
{
    // 288 WS2815 LEDs behind a diffuser. 864 bytes don't fit one universe,
    // so it starts at "group by 2" (see Presets::firstFittingPreset):
    // engine.addFixture<KasperDiffused<>>(288, "Kasper").
    template <uint8_t Pin = 5>
    using KasperDiffused = Strip<"KasperDiffused", Drivers::Ws2815, Pin>;
}
