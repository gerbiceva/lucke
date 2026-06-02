#include "FixtureHandler.h"
#include "Utils/Logger.h"

namespace Handler
{
    FixtureHandler::FixtureHandler(InputHandler& inputs)
        : m_inputs(inputs), m_storage(Utils::Storage("fixtures"))
    {}

    void FixtureHandler::_addFixture(Fixture* fix, bool factoryReset)
    {
        fix->setId(m_nextId++);

        std::string key = "fixture" + std::to_string(fix->id());
        if (m_storage.isKey(key) && !factoryReset)
        {
            fix->fromJson(m_storage.getString(key));
        }
        else
        {
            fix->fromConfig();
        }

        // Subscribe to all universes this fixture spans
        uint8_t baseUni = fix->getUniverse();
        uint16_t totalBytes = fix->getAddress() + fix->getFootprint();
        uint8_t numUni = totalBytes > 0 ? ((totalBytes - 1) / 512 + 1) : 1;

        std::vector<std::shared_ptr<SyncBuffer>> bufs;
        for (uint8_t i = 0; i < numUni; i++)
            bufs.push_back(m_inputs.subscribe(baseUni + i));

        fix->setSyncBuffers(std::move(bufs));
        m_fixtures.push_back(fix);
        persistFixture(fix->id());
    }

    void FixtureHandler::persistFixture(uint8_t id)
    {
        Fixture* fix = get(id);
        if (!fix) return;

        JsonDocument doc = fix->toJsonDoc();
        std::string serialized;
        serializeJson(doc, serialized);

        std::string key = "fixture" + std::to_string(id);
        m_storage.putString(key, serialized);
    }

    Fixture* FixtureHandler::get(uint8_t id)
    {
        for (auto fix : m_fixtures)
        {
            if (fix->id() == id) return fix;
        }
        return nullptr;
    }

    std::vector<Fixture*>& FixtureHandler::all()
    {
        return m_fixtures;
    }

    bool FixtureHandler::exists(uint8_t id)
    {
        return get(id) != nullptr;
    }

    bool FixtureHandler::setUniverse(uint8_t id, uint8_t uni)
    {
        Fixture* fix = get(id);
        if (!fix) return false;

        uint8_t oldBase = fix->getUniverse();
        if (oldBase == uni) return true;

        // Unsubscribe from all old universes
        uint8_t oldCount = fix->getNumUniverses();
        for (uint8_t i = 0; i < oldCount; i++)
            m_inputs.unsubscribe(oldBase + i);

        fix->setUniverse(uni);

        // Subscribe to new universe span
        uint16_t totalBytes = fix->getAddress() + fix->getFootprint();
        uint8_t numUni = totalBytes > 0 ? ((totalBytes - 1) / 512 + 1) : 1;

        std::vector<std::shared_ptr<SyncBuffer>> bufs;
        for (uint8_t i = 0; i < numUni; i++)
            bufs.push_back(m_inputs.subscribe(uni + i));

        fix->setSyncBuffers(std::move(bufs));
        persistFixture(id);
        return true;
    }

    bool FixtureHandler::setAddress(uint8_t id, uint16_t addr)
    {
        Fixture* fix = get(id);
        if (!fix) return false;

        fix->setAddress(addr);
        persistFixture(id);
        return true;
    }

    bool FixtureHandler::setPreset(uint8_t id, uint8_t preset)
    {
        Fixture* fix = get(id);
        if (!fix) return false;

        fix->setPreset(preset);
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

        fix->getHighlighted() = !fix->getHighlighted();
        return true;
    }

    void FixtureHandler::update()
    {
        for (auto fix : m_fixtures)
        {
            fix->update();

            for (auto o : fix->getOutputs())
            {
                o->postupdate();
            }
        }
    }

    void FixtureHandler::wifiAnimation()
    {
        for (auto fix : m_fixtures)
        {
            fix->wifiAnimation();

            for (auto o : fix->getOutputs())
            {
                o->postupdate();
            }
        }
    }

    void FixtureHandler::fromJson(std::string json)
    {
        // Handled per-fixture via Fixture::fromJson
    }

    void FixtureHandler::toJson(JsonObject& obj)
    {
        JsonArray arr = obj["fixtures"].to<JsonArray>();
        for (auto fix : m_fixtures)
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
