#pragma once
#include <Arduino.h>
#include <vector>
#include <string>
#include <functional>
#include "Interfaces/Sink.h"

namespace Reports
{
    // A periodic outbound report: build a payload, hand it to a Sink, repeat
    // every intervalMs. produce() returning an empty string skips that tick.
    struct Feed
    {
        std::string name;
        std::function<std::string()> produce;
        Interface::Sink* sink;
        uint32_t intervalMs;
        bool enabled = true;
        uint32_t nextDue = 0;
    };

    // Owns every periodic broadcast in one place and ticks them on a single
    // task. This is the outbound dual of the inbound CommandChannel/Input
    // abstraction: feeds push data out, channels read commands in.
    class Publisher
    {
        std::vector<Feed> m_feeds;

    public:
        void add(Feed feed)
        {
            m_feeds.push_back(std::move(feed));
        }

        // Toggle a feed at runtime (replaces suspend/resume of report tasks).
        void setEnabled(const std::string& name, bool enabled)
        {
            for (auto& f : m_feeds)
            {
                if (f.name == name)
                {
                    f.enabled = enabled;
                }
            }
        }

        // Fire any feed whose interval has elapsed. Call repeatedly from a task.
        void tick()
        {
            uint32_t now = millis();
            for (auto& f : m_feeds)
            {
                if (!f.enabled || now < f.nextDue)
                {
                    continue;
                }
                f.nextDue = now + f.intervalMs;

                if (!f.sink->ready())
                {
                    continue;
                }

                std::string payload = f.produce();
                if (!payload.empty())
                {
                    f.sink->publish(payload);
                }
            }
        }
    };
}
