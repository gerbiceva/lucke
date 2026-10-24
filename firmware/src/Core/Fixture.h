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

// Snapshot of a fixture, safe to hand to another task.
struct FixtureInfo
{
    Config::Fixture config;
    uint16_t footprint = 0;

    void toJson(JsonObject& obj)
    {
        config.toJson(obj);
        obj["footprint"] = footprint;
    }
};

class Fixture : public Interface::Serializable, public Interface::Deserializable
{
    void countPresets();
public:
    Fixture();
    Fixture(std::string name, std::string type, std::string presets);
    virtual ~Fixture() = default;

    void update();
    virtual void wifiAnimation() {};

    template<std::derived_from<Interface::Output> TOutput, typename... Args>
    TOutput& addOutput(Args&&... args)
    {
        auto& out = m_outputs.emplace_back(std::make_unique<TOutput>(std::forward<Args>(args)...));
        m_lastOffset += out->footprint();
        return static_cast<TOutput&>(*out);
    }

    // Holds the fixture lock across several calls, so a change and the
    // resubscribe it needs look like one step to the render task.
    [[nodiscard]] std::unique_lock<std::recursive_mutex> lock();

    void setSyncBuffers(std::vector<std::shared_ptr<SyncBuffer>> bufs);
    [[nodiscard]] uint16_t getFootprint() const;
    [[nodiscard]] uint8_t getNumUniverses() const;
    void setUniverse(uint16_t new_universe);
    void setAddress(uint16_t new_address);
    void setPreset(uint8_t new_preset);
    void setName(const std::string& other);
    void toggleHighlight();

    void setId(uint8_t id);
    [[nodiscard]] uint8_t id() const;
    [[nodiscard]] uint16_t getUniverse() const;
    [[nodiscard]] uint16_t getAddress() const;
    [[nodiscard]] uint8_t getSelectedPreset() const;
    [[nodiscard]] uint8_t getNumPresets() const;
    [[nodiscard]] std::string getName() const;
    [[nodiscard]] const std::string& getType() const;
    [[nodiscard]] std::vector<std::unique_ptr<Interface::Output>>& getOutputs();


    void fromConfig();
    void fromJson(const std::string& json) override;
    JsonDocument toJsonDoc() override;
    JsonDocument configJson();
    JsonDocument presetsJson();
    JsonDocument outputsJson();
    void toJson(JsonObject& doc) override;

    FixtureInfo info() const
    {
        std::scoped_lock lock(m_mutex);
        return {m_config, m_lastOffset};
    }

private:
    // Serializes update() (fixture task) against the mutators that reallocate
    // m_buffer / swap m_syncBuffers / repoint outputs (command task). Without
    // it, resize/swap under a live reader corrupts the heap. Also guards the
    // config read by the report/heartbeat tasks. Recursive so lock() can be
    // held around the setters, which lock it too.
    mutable std::recursive_mutex m_mutex;
    Config::Fixture m_config;
    std::vector<std::shared_ptr<SyncBuffer>> m_syncBuffers;
    std::vector<uint8_t> m_buffer;
    std::vector<std::unique_ptr<Interface::Output>> m_outputs;
    JsonDocument m_jsonPresets;

    uint16_t m_lastOffset = 0;
};