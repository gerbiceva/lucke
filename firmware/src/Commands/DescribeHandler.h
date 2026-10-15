#pragma once
#include "Interfaces/Command.h"
#include "Handlers/FixtureHandler.h"
#include "Handlers/InputHandler.h"
#include "Core/Config/EngineConfig.h"
#include "Network/Wifi.h"

namespace Commands
{
    class DescribeHandler : public Interface::CommandHandler
    {
    public:
        DescribeHandler(Config::Engine& config, Handler::FixtureHandler& fixtures, Handler::InputHandler& inputs, Net::Wifi& wifi);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        Config::Engine& m_config;
        Handler::FixtureHandler& m_fixtures;
        Handler::InputHandler& m_inputs;
        Net::Wifi& m_wifi;
    };
}
