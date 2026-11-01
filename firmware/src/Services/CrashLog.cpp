#include "CrashLog.h"
#include <esp_core_dump.h>
#include <esp_system.h>
#include <cstring>
#include <memory>
#include <ArduinoJson.h>
#include "Core/Config/EngineConfig.h"
#include "Utils/CrashTrail.h"
#include "Utils/Logger.h"

namespace
{
    constexpr Utils::Log s_log{"CRASH"};

    constexpr const char* c_crashKey = "crash";
    constexpr const char* c_errorsKey = "errors";

    // A recurring error mustn't wear out the flash: new Error lines are
    // looked for this often, and saved at most c_maxErrorSaves times a boot.
    constexpr TickType_t c_errorCheckInterval = pdMS_TO_TICKS(5000);
    constexpr uint32_t c_maxErrorSaves = 12;
    // Errors kept, the oldest dropped first. An NVS string must fit in one
    // 4 KB page, and the partition is small.
    constexpr size_t c_maxErrors = 10;
    constexpr size_t c_maxErrorBytes = 1536;

    struct ResetReason
    {
        esp_reset_reason_t reason;
        const char* code;   // in the crash record
        const char* name;   // in the log
        bool abnormal;
        bool coreDump;      // the panic handler wrote a core dump
    };

    constexpr ResetReason c_resetReasons[] = {
        {ESP_RST_POWERON,    "power_on",     "power on",                          false, false},
        {ESP_RST_EXT,        "external",     "external pin",                      false, false},
        {ESP_RST_SW,         "software",     "software restart",                  false, false},
        {ESP_RST_USB,        "usb",          "USB",                               false, false},
        {ESP_RST_JTAG,       "jtag",         "JTAG",                              false, false},
        {ESP_RST_DEEPSLEEP,  "deep_sleep",   "deep sleep wake",                   false, false},
        {ESP_RST_SDIO,       "sdio",         "SDIO",                              false, false},
        {ESP_RST_PANIC,      "panic",        "crash (panic)",                     true,  true},
        {ESP_RST_INT_WDT,    "int_wdt",      "interrupt watchdog",                true,  true},
        {ESP_RST_TASK_WDT,   "task_wdt",     "task watchdog",                     true,  true},
        {ESP_RST_WDT,        "wdt",          "watchdog",                          true,  false},
        {ESP_RST_BROWNOUT,   "brownout",     "brownout (supply voltage dropped)", true,  false},
        {ESP_RST_PWR_GLITCH, "power_glitch", "power glitch",                      true,  false},
        {ESP_RST_CPU_LOCKUP, "cpu_lockup",   "CPU lockup",                        true,  false},
        {ESP_RST_EFUSE,      "efuse",        "efuse error",                       true,  false},
    };
    constexpr ResetReason c_unknownReset{ESP_RST_UNKNOWN, "unknown", "unknown", true, false};

    const ResetReason& lookup(esp_reset_reason_t reason)
    {
        for (const ResetReason& r : c_resetReasons)
        {
            if (r.reason == reason) return r;
        }
        return c_unknownReset;
    }

    std::optional<Services::PanicInfo> readCoreDump()
    {
#if CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH && CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF
        if (esp_core_dump_image_check() != ESP_OK) return std::nullopt;

        // Over 1 KB (it holds a stack dump), too much for the caller's stack.
        auto summary = std::make_unique<esp_core_dump_summary_t>();
        if (esp_core_dump_get_summary(summary.get()) != ESP_OK) return std::nullopt;

        Services::PanicInfo panic;
        panic.task.assign(summary->exc_task, strnlen(summary->exc_task, sizeof(summary->exc_task)));
        panic.pc = summary->exc_pc;
        panic.ra = summary->ex_info.ra;
        panic.sp = summary->ex_info.sp;
        panic.mcause = summary->ex_info.mcause;
        panic.mtval = summary->ex_info.mtval;
        const auto* sha = reinterpret_cast<const char*>(summary->app_elf_sha256);
        panic.elfSha.assign(sha, strnlen(sha, sizeof(summary->app_elf_sha256)));

        char reason[128];
        if (esp_core_dump_get_panic_reason(reason, sizeof(reason)) == ESP_OK) panic.reason = reason;
        return panic;
#else
        return std::nullopt;
#endif
    }

    // Stored formats. Their keys stay as they are, so records written by
    // older firmware still load.
    void toJson(JsonObject obj, const Services::CrashRecord& crash)
    {
        obj["reset_reason"] = crash.resetReason;
        obj["boot_id"] = crash.bootId;
        obj["version"] = crash.version;
        obj["uptime_ms"] = crash.uptimeMs;
        if (crash.panic)
        {
            const Services::PanicInfo& p = *crash.panic;
            JsonObject panic = obj["panic"].to<JsonObject>();
            panic["task"] = p.task;
            panic["reason"] = p.reason;
            panic["pc"] = p.pc;
            panic["ra"] = p.ra;
            panic["sp"] = p.sp;
            panic["mcause"] = p.mcause;
            panic["mtval"] = p.mtval;
            panic["elf_sha"] = p.elfSha;
        }
        JsonArray lines = obj["lines"].to<JsonArray>();
        for (const std::string& line : crash.lines)
        {
            lines.add(line);
        }
    }

    Services::CrashRecord fromJson(JsonObjectConst obj)
    {
        Services::CrashRecord crash;
        crash.resetReason = obj["reset_reason"] | "";
        crash.bootId = obj["boot_id"] | "";
        crash.version = obj["version"] | "";
        crash.uptimeMs = obj["uptime_ms"] | 0u;
        if (JsonObjectConst p = obj["panic"]; !p.isNull())
        {
            Services::PanicInfo& panic = crash.panic.emplace();
            panic.task = p["task"] | "";
            panic.reason = p["reason"] | "";
            panic.pc = p["pc"] | 0u;
            panic.ra = p["ra"] | 0u;
            panic.sp = p["sp"] | 0u;
            panic.mcause = p["mcause"] | 0u;
            panic.mtval = p["mtval"] | 0u;
            panic.elfSha = p["elf_sha"] | "";
        }
        for (JsonVariantConst line : obj["lines"].as<JsonArrayConst>())
        {
            crash.lines.emplace_back(line | "");
        }
        return crash;
    }
}

namespace Services
{
    CrashLog::CrashLog(SettingsStore& store)
        : m_store(store)
    {}

    void CrashLog::begin(std::string_view bootId)
    {
        // The trail was taken over by the first line this boot logged.
        Utils::CrashTrail::Previous trail = Utils::CrashTrail::previous();
        Utils::CrashTrail::setBoot(bootId, ENGINE_VERSION);
        m_bootId = bootId;

        const ResetReason& reset = lookup(esp_reset_reason());
        if (reset.abnormal) s_log.warn("reset reason: %s", reset.name);
        else s_log.info("reset reason: %s", reset.name);

        std::scoped_lock lock(m_mutex);

        JsonDocument doc;
        if (auto json = m_store.loadDiag(c_crashKey); json && !deserializeJson(doc, *json))
        {
            m_crashCount = doc["count"] | 0u;
            if (JsonObjectConst last = doc["last"]; !last.isNull()) m_lastCrash = fromJson(last);
        }
        doc.clear();
        if (auto json = m_store.loadDiag(c_errorsKey); json && !deserializeJson(doc, *json))
        {
            for (JsonObjectConst e : doc.as<JsonArrayConst>())
            {
                m_errors.push_back({e["boot_id"] | "", e["line"] | ""});
            }
        }

        if (!reset.abnormal) return;

        CrashRecord crash;
        crash.resetReason = reset.code;
        // A deep enough brownout clears RTC memory, and the trail with it.
        if (trail.valid)
        {
            crash.bootId = std::move(trail.bootId);
            crash.version = std::move(trail.version);
            crash.uptimeMs = trail.uptimeMs;
            crash.lines = std::move(trail.lines);
        }
        // Only a reset that wrote a core dump: otherwise the one in flash is
        // from an older crash.
        if (reset.coreDump) crash.panic = readCoreDump();

        if (crash.panic)
        {
            s_log.warn("crashed in task '%s' at pc 0x%08lx: %s", crash.panic->task.c_str(),
                       static_cast<unsigned long>(crash.panic->pc), crash.panic->reason.c_str());
        }

        ++m_crashCount;
        m_lastCrash = std::move(crash);
        saveCrash();
    }

    void CrashLog::start(Tasks::Manager& tasks)
    {
        tasks.spawnTask("crash_log", [this]() {
            saveLoop();
        }, 1, 4096);
    }

    CrashLogInfo CrashLog::info() const
    {
        std::scoped_lock lock(m_mutex);
        return {m_crashCount, m_lastCrash, m_errors};
    }

    void CrashLog::clear()
    {
        std::scoped_lock lock(m_mutex);
        m_crashCount = 0;
        m_lastCrash.reset();
        m_errors.clear();
        m_store.clearDiag();
#if CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH
        esp_core_dump_image_erase();
#endif
    }

    void CrashLog::saveLoop()
    {
        const Utils::LogHistory& history = Utils::Log::history();
        char buf[Utils::LogHistory::MaxLine];
        uint32_t seq = 0;

        while (true)
        {
            vTaskDelay(c_errorCheckInterval);

            std::vector<std::string> found;
            while (auto line = history.read(seq, buf))
            {
                if (Utils::Log::isError(*line)) found.emplace_back(*line);
                ++seq;
            }
            if (found.empty()) continue;

            std::scoped_lock lock(m_mutex);
            for (std::string& line : found)
            {
                m_errors.push_back({m_bootId, std::move(line)});
            }

            size_t bytes = 0;
            for (const ErrorRecord& e : m_errors)
            {
                bytes += e.bootId.size() + e.line.size() + 32;   // with the JSON around them
            }
            size_t drop = 0;
            while (m_errors.size() - drop > c_maxErrors || (bytes > c_maxErrorBytes && m_errors.size() - drop > 1))
            {
                bytes -= m_errors[drop].bootId.size() + m_errors[drop].line.size() + 32;
                ++drop;
            }
            m_errors.erase(m_errors.begin(), m_errors.begin() + drop);

            // Past the limit they are still reported, just not kept.
            if (m_errorSaves < c_maxErrorSaves)
            {
                ++m_errorSaves;
                saveErrors();
            }
        }
    }

    void CrashLog::saveCrash()
    {
        JsonDocument doc;
        doc["count"] = m_crashCount;
        if (m_lastCrash) toJson(doc["last"].to<JsonObject>(), *m_lastCrash);
        std::string json;
        serializeJson(doc, json);
        m_store.saveDiag(c_crashKey, json);
    }

    void CrashLog::saveErrors()
    {
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();
        for (const ErrorRecord& e : m_errors)
        {
            JsonObject item = arr.add<JsonObject>();
            item["boot_id"] = e.bootId;
            item["line"] = e.line;
        }
        std::string json;
        serializeJson(doc, json);
        m_store.saveDiag(c_errorsKey, json);
    }
}
