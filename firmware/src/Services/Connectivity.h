#pragma once
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "Network/Wifi.h"
#include "Tasks/TaskExecutor.h"

namespace Services
{
    // Turns wifi link changes into callbacks. WiFi events only wake the
    // monitor task; listeners run on that task, never on the system event
    // task, so they may block briefly (sockets, mutexes).
    class Connectivity
    {
    public:
        using Listener = std::function<void(bool connected)>;

        Connectivity(const Net::Wifi& wifi, Tasks::Manager& tasks);

        // Register before start(). Listeners are called in registration
        // order, once per change, starting from "disconnected".
        void onChange(Listener listener);

        // Hooks the wifi events and spawns the monitor task. Call once.
        void start();

    private:
        void loop();

        struct SemaphoreDelete
        {
            void operator()(SemaphoreHandle_t s) const { vSemaphoreDelete(s); }
        };

        const Net::Wifi& m_wifi;
        Tasks::Manager& m_tasks;
        std::vector<Listener> m_listeners;
        std::unique_ptr<std::remove_pointer_t<SemaphoreHandle_t>, SemaphoreDelete> m_wake;
    };
}
