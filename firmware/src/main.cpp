#include "Controller.h"
#include "ButtonManager.h"

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);		// Set your gateway
// IPAddress subnet(255, 255, 255, 0);	 // Set your subnet mask

struct LampAstera : public Lamp {
    LampAstera() : Lamp("Astera", 60, {
		{1, "group by 1"},
		{2, "group by 2"},
		{4, "group by 4"},
		{15, "group by 15"}
	}) {};
};

struct LampAstera144 : public Lamp {
    LampAstera144() : Lamp("Astera", 144, {
		{1, "group by 1"},
		{2, "group by 2"},
		{4, "group by 4"},
		{15, "group by 15"}
	}) {};
};

struct LampLedbar : public Lamp {
    LampLedbar() : Lamp("Ledbar", 100, {
		{1, "group by 1"},
		{2, "group by 2"},
		{4, "group by 4"},
		{10, "group by 10"}
	}) {};
};

struct LampLedbar2m : public Lamp {
    LampLedbar2m() : Lamp("Ledbar2m", 120, {
		{1, "group by 1"},
		{2, "group by 2"},
		{4, "group by 4"},
		{10, "group by 10"}
	}, "Trak") {};
};

struct LampBlinder : public Lamp {
    LampBlinder() : Lamp("Blinder", (8*24), {
		{1, "group by 1"},
		{6, 4, "group by 4x2"},
		{6, 8, "group by 4x1"},
		{2, 2, "group by 12z8"}
	}, "Blinder") {};
};

void setup() {	
	// initialise the contorller
	Controller::get().setLamp(new LampLedbar2m());
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
