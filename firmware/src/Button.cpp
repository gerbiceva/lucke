#include "Button.h"

void Button::update() {
	bool state = digitalRead(buttonPin);
	bool executed = false;

	if(state != previousState) {
		// rising
		if(!state) {
			pressTime = millis();
			execute = true;
			isPressed = true;
		}
		// falling
		else {
			if(millis() - pressTime <= holdTime) {
				risingCallback();
				execute = false;
			}

			isPressed = false;
		}

		previousState = state;
	}

	// if we are holding the button long enough
	if(execute && millis() - pressTime > holdTime) {
		holdCallback();
		execute = false;
	}
}
