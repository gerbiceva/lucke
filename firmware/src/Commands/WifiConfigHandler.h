#pragma once
#include "Interfaces/Command.h"
#include "Network/Wifi.h"
#include "Services/SettingsStore.h"

namespace Commands
{
    class WifiConfigHandler : public Interface::CommandHandler
    {
    public:
        WifiConfigHandler(Net::Wifi& wifi, Services::SettingsStore& settings);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        Net::Wifi& m_wifi;
        Services::SettingsStore& m_settings;
    };
}
