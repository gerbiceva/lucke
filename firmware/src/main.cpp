#include "Core/Engine.h"
#include "Fixtures/Generics/StripWS2815.h"
#include <variant>

Engine engine;

void setup() {
  std::variant<int, float> v(0.0f);
  // 24 RGB LEDs on pin 5, driven by the native WS2815 SPI output.
  engine.addFixture<Fixtures::StripWS2815<60, 5>>();
  //   engine.addFixture<Fixtures::StripWS2815<10, 6>>();
  engine.init();
}

void loop() { vTaskDelete(NULL); }
