#pragma once
#include <cstdint>
#include <string>
#include "Interfaces/CommandChannel.h"
#include "Commands/CommandDispatcher.h"
#include "Network/TcpServer.h"
#include "Utils/Optional.h"

namespace Network
{
    class TcpChannel : public Interface::CommandChannel
    {
    public:
        TcpChannel(Commands::CommandDispatcher& dispatcher, uint16_t listenPort, uint16_t responsePort);

        void begin() override;
        void stop() override;
        void poll() override;

    private:
        Commands::CommandDispatcher& m_dispatcher;
        TcpServer m_server;
        uint16_t m_responsePort;

        Utils::Optional<TcpConnection> m_conn;
        std::string m_remoteIp;
    };
}
