#include "Core/Engine.h"
#include "Fixtures/Generics/Strip.h"

Engine engine;

void setup() {
  engine.addFixture<Fixtures::StripWS2815<5>>(60);
  engine.init();
}

void loop() { vTaskDelete(nullptr); }
