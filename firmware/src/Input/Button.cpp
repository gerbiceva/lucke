// #include "Button.h"
// #include <Arduino.h>

// namespace Input
// {

// Button::Button(const uint8_t pin, void(*risingFunction)() = [](){}, void(*holdFunction)() = [](){}, unsigned long holdTimeMillis) 
// 	: buttonPin(pin), risingCallback(risingFunction), holdCallback(holdFunction), holdTime(holdTimeMillis) 
// {
// 	pinMode(buttonPin, INPUT_PULLUP);
// }


// void Button::setRisingFunction(void(*risingFunction)()) 
// {
// 	risingCallback = risingFunction;
// }

// void Button::setHoldFunction(void(*holdFunction)()) 
// {
// 	holdCallback = holdFunction;
// }

// const uint8_t Button::getPin() const 
// {
// 	return buttonPin; 
// }

// const bool Button::buttonStatus() const 
// {
// 	return isPressed; 
// }

// void Button::update() 
// {
// 	bool state = digitalRead(buttonPin);

// 	if(state != previousState) 
// 	{
// 		// rising
// 		if(!state) 
// 		{
// 			pressTime = millis();
// 			execute = true;
// 			isPressed = true;
// 		}
// 		// falling
// 		else 
// 		{
// 			if(millis() - pressTime <= holdTime) 
// 			{
// 				risingCallback();
// 				execute = false;
// 			}

// 			isPressed = false;
// 		}

// 		previousState = state;
// 	}

// 	// if we are holding the button long enough
// 	if(execute && millis() - pressTime > holdTime) 
// 	{
// 		holdCallback();
// 		execute = false;
// 	}
// }

// }
