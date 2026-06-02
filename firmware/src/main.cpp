#include "Fixtures/Generics/Strip2815.h"
#include "Core/Engine.h"

Engine engine;

void setup()
{
	engine.addFixture<Fixtures::Strip2815<24>>();
	engine.addFixture<Fixtures::Strip2815<10, 6>>();
	engine.init();
}

void loop()
{
	vTaskDelete(NULL);
}

