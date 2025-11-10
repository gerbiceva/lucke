#include "FixtureConfig.h"

namespace Core
{
    namespace Fixture
    {
        uint8_t FixtureConfig::s_ID = 0;
        
        FixtureConfig::FixtureConfig(std::string fixtureName, std::string fixtureType)
            : name(fixtureName), type(fixtureType)
        {
            m_ID = s_ID++;
            m_storage.setPartition(std::to_string(m_ID));
        
            if(m_storage.isKey("universe"))
            {
                universe = m_storage.getUChar("universe");
                address = m_storage.getUShort("address");
                selectedPreset = m_storage.getUChar("preset_index");
                name = m_storage.getString("name");
            }
            else 
            {
                m_storage.putUChar("universe", universe);
                m_storage.putUShort("address", address);
                m_storage.putUChar("preset_index", selectedPreset);
                m_storage.putString("name", name);
            }

            // setupDefulat();
            
        }
    }
}
