#include "FixtureHandler.h"
#include "Utils/Wifi.h"
#include "Fixture/Fixture.h"
#include "Output/HardwareLED.h"

std::vector<Fixture*> FixtureHandler::fixtures;

void FixtureHandler::addFixture(Fixture* fixture)
{
    fixtures.push_back(fixture);
}

Fixture* FixtureHandler::get(uint8_t index)
{
    if(index > fixtures.size())
    {
        Utils::Logger::printf("Error: Fixture at index %d does not exist/n", index);
        return nullptr;
    }

    return fixtures[index];
}

std::vector<Fixture*>& FixtureHandler::allFixtures()
{
    return fixtures;
}

void FixtureHandler::updateTask()
{
    for(Fixture* fix : fixtures)
    {
        fix->update();
    }
        
}