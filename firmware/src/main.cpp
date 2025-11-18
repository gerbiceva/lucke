#include "Engine.h"

void setup() 
{	
	Engine::instance().addFixture<Astera60>(true);
	Engine::instance().createTasks();
}

void loop()
{
	// vTaskDelay(30);
	vTaskDelete(NULL);
}
