#pragma once
#include <cstdint>
#include "Traits/Serializable.h"

namespace Input 
{

class Button : public Traits::Serializable 
{
    std::string buttonName;
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
	explicit Button(const std::string& name, const uint8_t pin, void(*risingFunction)() = [](){}, void(*holdFunction)() = [](){}, unsigned long holdTimeMillis = 1000);
	Button(Button&& other) noexcept;

	void update();

	inline void setRisingFunction(void(*risingFunction)());
	inline void setHoldFunction(void(*holdFunction)());

	inline const uint8_t getPin() const;
	inline const bool buttonStatus() const;

    JsonDocument describe() override;
};

}
