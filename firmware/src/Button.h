#pragma once
#include <stdint.h>
#include <Arduino.h>

#define DEBOUNCE_TIME 50

class Button {
    const uint8_t buttonPin;
    uint16_t holdTime;
    
    // callback functions
    void(*risingCallback)();
    void(*fallingCallback)();
    void(*holdCallback)();

public:
    Button(const uint8_t pin, void(*risingFunction)() = [](){}, void(*fallingFunction)() = [](){}, void(*holdFunction)() = [](){}, uint16_t holdTimeMillis = 1000) 
        : buttonPin(pin), risingCallback(risingFunction), fallingCallback(fallingFunction), holdCallback(holdFunction), holdTime(holdTimeMillis) {
        
        pinMode(buttonPin, INPUT_PULLUP);
    }

    void update();
};
