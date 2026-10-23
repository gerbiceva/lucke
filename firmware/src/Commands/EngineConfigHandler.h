#pragma once
#include <Arduino.h>
#include "Interfaces/Command.h"
#include "Services/Telemetry.h"
#include "Services/SettingsStore.h"

namespace Commands
{
    class EngineConfigHandler : public Interface::CommandHandler
    {
    public:
        EngineConfigHandler(Services::SettingsStore& settings, Services::Telemetry& telemetry);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        Services::SettingsStore& m_settings;
        Services::Telemetry& m_telemetry;

        void respond(Interface::Transport& transport, const char* key, bool value);
    };
}
