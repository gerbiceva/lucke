#include "Core/Engine.h"

void setup()
{
	Engine::instance().addFixture<Fixtures::Strip2815<24>>(true);
	Engine::instance().init();
}

void loop()
{
	vTaskDelete(NULL);
}
