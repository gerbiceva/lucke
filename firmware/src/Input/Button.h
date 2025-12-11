#pragma once
#include <cstdint>
#include "Traits/Serializable.h"
#include <functional>

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
	std::function<void()> risingCallback;
	std::function<void()> holdCallback;

public:
	explicit Button(const std::string& name, const uint8_t pin, std::function<void()> risingFunction = [](){}, std::function<void()> holdFunction = [](){}, unsigned long holdTimeMillis = 1000);
	Button(Button&& other) noexcept;

	void update();

	inline void setRisingFunction(std::function<void()> risingFunction);
	inline void setHoldFunction(std::function<void()> holdFunction);

	inline const uint8_t getPin() const;
	inline const bool buttonStatus() const;

    JsonDocument describe() override;
};

}
