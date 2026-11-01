#include "RenderLoop.h"
#include "Utils/Logger.h"

namespace
{
    // Live mode re-checks the fixtures this often without a wake(), for the
    // periodic refresh (Fixture, 1 s) and for changes that don't wake it yet
    // (presets, output settings, highlight). New DMX wakes it right away.
    constexpr TickType_t c_liveWaitTicks = pdMS_TO_TICKS(250);
    constexpr TickType_t c_animationTicks = pdMS_TO_TICKS(20);
    constexpr TickType_t c_statsTicks = pdMS_TO_TICKS(60000);

    constexpr Utils::Log s_log{"RENDER"};
}

namespace Services
{
    RenderLoop::RenderLoop(Handler::FixtureManager& fixtures, Tasks::Manager& tasks)
        : m_fixtures(fixtures), m_tasks(tasks)
    {}

    void RenderLoop::start()
    {
        m_tasks.spawnTask("render", [this]() {
            loop();
        }, 2, 30000);
    }

    void RenderLoop::setMode(Mode mode)
    {
        m_mode.store(mode);
        wake();
    }

    RenderLoop::Mode RenderLoop::mode() const
    {
        return m_mode.load();
    }

    void RenderLoop::wake()
    {
        if (TaskHandle_t task = m_task.load()) xTaskNotifyGive(task);
    }

    void RenderLoop::loop()
    {
        m_task.store(xTaskGetCurrentTaskHandle());

        Mode previous = m_mode.load();
        uint32_t frames = 0;
        TickType_t statsStart = xTaskGetTickCount();
        while (true)
        {
            const Mode mode = m_mode.load();
            switch (mode)
            {
            case Mode::Live:
                // The animation (or whatever was shown before Off) is on
                // the LEDs: show the current DMX frame right away.
                if (previous != Mode::Live) m_fixtures.invalidate();
                // Cheap when nothing changed: fixtures only render on new
                // data, a setting change or the periodic refresh.
                if (m_fixtures.update()) ++frames;
                // Wakes that came in while rendering are kept, so nothing
                // is missed: the next wait returns right away.
                ulTaskNotifyTake(pdTRUE, c_liveWaitTicks);
                break;
            case Mode::Animation:
                m_fixtures.wifiAnimation();
                ulTaskNotifyTake(pdTRUE, c_animationTicks);
                break;
            case Mode::Off:
                ulTaskNotifyTake(pdTRUE, c_animationTicks);
                break;
            }
            previous = mode;

            const TickType_t now = xTaskGetTickCount();
            if (now - statsStart >= c_statsTicks)
            {
                s_log.debug("live frames sent in the last minute: %lu", static_cast<unsigned long>(frames));
                frames = 0;
                statsStart = now;
            }
        }
    }
}
