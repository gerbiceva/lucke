#include "Engine.h"

void setup() 
{	
	Engine::instance().addFixture<Strip1m60<5>>("", "", true);
}


void loop()
{
	

	// vTaskDelay(1000);
	vTaskDelete(NULL);
}
