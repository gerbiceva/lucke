#pragma once
#include "Button.h"
#include <vector>

class ButtonManager {
	static std::vector<Button> buttons;
	// std::unordered_map<uint8_t, bool> isPinRegistered;

    ButtonManager() {}
	
    static void update(void*) {
        while(true) {
            for(auto& b : buttons) {
                b.update();
            }

            vTaskDelay(20);
        }
	}
public:
    ButtonManager(const ButtonManager& other) = delete;

	// // returns singleton instance
	// static ButtonManager& get() {
	// 	static ButtonManager instance;
    // 	return instance;
	// }

	static void add(Button b) {
		// if(isPinRegistered.find(b.getPin()))
		buttons.push_back(b);
	}


    static void enable() {
        static bool enabled = false;

        if(!enabled) {
            xTaskCreate(ButtonManager::update, "Input", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
            enabled = true;
        }
    }

    
};