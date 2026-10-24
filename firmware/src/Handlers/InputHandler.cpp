#include "InputHandler.h"
#include <algorithm>
#include "Utils/Logger.h"

namespace
{
    // Longest the input task blocks waiting for packets, so a reconnect
    // request is picked up within this time even with no traffic.
    constexpr uint32_t c_waitMs = 100;

    // E1.31 6.7.1 data loss timeout
    constexpr TickType_t c_lossTimeout = pdMS_TO_TICKS(2500);
    constexpr TickType_t c_lossCheckInterval = pdMS_TO_TICKS(250);

    constexpr Utils::Log s_log{"INPUT"};
}

namespace Handler
{
    InputHandler::InputHandler(Tasks::Manager& tasks)
        : m_tasks(tasks)
    {}

    void InputHandler::start()
    {
        // No delay in the loop: receive() blocks until traffic arrives.
        m_tasks.spawnTask("input_update", [this]() {
            while (true)
            {
                update();
            }
        }, 3, 30000);
    }

    std::shared_ptr<SyncBuffer> InputHandler::subscribe(uint16_t uni)
    {
        std::scoped_lock lock(m_mutex);
        auto [it, inserted] = m_universes.try_emplace(uni);
        // Before the socket is open this fails; reopen() joins it later.
        if (inserted) m_sacn.join(uni);
        ++it->second.refcount;
        return it->second.buffer;
    }

    void InputHandler::unsubscribe(uint16_t uni)
    {
        std::scoped_lock lock(m_mutex);
        auto it = m_universes.find(uni);
        if (it == m_universes.end()) return;

        if (--it->second.refcount == 0)
        {
            m_sacn.leave(uni);
            m_universes.erase(it);
        }
    }

    void InputHandler::reconnect()
    {
        m_reopen.store(true);
    }

    void InputHandler::reopen()
    {
        std::scoped_lock lock(m_mutex);
        m_sacn.close();
        if (!m_sacn.open())
        {
            s_log.error("sACN socket failed to open");
            return;
        }
        // Memberships don't survive the link going down, and universes
        // subscribed before wifi was up were never joined.
        for (auto& [uni, entry] : m_universes)
        {
            m_sacn.join(uni);
        }
    }

    void InputHandler::update()
    {
        if (m_reopen.exchange(false)) reopen();
        checkLoss();
        if (!m_sacn.isOpen())
        {
            vTaskDelay(pdMS_TO_TICKS(c_waitMs));
            return;
        }

        // Waits here, without the lock, until traffic arrives; then drains
        // the queue.
        for (auto pkt = m_sacn.receive(c_waitMs); pkt; pkt = m_sacn.receive(0))
        {
            std::scoped_lock lock(m_mutex);
            auto it = m_universes.find(pkt->universe);
            if (it == m_universes.end()) continue;   // not a universe we use

            Entry& e = it->second;
            // E1.31 6.7.2: drop late/duplicate packets, but accept a big
            // jump back (the sender restarted).
            int8_t diff = static_cast<int8_t>(pkt->sequence - e.lastSeq);
            if (e.seen && diff <= 0 && diff > -20) continue;
            e.lastSeq = pkt->sequence;
            e.seen = true;
            e.packets++;
            e.lastPacket = xTaskGetTickCount();
            if (!e.active)
            {
                e.active = true;
                s_log.info("universe {} receiving", pkt->universe);
            }

            Dmx::Universe frame{};   // slots the sender didn't send read as 0
            std::ranges::copy(pkt->slots, frame.begin());
            e.buffer->write(frame);
        }
    }

    void InputHandler::checkLoss()
    {
        const TickType_t now = xTaskGetTickCount();
        if (now - m_lastLossCheck < c_lossCheckInterval) return;
        m_lastLossCheck = now;

        std::scoped_lock lock(m_mutex);
        for (auto& [uni, e] : m_universes)
        {
            if (!e.active || now - e.lastPacket <= c_lossTimeout) continue;
            e.active = false;
            e.seen = false;
            s_log.warn("universe {} lost (no data for 2.5 s)", uni);
        }
    }

    InputsInfo InputHandler::inputs()
    {
        std::scoped_lock lock(m_mutex);
        InputsInfo info;
        info.inputs.reserve(m_universes.size());
        for (auto& [uni, entry] : m_universes)
        {
            info.inputs.push_back({uni, entry.packets, entry.active});
        }
        return info;
    }

    void InputHandler::toJson(JsonObject& obj)
    {
        inputs().toJson(obj);
    }
}
