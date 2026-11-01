#pragma once
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "Services/SettingsStore.h"
#include "Tasks/TaskExecutor.h"

namespace Services
{
    // From the core dump the panic handler writes to flash. Addresses are for
    // addr2line against the firmware.elf of the crashed version.
    struct PanicInfo
    {
        std::string task;
        std::string reason;    // "Interrupt wdt timeout on CPU0", "abort() was called at PC ...", ...
        uint32_t pc = 0;
        uint32_t ra = 0;
        uint32_t sp = 0;
        uint32_t mcause = 0;
        uint32_t mtval = 0;
        std::string elfSha;    // start of the crashed firmware's ELF SHA-256
    };

    // The last abnormal reset.
    struct CrashRecord
    {
        std::string resetReason;   // "panic", "task_wdt", "brownout", ...
        // Of the boot that crashed. Empty if it went down before Engine::init.
        std::string bootId;
        std::string version;
        uint32_t uptimeMs = 0;     // when it logged its last line
        std::optional<PanicInfo> panic;
        std::vector<std::string> lines;   // its last log lines, oldest first
    };

    struct ErrorRecord
    {
        std::string bootId;
        std::string line;
    };

    struct CrashLogInfo
    {
        uint32_t crashCount = 0;   // abnormal resets since the log was cleared
        std::optional<CrashRecord> lastCrash;
        std::vector<ErrorRecord> errors;   // Error log lines from recent boots, oldest first
    };

    // Keeps what went wrong in NVS: the last abnormal reset (with the log
    // lines leading up to it, see Utils::CrashTrail) and recent Error lines.
    // The full core dump of the last panic stays in its flash partition.
    class CrashLog
    {
    public:
        explicit CrashLog(SettingsStore& store);

        // Logs the reset reason and, if it was abnormal, saves a record of it.
        // Call once at boot, after the store is loaded.
        void begin(std::string_view bootId);

        // Spawns the task that saves Error log lines. Call once, after begin().
        void start(Tasks::Manager& tasks);

        [[nodiscard]] CrashLogInfo info() const;

        // Forgets the crash count, the records and the core dump.
        void clear();

    private:
        void saveLoop();
        // Caller holds m_mutex.
        void saveCrash();
        void saveErrors();

        SettingsStore& m_store;
        std::string m_bootId;

        // Guards everything below; read by command tasks.
        mutable std::mutex m_mutex;
        uint32_t m_crashCount = 0;
        std::optional<CrashRecord> m_lastCrash;
        std::vector<ErrorRecord> m_errors;
        uint32_t m_errorSaves = 0;   // this boot
    };
}
