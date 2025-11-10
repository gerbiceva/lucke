#include "FixtureHandler.h"

namespace Handler
{
    std::vector<Fixture*> Fixtures::m_fixtures;
    WiFiUDP Fixtures::udp;
    std::atomic<bool> Fixtures::connected(false);
}