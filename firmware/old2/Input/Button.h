#pragma once
#include <cstdint>
#include <Arduino.h>
// #include <function>

namespace Input 
{

	class Button {
		static constexpr unsigned long k_buttonHoldTime = 100;
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
		Button(const uint8_t pin, void(*risingFunction)() = [](){}, void(*holdFunction)() = [](){}) 
			: buttonPin(pin), risingCallback(risingFunction), holdCallback(holdFunction) {
			
			pinMode(buttonPin, INPUT_PULLUP);
		}
	
		void update();
	
		inline void setRisingFunction(void(*risingFunction)()) {
			risingCallback = risingFunction;
		}
	
		inline void setHoldFunction(void(*holdFunction)()) {
			holdCallback = holdFunction;
		}
	
		inline const uint8_t getPin() const { return buttonPin; }
		inline const bool buttonStatus() const { return isPressed; }
	};
}

