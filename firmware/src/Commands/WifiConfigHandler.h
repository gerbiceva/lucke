#pragma once
#include "Interfaces/Command.h"
#include "Network/Wifi.h"
#include "Core/Config/EngineConfig.h"
#include "Utils/Storage.h"

namespace Commands
{
    class WifiConfigHandler : public Interface::CommandHandler
    {
    public:
        WifiConfigHandler(Net::Wifi& wifi, Config::Engine& config, Utils::Storage& storage);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        Net::Wifi& m_wifi;
        Config::Engine& m_config;
        Utils::Storage& m_storage;
    };
}
