#include "FixtureConfig.h"

using namespace Config;

uint8_t Fixture::s_ID = 0;

void Fixture::obtainID()
{
    m_ID = s_ID++;
}

void Fixture::toJson(JsonObject& obj)
{
    obj["id"] = m_ID;
    obj["name"] = name;
    obj["type"] = type;
    obj["universe"] = universe;
    obj["address"] = address;
    obj["presetIndex"] = selectedPreset;
    obj["numPresets"] = numPresets;
}

JsonDocument Fixture::toJsonDoc() 
{
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    toJson(obj);
    return doc;
}
