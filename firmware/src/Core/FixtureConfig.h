#pragma once
#include <string>
#include "Utils/Storage.h"

namespace Core
{
    namespace Fixture
    {

        class FixtureConfig
        {
            static uint8_t s_ID;
        protected:
            Utils::Storage m_storage;
        public:
            uint8_t m_ID = 0;
            uint8_t universe = 1U;
            uint16_t address = 0U;
            uint8_t selectedPreset = 0U;
        
            std::string name = "name";
            std::string type = "type";
        
            FixtureConfig(std::string fixtureName, std::string fixtureType);
        };
    }
}
