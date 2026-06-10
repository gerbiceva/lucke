#pragma once
#include "Interfaces/Sink.h"
#include "Utils/Logger.h"

namespace Reports
{
    // Prints reports to the serial console (respects Logger's enabled flag).
    struct SerialSink : public Interface::Sink
    {
        void publish(const std::string& data) override
        {
            Utils::Logger::println(data.c_str());
        }
    };
}
