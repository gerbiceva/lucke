#include "SerialChannel.h"

namespace Network
{
    SerialChannel::SerialChannel(Commands::CommandDispatcher& dispatcher)
        : m_dispatcher(dispatcher)
    {}

    void SerialChannel::poll()
    {
        if (m_transport.isAvailable())
        {
            std::string line = m_transport.readLine();
            if (!line.empty())
                m_dispatcher.dispatch(line, m_transport);
        }
    }
}
