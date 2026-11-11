#include "Core/Engine.h"
#include "Fixtures/KasperDiffused.h"

Engine engine;

void setup() {
  engine.setDefaultWifi("Ledique", "dasenebipovezau");
  engine.addFixture<Fixtures::KasperDiffused<6>>(288, "Kasper");
  engine.init();
}

void loop() { vTaskDelete(nullptr); }
