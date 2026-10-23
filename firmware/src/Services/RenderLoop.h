#pragma once
#include <atomic>
#include <cstdint>
#include "Handlers/FixtureHandler.h"
#include "Tasks/TaskExecutor.h"

namespace Services
{
    // The one task that drives the LEDs. What it renders is picked by the
    // mode, so switching between live DMX and the wifi animation never
    // suspends a task from outside (which could leave a mutex held).
    class RenderLoop
    {
    public:
        enum class Mode : uint8_t
        {
            Live,       // render fixtures from DMX input
            Animation,  // run the wifi animation
            Off         // render nothing; LEDs keep their last frame
        };

        RenderLoop(Handler::FixtureHandler& fixtures, Tasks::Manager& tasks);

        // Spawns the render task. Call once, after everything is constructed.
        void start();

        // Takes effect on the next frame. Safe to call from any task.
        void setMode(Mode mode);
        [[nodiscard]] Mode mode() const;

    private:
        void loop();

        Handler::FixtureHandler& m_fixtures;
        Tasks::Manager& m_tasks;
        // Boot shows the animation until wifi first connects.
        std::atomic<Mode> m_mode{Mode::Animation};
    };
}
