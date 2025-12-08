#include "Engine.h"

void setup() 
{	
	Engine::instance().addFixture<Astera60>(true);
	Engine::instance().init();
}

void loop()
{
	vTaskDelete(NULL);
}













