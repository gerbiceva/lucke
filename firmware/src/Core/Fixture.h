#pragma once
#include <vector>
#include "FixtureConfig.h"
#include "Traits/Outputs.h"
#include "Traits/Inputs.h"
#include "Traits/Serializable.h"

class Fixture : public Core::Fixture::FixtureConfig, public Traits::Serializable
{
    std::vector<Traits::OutputInterface*> m_outputs;
    uint8_t* m_srcBuffer;
    uint16_t m_lastOffset = 0U;
    JsonDocument jsonPreset;

    void updatePresets();
public:

    Fixture();
    Fixture(std::string name, std::string type, std::string presets);


    template<typename TOutput, typename... Args>
    void addOutput(Args&&... args)
    {
        m_outputs.push_back(new TOutput(args...));
        m_outputs.back()->bind();

        m_outputs.back()->setSrcBuffer(m_srcBuffer + address + m_lastOffset);
        m_lastOffset += m_outputs.back()->getSize();
        updatePresets();
    }

    uint8_t* getSrcBuffer() { return m_srcBuffer; }

    void setUniverse(uint8_t new_universe);
    void setAddress(uint16_t new_address);
    void setPreset(uint8_t new_preset);
    void setName(const std::string& other);

    void clear();
    virtual void update();
    virtual void wifiAnimation() {};

    JsonDocument describe() override;
};