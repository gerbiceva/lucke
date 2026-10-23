#pragma once
#include "Config/FixtureConfig.h"
#include "Utils/SyncBuffer.h"
#include "Interfaces/Output.h"
#include "Interfaces/Serializable.h"
#include "Interfaces/Deserializable.h"
#include <memory>
#include <vector>
#include <cstring>
#include <mutex>


class Fixture : public Interface::Serializable, public Interface::Deserializable
{
    void countPresets();
public:
    Fixture();
    Fixture(std::string name, std::string type, std::string presets);
    virtual ~Fixture() = default;

    void update();
    virtual void wifiAnimation() {};

    template<typename TOutput, typename... Args>
    TOutput& addOutput(Args&&... args)
    {
        auto& out = m_outputs.emplace_back(std::make_unique<TOutput>(std::forward<Args>(args)...));
        out->bind();
        m_lastOffset += out->footprint();
        return static_cast<TOutput&>(*out);
    }

    void setSyncBuffers(std::vector<std::shared_ptr<SyncBuffer>> bufs);
    uint16_t getFootprint() const;
    uint8_t getNumUniverses() const;
    void setUniverse(uint8_t new_universe);
    void setAddress(uint16_t new_address);
    void setPreset(uint8_t new_preset);
    void setName(const std::string& other);
    void toggleHighlight();

    void setId(uint8_t id);
    uint8_t id() const;
    uint8_t getUniverse() const;
    uint16_t getAddress() const;
    uint8_t getSelectedPreset() const;
    uint8_t getNumPresets() const;
    const std::string& getName() const;
    const std::string& getType() const;
    std::vector<std::unique_ptr<Interface::Output>>& getOutputs();


    void fromConfig();
    void fromJson(std::string json) override;
    JsonDocument toJsonDoc() override;
    JsonDocument configJson();
    JsonDocument presetsJson();
    JsonDocument outputsJson();
    void toJson(JsonObject& doc) override;

private:
    // Serializes update() (fixture task) against the mutators that reallocate
    // m_buffer / swap m_syncBuffers / repoint outputs (command task). Without
    // it, resize/swap under a live reader corrupts the heap.
    std::mutex m_mutex;
    Config::Fixture m_config;
    std::vector<std::shared_ptr<SyncBuffer>> m_syncBuffers;
    std::vector<uint8_t> m_buffer;
    std::vector<std::unique_ptr<Interface::Output>> m_outputs;
    JsonDocument m_jsonPresets;

    uint16_t m_lastOffset = 0;
};