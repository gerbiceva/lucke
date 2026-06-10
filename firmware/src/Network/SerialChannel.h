#pragma once
#include "Interfaces/CommandChannel.h"
#include "Commands/CommandDispatcher.h"
#include "Network/SerialTransport.h"

namespace Network
{
    class SerialChannel : public Interface::CommandChannel
    {
    public:
        explicit SerialChannel(Commands::CommandDispatcher& dispatcher);

        void begin() override {}
        void poll() override;

    private:
        Commands::CommandDispatcher& m_dispatcher;
        SerialTransport m_transport;
    };
}
