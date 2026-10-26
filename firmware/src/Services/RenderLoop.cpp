#include "RenderLoop.h"

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
    }

    RenderLoop::Mode RenderLoop::mode() const
    {
        return m_mode.load();
    }

    void RenderLoop::loop()
    {
        while (true)
        {
            switch (m_mode.load())
            {
            case Mode::Live:
                m_fixtures.update();
                vTaskDelay(1);
                break;
            case Mode::Animation:
                m_fixtures.wifiAnimation();
                vTaskDelay(20);
                break;
            case Mode::Off:
                vTaskDelay(20);
                break;
            }
        }
    }
}
