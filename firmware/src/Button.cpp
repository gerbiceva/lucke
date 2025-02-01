#include "Button.h"

void Button::update() {
    static bool isHeld = false;
    static bool prevState = digitalRead(buttonPin);
    bool state = digitalRead(buttonPin);

    static unsigned long pressStartTime = 0;
    static unsigned long prevTime = millis();
    unsigned long currentTime = millis();
    
    // Debounce logic
    if(state != prevState){
        prevTime = currentTime;
    }

    // If we didnt bounce
    if((currentTime - prevTime) > DEBOUNCE_TIME) {
        // Rising
        if (!prevState && state) {
            pressStartTime = currentTime;
            isHeld = false;
            risingCallback();
        }
        // Falling
        else if(prevState && !state && !isHeld){
            fallingCallback();
        }
    }

    // Hold check
    if(!state && (currentTime - pressStartTime) >= holdTime && !isHeld) {
        isHeld = true;
        holdCallback();
    }

    prevState = state;
}
