#include "Controller.h"

// bool Controller::connected = false;
volatile bool Controller::connected = false;
Lamp* Controller::m_Lamp = nullptr;

#if DIMENSION == DIMENSION_1D
void Controller::init(uint8_t uni, uint16_t dmxAddressOffset, int8_t presetIndex) {
	m_Universe = uni;
	m_Address = dmxAddressOffset;
	this->m_SelectedPreset = presetIndex;
	m_NumGroups = m_Lamp->presets[presetIndex].numOfGroups;
	static bool inited = false;

	prefs.begin("dmxConfig");

	if(!inited) {
	#ifdef ENABLE_LOGGING
		Serial.begin(BAUD_RATE);
	#endif
		if(prefs.isKey("universe")) {
			m_Universe = prefs.getUChar("universe");
			m_Address = prefs.getUShort("address");
			presetIndex = prefs.getChar("preset");
			m_NumGroups = m_Lamp->presets[presetIndex].numOfGroups;
		}
		else {
			prefs.putUChar("universe", uni);
			prefs.putUShort("address", dmxAddressOffset);
			prefs.putChar("preset", presetIndex);
		}

		setupWifi();
		setupSacn();
		m_LedBuffer = new uint8_t[m_Lamp->getLedSize()];

		mutex = xSemaphoreCreateMutex();
		cled = &FastLED.addLeds<LED_TYPE, HARDWARE_DATA_PIN, LED_ORDER>((CRGB *)m_LedBuffer, m_Lamp->numLeds);

		inited = true;
	}

	// Reinitialization means web config update
	else {
		prefs.putUChar("universe", uni);
		prefs.putUShort("address", dmxAddressOffset);
		prefs.putChar("preset", presetIndex);

		delete recv;
		setupSacn();
	}

	prefs.end();
}
#else
void Controller::init2D(int wsize, int hsize, int width, int height, uint8_t uni, uint16_t dmxAddressOffset) {
	universe = uni;
	dmxAddrOffset = dmxAddressOffset;
	static bool inited = false;

	if(!inited) {
	#ifdef ENABLE_LOGGING
		Serial.begin(BAUD_RATE);
	#endif
		setupWifi();
		setupSacn();

		mutex = xSemaphoreCreateMutex();
		cled = &FastLED.addLeds<LED_TYPE, HARDWARE_DATA_PIN, LED_ORDER>((CRGB *)ledBuffer, NUM_LEDS);

		inited = true;
	}

	// Reinitialization means web config update
	else {
		delete recv;
		grid = Grid(wsize, hsize, width, height);
		setupSacn();
	}
}
#endif

void Controller::setupWifi() {
	// setup mac address
	uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x14, m_Universe}; // MAC Adress of your device
	esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &mac[0]);
	if (err == ESP_OK)
	{
		LOG("Success changing Mac Address\nS");
	}

	WiFi.setScanMethod(WIFI_FAST_SCAN);
	WiFi.mode(WIFI_STA);
	WiFi.setSleep(false);
	WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void Controller::setupSacn() {
	// setup sacn
	recv = new Receiver(udp);
	recv->callbackDMX([](){ Controller::get().newPacket(); });
	recv->callbackSource([](){ Controller::get().printNewRecv(); });
	recv->callbackFramerate([](){ Controller::get().updateFramerate(); });
	recv->callbackSeqDiff([](){ Controller::get().seqDiff(); });
	recv->callbackTimeout([](){});
	recv->begin(m_Universe);
}

#if DIMENSION == DIMENSION_1D

void Controller::update() {
	if(!enabled) return;

	uint16_t groupSize = m_Lamp->numLeds / m_NumGroups;
	uint16_t ledIndex = 0;

	for (uint16_t i = 0; i < m_NumGroups; i++) {
		for (uint16_t j = 0; j < groupSize; j++) {
			for (uint16_t k = 0; k < m_Lamp->numPxls; k++) {
				// check if in bounds
				uint16_t dmxBufferIndex = m_Address + i * m_Lamp->numPxls + k;
				m_LedBuffer[ledIndex] = m_DmxBuffer[dmxBufferIndex];
				ledIndex++;
			}
		}
	}
}

#else

void Controller::update(){
	if(!enabled) return;

	for(uint16_t y = 0; y < grid.nh; y++) {
		for(uint16_t x = 0; x < grid.nw; x++) {
			const auto& indexes = grid.getGridIndexes(x,y);
			int dmxIndex = dmxAddrOffset + (x * lamp->numPxls) + (y * lamp->numPxls) * grid.nw;
			// printf("dmxIndex = %d\n", dmxIndex);
			for(auto index : indexes) {
				for (uint16_t k = 0; k < lamp->numPxls; k++) {
					// LOGF("led[%d] = dmx [%d]\n", (index * lamp->numPxls + k), (dmxIndex + k));
					ledBuffer[index * lamp->numPxls + k] = dmxBuffer[dmxIndex + k]; 
				}
			}
		}
	}
}

#endif

void Controller::updateLoop() {
	recv->update();
	FastLED.show();
}


void Controller::playIdleAnimation() {
	static unsigned long iterator = 0;
	m_LedBuffer[(iterator) % m_Lamp->getLedSize()] = WIFI_BRIGHTNESS;
	m_LedBuffer[(iterator++ - 1) % m_Lamp->getLedSize()] = 0;	
}


void Controller::clearDiffQueue(JsonArray& jarray) {
	if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
		while (!packetDiff.empty()) {
			jarray.add(packetDiff.front());
			packetDiff.pop();
		}
		xSemaphoreGive(mutex);
	}
}

void Controller::sendUdpPacket(JsonDocument& doc) {
	udp.beginPacket(WiFi.broadcastIP(), 12345);
	serializeJson(doc, udp);
	udp.endPacket();
}

void Controller::sendReport() {
	JsonDocument doc;
	doc["type"] = m_Lamp->type;
	doc["name"] = m_Lamp->name;
	doc["num_leds"] = m_Lamp->numLeds;
	doc["universe"] = m_Universe;
	doc["address"] = m_Address;
	doc["heap_size"] = ESP.getHeapSize();
	doc["heap_free"] = ESP.getFreeHeap();
	doc["local_ip"] = WiFi.localIP();
	doc["WIFI_SSID"] = WiFi.SSID();
	doc["rssi"] = WiFi.RSSI();
	doc["last_DMX_framerate"] = lastDMXFramerate;

	// add packet sequence diff to json
	doc["seq_diff"] = JsonDocument();
	JsonArray diffArray = doc["seq_diff"].to<JsonArray>();
	clearDiffQueue(diffArray);

	// add first 5 leds
	doc["first_5_leds"] = JsonDocument();
	JsonArray jsonArray = doc["first_5_leds"].to<JsonArray>();
	for (int i = 0; i < 15; i++) {
		jsonArray.add(m_LedBuffer[i]);
	}

	sendUdpPacket(doc);
}

void Controller::setUniverse(uint8_t uni) {
	m_Universe = uni;
	setupSacn();
}

void Controller::setAddress(uint16_t address) {
	m_Address = address;
}

void Controller::setPreset(uint8_t preset) {
	this->m_SelectedPreset = preset;
	m_NumGroups = m_Lamp->presets[preset].numOfGroups;
}

void Controller::setName(std::string name) {
	m_Lamp->name = name;
}

void Controller::togglePreset(bool reverse) {
	m_SelectedPreset = (m_SelectedPreset + (reverse ? -1 : 1)) % m_Lamp->presets.size();
	m_NumGroups = m_Lamp->presets[m_SelectedPreset].numOfGroups;
}


void Controller::newPacket() {
	recv->dmx(m_DmxBuffer);
	update();
}

void Controller::printNewRecv() {
	LOGF("%s\n", recv->name());
}

void Controller::updateFramerate() {
	lastDMXFramerate = recv->framerate();
}

void Controller::seqDiff() {
	uint8_t diff = recv->seqdiff();
	if (xSemaphoreTake(mutex, 0) == pdTRUE)	{
		packetDiff.push(diff);
		xSemaphoreGive(mutex);
	}
}
