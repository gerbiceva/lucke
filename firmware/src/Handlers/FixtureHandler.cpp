#include "FixtureHandler.h"

namespace Handler
{
    std::vector<Fixture*> Fixtures::m_fixtureConfigs;
    WiFiUDP Fixtures::udp;
}