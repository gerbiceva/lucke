# C++23 modernization list

The firmware builds with `-std=gnu++23` (GCC 14.2, `riscv32-esp-elf`, pioarduino
espressif32). Toolchain flags that matter: `-fexceptions -fno-rtti`, so
`std::expected` / `std::optional` are fine, `std::any` and `dynamic_cast` are not.

Scope: everything in `src/`. Not covered: vendored `lib/sACN`, the commented-out
BLE transport.

**Legend:** ✅ done · 🟡 partially done · ⬜ not started

| # | Area | Status |
|---|------|--------|
| 1 | Hand-rolled utilities → standard library | 🟡 |
| 2 | Raw `new` → RAII ownership | 🟡 |
| 3 | Raw buffers → `std::span` / `std::array` | ✅ |
| 4 | Macros → `constexpr` | ✅ |
| 5 | `enum` → `enum class` | ⬜ |
| 6 | Command dispatch | ⬜ |
| 7 | Parameters and signatures | 🟡 |
| 8 | Concurrency | ⬜ |
| 9 | Small C++20/23 idioms | 🟡 |
| 10 | Dead code | ⬜ |
| — | Bugs found along the way | 🟡 |

---

## 1. Hand-rolled utilities → standard library 🟡

| Status | Where | Now | Replace with |
|---|---|---|---|
| ✅ | `src/Utils/Optional.h` | Custom `Optional` that throws a `const char*` | `std::optional`. File deleted; `TcpServer::accept()` returns `std::optional<TcpConnection>` / `std::nullopt`. |
| ⬜ | `src/Utils/StringUtils.h` | `std::stringstream` concat helper plus a commented-out vsnprintf version. Not used anywhere. | Delete. Use `std::format` if formatting is needed later. |
| ⬜ | `src/Utils/PresetMaker.h` — `Divisors` / `FillDivisors` | Recursive templates | A `consteval` function with a plain loop returning `std::array` |
| ⬜ | `src/Utils/PresetMaker.h` — `ConstStr` / `MakeJSON::make()` | Builds the preset JSON at runtime | Make it `constexpr`/`consteval` so the JSON is a compile-time constant in flash. `push_int` → `std::to_chars`. |
| ⬜ | `src/Utils/Logger.h` — `printf` | Forwards to `Serial.printf`, format string unchecked | `std::format_string<Args...>` + `std::format_to` into a stack buffer, then `Serial.write`. Compile-time checked. |
| ⬜ | `src/Output/LED1D.h`, `src/Output/Base/LEDGridBase.h` — `toJson` | `char type[24]; snprintf(...)` | `std::format("{}-1D", TDriver::kind)` |

## 2. Raw `new` → RAII ownership 🟡

Everything is created once in `setup()` and lives until reboot, so most of these
leaks don't lose memory in practice today. The value is a real double-free fix
(2.5), safety for future runtime add/remove, and ownership that is visible in the
types (`unique_ptr` = owns, `T*` / `T&` = borrows, `std::span` = view).

### 2.1 `Fixture` owns its outputs ✅
`src/Core/Fixture.h`
- `m_outputs` is `std::vector<std::unique_ptr<Interface::Output>>`.
- `_addOutput(Output*)` removed. `addOutput<TOutput>(args...)` builds with
  `std::make_unique`, calls `bind()`, adds the footprint, and returns `TOutput&`
  so callers can configure the output afterwards (needed for
  `FastLEDGrid::addStrip`).
- `getOutputs()` returns `std::vector<std::unique_ptr<Interface::Output>>&`;
  loops in `Fixture.cpp`, `FixtureHandler.cpp`, `StripWS2815.h` use `auto&`.
- `Interface::Output` got `virtual ~Output() = default;` — required, otherwise
  deleting through `unique_ptr<Output>` is undefined behaviour.

### 2.2 `FixtureHandler` owns the fixtures ✅
`src/Handlers/FixtureHandler.*`
- `m_fixtures` is `std::vector<std::unique_ptr<Fixture>>`, built with
  `std::make_unique<TFixture>()`; `_addFixture` takes `std::unique_ptr<Fixture>`.
- `get(id)` uses `std::ranges::find(m_fixtures, id, &Fixture::id)` and still
  returns a borrowed `Fixture*`.
- `all()` returns `const std::vector<std::unique_ptr<Fixture>>&` and is `const`.
- Callers (`Engine.cpp` heartbeat, internal loops) use `auto&` / `const auto&`.

### 2.3 `LEDGridBase::m_cells` ⬜
`src/Output/Base/LEDGridBase.h` — `GColor** m_cells = new GColor*[w * h]()`,
never freed. → `std::vector<GColor*> m_cells(w * h, nullptr)`.

### 2.4 `FastLEDGrid` strip buffers ⬜
`src/Output/FastLEDGrid.h` — `new uint8_t[numLeds * 3]()` per strip, stored as
raw pointers, never freed. → `std::vector<std::unique_ptr<uint8_t[]>>` owned by
the grid. Caveat: FastLED keeps a pointer to each buffer in its global
controller list and controllers can't be removed, so a grid must never be
destroyed while FastLED may still `show()` it. `FastLEDGrid` isn't used by any
fixture yet — low priority.

### 2.5 `Core::WS2815` ✅
`src/Hardware/WS2815.*`
- `pixels` is a non-owning `std::span<RGB>`; the buffer is owned by `LEDBase`.
  The owning constructor / `init(n, pin)` and all `delete[] pixels` calls are
  removed (this was the double free — see Bugs).
- New signature: `init(int pin, std::span<uint8_t> buffer)`. `n_leds` replaced
  by `pixels.size()`.
- DMA buffer: `std::unique_ptr<uint16_t[], HeapCapsFree>` (`heap_caps_free`).
- Mutex: `std::unique_ptr<..., SemaphoreDelete>` (`vSemaphoreDelete`).
- SPI device / bus released by `release()` (`spi_bus_remove_device`,
  `spi_bus_free`), called from the destructor and when `init()` fails.
- Copying deleted. `show()` is a no-op if init failed.
- `spi_settings_t` member and unused `myMux` removed.
- Open design point: the SPI bus (and its MOSI pin) belongs to `SPI2_HOST`, so
  only one WS2815 strip can exist. A second one now fails cleanly with
  `ESP_ERR_INVALID_STATE` instead of corrupting the heap. Multiple strips would
  need another backend (e.g. RMT).

### 2.6 `LEDBase::m_dstBuffer` ✅
`src/Output/Base/LEDBase.h`
- `std::make_unique<uint8_t[]>(n)` instead of `reset(new uint8_t[n]())`.
- Allocated in the initializer list, before the pin-in-use check, so `update()`
  never writes through a null buffer (see Bugs).

## 3. Raw buffers → `std::span` / `std::array` ✅

Raw `uint8_t*` was passed around with an implied size of 512 or `footprint()`.
- ✅ New `src/Utils/Dmx.h`: `Dmx::c_universeSize`, `Dmx::Universe`
  (`std::array<uint8_t, 512>`), `Dmx::UniverseView` / `UniverseCView`
  (fixed-extent `std::span`). A wrong-sized buffer doesn't compile.
- ✅ `SyncBuffer::read/write`, `Interface::Input::poll`, `SacnSource::poll`,
  `SacnParser::dmx` take `Dmx::UniverseView` / `UniverseCView`; `memcpy(..., 512)`
  → `std::ranges::copy`.
- ✅ `Interface::Output::update` takes `std::span<const uint8_t>` (exactly the
  output's footprint). `LED1D`, `LEDGridBase` updated.
- ✅ `Fixture::update` and `StripWS2815::wifiAnimation` slice with
  `subspan` / `first` instead of `ptr += footprint()`. `Fixture::update` skips the
  frame if `address + footprint` doesn't fit `m_buffer` (see bug 8).
- ✅ C arrays → `std::array`: `SacnParser` `m_packet` / `m_buffer`,
  `StripWS2815::m_animBuf`, `InputHandler::update` `tmp`.
- ✅ The `std::queue<uint8_t>` of sequence diffs and `getSeqDiffs()` were
  deleted (it had no readers and allocated per packet).
- ⬜ Optional: a debug env with `-D_GLIBCXX_ASSERTIONS` so `span` / `array` /
  `vector` indexing is bounds-checked at runtime.

## 4. Macros → `constexpr` ✅
- ✅ `src/Core/Config/EngineConfig.h` — `ENGINE_VERSION`, `DEFAULT_WIFI_SSID`,
  `DEFAULT_WIFI_PASSWORD` → `inline constexpr std::string_view`; ports →
  `inline constexpr uint16_t`. Names kept, so call sites are unchanged.
  `std::string` members are brace-initialized (the `string_view` constructor is
  `explicit`).
- ✅ `src/Utils/SacnParser.cpp` — `ADDR_*` offsets → `constexpr size_t` in an
  anonymous namespace.
- ✅ `src/Hardware/WS2815.*` — `timing_bits` → `static constexpr std::array`,
  `reset_delay` → `static constexpr uint8_t`, both in the header; out-of-line
  definitions removed.

## 5. `enum` → `enum class` ⬜
- ⬜ `Logger::Level` (`src/Utils/Logger.h`), `Manager::TaskStatus`
  (`src/Tasks/TaskExecutor.h`) → `enum class`.
- ⬜ Task IDs are bare `uint32_t` and `0` means both "unset" and the first real
  ID → strong `enum class TaskId : uint32_t {}`.
- ⬜ `std::to_underlying` where the raw value is needed.

## 6. Command dispatch ⬜
- ⬜ `src/Commands/EngineConfigHandler.cpp` — `strcmp` chain and three
  near-identical bool branches → `constexpr` table of
  `{std::string_view name, bool Config::Engine::*member, toggle fn}`.
- ⬜ `src/Commands/GetFixtureHandler.cpp` — `if (field == "...")` chain → table
  of `{name, lambda}`.
- ⬜ `src/Commands/CommandDispatcher.h` — `unordered_map<std::string, ...>`
  builds a `std::string` per lookup → transparent hash + `std::equal_to<>` for
  `find(std::string_view)`, or a small sorted `std::array` (11 commands).
- ⬜ "Serialize doc → send" repeated 8× (`DescribeHandler`, `GetFixtureHandler`
  ×2, `InputsHandler`, `SetFixtureHandler`, `WifiConfigHandler` ×2,
  `EngineConfigHandler`) → `transport.send(const JsonDocument&)` helper.
- ⬜ Handlers return `std::expected<JsonDocument, std::string_view>`; the
  dispatcher sends and formats errors, replacing hand-written
  `"{\"err\":...}"` strings.

## 7. Parameters and signatures 🟡
- 🟡 Virtual destructors on interfaces: ✅ `Interface::Output`; ⬜
  `Serializable`, `Transport`, `CommandHandler`, `Input`.
- ⬜ Read-only `std::string` by value → `std::string_view` / `const&`:
  `JsonUtils.h` `name` (×4), `fromJson(std::string)` (`Deserializable.h`,
  `Fixture.h`, `FixtureHandler.h`), `Manager::spawnTask(std::string)` /
  `find(std::string)`.
- ⬜ `src/Utils/JsonUtils.h` — four near-identical overloads → two templates
  over `JsonVariantConst`.
- ⬜ `src/Core/Fixture.cpp` constructor copies three strings → take by value and
  `std::move`.
- ⬜ `[[nodiscard]]` on getters and on the `bool` returns of
  `FixtureHandler::set*`.
- ⬜ `src/Core/Config/EngineConfig.h` — `const std::string version` member makes
  `Config::Engine` non-assignable → `static constexpr std::string_view`.

## 8. Concurrency ⬜
- ⬜ 13× `std::lock_guard<std::mutex> lock(m_mutex)` → `std::scoped_lock`.
- ⬜ `src/Handlers/InputHandler.h` — three parallel maps keyed by universe →
  one `unordered_map<uint8_t, Entry{source, buffer, refcount}>`;
  `match()` → `contains()`.
- ⬜ `src/Handlers/PinHandler.cpp` — `find == end` + `emplace` →
  `return m_usedPins.insert(pin).second;`.
- ⬜ `src/Tasks/TaskExecutor.h` — uses `std::function` without
  `#include <functional>`; `std::move_only_function<void()>` would allow
  move-only captures.

## 9. Small C++20/23 idioms 🟡
- ✅ `FixtureHandler::get` loop → `std::ranges::find` with projection.
- ✅ `WS2815` loops → range-for over `std::span` / `std::ranges::fill`.
- ⬜ `src/Tasks/TaskExecutor.cpp` `find_if` + lambda →
  `std::ranges::find(m_tasks, id, &TaskRecord::id)`.
- ⬜ `src/Utils/SacnParser.cpp` `getSeqDiffs` drain loop →
  `std::ranges::to<std::vector>()` (once it's a ring buffer).
- ⬜ `src/Hardware/WS2815.h` — anonymous struct in a union is a GNU extension and
  relies on type punning → plain `struct RGB { uint8_t r, g, b; }` +
  `static_assert(sizeof(RGB) == 3)`.
- 🟡 C-style casts → `static_cast` / `reinterpret_cast`: ✅ `WS2815.cpp`; ⬜
  `(CRGB*)` in `Drivers.h`, `FastLEDGrid.h`.
- 🟡 `NULL` → `nullptr`: ✅ `WS2815`; ⬜ `main.cpp`, `TaskExecutor.h`.

## 10. Dead code ⬜
- ⬜ `src/main.cpp` — unused `std::variant<int, float> v` and `<variant>`.
- ⬜ `Net::Wifi::m_connection_status_callback` and its commented-out API.
- ⬜ `TcpServer::m_port` / `restart()`, `Manager::find(std::string)`,
  `isTaskRunning`, `SacnParser::getSeqDiffs`, `Output::appendBasicInfo` (never
  called), `Output::bind()` (every override is empty).
- ⬜ Commented-out BLE transport, old Wi-Fi credentials in `EngineConfig.h`
  (both in git history).
- ✅ `WS2815::myMux`, `WS2815::spi_settings_t` (removed with 2.5).

---

## Bugs found along the way 🟡

Not modernization, but found while reading.

1. ✅ **Double free in `WS2815`.** `initSPI` called `delete[] pixels` on every
   error path, but `pixels` pointed into `LEDBase::m_dstBuffer` (a
   `unique_ptr`). Any SPI init failure (e.g. a second strip on `SPI2_HOST`)
   freed borrowed memory, kept rendering into it, and freed it again on
   destruction. Fixed by 2.5.
2. ✅ **Null buffer in `LEDBase`.** If the pin was already in use the
   constructor returned before allocating `m_dstBuffer`, and `update()` wrote
   through null. Fixed by 2.6 (plus the `show()` guard in 2.5).
3. ✅ **Buffer overflow in `SacnParser::update`.** `m_udp.read(packet, size)`
   uses the packet size unchecked against the 1500-byte buffer, and packets
   shorter than 638 bytes aren't rejected (stale data copied as DMX).
   Fixed with section 3: packets larger than the buffer or shorter than
   `ADDR_DMX + 512` are dropped.
4. ⬜ **Division by zero in `LED1D::update`.** `m_numLeds / m_numGroups` with a
   preset of `num_groups: 0`.
5. ⬜ **`Pin::toJson` adds nothing.** `doc["used_pins"].as<JsonArray>()` on a
   missing key yields a null array → should be `.to<JsonArray>()`. Currently
   unused.
6. ⬜ **`Fixture::setPreset` resets the footprint before validating.**
   `m_lastOffset = 0` runs before the preset index / settings-count checks, so an
   invalid preset leaves the fixture with footprint 0 (wrong universe span).
   Moving the reset below validation makes the footprint from `addOutput` the
   fallback.
7. ⬜ **Member initializer order in `Engine`.** Command handlers get references to
   `m_fixtures` / `m_inputs`, which are declared later in `Engine.h`. Harmless
   while no constructor uses them, but the initializer list doesn't match
   declaration order.

8. 🟡 **Preset/address change races `Fixture::update`.** `FixtureHandler::setPreset`
   / `setAddress` change the footprint under the fixture mutex, release it, then
   `resubscribe()` resizes `m_buffer` under a second lock. In between, the fixture
   task could read past `m_buffer`. Mitigated in section 3: `update()` skips the
   frame when the outputs don't fit. Real fix: change and resize under one lock.

## Suggested next steps
1. Bugs 4, 6, 8 — small, real defects.
2. Remaining virtual destructors (7) — cheap, prevents UB.
3. 1 and 6 — delete the most code.
4. 3 and 8 — mechanical, low-risk.
