#pragma once
#include <vector>
#include <cstdint>
#include "Input/InputHandler.h"
#include <atomic>
// #include "Fixture.h"

class Fixture;

class FixtureHandler
{
    static std::vector<Fixture*> fixtures;
    static std::atomic<bool> connected;
public:
    static void addFixture(Fixture* fixture)
    {
        fixtures.push_back(fixture);
    }

    static Fixture* get(uint8_t id)
    {
        return fixtures[id];
    }

    // static void animateConnecting(void*)
    // {
    //     static uint16_t off = 0;
	// astera60.getSrcBuffer()[off] = 255;
	// astera60.getSrcBuffer()[off == 0 ? 179 : off - 1] = 0;
	// off = (off + 1) % 180;
	// astera60.update();
	// Output::updateFastLED();
    // }

    static void checkNetwork(void*);

    static void updateTask(void*);

    static void wirelessConfig(void*);
};