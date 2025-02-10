#include "Controller.h"
#include "ButtonManager.h"

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);		// Set your gateway
// IPAddress subnet(255, 255, 255, 0);	 // Set your subnet mask


void setup() {	
	// initialise the contorller
#if DIMENSION == DIMENSION_1D
	Controller::get().init();
	Controller::get().setPresets({
		{10, "grouped by 1"},
		{5, "grouped by 2"},
		{2, "grouped by 5"},
		{1, "grouped by 10"},
	});
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
