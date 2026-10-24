#include "Connectivity.h"
#include <WiFi.h>
#include "Utils/Logger.h"

namespace
{
    // Safety net: re-check the link this often even without an event.
    constexpr TickType_t c_recheckTicks = 1000;

    constexpr Utils::Log s_log{"CONN"};
}

namespace Services
{
    Connectivity::Connectivity(const Net::Wifi& wifi, Tasks::Manager& tasks)
        : m_wifi(wifi), m_tasks(tasks)
    {}

    void Connectivity::onChange(Listener listener)
    {
        m_listeners.push_back(std::move(listener));
    }

    void Connectivity::start()
    {
        m_wake.reset(xSemaphoreCreateBinary());
        if (!m_wake)
        {
            s_log.error("out of memory, wifi changes will be missed");
            return;
        }
        // Check once right away: the link may have come up before the
        // event handler below was registered.
        xSemaphoreGive(m_wake.get());

        // Runs on the Arduino event task: only wake the monitor.
        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t) {
            switch (event)
            {
            case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            case ARDUINO_EVENT_WIFI_STA_STOP:
                xSemaphoreGive(m_wake.get());
                break;
            default:
                break;
            }
        });

        m_tasks.spawnTask("wifi_monitor", [this]() {
            loop();
        }, 1, 4096);
    }

    void Connectivity::loop()
    {
        bool wasConnected = false;
        while (true)
        {
            // The event only says "something changed"; the link state is
            // read here, so a missed or duplicated event can't desync us.
            xSemaphoreTake(m_wake.get(), c_recheckTicks);

            bool connected = m_wifi.isConnected();
            if (connected == wasConnected) continue;
            wasConnected = connected;

            if (connected) s_log.info("wifi up, ip {}", m_wifi.localIP());
            else s_log.warn("wifi down");
            for (auto& listener : m_listeners)
            {
                listener(connected);
            }
        }
    }
}
