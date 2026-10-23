#pragma once
#include <Arduino.h>
#include "Interfaces/Command.h"
#include "Core/Config/EngineConfig.h"
#include "Services/Telemetry.h"
#include "Utils/Storage.h"

namespace Commands
{
    class EngineConfigHandler : public Interface::CommandHandler
    {
    public:
        EngineConfigHandler(Config::Engine& config, Utils::Storage& storage, Services::Telemetry& telemetry);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        Config::Engine& m_config;
        Utils::Storage& m_storage;
        Services::Telemetry& m_telemetry;

        void save();
        void respond(Interface::Transport& transport, const char* key, bool value);
    };
}
