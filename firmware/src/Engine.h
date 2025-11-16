#pragma once
#include "Fixtures.h"
#include "Handlers/FixtureHandler.h"


class Engine
{
    
    Engine ();
    
public:
    static Fixture* wifiAnimFix;
    static Engine& instance();

    template<typename TFixture>
    Fixture* addFixture(bool animateWifiConnecting = false)
    {
        Fixture* fix = FixtureHandler::addFixture<TFixture>();
        if(animateWifiConnecting)
        {
            wifiAnimFix = fix;
        }

        return fix;
    }

    Fixture* getFixture(uint16_t index);
    Fixture* operator[](uint16_t index);

    static void update(void*);
};