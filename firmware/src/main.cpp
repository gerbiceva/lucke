#include "Core/Engine.h"
#include "Fixtures/Generics/Strip.h"

Engine engine;

void setup() {
  // WS2815 strip on pin 5, driven by the native SPI output. 60 LEDs until a
  // count is set with setfixture "leds"; every fixture runs this same build.
  engine.addFixture<Fixtures::StripWS2815<5>>(60);
  // Only one WS2815 strip is possible (it owns SPI2); add more as FastLED:
  // engine.addFixture<Fixtures::StripWS2812<6>>(10);
  engine.init();
}

void loop() { vTaskDelete(nullptr); }
