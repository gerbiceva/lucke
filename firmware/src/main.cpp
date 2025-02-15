#include "Controller.h"
#include "ButtonManager.h"

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);		// Set your gateway
// IPAddress subnet(255, 255, 255, 0);	 // Set your subnet mask

std::vector<DMXPreset> asteraPresets = {
	{60, "group by 1"},
	{30, "group by 2"},
	{15, "group by 4"},
	{4, "group by 15"},
};

std::vector<DMXPreset> ledbarPresets = {
	{100, "group by 1"},
	{50, "group by 2"},
	{25, "group by 4"},
	{10, "group by 10"},
};

void setup() {	
	// initialise the contorller
#if DIMENSION == DIMENSION_1D
	Controller::get().setPresets(asteraPresets);
	Controller::get().init();
#else
	Controller::get().init2D();
#endif
	LOGF("Initialising the controller in %d\n", DIMENSION);

	// create all tasks
	Controller::createTasks();

	Button b1(
		9, 
		[](){ Controller::get().togglePreset(); },
		[](){ Controller::get().on(); },
		3000
	);
	Button b2(
		21, 
		[](){ Controller::get().togglePreset(true); },
		[](){ Controller::get().off(); }, 
		3000
	);

	ButtonManager::add(b1);
	ButtonManager::add(b2);
	ButtonManager::enable();
}

void loop()
{
	vTaskDelete(NULL);
}
