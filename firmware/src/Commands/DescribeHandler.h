#pragma once
#include "Interfaces/Command.h"
#include "API/API.h"

namespace Commands
{
    class DescribeHandler : public Interface::CommandHandler
    {
    public:
        explicit DescribeHandler(const API& api);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        const API& m_api;
    };
}
