# Lucke Firmware — Current Architecture

> Branch `cpp23`, October 2026. Scope: `src/`. `unused/src_prev/` is the old
> tree and is not described here. Diagram: [`current_architecture.puml`](./current_architecture.puml).
> Wire protocol: [`apiv2.md`](./apiv2.md). Open work: [`modernization.md`](./modernization.md),
> [`improvement.md`](./improvement.md).

Target: ESP32-C3 (`lolin_c3_mini`), Arduino core on pioarduino, GCC 14,
`-std=gnu++23`, `-fno-rtti -fexceptions`.

---

## 1. Shape

```
main.cpp ── engine.addFixture<Fixtures::StripWS2815<60, 5>>() ── engine.init()

Engine (composition root: owns everything, wires it, starts the services)
 ├─ SettingsStore      the only NVS owner: engine settings + fixture records
 ├─ Tasks::Manager     spawns the FreeRTOS tasks
 ├─ Net::Wifi          station connect / reconnect / status
 ├─ InputHandler       sACN socket, one SyncBuffer per subscribed universe
 ├─ FixtureHandler     owns the Fixtures, ref-counts their universe span
 ├─ Services
 │   ├─ RenderLoop     the one task that drives the LEDs (Live / Animation / Off)
 │   ├─ Connectivity   wifi events -> "connected?" callbacks
 │   ├─ StatusReport   builds status JSON (never includes the wifi password)
 │   ├─ Telemetry      UDP heartbeat + optional serial / UDP reports
 │   ├─ CommandSession line protocol over serial
 │   └─ TcpListener    line protocol over TCP 8888
 └─ Commands           CommandDispatcher + one handler per request
```

`Engine` members are declared in dependency order (each holds references to
members above it), so the constructor initializer list is the wiring diagram.

## 2. Boot

1. Global `Engine engine;` constructs everything. NVS isn't ready yet, so
   nothing touches storage.
2. `setup()` calls `engine.addFixture<T>()` for each compiled-in fixture.
   `FixtureHandler::_addFixture` loads settings (first call opens NVS), gives
   the fixture an id, restores its stored record (`"fixtureN"`) or — first
   boot — assigns universe `id + 1` and applies the default preset, subscribes
   its universe span, and queues its record for saving.
3. `engine.init()`: load settings, start wifi, register commands, then
   `startServices()` spawns the tasks (below) and hooks Connectivity.

## 3. Tasks

| Task | Owner | Prio | Stack | Does |
|---|---|---|---|---|
| `input_update` | InputHandler | 3 | 30000 | waits on the sACN socket, drains packets into SyncBuffers |
| `render` | RenderLoop | 2 | 30000 | per mode: fixtures `update()` + `show()`, or the wifi animation, or nothing |
| `tcp_cmd` | TcpListener | 2 | 8192 | accepts one client at a time, line in / line out |
| `serial_cmd` | CommandSession | 1 | 8192 | same protocol over USB serial |
| `wifi_monitor` | Connectivity | 1 | 4096 | woken by wifi events, calls listeners on link change |
| `heartbeat` | Telemetry | 1 | 4096 | UDP broadcast every 500 ms |
| `report` | Telemetry | 1 | 15000 | every 10 s: serial report and/or UDP report, per flags |
| `settings_save` | SettingsStore | 1 | 4096 | writes queued fixture records after 1 s of quiet |

Rule: **no task is stopped or suspended from outside.** Deleting a task can
leave a lock it holds taken forever (this froze DMX input once). Tasks that
pause check a flag or mode instead (RenderLoop mode, Telemetry report flags).
`Tasks::Manager` therefore only spawns.

Connectivity listeners (in order): render mode (Live on connect; Animation or
Off on disconnect, per `wifi_animation`), open/close the TCP listener, ask
InputHandler to reopen the sACN socket.

## 4. DMX data path

```
UDP 5568 ─► SacnReceiver ─► InputHandler ─► SyncBuffer ─► Fixture::update ─► Output ─► Driver
 (1 socket,   validate      route by         512-byte       copy universe    map to     show()
  N groups)   E1.31         universe field   locked copy    span, slice      pixels
```

- **`Input::SacnReceiver`** — one UDP socket bound to `0.0.0.0:5568`;
  `join(u)` adds multicast group 239.255.hi.lo. Unicast sACN arrives on the
  same socket. `receive()` waits with `select()`, then validates: ACN id,
  root/framing/DMP vectors, start code `0x00` (skips `0xDD` priority), skips
  preview and stream-terminated packets, accepts partial universes.
- **`InputHandler`** — `unordered_map<uint16_t, Entry{buffer, refcount,
  lastSeq, seen, packets}>`. Drops late/duplicate packets by sequence number
  (E1.31 §6.7.2), zero-fills unsent slots, writes the SyncBuffer. The socket
  is opened/closed only by the input task; `reconnect()` (wifi task) just
  sets a flag.
- **`SyncBuffer`** — mutex-protected 512-byte copy in / copy out; no torn
  frames between the input and render tasks.
- **`Fixture`** — config (`Config::Fixture`), its outputs, preset JSON, and
  the SyncBuffers of every universe its `address + footprint` spans.
  `update()` copies those universes into one flat buffer and hands each
  output its slice. Highlight fills the slice with 255.
- **Outputs** — `Interface::Output`: `footprint()`, `update(span)`,
  `postupdate()` (= show), `setPreset(json)`. Only `Output::Strip1D<TDriver,
  Pin>` exists today: N LEDs, `num_groups` source colors spread evenly.
  The grid output (`Output/Base/LEDGridBase.h`) is commented out until it is
  ported to the driver model.
- **Drivers** — `Output::Driver` (`show`, `kind`, `pin`). Policies make one
  per output: `Drivers::Ws2815` (native SPI driver, runtime pin, only one
  strip because it owns `SPI2_HOST`) and `Drivers::FastLed<Chip, Order>`
  (needs the pin at compile time; shows only its own controller). A taken pin
  or failed init gives a null driver: the output renders, nothing is sent.
- **Fixture types** — `Fixtures::Strip<Type, TDriver, N, Pin>` with aliases
  `StripWS2815`, `StripWS2812`, `StripSK6812`. Presets are generated at
  compile time (`Presets::c_groupPresets<N>`: one "group by d" per divisor of
  N).

## 5. Commands

Line protocol: one JSON object per line, reply one JSON line on the same
transport (TCP 8888 or serial). See [`apiv2.md`](./apiv2.md).

- `CommandDispatcher` parses once, routes on `"request"`, and **runs one
  command at a time** (a mutex), so serial and TCP handlers never race each
  other.
- `TcpListener` serves one client at a time, buffers into lines (max 4096
  bytes), and drops a client after 2 s without data.
- Handlers: `describe`, `fixtures`/`getfixture`, `setfixture`, `inputs`,
  `wifi`, `reboot`, `factory_reset`, `serial_report_task`,
  `wireless_report_task`, `wifi_animation`.

## 6. Persistence (NVS)

Everything goes through `Services::SettingsStore`.

| Namespace / key | Content | Written |
|---|---|---|
| `engine` / `settings` | `Config::Engine` JSON: report flags, `wifi_animation`, `ssid`, `password` | at once, via `update(fn)` |
| `fixtures` / `fixtureN` | fixture config JSON: id, name, type, universe, address, presetIndex, numPresets, highlighted, footprint | queued; written 1 s after the last fixture change |

- Delayed fixture writes: a burst of changes (a UI slider) costs one flash
  write. `reboot` flushes first; `factory_reset` drops the queue and erases
  both namespaces. A power cut within 1 s of a change loses that change.
- `highlighted` is stored but has no effect on boot.
- The record is **config only**. Outputs and presets come from the compiled
  fixture type, and records bind by position (`fixture0` = first
  `addFixture`). Fixing that is [`improvement.md`](./improvement.md).

## 7. Locking

| Lock | Guards | Taken by |
|---|---|---|
| `CommandDispatcher::m_mutex` | one command at a time | `tcp_cmd`, `serial_cmd` |
| `Fixture::m_mutex` (recursive) | buffers, outputs, config | render, commands, report/heartbeat readers. `FixtureHandler` holds it across a change **and** its resubscribe, so the render task never sees a new footprint with an old buffer |
| `InputHandler::m_mutex` | universe map; socket open/close vs join/leave | input task (per packet, not while waiting), commands |
| `SyncBuffer` mutex | one universe's bytes | input (write), render (read) |
| `SettingsStore::m_mutex` | NVS, engine settings, pending fixture records | everyone; readers use `snapshot()` |

Order when nested: Fixture → InputHandler → SyncBuffer. Nothing takes them
the other way round.

## 8. Logging

`Utils::Log`, one per module with a tag (`constexpr Utils::Log s_log{"WIFI"}`).
Levels debug/info/warn/error; a call below the level costs one comparison.
`printf`-style, checked at compile time (`format(printf)` + `-Werror=format`
for `src/`); `std::format` was tried and cost ~411 KB of flash.

Each line (`I   12.345 [WIFI] message`) is formatted once, then:

- kept in `LogHistory` (Info and up, 4 KB ring, readable with the `logs`
  command), and
- written to USB serial without blocking: skipped when no host is plugged in
  or the TX buffer is full (a `-- N serial lines dropped --` note follows).

`API` logs every change with its origin (`fixture 0 changed via 192.168.0.5`).

## 9. Known gaps

- Fixtures are compiled-in types; stored records can't rebuild a fixture
  (see [`improvement.md`](./improvement.md)).
- No grid output (commented out, needs porting to `Output::Driver`).
- Only one WS2815 strip (SPI2); more strips must use FastLED.
- sACN: one source per universe, no priority merge. Source loss is detected
  and logged, but LEDs still hold the last frame.
- Serialization overlaps: `toJson` / `describe` / `configJson` /
  `outputsJson` emit overlapping fields.

## 10. Other diagrams in this folder

`architecture.puml` and `proposed_architecture.puml` are the original
pre-refactor design and proposal, kept for history. `json_fixture_architecture.puml`
belongs to [`improvement.md`](./improvement.md). `led_grid*.puml` belong to
[`led_grid.md`](./led_grid.md).
