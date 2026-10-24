# Firmware Command API

Firmware `0.99`. Examples below are real replies from a board with one
60-LED `StripWS2815` fixture.

## Network Overview

| Port  | Protocol | Direction | Description                        |
|-------|----------|-----------|------------------------------------|
| 8888  | TCP      | Both      | Commands and their responses       |
| 12343 | UDP      | Send      | Heartbeat broadcast (every 500 ms) |
| 12344 | UDP      | Send      | Wireless report (every 10 s)       |
| 5568  | UDP      | Receive   | sACN / E1.31 DMX input             |
| 3232  | TCP/UDP  | Receive   | Firmware update (only while `ota` is open) |
| Serial| USB CDC  | Both      | Commands and responses; logs       |

Over TCP, each request is one line of JSON ending in `\n`; its response comes
back as one `\n`-terminated JSON line on the same connection. A client may send
several requests on one connection. The device serves one client at a time and
closes a connection after 10 s without data, or if a request line exceeds
4096 bytes. Serial works the same way, line in / line out.

Older clients that waited for responses on port 12345 (TCP or UDP) must switch
to reading the same connection; see `helper/sender2.py`.

---

## Response keys

| Key      | Meaning                        |
|----------|--------------------------------|
| `err`    | Error message                  |
| `status` | Status string (`ok`, etc.)     |
| `req`    | Echo of what was requested     |
| `val`    | Returned value                 |

---

## describe

Settings (never the wifi password), wifi, fixtures and inputs in one reply.

```
-> {"request": "describe"}
<- {"version": "v0.98", "serial_report_task": true, "wireless_report_task": true,
    "wifi_animation": false, "ssid": "Ledique",
    "wifi": {"rssi": -55, "local_ip": "192.168.0.140", "connected": true},
    "fixtures": {"fixtures": [{"id": 0, "name": "miza", "type": "StripWS2815", "universe": 10,
                               "address": 0, "presetIndex": 0, "numPresets": 12,
                               "highlighted": false, "footprint": 180}]},
    "inputs": {"num_inputs": 1, "inputs": [{"type": "SACN", "universe": 10, "packets": 68}]}}
```

## fixtures / getfixture

`fixtures`, and `getfixture` without an `id`, return every fixture.

```
-> {"request": "fixtures"}
<- {"fixtures": [{"id": 0, "name": "miza", "type": "StripWS2815", "universe": 10, "address": 0,
                  "presetIndex": 0, "numPresets": 12, "highlighted": false, "footprint": 180}]}

-> {"request": "getfixture", "id": 0}
<- {"id": 0, "name": "miza", "type": "StripWS2815", "universe": 10, "address": 0,
    "presetIndex": 0, "numPresets": 12, "highlighted": false, "footprint": 180}
```

`footprint` is the number of DMX channels the fixture uses under its current
preset (3 per color group).

Single values: `"value"` is one of `universe`, `address`, `preset`, `name`,
`type`, `config`, `presets`, `outputs`.

```
-> {"request": "getfixture", "id": 0, "value": "universe"}
<- {"req": "universe", "val": 10}

-> {"request": "getfixture", "id": 0, "value": "config"}
<- {"req": "config", "val": {
     "id": 0, "name": "miza", "type": "StripWS2815", "universe": 10, "address": 0,
     "presetIndex": 0, "numPresets": 12, "highlighted": false, "footprint": 180,
     "outputs": [{"uuid": 0, "type": "WS2815-1D", "hardware_pin": 5, "num_leds": 60, "num_groups": 60,
                  "description": {"name": "LED1D", "footprint": 180, "hardware_pin": 5, "num_leds": 60}}]
   }}

-> {"request": "getfixture", "id": 0, "value": "presets"}
<- {"req": "presets", "val": {"selected": 0, "presets": {"groups": [
     {"name": "group by 1", "settings": [{"num_groups": 60}]},
     {"name": "group by 2", "settings": [{"num_groups": 30}]},
     ...
     {"name": "group by 60", "settings": [{"num_groups": 1}]}
   ]}}}
```

`outputs` lists each output with its footprint under every preset (index i =
preset i):

```
-> {"request": "getfixture", "id": 0, "value": "outputs"}
<- {"req": "outputs", "val": [{"name": "LED1D", "footprint": [180, 90, 60, 45, 36, 30, 18, 15, 12, 9, 6, 3],
                               "hardware_pin": 5, "num_leds": 60}]}
```

Known quirk: `num_leds` under `description` and in `outputs` is the current
group count, not the LED count (they're equal on the default preset). Use
`config.outputs[].num_leds` for the LED count.

Errors:

```
-> {"request": "getfixture", "id": 99}
<- {"err": "fixture not found"}

-> {"request": "getfixture", "id": 0, "value": "bogus"}
<- {"err": "unknown value"}
```

## setfixture

Any combination of `universe`, `address`, `presetIndex`, `name`, `highlight`,
`leds`. The reply is the fixture after the change, with `status` `ok`,
`no changes`, or `ok, rebooting` (after a new `leds`).

```
-> {"request": "setfixture", "id": 0, "universe": 5, "address": 10}
<- {"status": "ok", "id": 0, "name": "miza", "universe": 5, "address": 10, ...}

-> {"request": "setfixture", "id": 0}
<- {"status": "no changes", "id": 0, ...}
```

- `universe`: 1–63999. A new fixture starts on universe `id + 1`.
- `presetIndex`: an invalid index is ignored (the fixture keeps its preset).
- `leds`: LED count of a strip, 1–1000. Saved, the preset is reset to 0, and
  the device reboots after 1 s to rebuild the strip. Until a count is set, a
  fixture uses the default from `main.cpp`. Reported as `num_leds`.
- Changes are saved to flash 1 s after the last change; `reboot` saves first.
- `highlight` **toggles** all of the fixture's LEDs to white, whatever value is
  sent — `"highlight": true` again turns it off. Not saved across reboots.

Errors:

```
-> {"request": "setfixture"}
<- {"err": "missing id"}

-> {"request": "setfixture", "id": 0, "universe": 0}
<- {"err": "universe must be 1-63999"}

-> {"request": "setfixture", "id": 0, "leds": 0}
<- {"err": "leds must be 1-1000"}
```

## inputs

One entry per subscribed universe. `packets` counts accepted sACN packets
since boot (or since the universe was first subscribed).

```
-> {"request": "inputs"}
<- {"num_inputs": 1, "inputs": [{"type": "SACN", "universe": 10, "packets": 68, "active": true}]}
```

`active`: data arrived within the last 2.5 s (E1.31 source-loss timeout).

## wifi

```
-> {"request": "wifi"}
<- {"rssi": -54, "local_ip": "192.168.0.140", "connected": true, "ssid": "Ledique"}

-> {"request": "wifi", "ssid": "MyNet", "password": "pass123"}
<- {"status": "ok", "rssi": -45, "local_ip": "...", "connected": true, "ssid": "MyNet"}
```

Saves the credentials, replies, then reconnects after 1 s.

## reboot

```
-> {"request": "reboot"}
<- {"status": "rebooting"}
```

Saves pending fixture changes, restarts after 1 s.

## factory_reset

```
-> {"request": "factory_reset"}
<- {"status": "factory reset, rebooting"}
```

Erases all stored settings and fixture records, restarts after 1 s. Changes
made in that second are not saved. The next boot uses defaults (default wifi,
universe `id + 1` per fixture).

## ota

Opens a window in which the device accepts a firmware update over WiFi
(ArduinoOTA / espota, port 3232, password `OTA_PASSWORD` in `EngineConfig.h`).
Closed by default; it closes again after `minutes` (1–30, default 5) or after
an update.

```
-> {"request": "ota"}
<- {"status": "ota open", "minutes": 5, "port": 3232}

-> {"request": "ota", "minutes": 10}
<- {"status": "ota open", "minutes": 10, "port": 3232}
```

### Updating over WiFi

1. Open the window: `printf '{"request":"ota"}\n' | nc <ip> 8888`
2. Upload: `pio run -e lolin_c3_mini_ota -t upload --upload-port <ip>`
   (the env defaults to `192.168.0.140`).

The LEDs go off during the update (~10 s), then the device reboots into the
new firmware. That image is confirmed after 30 s with wifi up; if it crashes
or is reset before that, the bootloader boots the previous firmware again.
Don't power-cycle or open the serial port in those first 30 s, or it rolls
back.

The first switch to this layout (`min_spiffs.csv`, two app slots) must be
flashed over USB once. Settings survive it.

## logs

Recent log lines (Info and up) kept in a 4 KB ring buffer, oldest first.

```
-> {"request": "logs"}
<- {"first": 0, "next": 6, "lost": 0, "lines": ["I    0.023 [ENGINE] reset reason: USB", ...]}

-> {"request": "logs", "since": 6}
<- {"first": 6, "next": 11, "lost": 0, "lines": [...]}
```

Pass the previous `next` as `since` to get only new lines. `lost` counts lines
after `since` that were already overwritten. A `since` from before a reboot
starts over from the oldest line.

## serial_report_task / wireless_report_task / wifi_animation

Without `value`: read. With a bool `value`: set and save.

```
-> {"request": "serial_report_task"}
<- {"req": "serial_report_task", "val": true}

-> {"request": "wireless_report_task", "value": false}
<- {"req": "wireless_report_task", "val": false}
```

Report changes take effect on the next 10 s cycle.

`wifi_animation`: a running-pixel animation shows until wifi first connects.
If enabled, it also runs while wifi is down later; if disabled, the LEDs keep
their last frame.

## Dispatcher errors

```
-> not json
<- {"err": "invalid json"}

-> {"foo": "bar"}
<- {"err": "missing cmd"}

-> {"request": "unknown"}
<- {"err": "unknown cmd"}
```

## Heartbeat (UDP broadcast, port 12343, every 500 ms)

Sent while wifi is connected.

```
{"version": "v0.98", "fixtures": [{"name": "miza", "type": "StripWS2815"}]}
```

## Wireless report (UDP broadcast, port 12344, every 10 s)

Engine settings only (no password). Sent when `wireless_report_task` is on and
wifi is connected.

```
{"version": "v0.98", "serial_report_task": true, "wireless_report_task": true,
 "wifi_animation": false, "ssid": "Ledique"}
```

## Serial report (USB serial, every 10 s)

Full state, one line. Active when `serial_report_task` is on.

```
{"settings": {...as above...}, "wifi": {...}, "fixture_handler": {"fixtures": [...]},
 "input_handler": {"num_inputs": 1, "inputs": [...]}}
```
