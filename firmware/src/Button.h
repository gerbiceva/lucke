#pragma once
#include "Config.h"


class Button {
	const uint8_t buttonPin;
	unsigned long holdTime;

	unsigned long pressTime = 0;
	bool execute = false;
	bool isPressed = false;
	bool previousState = true;

	// callback functions
	void(*risingCallback)();
	void(*holdCallback)();

public:
	Button(const uint8_t pin, void(*risingFunction)() = [](){}, void(*holdFunction)() = [](){}, unsigned long holdTimeMillis = DEFAULT_BUTTON_HOLD_TIME) 
		: buttonPin(pin), risingCallback(risingFunction), holdCallback(holdFunction), holdTime(holdTimeMillis) {
		
		pinMode(buttonPin, INPUT_PULLUP);
	}

	void update();
	const bool buttonStatus() const { return isPressed; }
};
