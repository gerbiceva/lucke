#include "Core/Engine.h"

void setup()
{
	sleep(6);
	Engine::instance().addFixture<Fixtures::Strip2815<24>>(true);
	// Engine::instance().addFixture<Fixtures::Strip2815<10, 6>>(false);
	Engine::instance().init();
}

void loop()
{
	vTaskDelete(NULL);
}

