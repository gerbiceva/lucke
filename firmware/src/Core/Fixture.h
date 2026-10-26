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
#include <string_view>

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
    void setHighlight(bool on);

    // Output settings (see Interface::SettingDesc). The caller has already
    // checked the key and value against the output's descriptor. Returns true
    // if the value changed. A requiresReboot setting is only saved; the others
    // are applied now. Saved with the fixture record.
    bool setOutputSetting(size_t output, std::string_view key, JsonVariantConst value);

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
    // The stored record: config plus the saved output settings.
    void fromJson(const std::string& json) override;
    JsonDocument toJsonDoc() override;
    // API shapes: an array of {name, settings}, and an array of outputs.
    JsonDocument presetsJson();
    JsonDocument outputsJson();
    void toJson(JsonObject& doc) override;

    FixtureInfo info() const
    {
        std::scoped_lock lock(m_mutex);
        return {m_config, m_lastOffset};
    }

private:
    // Guards buffers, outputs and config between the render, command and report
    // tasks. Recursive so lock() can be held around the setters.
    mutable std::recursive_mutex m_mutex;
    Config::Fixture m_config;
    std::vector<std::shared_ptr<SyncBuffer>> m_syncBuffers;
    std::vector<uint8_t> m_buffer;
    std::vector<std::unique_ptr<Interface::Output>> m_outputs;
    JsonDocument m_jsonPresets;
    // Saved output settings: one object per output index, key -> value.
    JsonDocument m_savedOutputs;

    uint16_t m_lastOffset = 0;
};