#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "Interfaces/Serializable.h"
#include "Input/SacnReceiver.h"
#include "Utils/SyncBuffer.h"
#include "Tasks/TaskExecutor.h"

namespace Handler
{
    // Owns the sACN socket and one SyncBuffer per subscribed universe.
    // Packets are routed by the universe field they carry.
    class InputHandler : public Interface::Serializable
    {
        void update();
        void reopen();
        // Logs universes whose sender went quiet. Input task only.
        void checkLoss();
    public:
        explicit InputHandler(Tasks::Manager& tasks);

        // Spawns the input task. Call once, after everything is constructed.
        void start();

        std::shared_ptr<SyncBuffer> subscribe(uint16_t uni);
        void unsubscribe(uint16_t uni);

        // Call when wifi (re)connects. The input task reopens the socket and
        // rejoins every universe on its next pass.
        void reconnect();

        void toJson(JsonObject& obj) override;
    private:
        struct Entry
        {
            std::shared_ptr<SyncBuffer> buffer = std::make_shared<SyncBuffer>();
            uint16_t refcount = 0;
            uint8_t lastSeq = 0;
            bool seen = false;
            uint32_t packets = 0;
            bool active = false;        // data within the loss timeout
            TickType_t lastPacket = 0;
        };

        Tasks::Manager& m_tasks;
        // Guards m_universes and the socket's open/close against join/leave:
        // subscribe/unsubscribe run on the command task, update()/reopen()
        // on the input task.
        std::mutex m_mutex;
        std::unordered_map<uint16_t, Entry> m_universes;
        // Opened, closed and read only by the input task. Closing it from
        // the wifi task could pull it out from under a blocked receive().
        Input::SacnReceiver m_sacn;
        std::atomic<bool> m_reopen{false};
        TickType_t m_lastLossCheck = 0;   // input task only
    };
}
