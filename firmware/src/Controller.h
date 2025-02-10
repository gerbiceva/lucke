#pragma once
#include "Config.h"
#include <FastLED.h>
#include "WiFi.h"
#include <esp_wifi.h>
#include "sACN.h"
#include <ArduinoJson.h>
#include <queue>
#include <vector>


#if DIMENSION == DIMENSION_2D

struct Grid {
	int width = GRID_WIDTH;
	int heigth = GRID_HEIGHT;
	int wsize = GRID_WSIZE;
	int hsize = GRID_HSIZE;
	int nw, nh;

	std::unordered_map<int, std::vector<int>> hash;

	Grid() : nw(width / wsize), nh(heigth / hsize) {};
	Grid(int wsize, int hsize, int width, int height) : wsize(wsize), hsize(hsize), nh(heigth / hsize) {};

	const std::vector<int> &getGridIndexes(int x, int y) {
    	// TODO: Find bigger prime numbers or better hashing
		int hnum = x * 7741 + y * 7757;
		
		if(hash.find(hnum) != hash.end()) {
			return hash[hnum];
		}

		for(int yi = y * hsize; yi < (y + 1) * hsize; yi++) {
			for(int xi = x * wsize; xi < (x + 1) * wsize; xi++) {
				hash[hnum].push_back(xi + yi * width);
			}
		}

		return hash[hnum];
	}
};

#endif

struct DMXPreset {
	uint16_t numOfGroups;
	std::string description;

	DMXPreset(uint16_t numberOfGroups, std::string description = "") : numOfGroups(numberOfGroups), description(description) {}
};

class Controller {
	Controller() {}

	void setupWifi();
	void setupSacn();

	// transfer data from dmx to ledbuffer (group if necesarry)
	void update();

	// thread that updates dmx, basically main thread
	static void dmxLoop(void *) {
		while (true) {
			Controller::get().updateLoop();
			vTaskDelay(15);
		}
	}

	// thread that sends reports via udp
	static void statReportLoop(void *) {
		while (true) {
			Controller::get().sendReport();
			vTaskDelay(1000);
		}
	}

	// thread that plays idle animation (wifi connecting)
	static void playIdleAnimation(void *) {
		while (true) {
			Controller::get().playIdleAnimation();
			vTaskDelay(WIFI_DELAY);
		}
	}

	// thread that checks if wifi connected
	static void checkNetwork(void *) {
		while (true) {
			// if not connected
			if (WiFi.status() != WL_CONNECTED) {
				LOG("Lost connection\n");

				TaskHandle_t animation = NULL;
				xTaskCreate(
					Controller::playIdleAnimation, 		// Task function
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

public:
	Controller(const Controller& other) = delete;

	// returns singleton instance
	static Controller& get() {
		static Controller instance;
    	return instance;
	}

  // main init function; class can be reinitialised
#if DIMENSION == DIMENSION_1D
	void init(uint8_t uni = UNIVERSE, uint16_t dmxAddressOffset = ADDR_OFFSET);
#else
	void init2D(
		int wsize = GRID_WSIZE, 
		int hsize = GRID_HSIZE, 
		int width = GRID_WIDTH, 
		int height = GRID_HEIGHT,
		uint8_t uni = UNIVERSE, 
		uint16_t dmxAddressOffset = ADDR_OFFSET); 
#endif
	
	void setPresets(std::vector<DMXPreset> newPresets) { presets = newPresets; }
	// retrieve dmx data
	void updateLoop();

	// wifi connect annimation
	void playIdleAnimation();

	// functions for sending repot
	void clearDiffQueue(JsonArray& jarray);
	void sendUdpPacket(JsonDocument& doc);
	void sendReport();

	// ledstrip interactions
	void clear() { memset(ledBuffer, 0, LED_SIZE); FastLED.show(); }
	void togglePreset(bool reverse = false);

	// threading functions
	void newPacket();
	void printNewRecv();
	void updateFramerate();
	void seqDiff();

	static void createTasks() {
		static bool tasksCreated = false;
		
		if(tasksCreated)
			return;

		xTaskCreate(Controller::dmxLoop, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
		xTaskCreate(Controller::checkNetwork, "Wifi check", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
		xTaskCreate(Controller::statReportLoop, "Logging", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL);

		tasksCreated = true;
	}

	volatile bool enabled = true;
	void off() { enabled = false; }
	void on() { enabled = true; }

private:
	uint8_t universe = UNIVERSE;
	uint16_t dmxAddrOffset = ADDR_OFFSET;

	uint16_t numGroups = NUM_GROUPS;
	std::string name = FIXTURE_NAME;
	
	int8_t presetIndex = 0;
	std::vector<DMXPreset> presets = {
		{NUM_LEDS, "grouped by numLeds"},
	};

	uint8_t ledBuffer[LED_SIZE] = {};
	uint8_t dmxBuffer[DMX_SIZE] = {};	

	int droppedPackets = 0;
	int lastDMXFramerate = 0;
	std::queue<uint8_t> packetDiff;

	CLEDController *cled;
	SemaphoreHandle_t mutex;

	WiFiUDP udp;
	Receiver* recv;
	
#if DIMENSION == DIMENSION_2D
	Grid grid;
#endif
};
