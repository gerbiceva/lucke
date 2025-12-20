#include "Fixtures.h"
#include "Core/Engine.h"
#include "Utils/TaskExecutor.h"



struct Example {
	void run() {
		while (true)
		{
			vTaskDelay(1000);
			Utils::Logger::print("Success");
		}
	}
};


void setup() 
{	
	Engine::instance().addFixture<SGMQ1>(true);
	Engine::instance().init();
	// Utils::Logger::enable();
	// Example e;

	// Utils::TaskExecutor executor; 

	// executor.spawnTask([]() {
	// 	for (uint8_t i = 0; i < 10; i++)
	// 	{
	// 		vTaskDelay(1000);
	// 		Utils::Logger::print("TaskA\n");
	// 	}
	// });

	// executor.spawnTask([]() {
	// 	for (uint8_t i = 0; i < 10; i++)
	// 	{
	// 		vTaskDelay(500);
	// 		Utils::Logger::print("TaskB\n");
	// 	}
	// });

	// executor.spawnTask([]() {
	// 	for (uint8_t i = 0; i < 10; i++)
	// 	{
	// 		vTaskDelay(2000);
	// 		Utils::Logger::print("TaskC\n");
	// 	}
	// });

	// executor.spawnTask([]() {
	// 	for (uint8_t i = 0; i < 10; i++)
	// 	{
	// 		vTaskDelay(6000);
	// 		Utils::Logger::print("TaskD\n");
	// 	}
	// });
}

void loop()
{
	vTaskDelete(NULL);
}













