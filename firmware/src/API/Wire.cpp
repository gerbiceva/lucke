#include "Wire.h"
#include <esp_mac.h>
#include <esp_random.h>

namespace Wire
{
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
}
