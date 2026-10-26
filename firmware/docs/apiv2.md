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
| `forbidden`    | Needs confirmation (`err.arg` is `confirm`), or the setting is read-only (`err.arg` is `settings.<key>`) |

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
- Output settings (`num_leds`, `num_groups`, ...): described by each output's own `settings` list; see [Outputs and settings](#outputs-and-settings).
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

## Outputs and settings

A fixture has one or more **outputs** (an LED strip, a pan or tilt motor, ...).
Fixtures and outputs differ, so their options are not fixed API fields. Each
output reports a list of **settings**, and a client reads that list and writes
by key. A client needs no built-in knowledge of what a tilt motor can do.

A setting:

```
{"key": "invert", "type": "bool", "value": false, "writable": true, "requires_reboot": false}
{"key": "range_deg", "type": "int", "value": 180, "min": 0, "max": 270, "writable": true, "requires_reboot": false}
{"key": "mode", "type": "enum", "value": "smooth", "options": ["smooth", "fast"], "writable": true, "requires_reboot": false}
```

- `type` is `bool`, `int` or `enum`. `min`/`max` are present for `int`,
  `options` for `enum`.
- `writable: false` is read-only (for example a value set through a preset).
- `requires_reboot: true`: the value is saved and the device restarts after
  1 s to apply it, so the reply has `"rebooting": true`.

An output:

```
{"index": 0, "type": "WS2815-1D", "name": "LED1D", "hardware_pin": 5,
 "footprints": [180, 90, 60, ...],
 "settings": [
   {"key": "num_leds",   "type": "int", "value": 60, "min": 1, "max": 1000, "writable": true,  "requires_reboot": true},
   {"key": "num_groups", "type": "int", "value": 60, "min": 1, "max": 1000, "writable": false, "requires_reboot": false}]}
```

- `index` is the output's position in the fixture and is what requests use.
- `footprints[i]` is the output's footprint under preset `i`.
- Which keys exist depends on the output type. A key a client does not know is
  shown as-is; a UI can build its control from `type`, `min`/`max` or
  `options`.
- Settings are saved with the fixture, per output. A saved key the output no
  longer has is ignored on boot.

Fixtures with settings of their own (not tied to one output) are not
supported yet. The core fixture fields (`name`, `universe`, `address`,
`preset_index`, `highlight`) are fixed API fields, below.

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
     "outputs": [{"index": 0, "type": "WS2815-1D", "name": "LED1D", "hardware_pin": 5,
                  "footprints": [180, 90, 60, ..., 3],
                  "settings": [{"key": "num_leds", ...}, {"key": "num_groups", ...}]}],
     "presets": [{"name": "group by 1",  "settings": [{"num_groups": 60}]},
                 {"name": "group by 2",  "settings": [{"num_groups": 30}]},
                 ...
                 {"name": "group by 60", "settings": [{"num_groups": 1}]}]}}
```

- A preset is a named bundle of output settings: `settings[i]` holds the
  values it applies to output `i`. `presets[i]` has the same index as
  `preset_index`.
- `describe` and `fixtures` never include `outputs` or `presets`; they stay
  small. A client calls `get_fixture` with `include` once per fixture.

Errors: `missing_arg` (`id`), `not_found` (no such fixture), `invalid_arg`
(unknown entry in `include`).

## set_fixture

Any combination of the fixture fields `universe`, `address`, `preset_index`,
`name` and `highlight`, and output `settings`. `id` is required.

```
-> {"cmd": "set_fixture", "id": 0, "universe": 5, "address": 10}
<- {"ok": true, "data": {"changed": true, "fixture": {"id": 0, "name": "miza", "universe": 5, "address": 10, ...}}}

-> {"cmd": "set_fixture", "id": 0}
<- {"ok": true, "data": {"changed": false, "fixture": {...}}}
```

`changed` replaces the `no changes` status. `fixture` is the fixture after the
change, the same object `get_fixture` returns.

- `universe`: 1–63999. A new fixture starts on universe `id + 1`.
- `address`: 0–511 (start channel minus one).
- `preset_index`: `0` to `num_presets - 1`. An invalid index is
  `out_of_range` (protocol 1 ignored it silently).
- `highlight`: boolean. `true` lights all of the fixture's LEDs white, `false`
  returns to DMX. Not saved across reboots.
- Changes are saved to flash 1 s after the last change; `reboot` saves first.
- If any argument is invalid, nothing is applied.

### Output settings

`output` is an output `index`, and `settings` is an object of key/value
pairs for that output. Both are required together.

```
-> {"cmd": "set_fixture", "id": 1, "output": 0, "settings": {"invert": true, "range_deg": 120}}
<- {"ok": true, "data": {"changed": true, "fixture": {"id": 1, ..., "outputs": [...]}}}

-> {"cmd": "set_fixture", "id": 0, "output": 0, "settings": {"num_leds": 120}}
<- {"ok": true, "rebooting": true, "data": {"changed": true, "fixture": {...}}}
```

- When `settings` is sent, `fixture` also includes `outputs`, so the client
  sees the new values.
- Each key is checked against the output's setting list (type, `min`/`max`,
  `options`, `writable`). Fixture fields and settings in one request are
  checked together, and nothing is applied if any of them is invalid.
- A setting with `requires_reboot` saves, then the device restarts after
  1 s and the reply has `"rebooting": true`. The preset also resets to 0, because
  presets depend on the setting (`num_leds` sets which group sizes exist).
- Setting a key to the value it already has is not a change.

Errors:

```
-> {"cmd": "set_fixture"}
<- {"ok": false, "err": {"code": "missing_arg", "arg": "id", "msg": "missing id"}}

-> {"cmd": "set_fixture", "id": 0, "universe": 0}
<- {"ok": false, "err": {"code": "out_of_range", "arg": "universe", "msg": "universe must be 1-63999"}}

-> {"cmd": "set_fixture", "id": 0, "preset_index": 99}
<- {"ok": false, "err": {"code": "out_of_range", "arg": "preset_index", "msg": "preset_index out of range"}}

-> {"cmd": "set_fixture", "id": 0, "output": 7, "settings": {"invert": true}}
<- {"ok": false, "err": {"code": "not_found", "arg": "output", "msg": "no such output"}}

-> {"cmd": "set_fixture", "id": 0, "output": 0, "settings": {"bogus": 1}}
<- {"ok": false, "err": {"code": "invalid_arg", "arg": "settings.bogus", "msg": "unknown setting"}}

-> {"cmd": "set_fixture", "id": 0, "output": 0, "settings": {"num_groups": 5}}
<- {"ok": false, "err": {"code": "forbidden", "arg": "settings.num_groups", "msg": "setting is read-only"}}

-> {"cmd": "set_fixture", "id": 0, "output": 0, "settings": {"num_leds": 5000}}
<- {"ok": false, "err": {"code": "out_of_range", "arg": "settings.num_leds", "msg": "num_leds must be 1-1000"}}

-> {"cmd": "set_fixture", "id": 0, "settings": {"invert": true}}
<- {"ok": false, "err": {"code": "missing_arg", "arg": "output", "msg": "missing output"}}
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
<- {"ok": true, "data": {"ssid": "MyNet", ...}}
```

`set_wifi` needs `"confirm": true`, otherwise it fails with `forbidden`. It
saves the credentials, replies, then reconnects after 1 s. `ssid` is required. `password` is optional: omitted keeps the stored one, empty
sets an open network. The device drops off the network briefly, so the
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
| `setfixture` `leds` | `num_leds`, now an output setting: `set_fixture` with `output` and `settings` |
| Fixture and output options as fixed fields | per-output `settings` list (type, range, `requires_reboot`), written by key |
| `setfixture` invalid `presetIndex` ignored | `out_of_range` |
| `presetIndex`, `numPresets` | `preset_index`, `num_presets` |
| `num_leds` documented as group count in `outputs` and `description` (the firmware already reports LEDs) | `num_leds` is LEDs, `num_groups` is groups; both are settings of the output |
| `presets` as `{selected, presets: {groups: [...]}}` | `presets: [{name, settings: [...]}]`; selection is `preset_index` |
| `outputs[].footprint` (array in `outputs`, number in `description`), `uuid`, `hardware_pin` in `config` | `index`, `footprints` array, `settings` list; no `description` object |
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
