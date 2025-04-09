#include "Controller.h"
#include "ButtonManager.h"

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);		// Set your gateway
// IPAddress subnet(255, 255, 255, 0);	 // Set your subnet mask

struct LampAstera : public Lamp {
    LampAstera() {
        type = "Astera";
        numLeds = 60;
        presets = {
            {60, "group by 1"},
            {30, "group by 2"},
            {15, "group by 4"},
            {4, "group by 15"}
        };
    };
};

struct LampLedbar : public Lamp {
    LampLedbar() {
        type = "Ledbar";
        numLeds = 100;
        presets = {
            {100, "group by 1"},
            {50, "group by 2"},
            {25, "group by 4"},
            {10, "group by 10"}
        };
    };
};

struct LampLedbar2m : public Lamp {
    LampLedbar2m() {
        type = "Led2m";
        numLeds = 120;
        presets = {
            {120, "group by 1"},
            {60, "group by 2"},
            {30, "group by 4"},
            {12, "group by 10"}
        };
    };
};

void setup() {	
	// initialise the contorller
	Controller::get().setLamp(new LampLedbar());
	Controller::get().init(UNIVERSE, ADDR_OFFSET);
	Controller::createTasks();

	// Button b1(
	// 	9, 
	// 	[](){ Controller::get().togglePreset(); },
	// 	[](){ Controller::get().on(); },
	// 	3000
	// );
	// Button b2(
	// 	21, 
	// 	[](){ Controller::get().togglePreset(true); },
	// 	[](){ Controller::get().off(); }, 
	// 	3000
	// );

	// ButtonManager::add(b1);
	// ButtonManager::add(b2);
	// ButtonManager::enable();
}

void loop()
{
	vTaskDelete(NULL);
}
