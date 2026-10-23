#include "Core/Engine.h"
#include "Fixtures/Generics/Strip.h"

Engine engine;

void setup() {
  // 60 LEDs on pin 5, driven by the native WS2815 SPI output.
  engine.addFixture<Fixtures::StripWS2815<60, 5>>();
  // Only one WS2815 strip is possible (it owns SPI2); add more as FastLED:
  // engine.addFixture<Fixtures::StripWS2812<10, 6>>();
  engine.init();
}

void loop() { vTaskDelete(nullptr); }
