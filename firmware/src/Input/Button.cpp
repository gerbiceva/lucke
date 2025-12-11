#include "Button.h"
#include <Arduino.h>
#include "Handlers/PinHandler.h"
#include "Utils/Logger.h"

namespace Input
{

Button::Button(const std::string& name, const uint8_t pin, std::function<void()> risingFunction, std::function<void()> holdFunction, unsigned long holdTimeMillis) 
	: buttonName(name), buttonPin(pin), risingCallback(risingFunction), holdCallback(holdFunction), holdTime(holdTimeMillis) 
{
    if(Handler::PinHandler::available(buttonPin))
	{
        pinMode(buttonPin, INPUT_PULLUP);
    }
    else 
    {
        Utils::Logger::printf("Error adding button '%s': pin %d already used.\n", buttonName.c_str(), buttonPin);
    }
}

Button::Button(Button&& other) noexcept
    : buttonName(other.buttonName),       // copy const member
      buttonPin(other.buttonPin),       // copy const member
      holdTime(other.holdTime),
      pressTime(other.pressTime),
      execute(other.execute),
      isPressed(other.isPressed),
      previousState(other.previousState),
      risingCallback(other.risingCallback),
      holdCallback(other.holdCallback)
{
    // Put the moved-from object into a safe state
    other.pressTime = 0;
    other.execute = false;
    other.isPressed = false;
    other.previousState = true;

    other.risingCallback = [](){};
    other.holdCallback   = [](){};
}



void Button::setRisingFunction(std::function<void()> risingFunction)
{
	risingCallback = risingFunction;
}

void Button::setHoldFunction(std::function<void()> holdFunction) 
{
	holdCallback = holdFunction;
}

const uint8_t Button::getPin() const 
{
	return buttonPin; 
}

const bool Button::buttonStatus() const 
{
	return isPressed; 
}

void Button::update() 
{
	bool state = digitalRead(buttonPin);

	if(state != previousState) 
	{
		// rising
		if(!state) 
		{
			pressTime = millis();
			execute = true;
			isPressed = true;
		}
		// falling
		else 
		{
			if(millis() - pressTime <= holdTime) 
			{
				risingCallback();
				execute = false;
			}

			isPressed = false;
		}

		previousState = state;
	}

	// if we are holding the button long enough
	if(execute && millis() - pressTime > holdTime) 
	{
		holdCallback();
		execute = false;
	}
}

JsonDocument Button::describe() 
{
    JsonDocument doc;
    doc["name"] = buttonName;
    doc["pin"] = buttonPin;
    return doc;
}

}
