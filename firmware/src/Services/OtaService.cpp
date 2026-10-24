#include "OtaService.h"
#include <ArduinoOTA.h>
#include <esp_ota_ops.h>
#include "Core/Config/EngineConfig.h"
#include "Utils/Logger.h"

namespace
{
    constexpr Utils::Log s_log{"OTA"};

    // A new image counts as good after this long with wifi up.
    constexpr uint32_t c_confirmAfterMs = 30000;

    const char* errorName(ota_error_t error)
    {
        switch (error)
        {
            case OTA_AUTH_ERROR:    return "auth failed";
            case OTA_BEGIN_ERROR:   return "begin failed";
            case OTA_CONNECT_ERROR: return "connect failed";
            case OTA_RECEIVE_ERROR: return "receive failed";
            case OTA_END_ERROR:     return "end failed";
        }
        return "unknown error";
    }
}

// Keeps the Arduino core from marking a freshly flashed image good at boot;
// OtaService::confirmImage() does it once the image has proven itself.
extern "C" bool verifyRollbackLater()
{
    return true;
}

namespace Services
{
    OtaService::OtaService(RenderLoop& render, const Net::Wifi& wifi, Tasks::Manager& tasks)
        : m_render(render), m_wifi(wifi), m_tasks(tasks)
    {}

    void OtaService::start()
    {
        esp_ota_img_states_t state;
        if (esp_ota_get_state_partition(esp_ota_get_running_partition(), &state) == ESP_OK)
        {
            m_pendingVerify = state == ESP_OTA_IMG_PENDING_VERIFY;
        }
        if (m_pendingVerify) s_log.warn("new firmware, not confirmed yet");

        ArduinoOTA.setPort(OTA_PORT);
        ArduinoOTA.setPassword(OTA_PASSWORD.data());
        ArduinoOTA.setMdnsEnabled(false);

        ArduinoOTA.onStart([this]() {
            s_log.info("update started");
            m_modeBefore = m_render.mode();
            m_render.setMode(RenderLoop::Mode::Off);
            m_lastProgress = 0;
        });
        ArduinoOTA.onProgress([this](unsigned int done, unsigned int total) {
            uint8_t pct = total ? done * 100ULL / total : 0;
            if (pct >= m_lastProgress + 10)
            {
                m_lastProgress = pct - pct % 10;
                s_log.info("update %u%%", m_lastProgress);
            }
        });
        ArduinoOTA.onEnd([]() {
            s_log.info("update done, rebooting");
        });
        ArduinoOTA.onError([this](ota_error_t error) {
            s_log.error("update failed: %s", errorName(error));
            m_render.setMode(m_modeBefore);
        });

        // Stack: the whole update (TCP receive + flash write) runs inside handle().
        m_tasks.spawnTask("ota", [this]() {
            loop();
        }, 1, 8192);
    }

    void OtaService::enable(uint32_t minutes)
    {
        // | 1 so a deadline that happens to be 0 doesn't read as "closed".
        m_openUntil.store((millis() + minutes * 60000) | 1);
    }

    void OtaService::loop()
    {
        while (true)
        {
            const uint32_t until = m_openUntil.load();
            const bool open = until != 0 && static_cast<int32_t>(until - millis()) > 0;

            if (open && !m_listening)
            {
                ArduinoOTA.begin();
                m_listening = true;
                s_log.info("listening on port %u for %lu s", OTA_PORT,
                           static_cast<unsigned long>((until - millis()) / 1000));
            }
            else if (!open && m_listening)
            {
                ArduinoOTA.end();
                m_listening = false;
                m_openUntil.store(0);
                s_log.info("closed");
            }

            if (m_listening) ArduinoOTA.handle();
            if (m_pendingVerify) confirmImage();

            vTaskDelay(pdMS_TO_TICKS(m_listening ? 20 : 500));
        }
    }

    void OtaService::confirmImage()
    {
        if (millis() < c_confirmAfterMs || !m_wifi.isConnected()) return;
        esp_ota_mark_app_valid_cancel_rollback();
        m_pendingVerify = false;
        s_log.info("new firmware confirmed");
    }
}
