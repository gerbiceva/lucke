#pragma once
#include <vector>
#include "Traits/Outputs.h"
#include "Traits/Inputs.h"
#include "Traits/Serializable.h"
#include "Traits/Deserializable.h"
#include <memory>

#include "Config/FixtureConfig.h"

class Fixture : public Traits::Serializable, public Traits::Deserializable
{

    void obtainSrcBuffer();
    void configureOutput(Traits::OutputInterface* output);
    
public:

    Fixture();
    Fixture(FixtureConfig config, std::string presets);
    Fixture(std::string name, std::string type, std::string presets);

    virtual ~Fixture() = default;

    template<typename TOutput, typename... Args>
    void addOutput(Args&&... args)
    {
        configureOutput(new TOutput(args...));
        // m_outputs.push_back(new TOutput(args...));
        // m_outputs.back()->bind();

        // m_outputs.back()->setSrcBuffer(m_srcBuffer + m_config.address + m_lastOffset);
        // m_lastOffset += m_outputs.back()->getSize();
        // updatePresets();
    }

    void updatePresets();

    uint8_t* getSrcBuffer();
    uint8_t* getOffsetSrcBuffer();
    uint8_t id() const;
    const std::string& getName() const;
    const std::string& getType() const;

    void setUniverse(uint8_t new_universe);
    void setAddress(uint16_t new_address);
    void setPreset(uint8_t new_preset);
    void setName(const std::string& other);

    virtual void update();
    virtual void wifiAnimation() = 0;
    virtual void highlight() {};

    std::string getSelectedPresetName();

    void getPresets(JsonObject& obj);
    void fromJson(std::string json) override;
    JsonDocument toJsonDoc() override;
    void toJson(JsonObject& doc) override;
    void toJsonFull(JsonObject& doc) override;

    bool m_isHighlighted = false;
private:
    static uint8_t s_ID;
    uint8_t m_ID;

    FixtureConfig m_config;

    std::shared_ptr<Traits::InputInterface> m_dmxIn;
    std::vector<Traits::OutputInterface*> m_outputs;
    
    uint8_t* m_srcBuffer;
    uint16_t m_lastOffset = 0U;
    JsonDocument jsonPreset;
};