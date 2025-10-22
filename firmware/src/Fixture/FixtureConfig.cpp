#include "FixtureConfig.h"

uint8_t FixtureConfig::s_ID = 0;

FixtureConfig::FixtureConfig(std::string fixtureName)
    : name(fixtureName)
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
}
