#pragma once
#include "Fixture/Base/Fixture.h"

namespace Core
{
    namespace Fixture
    {
        class Fixture1D : public BaseClass<Preset1D>
        {
            uint16_t m_numGroups = 1;
        public:
            Fixture1D(std::string name, std::string type, uint16_t numLeds, std::vector<Preset1D> presets)
                : BaseClass(name, type, numLeds, presets)
            {}

            void update() override
            {
                uint16_t groupSize = m_numLeds / m_numGroups;
                uint16_t ledIndex = 0;

                for (uint16_t i = 0; i < m_numGroups; i++) {
                    for (uint16_t j = 0; j < groupSize; j++) {
                        for (uint16_t k = 0; k < m_numPxls; k++) {
                //             // check if in bounds
                            uint16_t dmxBufferIndex = address + i * m_numPxls + k;
                            m_ledBuffer[ledIndex] = m_sourceBuffer[dmxBufferIndex];
                            ledIndex++;
                        }
                    }
                }
            }

            void setPreset(uint8_t newPreset)
            {
                selectedPreset = newPreset;
                m_storage.putUChar("preset_index", selectedPreset);
                m_numGroups = m_presets[newPreset % m_presets.size()].nGroups;
                // TODO: add presets
            }
        };
    }
}