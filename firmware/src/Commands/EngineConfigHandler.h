#pragma once
#include <Arduino.h>
#include "Interfaces/Command.h"
#include "Core/Config/EngineConfig.h"
#include "Reports/Publisher.h"
#include "Utils/Storage.h"

namespace Commands
{
    // Feed names registered with the Publisher in Engine::spawnTasks; this
    // handler toggles them by name when the corresponding setting changes.
    constexpr const char* SERIAL_REPORT_FEED   = "serial_report";
    constexpr const char* WIRELESS_REPORT_FEED = "wireless_report";

    class EngineConfigHandler : public Interface::CommandHandler
    {
    public:
        EngineConfigHandler(Config::Engine& config, Utils::Storage& storage, Reports::Publisher& publisher);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        Config::Engine& m_config;
        Utils::Storage& m_storage;
        Reports::Publisher& m_publisher;

        void save();
        void respond(Interface::Transport& transport, const char* key, bool value);
    };
}
