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


#include "Core/Fixture.h"
#include "Output/HardwareLED.h"

Fixture astera60("nek", "astera60",
R"(
{
  "groups": [
    {
      "name": "group by 1",
      "settings": [
        [
          { "num_groups": 60 }
        ]
      ]
    },
    {
      "name": "group by 2",
      "settings": [
        [
          { "num_groups": 30 }
        ]
      ]
    },
    {
      "name": "group by 4",
      "settings": [
        [
          { "num_groups": 15 }
        ]
      ]
    },
    {
      "name": "group by 10",
      "settings": [
        [
          { "num_groups": 6 }
        ]
      ]
    }
  ]
}

)");

void setup() {	
	astera60.addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (60);
	// xTaskCreate(Handler::Fixtures::update, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
}

void loop()
{
	static uint16_t off = 0;
	astera60.getSrcBuffer()[off] = 255;
	astera60.getSrcBuffer()[off == 0 ? 179 : off - 1] = 0;
	off = (off + 1) % 180;
	astera60.update();
	Output::updateFastLED();
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
