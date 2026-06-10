#pragma once

namespace Interface
{
    // A source of incoming commands (TCP, Serial, BLE, ...). Each channel owns
    // the Transport it answers on and feeds complete command lines to the
    // CommandDispatcher. Engine drives every channel through poll() so they are
    // interchangeable and new ones can be added without touching dispatch logic.
    struct CommandChannel
    {
        virtual void begin() = 0;   // open the underlying socket / start advertising
        virtual void stop() {}      // tear down (e.g. on wifi loss)
        virtual void poll() = 0;    // non-blocking: read what is available, dispatch it
        virtual ~CommandChannel() = default;
    };
}
