#include "Fixture1D.h"

Fixture1D::Fixture1D(std::string name, std::string type, uint16_t numLeds, std::vector<Preset1D> presets)
    : Fixture(name, type, numLeds), m_presets(presets)
{}

void Fixture1D::update()
{
    uint16_t groupSize = m_numLeds / m_numGroups;
    uint16_t ledIndex = 0;

    for (uint16_t i = 0; i < m_numGroups; i++) {
        for (uint16_t j = 0; j < groupSize; j++) {
            for (uint16_t k = 0; k < k_numPxls; k++) {
    //             // check if in bounds
                uint16_t dmxBufferIndex = address + i * k_numPxls + k;
                m_ledBuffer[ledIndex] = m_dmxBuffer[dmxBufferIndex];
                ledIndex++;
            }
        }
    }
}

void Fixture1D::setPreset(uint8_t newPreset)
{
    selectedPreset = newPreset;
    m_numGroups = m_presets[newPreset % m_presets.size()];
    // TODO: add presets
}