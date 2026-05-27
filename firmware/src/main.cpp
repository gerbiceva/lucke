#include "Fixtures/Generics/Strip2815.h"
#include "Core/Engine.h"

void setup()
{
	sleep(2);
	static Engine engine;
	engine.addFixture<Fixtures::Strip2815<24>>();
	engine.addFixture<Fixtures::Strip2815<10, 6>>();
	engine.init();
}

void loop()
{
	vTaskDelete(NULL);
}

