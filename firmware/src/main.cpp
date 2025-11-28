#include "Engine.h"

void setup() 
{	
	Engine::instance().addFixture<Astera60>(true);
	Engine::instance().init();
}

void loop()
{
	// Utils::Logger::println(Engine::instance().toString().c_str());
	// vTaskDelay(5000);
	vTaskDelete(NULL);
	// Engine::instance().update(nullptr);
}













