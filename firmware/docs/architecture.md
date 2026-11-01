# Lucke Firmware — Current Architecture

> October 2026. Scope: `src/`. `unused/src_prev/` is the old tree and is not
> described here. Diagram: [`architecture.puml`](./architecture.puml).
> Wire protocol: [`api.md`](./api.md). Open work: [`improvements.md`](./improvements.md).

Target: ESP32-C3 (`lolin_c3_mini`), Arduino core on pioarduino, GCC 14,
`-std=gnu++23`, `-fno-rtti -fexceptions`.

---

## 1. Shape

```
main.cpp ── engine.addFixture<Fixtures::StripWS2815<60, 5>>() ── engine.init()

Engine (composition root: owns everything, wires it, starts the services)
 ├─ SettingsStore      the only NVS owner: engine settings + fixture records
 │                     + diagnostic records
 ├─ Tasks::Manager     spawns the FreeRTOS tasks
 ├─ Net::Wifi          station connect / reconnect / status
 ├─ InputHandler       sACN socket, one SyncBuffer per subscribed universe
 ├─ FixtureHandler     owns the Fixtures, ref-counts their universe span
 ├─ Services
 │   ├─ RenderLoop     the one task that drives the LEDs (Live / Animation / Off)
 │   ├─ CrashLog       last abnormal reset + recent Error lines, kept in NVS
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
3. `engine.init()`: load settings, `CrashLog::begin()` (logs the reset reason
   and records an abnormal one, see [Logging](#8-logging)), start wifi,
   register commands, then `startServices()` wires the render wakes, spawns
   the tasks (below) and hooks Connectivity.

## 3. Tasks

| Task | Owner | Prio | Stack | Does |
|---|---|---|---|---|
| `input_update` | InputHandler | 5 | 30000 | waits on the sACN socket, drains packets into SyncBuffers, wakes `render` |
| `render` | RenderLoop | 4 | 30000 | per mode: fixtures `update()` + `show()` when woken or every 250 ms, or the wifi animation (20 ms), or nothing |
| `tcp_cmd` | TcpListener | 1 | 8192 | accepts one client at a time, line in / line out |
| `serial_cmd` | CommandSession | 1 | 8192 | same protocol over USB serial |
| `wifi_monitor` | Connectivity | 1 | 4096 | woken by wifi events, calls listeners on link change |
| `heartbeat` | Telemetry | 1 | 4096 | UDP broadcast every 500 ms |
| `report` | Telemetry | 1 | 15000 | every 10 s: serial report and/or UDP report, per flags |
| `settings_save` | SettingsStore | 1 | 4096 | writes queued fixture records after 1 s of quiet |
| `crash_log` | CrashLog | 1 | 4096 | every 5 s: saves new Error log lines |

Priorities: input highest, because the sACN socket queues only 6 packets;
then render, so a frame never waits behind a command, a save or a report.
Everything else is 1. lwIP (18) and the wifi driver (23) stay above all of
these. A lower task holding a lock render needs (a command in a fixture's
mutex) is raised by priority inheritance meanwhile.

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
  sets a flag. After draining the queued packets, if any write changed data,
  it calls its `onFrame` listener once (`RenderLoop::wake()`), so universes
  that arrive together are rendered together.
- **`SyncBuffer`** — mutex-protected 512-byte copy in / copy out; no torn
  frames between the input and render tasks. A version counter goes up only
  when a write changes the data (`write()` returns whether it did), so a
  repeated static frame is not news and doesn't wake the render task.
- **`Fixture`** — config (`Config::Fixture`), its outputs, preset JSON, and
  the SyncBuffers of every universe its `address + footprint` spans.
  `update()` copies the universes whose version moved into one flat buffer
  and hands each output its slice. Highlight fills the slice with 255. It
  renders (and the outputs are sent) only on new data, after a setter or a
  mode switch back to Live (`invalidate()`), or once a second as a refresh;
  otherwise it returns false and nothing is sent.
- **Render wake-up** — the render task doesn't poll. It waits on a task
  notification (`RenderLoop::wake()`) with a timeout: 250 ms in Live (for
  the 1 s refresh), 20 ms in Animation and Off. Wakes come from new DMX
  (`InputHandler::onFrame`), fixture setters that change what is shown
  (`FixtureHandler::onChange`: universe, address, preset, highlight, output
  settings) and `setMode()`. A wake during a render isn't lost (the next
  wait returns at once), and several wakes give one render. The version
  check in `Fixture::update()` stays the source of truth, so a missed or
  extra wake costs at most one late or one empty pass.
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
transport (TCP 8888 or serial). See [`api.md`](./api.md).

- `CommandDispatcher` parses once, routes on `"cmd"`, and **runs one
  command at a time** (a mutex), so serial and TCP handlers never race each
  other.
- `TcpListener` serves one client at a time, buffers into lines (max 4096
  bytes), and drops a client after 2 s without data.
- Handlers: `describe`, `fixtures`, `get_fixture`, `set_fixture`, `inputs`,
  `get_wifi`/`set_wifi`, `get_settings`/`set_settings`, `reboot`,
  `factory_reset`, `logs`, `crash_log`/`clear_crash_log`, `ota`.

## 6. Persistence (NVS)

Everything goes through `Services::SettingsStore`.

| Namespace / key | Content | Written |
|---|---|---|
| `engine` / `settings` | `Config::Engine` JSON: report flags, `wifi_animation`, `ssid`, `password` | at once, via `update(fn)` |
| `fixtures` / `fixtureN` | fixture config JSON: id, name, type, universe, address, presetIndex, numPresets, highlighted, footprint | queued; written 1 s after the last fixture change |
| `diag` / `crash` | `{count, last}`: abnormal resets since cleared, and the last one (reason, boot id, version, uptime, core dump summary, log trail) | once at boot, after an abnormal reset |
| `diag` / `errors` | the last 10 Error lines (≤ 1.5 KB) with their boot ids | at most every 5 s and 12 times a boot |

- Delayed fixture writes: a burst of changes (a UI slider) costs one flash
  write. `reboot` flushes first; `factory_reset` drops the queue and erases
  `engine` and `fixtures` (not `diag`). A power cut within 1 s of a change
  loses that change.
- `diag` is written by `Services::CrashLog` through `loadDiag`/`saveDiag`/
  `clearDiag`, and cleared only by `clear_crash_log`.
- `highlighted` is stored but has no effect on boot.
- The record is **config only**. Outputs and presets come from the compiled
  fixture type, and records bind by position (`fixture0` = first
  `addFixture`). Fixing that is [`improvements.md`](./improvements.md).

## 7. Locking

| Lock | Guards | Taken by |
|---|---|---|
| `CommandDispatcher::m_mutex` | one command at a time | `tcp_cmd`, `serial_cmd` |
| `Fixture::m_mutex` (recursive) | buffers, outputs, config | render, commands, report readers. `FixtureHandler` holds it across a change **and** its resubscribe, so the render task never sees a new footprint with an old buffer |
| `InputHandler::m_mutex` | universe map; socket open/close vs join/leave | input task (per packet, not while waiting), commands |
| `SyncBuffer` mutex | one universe's bytes | input (write), render (read) |
| `SettingsStore::m_mutex` | NVS, engine settings, pending fixture records | everyone; readers use `snapshot()` |
| `CrashLog::m_mutex` | crash count, last crash, error list | `crash_log` task, commands; holds `SettingsStore::m_mutex` inside it while saving |
| `CrashTrail` spinlock | the RTC trail | any task that logs (critical section, never blocks) |

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
- appended to the crash trail (Info and up, below), and
- written to USB serial without blocking: skipped when no host is plugged in
  or the TX buffer is full (a `-- N serial lines dropped --` note follows).

`API` logs every change with its origin (`fixture 0 changed via 192.168.0.5`).

### Crash log

What went wrong survives the reset that ends it:

- **`Utils::CrashTrail`** — a 1.5 KB ring of the last log lines in
  `RTC_NOINIT` memory, plus this boot's id and version. RTC memory keeps its
  contents through a panic, a watchdog or a brownout reset, not a power cut.
  The first log line of a boot copies what the previous boot left aside and
  starts a fresh trail.
- **Core dump** — the framework's panic handler writes an ELF core dump to
  the `coredump` partition (in `min_spiffs.csv`) on a panic, an interrupt
  watchdog or a task watchdog. `esp_core_dump_get_summary()` gives the task,
  PC, RA, SP and trap registers on the next boot.
- **`Services::CrashLog::begin()`** — at boot, after settings load: logs the
  reset reason, and if it was abnormal, saves the trail and the core dump
  summary (only for resets that write one, so a stale dump isn't
  attributed to a brownout) to `diag/crash`, and counts it.
- **Error lines** — the `crash_log` task scans `LogHistory` every 5 s for new
  Error lines and saves the last 10 to `diag/errors`, so they survive a power
  cut too. Capped at 12 saves a boot.

Read with `crash_log`, cleared with `clear_crash_log` (also erases the core
dump). See [`api.md`](./api.md#crash_log--clear_crash_log).

## 9. Known gaps

- Fixtures are compiled-in types; stored records can't rebuild a fixture
  (see [`improvements.md`](./improvements.md)).
- No grid output (commented out, needs porting to `Output::Driver`).
- Only one WS2815 strip (SPI2); more strips must use FastLED.
- sACN: one source per universe, no priority merge. Source loss is detected
  and logged, but LEDs still hold the last frame.
- Serialization overlaps: `toJson` / `describe` / `configJson` /
  `outputsJson` emit overlapping fields.
