#include "Wire.h"
#include <esp_mac.h>
#include <esp_random.h>

namespace Wire
{
    namespace
    {
        // "0x4200a1b2", ready for addr2line.
        std::string hex(uint32_t value)
        {
            char buf[11];
            snprintf(buf, sizeof(buf), "0x%08lx", static_cast<unsigned long>(value));
            return buf;
        }
    }

    std::string_view deviceId()
    {
        static const std::string id = [] {
            uint8_t mac[6];
            esp_efuse_mac_get_default(mac);
            char buf[13];
            snprintf(buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            return std::string(buf);
        }();
        return id;
    }

    std::string_view bootId()
    {
        static const std::string id = [] {
            char buf[9];
            snprintf(buf, sizeof(buf), "%08lx", static_cast<unsigned long>(esp_random()));
            return std::string(buf);
        }();
        return id;
    }

    void fixture(JsonObject obj, const FixtureInfo& info)
    {
        const Config::Fixture& c = info.config;
        obj["id"] = c.m_ID;
        obj["name"] = c.name;
        obj["type"] = c.type;
        obj["universe"] = c.universe;
        obj["address"] = c.address;
        obj["preset_index"] = c.selectedPreset;
        obj["num_presets"] = c.numPresets;
        obj["highlighted"] = c.highlighted;
        obj["footprint"] = info.footprint;
    }

    void fixtures(JsonArray arr, const Handler::FixturesInfo& info)
    {
        for (const FixtureInfo& fix : info.fixtures)
        {
            fixture(arr.add<JsonObject>(), fix);
        }
    }

    void inputs(JsonArray arr, const Handler::InputsInfo& info)
    {
        for (const Handler::InputInfo& input : info.inputs)
        {
            JsonObject item = arr.add<JsonObject>();
            item["type"] = "SACN";
            item["universe"] = input.universe;
            item["packets"] = input.packets;
            item["active"] = input.active;
        }
    }

    void settings(JsonObject obj, const Config::Engine& config)
    {
        obj["serial_report"] = config.serial_report_task;
        obj["wireless_report"] = config.wireless_report_task;
        obj["wifi_animation"] = config.wifi_animation;
    }

    void wifi(JsonObject obj, const Net::WifiInfo& info, const std::string& ssid)
    {
        obj["ssid"] = ssid;
        obj["rssi"] = info.rssi;
        obj["local_ip"] = info.ip;
        obj["connected"] = info.connected;
    }

    void describe(JsonObject obj, const DeviceInfo& info)
    {
        obj["version"] = ENGINE_VERSION;
        obj["protocol"] = PROTOCOL_VERSION;
        obj["device_id"] = deviceId();
        obj["boot_id"] = bootId();
        settings(obj["settings"].to<JsonObject>(), info.settings);
        wifi(obj["wifi"].to<JsonObject>(), info.wifi, info.settings.ssid);
        fixtures(obj["fixtures"].to<JsonArray>(), info.fixtures);
        inputs(obj["inputs"].to<JsonArray>(), info.inputs);
    }

    void logs(JsonObject obj, const LogsInfo& info)
    {
        obj["boot_id"] = bootId();
        obj["first"] = info.first;
        obj["next"] = info.next;
        obj["lost"] = info.lost;
        obj["reset"] = info.reset;
        JsonArray lines = obj["lines"].to<JsonArray>();
        for (const std::string& line : info.lines)
        {
            lines.add(line);
        }
    }

    void crashLog(JsonObject obj, const Services::CrashLogInfo& info)
    {
        obj["crash_count"] = info.crashCount;

        if (info.lastCrash)
        {
            const Services::CrashRecord& c = *info.lastCrash;
            JsonObject crash = obj["crash"].to<JsonObject>();
            crash["reset_reason"] = c.resetReason;
            crash["boot_id"] = c.bootId;
            crash["version"] = c.version;
            crash["uptime_ms"] = c.uptimeMs;
            if (c.panic)
            {
                JsonObject panic = crash["panic"].to<JsonObject>();
                panic["task"] = c.panic->task;
                panic["reason"] = c.panic->reason;
                panic["pc"] = hex(c.panic->pc);
                panic["ra"] = hex(c.panic->ra);
                panic["sp"] = hex(c.panic->sp);
                panic["mcause"] = c.panic->mcause;
                panic["mtval"] = hex(c.panic->mtval);
                panic["elf_sha"] = c.panic->elfSha;
            }
            else
            {
                crash["panic"] = nullptr;
            }
            JsonArray lines = crash["lines"].to<JsonArray>();
            for (const std::string& line : c.lines)
            {
                lines.add(line);
            }
        }
        else
        {
            obj["crash"] = nullptr;
        }

        JsonArray errors = obj["errors"].to<JsonArray>();
        for (const Services::ErrorRecord& e : info.errors)
        {
            JsonObject item = errors.add<JsonObject>();
            item["boot_id"] = e.bootId;
            item["line"] = e.line;
        }
    }
}
