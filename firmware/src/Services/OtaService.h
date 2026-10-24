#pragma once
#include <atomic>
#include <cstdint>
#include "Network/Wifi.h"
#include "Services/RenderLoop.h"
#include "Tasks/TaskExecutor.h"

namespace Services
{
    // Firmware updates over WiFi (ArduinoOTA / espota, port OTA_PORT).
    //
    // Closed by default: it only listens for a few minutes after enable().
    // A new image boots "pending verify" and is marked good once it has run
    // with wifi up for a while; if it crashes or resets before that, the
    // bootloader goes back to the previous image.
    class OtaService
    {
    public:
        OtaService(RenderLoop& render, const Net::Wifi& wifi, Tasks::Manager& tasks);

        // Spawns the OTA task. Call once, after everything is constructed.
        void start();

        // Listens for an update for the next `minutes`. Safe from any task.
        void enable(uint32_t minutes);

    private:
        void loop();
        void confirmImage();

        RenderLoop& m_render;
        const Net::Wifi& m_wifi;
        Tasks::Manager& m_tasks;

        std::atomic<uint32_t> m_openUntil{0};   // millis(); 0 = closed
        // OTA task only
        bool m_listening = false;
        bool m_pendingVerify = false;
        RenderLoop::Mode m_modeBefore = RenderLoop::Mode::Live;
        uint8_t m_lastProgress = 0;
    };
}
