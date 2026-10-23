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
        using ReportFn = std::function<std::string()>;

        EngineConfigHandler(Config::Engine& config, Utils::Storage& storage, Tasks::Manager& tasks);
        void handle(JsonDocument& req, Interface::Transport& transport) override;
        void initTasks(ReportFn serialFn, ReportFn wirelessFn);

    private:
        Config::Engine& m_config;
        Utils::Storage& m_storage;
        Tasks::Manager& m_tasks;

        ReportFn m_serialReportFn;
        ReportFn m_wirelessReportFn;

        Tasks::TaskId m_serialReportTaskId = Tasks::TaskId::Invalid;
        Tasks::TaskId m_wirelessReportTaskId = Tasks::TaskId::Invalid;

        void save();
        void respond(Interface::Transport& transport, const char* key, bool value);
        void toggleSerialReport(bool enable);
        void toggleWirelessReport(bool enable);
    };
}
