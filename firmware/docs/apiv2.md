# Firmware Command API, protocol 2

Draft for `protocol: 2` (see [`api.md`](./api.md) for protocol 1). Protocol 2
is a breaking change. Everything that changed is listed under
[Migrating from protocol 1](#migrating-from-protocol-1).

Examples assume one 60-LED `StripWS2815` fixture.

## Network Overview

Unchanged from protocol 1.

| Port  | Protocol | Direction | Description                        |
|-------|----------|-----------|------------------------------------|
| 8888  | TCP      | Both      | Commands and their responses       |
| 12343 | UDP      | Send      | Heartbeat broadcast (every 500 ms) |
| 12344 | UDP      | Send      | Wireless report (every 10 s)       |
| 5568  | UDP      | Receive   | sACN / E1.31 DMX input             |
| 3232  | TCP/UDP  | Receive   | Firmware update (only while `ota` is open) |
| Serial| USB CDC  | Both      | Commands and responses; logs       |

Each request is one line of JSON ending in `\n`; its response is one
`\n`-terminated JSON line on the same connection. A connection may carry
several requests. The device serves one client at a time and closes a
connection after 2 s without data, or if a request line exceeds 4096 bytes.
Serial works the same way.

---

## Conventions

### Requests

```
{"cmd": "<command>", ...arguments}
```

The command key is `cmd` (it was `request` in protocol 1). Command names,
argument names and reply keys are all `snake_case`.

### Replies

Every reply is an envelope with a boolean `ok`.

Success:

```
{"ok": true, "data": {...}}
```

`data` is always present. It is an object or an array, and `{}` when a command
has nothing to return. If the device is about to restart because of this
command, the envelope also has `"rebooting": true` (it is omitted otherwise).

Error:

```
{"ok": false, "err": {"code": "out_of_range", "msg": "universe must be 1-63999"}}
```

`code` is stable and meant for programs; `msg` is for humans and may change.

| Code           | Meaning                                              |
|----------------|------------------------------------------------------|
| `invalid_json` | The line was not valid JSON                          |
| `missing_cmd`  | No `cmd` key                                         |
| `unknown_cmd`  | `cmd` is not a command                               |
| `missing_arg`  | A required argument is absent (`err.arg` names it)   |
| `invalid_arg`  | Wrong type or unknown enum value (`err.arg`)         |
| `out_of_range` | A number is outside its allowed range (`err.arg`)    |
| `not_found`    | The fixture or other target does not exist           |
| `forbidden`    | Needs confirmation, or its window is closed (`err.arg` for a missing `confirm`) |

### Versions

Two numbers, in the heartbeat and in `describe`:

- `version`: the firmware build (`"1.1.0"`). Changes every release.
- `protocol`: the command API (`2`). Bumped only for a breaking change.
  Added fields and commands don't bump it; clients ignore what they don't
  know.

A client reads `protocol` from the heartbeat (or `describe`) before sending
commands, and picks its request and reply format from it. Protocol 1 devices
don't understand `cmd` and answer `{"err": "missing cmd"}`, so a client that
isn't sure can send `{"cmd": "describe"}` and fall back to protocol 1 on that
reply.

### Types

- Fixture `id`: integer index, `0`-based.
- `universe`: 1–63999.
- `num_leds`: physical LEDs, 1–1000.
- `num_groups`: color groups, which depends on the preset.
- Booleans are JSON booleans only. `"true"` or `1` gives `invalid_arg`.

---

## describe

Everything in one reply (never the wifi password).

```
-> {"cmd": "describe"}
<- {"ok": true, "data": {
     "version": "1.1.0", "protocol": 2, "boot_id": "a3f91c20",
     "settings": {"serial_report": true, "wireless_report": true, "wifi_animation": false},
     "wifi": {"ssid": "Ledique", "rssi": -55, "local_ip": "192.168.0.140", "connected": true},
     "fixtures": [{"id": 0, "name": "miza", "type": "StripWS2815", "universe": 10,
                   "address": 0, "preset_index": 0, "num_presets": 12,
                   "highlighted": false, "footprint": 180}],
     "inputs": [{"type": "SACN", "universe": 10, "packets": 68, "active": true}]}}
```

`boot_id` changes on every boot (see [`logs`](#logs)). `fixtures` and `inputs`
are plain arrays, with no `num_fixtures`/`num_inputs` and no nested
`fixtures.fixtures`.

## fixtures

All fixtures; the same objects as `describe.fixtures`.

```
-> {"cmd": "fixtures"}
<- {"ok": true, "data": [{"id": 0, "name": "miza", ...}]}
```

## get_fixture

One fixture. `id` is required (protocol 1's `getfixture` without `id` is gone;
use `fixtures`).

```
-> {"cmd": "get_fixture", "id": 0}
<- {"ok": true, "data": {"id": 0, "name": "miza", "type": "StripWS2815", "universe": 10,
                         "address": 0, "preset_index": 0, "num_presets": 12,
                         "highlighted": false, "footprint": 180}}
```

`footprint` is the number of DMX channels the fixture uses under its current
preset (3 per color group).

Optional `include`: an array of `"outputs"` and/or `"presets"`, which adds the
matching key to the object. This replaces protocol 1's `getfixture` `value`
(single values are just fields of the object).

```
-> {"cmd": "get_fixture", "id": 0, "include": ["outputs", "presets"]}
<- {"ok": true, "data": {
     "id": 0, "name": "miza", ...,
     "outputs": [{"uuid": 0, "type": "WS2815-1D", "name": "LED1D", "hardware_pin": 5,
                  "num_leds": 60, "num_groups": 60,
                  "footprints": [180, 90, 60, 45, 36, 30, 18, 15, 12, 9, 6, 3]}],
     "presets": [{"name": "group by 1", "num_groups": 60},
                 {"name": "group by 2", "num_groups": 30},
                 ...
                 {"name": "group by 60", "num_groups": 1}]}}
```

- `outputs[].num_leds` is the physical LED count and `num_groups` the current
  group count. (In protocol 1 `num_leds` in `outputs` and `description` held
  the group count.)
- `footprints[i]` is the output's footprint under preset `i`.
- `presets[i]` has the same index as `preset_index`.

Errors: `missing_arg` (`id`), `not_found` (no such fixture), `invalid_arg`
(unknown entry in `include`).

## set_fixture

Any combination of `universe`, `address`, `preset_index`, `name`, `highlight`
and `num_leds`. `id` is required.

```
-> {"cmd": "set_fixture", "id": 0, "universe": 5, "address": 10}
<- {"ok": true, "data": {"changed": true, "fixture": {"id": 0, "name": "miza", "universe": 5, "address": 10, ...}}}

-> {"cmd": "set_fixture", "id": 0}
<- {"ok": true, "data": {"changed": false, "fixture": {...}}}

-> {"cmd": "set_fixture", "id": 0, "num_leds": 120}
<- {"ok": true, "rebooting": true, "data": {"changed": true, "fixture": {...}}}
```

`changed` replaces the `no changes` status. `fixture` is the fixture after the
change, the same object `get_fixture` returns.

- `universe`: 1–63999. A new fixture starts on universe `id + 1`.
- `address`: 0–511 (start channel minus one). An address that leaves no room
  for the fixture's footprint is `out_of_range`.
- `preset_index`: `0` to `num_presets - 1`. An invalid index is
  `out_of_range` (protocol 1 ignored it silently).
- `num_leds`: 1–1000 (was `leds`). Saved, the preset resets to 0, and the
  device reboots after 1 s to rebuild the strip; the reply has
  `"rebooting": true`.
- `highlight`: boolean. `true` lights all of the fixture's LEDs white, `false`
  returns to DMX. Not saved across reboots.
- Changes are saved to flash 1 s after the last change; `reboot` saves first.
- If any argument is invalid, nothing is applied.

Errors:

```
-> {"cmd": "set_fixture"}
<- {"ok": false, "err": {"code": "missing_arg", "arg": "id", "msg": "missing id"}}

-> {"cmd": "set_fixture", "id": 0, "universe": 0}
<- {"ok": false, "err": {"code": "out_of_range", "arg": "universe", "msg": "universe must be 1-63999"}}

-> {"cmd": "set_fixture", "id": 0, "preset_index": 99}
<- {"ok": false, "err": {"code": "out_of_range", "arg": "preset_index", "msg": "preset_index must be 0-11"}}
```

## inputs

One entry per subscribed universe. `packets` counts accepted sACN packets
since boot (or since the universe was first subscribed). `active`: data
arrived within the last 2.5 s (E1.31 source-loss timeout).

```
-> {"cmd": "inputs"}
<- {"ok": true, "data": [{"type": "SACN", "universe": 10, "packets": 68, "active": true}]}
```

## get_wifi / set_wifi

```
-> {"cmd": "get_wifi"}
<- {"ok": true, "data": {"ssid": "Ledique", "rssi": -54, "local_ip": "192.168.0.140", "connected": true}}

-> {"cmd": "set_wifi", "ssid": "MyNet", "password": "pass123", "confirm": true}
<- {"ok": true, "rebooting": false, "data": {"ssid": "MyNet", ...}}
```

`set_wifi` needs `"confirm": true`, otherwise it fails with `forbidden`. It
saves the credentials, replies, then reconnects after 1 s. `password` may be
empty for an open network. The device drops off the network briefly, so the
reply may not arrive if the new credentials are wrong.

The password is sent in plaintext; use this only on a network you trust.

## get_settings / set_settings

Replaces protocol 1's `serial_report_task`, `wireless_report_task` and
`wifi_animation` commands.

```
-> {"cmd": "get_settings"}
<- {"ok": true, "data": {"serial_report": true, "wireless_report": true, "wifi_animation": false}}

-> {"cmd": "set_settings", "wireless_report": false}
<- {"ok": true, "data": {"serial_report": true, "wireless_report": false, "wifi_animation": false}}
```

`set_settings` takes any subset of the three booleans, saves, and returns all
of them. Report changes take effect on the next 10 s cycle.

`wifi_animation`: a running-pixel animation shows until wifi first connects. If
enabled, it also runs while wifi is down later; if disabled, the LEDs keep
their last frame.

## reboot

```
-> {"cmd": "reboot"}
<- {"ok": true, "rebooting": true, "data": {}}
```

Saves pending fixture changes, restarts after 1 s.

## factory_reset

```
-> {"cmd": "factory_reset", "confirm": true}
<- {"ok": true, "rebooting": true, "data": {}}

-> {"cmd": "factory_reset"}
<- {"ok": false, "err": {"code": "forbidden", "arg": "confirm", "msg": "confirm required"}}
```

Erases all stored settings and fixture records, restarts after 1 s. Changes
made in that second are not saved. The next boot uses defaults (default wifi,
universe `id + 1` per fixture).

## ota

Opens a window in which the device accepts a firmware update over WiFi
(ArduinoOTA / espota, port 3232). Closed by default; it closes again after
`minutes` (1–30, default 5) or after an update.

```
-> {"cmd": "ota"}
<- {"ok": true, "data": {"minutes": 5, "port": 3232}}

-> {"cmd": "ota", "minutes": 10}
<- {"ok": true, "data": {"minutes": 10, "port": 3232}}
```

`minutes` outside 1–30 is `out_of_range`. The update procedure, rollback and
the first-flash note are the same as in protocol 1.

## logs

Recent log lines (Info and up) kept in a 4 KB ring buffer, oldest first.

```
-> {"cmd": "logs"}
<- {"ok": true, "data": {"boot_id": "a3f91c20", "first": 0, "next": 6, "lost": 0, "reset": false,
                         "lines": ["I    0.023 [ENGINE] reset reason: USB", ...]}}

-> {"cmd": "logs", "since": 6, "boot_id": "a3f91c20"}
<- {"ok": true, "data": {"boot_id": "a3f91c20", "first": 6, "next": 11, "lost": 0, "reset": false, "lines": [...]}}
```

Pass the previous `next` as `since`, and the `boot_id` you got with it. If
`boot_id` is omitted, `since` is trusted as in protocol 1. If it doesn't match
the current boot, the device ignores `since`, returns every line from the
start, and sets `reset: true` so the client can discard what it held. `lost`
counts lines after `since` that were already overwritten.

## Heartbeat (UDP broadcast, port 12343, every 500 ms)

Sent while wifi is connected. The shape is deliberately the same as in
protocol 1: it's how a client learns `protocol` before any command.

```
{"version": "1.1.0", "protocol": 2, "boot_id": "a3f91c20",
 "fixtures": [{"name": "miza", "type": "StripWS2815"}]}
```

`boot_id` is new; a client can use a change in it to notice a reboot.

## Wireless report (UDP broadcast, port 12344, every 10 s)

Settings only (no password). Sent when `wireless_report` is on and wifi is
connected. It is a bare object, not an envelope, like the heartbeat.

```
{"version": "1.1.0", "protocol": 2,
 "settings": {"serial_report": true, "wireless_report": true, "wifi_animation": false},
 "ssid": "Ledique"}
```

## Serial report (USB serial, every 10 s)

One line, active when `serial_report` is on. It is exactly `describe`'s `data`
(not wrapped in an envelope), so one parser covers both.

```
{"version": "1.1.0", "protocol": 2, "boot_id": "a3f91c20", "settings": {...}, "wifi": {...},
 "fixtures": [...], "inputs": [...]}
```

---

## Migrating from protocol 1

| Protocol 1 | Protocol 2 |
|---|---|
| `{"request": ...}` | `{"cmd": ...}` |
| Five reply shapes (bare, `{status,...}`, `{req,val}`, `{fixtures}`, `{status}`) | One envelope: `{ok, data}` or `{ok:false, err}` |
| `{"err": "text"}` | `{"ok": false, "err": {"code", "msg"}}` |
| `status` strings (`ok`, `no changes`, `ok, rebooting`, `rebooting`, `ota open`, `factory reset, rebooting`) | `ok`, `changed` and `rebooting` booleans; no status string |
| `getfixture` (no id) | removed; use `fixtures` |
| `getfixture` | `get_fixture` |
| `getfixture` + `value` | fields of `get_fixture`; `include` for `outputs` and `presets`; `config` is gone (it was the fixture plus `outputs`) |
| `setfixture` | `set_fixture`; reply is `{changed, fixture}` |
| `setfixture` `leds` | `num_leds` |
| `setfixture` invalid `presetIndex` ignored | `out_of_range` |
| `presetIndex`, `numPresets` | `preset_index`, `num_presets` |
| `num_leds` meant group count in `outputs` and `description` | `num_leds` is LEDs, `num_groups` is groups |
| `presets` as `{selected, presets: {groups: [{name, settings: [{num_groups}]}]}}` | `presets: [{name, num_groups}]`; selection is `preset_index` |
| `outputs[].footprint` (array in `outputs`, number in `description`) | `footprints` array, no `description` object |
| `serial_report_task`, `wireless_report_task`, `wifi_animation` commands | `get_settings` / `set_settings` with `serial_report`, `wireless_report`, `wifi_animation` |
| `wifi` (read and write) | `get_wifi` / `set_wifi` |
| `wifi` write unconfirmed | `set_wifi` needs `"confirm": true` |
| `factory_reset` unconfirmed | needs `"confirm": true` |
| `factory_reset` | unchanged |
| `describe` `fixtures: {fixtures: [...]}`, `inputs: {num_inputs, inputs}` | `fixtures: [...]`, `inputs: [...]` |
| `describe` flat settings keys | `settings` object |
| `logs` after a reboot restarted silently | `boot_id` in the reply; send it back, `reset` flags a mismatch |
| Heartbeat | adds `boot_id`; `protocol` is `2` |
| Serial report with `fixture_handler`/`input_handler` | `describe`'s `data` shape |
| Dispatcher `missing cmd` / `unknown cmd` text | `missing_cmd` / `unknown_cmd` codes |

Not changed (and why):

- **One client at a time.** Each request is short, so this only matters when a
  client holds a connection open. Protocol 2 does not change it.
- **No authentication.** `confirm` stops accidents, not attackers. Anyone on
  the network can still call every command.
- **OTA password** is still the build-time `OTA_PASSWORD` in `EngineConfig.h`.
