#include "Controller.h"
#include "ButtonManager.h"

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);		// Set your gateway
// IPAddress subnet(255, 255, 255, 0);	 // Set your subnet mask

// thread that sends reports via udp
void statReportLoop(void *) {
	while (true) {
		Controller::get().sendReport();
		vTaskDelay(1000);
	}
}

// thread that plays idle animation (wifi connecting)
void playIdleAnimation(void *) {
	while (true) {
		Controller::get().playIdleAnimation();
		vTaskDelay(WIFI_DELAY);
	}
}

// thread that checks if wifi connected
void checkNetwork(void *) {
	while (true) {
		// if not connected
		if (WiFi.status() != WL_CONNECTED) {
			LOG("Lost connection\n");

			TaskHandle_t animation = NULL;
			xTaskCreate(
				playIdleAnimation, 		// Task function
				"Animation",			// Name of the task (for debugging)
				2000,					// Stack size in words
				NULL,					// Parameter passed to the task
				2,						// Task priority
				&animation				// Handle to the task
			);

			while (WiFi.status() != WL_CONNECTED) {
				vTaskDelay(50);
			}

			// connection established
			LOG("Connected\n");
			vTaskDelete(animation);
			Controller::get().clear();
		}

		vTaskDelay(50);
	}
}

// main dmx loop
void dmxLoop(void *) {
	while (true) {
		Controller::get().updateLoop();
		vTaskDelay(15);
	}
}


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
	xTaskCreate(dmxLoop, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
	xTaskCreate(checkNetwork, "Wifi check", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
	xTaskCreate(statReportLoop, "Logging", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL);


	Button b1(9, 
	[](){ Controller::get().togglePreset(); },
	[](){ 
		Controller::get().on(); 
	}, 3000);
	Button b2(21, 
	[](){ Controller::get().togglePreset(true); },
	[](){ 
		Controller::get().off(); 
	}, 3000);

	ButtonManager::add(b1);
	ButtonManager::add(b2);
	ButtonManager::enable();
}

void loop()
{
	vTaskDelete(NULL);
}
