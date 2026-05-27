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

        // Subscribe to the fixture's universe and give it the SyncBuffer
        fix->setSyncBuffer(m_inputs.subscribe(fix->getUniverse()));
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

        uint8_t oldUni = fix->getUniverse();
        if (oldUni == uni) return true;

        // Unsubscribe from old, subscribe to new
        m_inputs.unsubscribe(oldUni);
        fix->setSyncBuffer(m_inputs.subscribe(uni));
        fix->setUniverse(uni);

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
