#pragma once
#include "Output/HardwareLED.h"
#include "Preset.h"
#include "FixtureConfig.h"

#include <vector>

namespace Core
{
    namespace Fixture
    {
	    // template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
        template<typename TPresetDim>
        class BaseClass : public Output::HardwareLED, public FixtureConfig
        {
        protected:
            uint8_t* m_sourceBuffer;
            std::vector<TPresetDim> m_presets;
        public:
            BaseClass(std::string name, 
                    std::string type, uint16_t num_leds,
                    std::vector<TPresetDim> presets)
                : Output::HardwareLED(num_leds), FixtureConfig(name, type),
                m_presets(presets)
            {
    
            }
    
            virtual void update() = 0;
    
            void setUniverse(uint8_t universe) override
            {}
        };
    }
};