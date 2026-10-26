#include "BoolSettingHandler.h"

namespace Commands
{
    BoolSettingHandler::BoolSettingHandler(API& api, const char* name, Member member, Setter setter)
        : m_api(api), m_name(name), m_member(member), m_setter(setter)
    {}

    void BoolSettingHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        if (req["value"].is<bool>())
            (m_api.*m_setter)(req["value"].as<bool>(), transport.remoteAddress());

        JsonDocument doc;
        doc["req"] = m_name;
        doc["val"] = m_api.settings().*m_member;

        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }
}
