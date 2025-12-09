#pragma once
#include <vector>
#include "Traits/Outputs.h"
#include "Traits/Inputs.h"
#include "Traits/Serializable.h"
#include "Utils/Storage.h"

class Fixture : public Traits::Serializable
{
    struct Config
    {
        uint8_t universe = 9U;
        uint16_t address = 0U;
        uint8_t selectedPreset = 0U;

        std::string name = "name";
        std::string type = "type";
    };

    static uint8_t s_ID;
    uint8_t m_ID;

    Config m_config;

    std::vector<Traits::OutputInterface*> m_outputs;
    uint8_t* m_srcBuffer;
    uint16_t m_lastOffset = 0U;
    JsonDocument jsonPreset;

    Utils::Storage* m_storage;

    void loadIfExist();
    void init();
    void updatePresets();
public:

    Fixture();
    Fixture(std::string name, std::string type, std::string presets);


    template<typename TOutput, typename... Args>
    void addOutput(Args&&... args)
    {
        m_outputs.push_back(new TOutput(args...));
        m_outputs.back()->bind();

        m_outputs.back()->setSrcBuffer(m_srcBuffer + m_config.address + m_lastOffset);
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