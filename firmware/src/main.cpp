#include "Core/Engine.h"

void setup()
{
	Engine::instance().addFixture<Fixtures::Strip2815<60>>(true);
	Engine::instance().init();
}

void loop()
{
	vTaskDelete(NULL);
}
