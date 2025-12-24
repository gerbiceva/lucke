#pragma once
#include <cstdint>
#include "Traits/Deserializable.h"
#include <functional>

namespace Input 
{

class Button : public Traits::Deserializable 
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

	void setRisingFunction(std::function<void()> risingFunction);
	void setHoldFunction(std::function<void()> holdFunction);

	const std::string& getName() const;
	const uint8_t getPin() const;
	const bool buttonStatus() const;

    void toJson(JsonObject& doc) override;
};

}
