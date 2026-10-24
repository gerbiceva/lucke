#include "FixtureHandler.h"
#include "Utils/Logger.h"
#include <algorithm>

namespace Handler
{
    FixtureHandler::FixtureHandler(InputHandler& inputs, Services::SettingsStore& settings)
        : m_inputs(inputs), m_settings(settings)
    {}

    void FixtureHandler::_addFixture(std::unique_ptr<Fixture> fix)
    {
        // Fixtures are added from setup(), before Engine::init().
        m_settings.load();

        fix->setId(m_nextId++);

        if (auto stored = m_settings.loadFixture(fix->id()))
        {
            fix->fromJson(*stored);
        }
        else
        {
            // First boot: one universe per fixture, so they don't all
            // show the same data.
            fix->setUniverse(fix->id() + 1);
            fix->fromConfig();
        }

        // Subscribe to all universes this fixture spans. No prior span
        // exists yet (oldCount = 0), so nothing is unsubscribed.
        resubscribe(fix.get(), fix->getUniverse(), 0);

        uint8_t id = fix->id();
        m_fixtures.push_back(std::move(fix));
        persistFixture(id);
    }

    void FixtureHandler::resubscribe(Fixture* fix, uint16_t oldBase, uint8_t oldCount)
    {
        for (uint8_t i = 0; i < oldCount; i++)
            m_inputs.unsubscribe(oldBase + i);

        uint16_t base = fix->getUniverse();
        uint16_t totalBytes = fix->getAddress() + fix->getFootprint();
        uint8_t numUni = totalBytes > 0 ? ((totalBytes - 1) / 512 + 1) : 1;

        std::vector<std::shared_ptr<SyncBuffer>> bufs;
        for (uint8_t i = 0; i < numUni; i++)
            bufs.push_back(m_inputs.subscribe(base + i));

        fix->setSyncBuffers(std::move(bufs));
    }

    void FixtureHandler::persistFixture(uint8_t id)
    {
        Fixture* fix = get(id);
        if (!fix) return;

        JsonDocument doc = fix->toJsonDoc();
        std::string serialized;
        serializeJson(doc, serialized);

        m_settings.saveFixture(id, serialized);
    }

    Fixture* FixtureHandler::get(uint8_t id)
    {
        auto it = std::ranges::find(m_fixtures, id, &Fixture::id);
        return it != m_fixtures.end() ? it->get() : nullptr;
    }

    const std::vector<std::unique_ptr<Fixture>>& FixtureHandler::all() const
    {
        return m_fixtures;
    }

    bool FixtureHandler::exists(uint8_t id)
    {
        return get(id) != nullptr;
    }

    bool FixtureHandler::setUniverse(uint8_t id, uint16_t uni)
    {
        Fixture* fix = get(id);
        if (!fix) return false;

        {
            auto lock = fix->lock();
            uint16_t oldBase = fix->getUniverse();
            if (oldBase == uni) return true;

            uint8_t oldCount = fix->getNumUniverses();
            fix->setUniverse(uni);
            resubscribe(fix, oldBase, oldCount);
        }
        persistFixture(id);
        return true;
    }

    bool FixtureHandler::setAddress(uint8_t id, uint16_t addr)
    {
        Fixture* fix = get(id);
        if (!fix) return false;

        {
            // One lock for change + resubscribe: the render task never sees
            // the new footprint with the old buffer.
            auto lock = fix->lock();
            fix->setAddress(addr);
            // Address shifts totalBytes, so the span may grow/shrink.
            resubscribe(fix, fix->getUniverse(), fix->getNumUniverses());
        }
        persistFixture(id);
        return true;
    }

    bool FixtureHandler::setPreset(uint8_t id, uint8_t preset)
    {
        Fixture* fix = get(id);
        if (!fix) return false;

        {
            auto lock = fix->lock();
            fix->setPreset(preset);
            // Preset changes the footprint (e.g. pixel grouping), which changes
            // how many universes the fixture spans.
            resubscribe(fix, fix->getUniverse(), fix->getNumUniverses());
        }
        persistFixture(id);
        return true;
    }

    bool FixtureHandler::setName(uint8_t id, const std::string& name)
    {
        Fixture* fix = get(id);
        if (!fix) return false;

        fix->setName(name);
        persistFixture(id);
        return true;
    }

    bool FixtureHandler::highlight(uint8_t id)
    {
        Fixture* fix = get(id);
        if (!fix) return false;

        fix->toggleHighlight();
        return true;
    }

    void FixtureHandler::update()
    {
        for (auto& fix : m_fixtures)
        {
            fix->update();

            for (auto& o : fix->getOutputs())
            {
                o->postupdate();
            }
        }
    }

    void FixtureHandler::wifiAnimation()
    {
        for (auto& fix : m_fixtures)
        {
            fix->wifiAnimation();

            for (auto& o : fix->getOutputs())
            {
                o->postupdate();
            }
        }
    }

    void FixtureHandler::fromJson(const std::string& json)
    {
        // Handled per-fixture via Fixture::fromJson
    }

    void FixtureHandler::toJson(JsonObject& obj)
    {
        JsonArray arr = obj["fixtures"].to<JsonArray>();
        for (auto& fix : m_fixtures)
        {
            JsonObject entry = arr.add<JsonObject>();
            fix->toJson(entry);
        }
    }

    JsonDocument FixtureHandler::toJsonDoc()
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        toJson(obj);
        return doc;
    }
}
