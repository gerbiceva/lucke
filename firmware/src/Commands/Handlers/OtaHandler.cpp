#include "OtaHandler.h"
#include <algorithm>

namespace Commands
{
    OtaHandler::OtaHandler(API& api)
        : m_api(api)
    {}

    Interface::Reply OtaHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        // {"request":"ota"} or {"request":"ota","minutes":10}; 1..30, default 5.
        const uint32_t minutes = std::clamp<uint32_t>(req["minutes"] | 5u, 1, 30);
        m_api.enableOta(minutes, ctx.from);

        JsonDocument doc;
        doc["status"] = "ota open";
        doc["minutes"] = minutes;
        doc["port"] = OTA_PORT;
        return doc;
    }
}
