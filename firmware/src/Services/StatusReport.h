#pragma once
#include <string>
#include "Services/SettingsStore.h"
#include "Network/Wifi.h"
#include "Core/Managers/FixtureManager.h"
#include "Core/Managers/InputManager.h"

namespace Services
{
    // Builds the status documents sent off the device (serial / UDP reports).
    // Secrets (the wifi password) are never included.
    class StatusReport
    {
    public:
        StatusReport(const SettingsStore& settings, Net::Wifi& wifi,
                     Handler::FixtureManager& fixtures, Handler::InputManager& inputs);

        // Full status: the same document as the describe command's data.
        std::string build();

        // Version, report switches and ssid only.
        std::string settings();

    private:
        const SettingsStore& m_settings;
        Net::Wifi& m_wifi;
        Handler::FixtureManager& m_fixtures;
        Handler::InputManager& m_inputs;
    };
}
