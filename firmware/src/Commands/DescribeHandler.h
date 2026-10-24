#pragma once
#include "Interfaces/Command.h"
#include "Handlers/FixtureHandler.h"
#include "Handlers/InputHandler.h"
#include "Services/SettingsStore.h"
#include "Network/Wifi.h"

namespace Commands
{
    class DescribeHandler : public Interface::CommandHandler
    {
    public:
        DescribeHandler(const Services::SettingsStore& settings, Handler::FixtureHandler& fixtures, Handler::InputHandler& inputs, Net::Wifi& wifi);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        const Services::SettingsStore& m_settings;
        Handler::FixtureHandler& m_fixtures;
        Handler::InputHandler& m_inputs;
        Net::Wifi& m_wifi;
    };
}
