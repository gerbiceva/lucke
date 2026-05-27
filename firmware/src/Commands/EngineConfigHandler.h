#pragma once
#include <Arduino.h>
#include "Interfaces/Command.h"
#include "Core/Config/EngineConfig.h"
#include "Tasks/TaskExecutor.h"
#include "Utils/Storage.h"

namespace Commands
{
    class EngineConfigHandler : public Interface::CommandHandler
    {
    public:
        EngineConfigHandler(Config::Engine& config, Utils::Storage& storage, Tasks::Manager& tasks);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        Config::Engine& m_config;
        Utils::Storage& m_storage;
        Tasks::Manager& m_tasks;

        uint32_t m_serialReportTaskId = 0;
        uint32_t m_wirelessReportTaskId = 0;

        void save();
        void respond(Interface::Transport& transport, const char* key, bool value);
        void toggleSerialReport(bool enable);
        void toggleWirelessReport(bool enable);
    };
}
