# C++23 modernization list

The firmware builds with `-std=gnu++23` (GCC 14.2, `riscv32-esp-elf`, pioarduino
espressif32). Toolchain flags that matter: `-fexceptions -fno-rtti`, so
`std::expected` / `std::optional` are fine, `std::any` and `dynamic_cast` are not.
The framework doesn't enable `-Wall`; `src/` gets `-Werror=format` from
`platformio.ini`.

Scope: everything in `src/`. Not covered: vendored `lib/sACN` (unused), the
commented-out BLE transport, the commented-out grid output.

**Legend:** ✅ done · 🟡 partially done · ⬜ not started · — no longer applies

| # | Area | Status |
|---|------|--------|
| 1 | Hand-rolled utilities → standard library | 🟡 |
| 2 | Raw `new` → RAII ownership | ✅ |
| 3 | Raw buffers → `std::span` / `std::array` | ✅ |
| 4 | Macros → `constexpr` | ✅ |
| 5 | `enum` → `enum class` | ✅ |
| 6 | Command dispatch | ✅ |
| 7 | Parameters and signatures | ✅ |
| 8 | Concurrency | 🟡 |
| 9 | Small C++20/23 idioms | 🟡 |
| 10 | Dead code | 🟡 |
| — | Bugs found along the way | 🟡 |

---

## 1. Hand-rolled utilities → standard library 🟡

| Status | Where | What |
|---|---|---|
| ✅ | `Utils/Optional.h` | Deleted; `std::optional`. |
| ✅ | `Utils/StringUtils.h` | Deleted (was unused). |
| ✅ | `Utils/PresetMaker.h` | `Presets::c_groupPresets<N>`: one `constexpr` writer run twice (measure, then fill an exactly-sized `std::array`), stored in flash. The old runtime builder overflowed its fixed 1024-byte buffer from N = 120 (e.g. 300 LEDs); it's kept commented out below the new code. Output is byte-identical for every N it could handle. |
| ✅ | `Utils/Logger.h` — `printf` | C-variadic `Log::info/warn/...` with `[[gnu::format(printf, 2, 3)]]` + `-Werror=format`: a format/argument mismatch is a compile error. `std::format` was tried and measured at +411 KB flash (float/locale/unicode tables), so printf stays. |
| ⬜ | `Output/Strip1D.h` — `toJson` | `char type[24]; snprintf(...)` → `std::format("{}-1D", TDriver::kind)` (or build the string at compile time). |

## 2. Raw `new` → RAII ownership ✅

- ✅ `Fixture` owns its outputs (`vector<unique_ptr<Output>>`, `addOutput<T>()`
  returns `T&`).
- ✅ `FixtureHandler` owns the fixtures (`vector<unique_ptr<Fixture>>`).
- ✅ `Core::WS2815`: borrowed `std::span<RGB>` pixels, DMA buffer and mutex in
  `unique_ptr` with custom deleters, SPI released in `release()`. Fixed a
  double free.
- ✅ `Output::Strip1D` owns its pixels (`unique_ptr<RGB[]>`) and its driver
  (`unique_ptr<Driver>`), declared so the pixels outlive the driver.
- — `LEDGridBase::m_cells`, `FastLEDGrid` strip buffers: the grid is commented
  out and `FastLEDGrid` was deleted in the drivers rework. Handle ownership
  when the grid is ported (see `led_grid.md`).

## 3. Raw buffers → `std::span` / `std::array` ✅

- ✅ `Utils/Dmx.h`: `Dmx::Universe`, `UniverseView`, `UniverseCView`
  (fixed-extent spans).
- ✅ `SyncBuffer`, `Output::update(span)`, `Fixture::update` slice with
  `subspan` / `first`.
- ✅ C arrays → `std::array` (receive buffer, animation buffer).
- ✅ `lolin_c3_mini_debug` env with `-D_GLIBCXX_ASSERTIONS`.

## 4. Macros → `constexpr` ✅

`EngineConfig.h` constants, E1.31 offsets (`Input/SacnReceiver.cpp`), WS2815
timing table.

## 5. `enum` → `enum class` ✅

`Utils::Level`, `Tasks::TaskId` (strong id, `Invalid = 0` never handed out),
`RenderLoop::Mode`.

## 6. Command dispatch ✅

- ✅ `EngineConfigHandler` (re-dispatched on `"request"` with a `strcmp`
  chain) split into `FactoryResetHandler` and one `BoolSettingHandler` per
  setting, built with `{name, bool Config::Engine::*member, API setter}`.
- ✅ `Commands/Handlers/GetFixtureHandler.cpp` — `if (field == "...")` chain →
  `constexpr` table of `{name, getter}` searched with `ranges::find`.
- ✅ `Commands/CommandDispatcher.h` — handler map uses a transparent hash +
  `std::equal_to<>`, so `find(string_view)` no longer builds a `std::string`.
- ✅ "Serialize doc → send" repeated in most handlers →
  `Interface::Transport::send(const JsonDocument&)`.
- ✅ Handlers return `Interface::Reply` (`std::expected<JsonDocument,
  std::string_view>`) and get an `Interface::Context` (the origin) instead of
  the transport; the dispatcher sends the reply and formats every error,
  including its own, as `{"err": ...}`.

## 7. Parameters and signatures ✅

Virtual destructors on all interfaces, `JsonUtils` templates over
`JsonVariantConst`, `const&` / moved strings, `[[nodiscard]]` on getters and
fallible setters, `Config::Engine::version` as `static constexpr`.

## 8. Concurrency 🟡

- ✅ `std::lock_guard<std::mutex>` → `std::scoped_lock` (CTAD) everywhere.
- ✅ `InputHandler`: three parallel maps → one
  `unordered_map<uint16_t, Entry>`.
- ✅ Locking model documented in `current_architecture.md` §7 (dispatcher
  mutex, recursive fixture lock held across change + resubscribe,
  `SettingsStore::snapshot()` / `update()`).
- ⬜ `Handlers/PinHandler.cpp` — `find == end` + `emplace` →
  `return m_usedPins.insert(pin).second;`.
- ⬜ `Tasks/TaskExecutor.h` — `std::function` → `std::move_only_function`
  would allow move-only captures. Not needed by any task today.

## 9. Small C++20/23 idioms 🟡

- ✅ `ranges::find` with projection in `FixtureHandler::get`.
- ✅ `Tasks::Manager` reduced to `spawnTask` (checks `xTaskCreate`).
- ✅ `NULL` → `nullptr` (outside the commented BLE code).
- ✅ C-style casts → `reinterpret_cast` (`WS2815`, FastLED driver).
- ✅ C++20 class-type template parameter (`Fixtures::FixedString`) for fixture
  type names.
- ⬜ `Hardware/WS2815.h` — anonymous struct in a union is a GNU extension
  relying on type punning → `struct RGB { uint8_t r, g, b; }` +
  `static_assert(sizeof(RGB) == 3)` (check `raw` users first).

## 10. Dead code 🟡

- ✅ Removed: `main.cpp` `std::variant`; `TcpServer::m_port` / `restart()`;
  `Manager::find`, `suspendTask`, `resumeTask`, `stopTask`, `isTaskRunning`,
  `TaskStatus`; `SacnParser::getSeqDiffs` (whole class replaced);
  `Output::appendBasicInfo`, `Output::bind`; old Wi-Fi credentials comment;
  `RESPONSE_PORT`; `helper/sender.py`.
- ⬜ `Net::Wifi::m_connection_status_callback` and its commented-out API.
- ⬜ `Utils::Storage` — `putChar/UChar/UShort`, the getters and the whole
  `putIfNExist*` family are unused (only `isKey/putString/getString/clear`).
- ⬜ Commented-out BLE transport (`Network/BleTransport.*`), in git history.

---

## Bugs found along the way 🟡

Not modernization, but found while reading. Fixed ones are kept for the
record.

1. ✅ **Double free in `WS2815`** on SPI init failure (borrowed buffer freed).
2. ✅ **Null buffer in `LEDBase`** when the pin was taken (class since
   replaced by `Strip1D`).
3. ✅ **Buffer overflow in the sACN parser**: unchecked packet size.
4. ✅ **Division by zero** with `num_groups: 0` — `Strip1D` rejects 0 and
   non-divisors.
5. ✅ **`Pin::toJson` adds nothing** (removed): `.as<JsonArray>()` on a missing key →
   should be `.to<JsonArray>()`. Unused.
6. ✅ **`setPreset` zeroed the footprint before validating** — an invalid
   preset now leaves preset and footprint unchanged.
7. ✅ **Member initializer order in `Engine`** — members are declared in
   dependency order.
8. ✅ **Preset/address change raced `Fixture::update`** — change and
   resubscribe now happen under one fixture lock.
9. ✅ **`describe` sent the wifi password.**
10. ✅ **Turning a report off could freeze DMX input** — `vTaskDelete` on a
    report task holding `InputHandler`'s mutex. Reports are now flags on one
    long-lived task.
11. ✅ **Serial and TCP commands ran concurrently** (fixture/settings races,
    heartbeat reading a name mid-write). Dispatcher runs one command at a
    time; cross-task reads are locked.
12. ✅ **Wifi animation wrote outputs unlocked** — a preset change mid-frame
    could write past a strip's pixel buffer.
13. ✅ **`MakeJSON<N>` overflow** from N = 120 (see §1).
14. ✅ **sACN: partial universes dropped, `0xDD` priority packets shown as
    levels** — replaced by `SacnReceiver`.
15. ✅ **Universes limited to 0–255**, all fixtures defaulted to universe 8.
16. ✅ **Logger stalled its task ~2 s per write** when USB was plugged in but
    nothing read it (HWCDC retries 20 × 100 ms).
17. ✅ **Every `setfixture` wrote flash** — fixture records are now written
    1 s after the last change.
18. ✅ **`highlight` toggles whatever value is sent** (`"highlight": false`
    turns it on).
19. ✅ **`describe()`'s `num_leds` is the group count**, not the LED count
    (`Strip1D::describe` uses `footprint() / 3`).

## Suggested next steps

1. Bugs 18 and 19 — small, client-visible.
2. Section 6 — removes the most repetitive code in the command handlers.
3. Section 10 leftovers — pure deletion.
