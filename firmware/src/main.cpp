// #include "Controller.h"
// #include "ButtonManager.h"

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);		// Set your gateway
// IPAddress subnet(255, 255, 255, 0);	 // Set your subnet mask
/*
struct LampAstera : public Lamp {
    LampAstera() : Lamp("Astera", 60, {
		{60, "group by 1"},
		{30, "group by 2"},
		{15, "group by 4"},
		{4, "group by 15"}
	}) {};
};

struct LampAstera144 : public Lamp {
    LampAstera144() : Lamp("Astera", 144, {
		{144, "group by 1"},
		{72, "group by 2"},
		{48, "group by 4"},
		{36, "group by 15"}
	}) {};
};

struct LampLedbar : public Lamp {
    LampLedbar() : Lamp("Ledbar", 100, {
		{100, "group by 1"},
		{50, "group by 2"},
		{25, "group by 4"},
		{10, "group by 10"}
	}) {};
};

struct LampLedbar2m : public Lamp {
    LampLedbar2m() : Lamp("Ledbar2m", 120, {
		{120, "group by 1"},
		{60, "group by 2"},
		{30, "group by 4"},
		{12, "group by 10"}
	}, "Trak") {};
};
*/

// #include "Utils/Wifi.h"
// #include "Fixture/Fixture1D.h"
// #include "Handlers/SacnHandler.h"
// #include "Handlers/FixtureHandler.h"

// using Astera60 =
// struct Astera60 : public Core::Fixture::Fixture1D
// {
// 	Astera60() : Core::Fixture::Fixture1D("astera", "astera60", 60,
// 	std::vector<Core::Fixture::Preset1D>{
// 			{"", 1},
// 			{"", 2},
// 			{"", 4},
// 			{"", 10}
// 		})
// 		{}
// };


#include "Fixtures.h"
#include "Utils/Wifi.h"
#include "Utils/Logger.h"
#include "Handlers/FixtureHandler.h"
Fixture* fix;
void setup() {	
	Utils::Logger::enable();
	sleep(5);
	if(!Utils::Wifi::setup("ledique", "dasenebipovezau"))
	{
		Utils::Logger::println("Error connecting");
	}
	// Utils::Wifi::checkNetwork(nullptr);

	fix = FixtureHandler::addFixture<Astera60>();
	// xTaskCreate(Handler::Fixtures::update, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
}

void loop()
{
	static uint16_t off = 0;
	fix->getSrcBuffer()[off] = 255;
	fix->getSrcBuffer()[off == 0 ? 179 : off - 1] = 0;
	off = (off + 1) % 180;
	// fix->update();
	// Output::updateFastLED();
	FixtureHandler::updateTask(nullptr);
	// for(int i = 0; i < 15; i += 1)
	// {
	// 	Serial.print(led.getBufferPtr()[i]);
	// 	Serial.print(',');
	// 	// led.getBufferPtr()[i] = 255;
	// }
	// Serial.println();
	// Serial.println();

	// sleep(2);
	vTaskDelay(30);
	// vTaskDelete(NULL);
}
