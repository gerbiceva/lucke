#include "BoolSettingHandler.h"

namespace Commands
{
    BoolSettingHandler::BoolSettingHandler(API& api, const char* name, Member member, Setter setter)
        : m_api(api), m_name(name), m_member(member), m_setter(setter)
    {}

    Interface::Reply BoolSettingHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        if (req["value"].is<bool>())
            (m_api.*m_setter)(req["value"].as<bool>(), ctx.from);

        JsonDocument doc;
        doc["req"] = m_name;
        doc["val"] = m_api.settings().*m_member;
        return doc;
    }
}
