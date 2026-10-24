#pragma once
#include <Arduino.h>
#include "Interfaces/Command.h"
#include "Services/SettingsStore.h"

namespace Commands
{
    class RebootHandler : public Interface::CommandHandler
    {
    public:
        explicit RebootHandler(Services::SettingsStore& settings);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        Services::SettingsStore& m_settings;
    };
}
