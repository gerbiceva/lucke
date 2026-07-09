# Lucke Firmware — Current Architecture & Analysis

> Branch `refactor`. Scope: `src/` (the live tree). `src_prev/` and `tmp/` are
> legacy/scratch and are ignored here.

Companion diagram: [`current_architecture.puml`](./current_architecture.puml).

---

## 1. Persistent storage — where do we read/write?

All persistence goes through **`Utils::Storage`**, a thin wrapper over the ESP32
`Preferences` library (NVS — non-volatile key/value flash). There are **two
independent NVS namespaces**, each owned by a different object.

### Namespace `"engine"` — owned by `Engine::m_storage`

| Key | Value | Written by | Read by |
|-----|-------|-----------|---------|
| `"settings"` | JSON of `Config::Engine` (version, `serial_report_task`, `wireless_report_task`, `wifi_animation`, `ssid`, `password`) | `Engine::saveConfig()` (`Engine.cpp:97`), `EngineConfigHandler::save()` (`EngineConfigHandler.cpp:66`), `WifiConfigHandler` on ssid/pw change (`WifiConfigHandler.cpp:27`) | `Engine::loadConfig()` on boot (`Engine.cpp:62-64`) |

Partition opened in `Engine::init()` → `m_storage.setPartition("engine")`
(`Engine.cpp:22`).

### Namespace `"fixtures"` — owned by `FixtureHandler::m_storage`

| Key | Value | Written by | Read by |
|-----|-------|-----------|---------|
| `"fixture0".."fixtureN"` | JSON of `Fixture::toJsonDoc()` — **config only** (id, name, type, universe, address, presetIndex, numPresets, footprint) | `FixtureHandler::persistFixture()` (`FixtureHandler.cpp:66`), called after `addFixture` and after every `setUniverse/setAddress/setPreset/setName` | `FixtureHandler::_addFixture()` on boot (`FixtureHandler.cpp:23-25`) |

Partition is opened **lazily** the first time a fixture is added
(`FixtureHandler.cpp:14-18`) — deliberately, because NVS flash isn't initialized
at global-constructor time.

### Read/write flow summary

- **Boot read (engine):** `loadConfig()` → if `"settings"` exists, parse and
  overlay selected fields onto `m_config`; honour `to_factory_settings` (clears
  the flag, rewrites). Otherwise `saveConfig()` writes defaults.
- **Boot read (fixtures):** for each compiled-in fixture, if `"fixtureN"` exists
  **and not** `factoryReset` → `fix->fromJson(stored)`; else `fix->fromConfig()`
  (defaults). Then immediately `persistFixture()` to write it back.
- **Runtime writes:** every fixture mutation and every engine-config/wifi change
  re-serializes the whole object and does one `putString`.

### Notes / risks in the storage layer

1. **`highlight` is intentionally not persisted** (transient) — correct, but
   undocumented.
2. **Write amplification / flash wear.** Each setter writes the *entire* JSON
   blob to NVS synchronously. Dragging an address slider = one flash write per
   step. Consider debouncing or a dirty-flag flush.
3. **Dead API.** `Storage` exposes `putChar/UChar/UShort`, the matching getters,
   and the whole `putIfNExist*` family. None are used anywhere in `src/` — only
   `isKey/putString/getString` are. Dead surface to delete.
4. **`setPartition` never `end()`s a previously-open namespace** (the in-code
   `// TODO: check if open` at `Storage.cpp:16`). Not hit today (each Storage
   opens exactly one namespace), but a latent footgun.
5. **Factory reset doesn't erase NVS** — it overwrites with defaults. Orphaned
   keys (e.g. a fixture that no longer exists in firmware) are never cleared.
6. **The persisted fixture JSON is not self-sufficient** — see §3.

---

## 2. The class model (how it fits together)

```
main.cpp ─ engine.addFixture<StripWS2815<24,5>>() ─ engine.init()
                                                         │
                                       ┌─────────────────┴─────────────────┐
                                    Engine (composition root, owns everything)
       ┌──────────────┬───────────────┬──────────────┬─────────────┬───────────┐
   Config::Engine  FixtureHandler  InputHandler   CommandDispatcher  Tasks::Mgr  Network
                       │   │            │              │  (+handlers)
                  Fixture* │       SacnSource      handlers ──> FixtureHandler / Config
                  (owns Output*)   (+SyncBuffer)
```

- **`Engine`** is the composition root: it owns the config, both handlers, the
  dispatcher + all command handlers, the task manager, and all networking. It
  wires everything in its constructor initializer list and `init()`.
- **`Fixture`** owns a `Config::Fixture` (data), a vector of `Output*`, a parsed
  preset document (`m_jsonPresets`), and the per-universe `SyncBuffer`s. Its
  `update()` stitches the subscribed universes into one flat buffer and walks the
  outputs. A `std::mutex` guards `update()` against the command-task mutators.
- **`Output`** is the hardware-mapping interface (`LED1D`, `LEDGridBase`/
  `WS2815Grid`, FastLED variants), templated on a backend `TDriver`.
- **`FixtureHandler`** owns the fixtures, persists them, and ref-counts universe
  subscriptions through **`InputHandler`** (which today hardcodes `SacnSource`).
- **`SyncBuffer`** is the lock-protected 512-byte hand-off between the input task
  (writer) and the fixture task (reader).
- **Tasks** (`Tasks::Manager`) spawns the FreeRTOS tasks: input poll, fixture
  update, wifi animation, wifi monitor, heartbeat, serial cmd, tcp cmd, and the
  two report tasks.
- **Commands** parse a JSON line and dispatch by the `"request"` field over a
  `Transport` (TCP or Serial).

### Threading model (sound, worth keeping)

- `SyncBuffer` lock-copies on both `read` and `write` → no torn DMX frames.
- `Fixture::m_mutex` serializes `update()` against resize/swap mutators.
- `InputHandler::m_mutex` guards the maps against concurrent subscribe/iterate.
- These three are correct and the comments explaining *why* are good.

---

## 3. The JSON / "describe" story — you're right, it's the weak spot

You said *"I think the JSON describe is not good"* — agreed, and here is the
concrete diagnosis. The **idea** (JSON as the lingua franca for transport,
presets, and persistence) is good. The **current implementation** has three
problems.

### 3a. Too many overlapping serialization methods, fuzzy contracts

A single fixture can be turned into JSON through **five** different methods, and
an output through **four**:

| Level | Methods | Overlap |
|-------|---------|---------|
| `Output` | `toJson()`, `describe()`, `appendBasicInfo()`, `toJsonDoc()` | `num_leds`, `footprint`, `hardware_pin` appear in *both* `toJson` and `describe` (sometimes nested under `"description"`, sometimes flat) |
| `Fixture` | `toJson()`, `toJsonDoc()`, `configJson()`, `presetsJson()`, `outputsJson()` | `configJson` embeds `toJson` + outputs' `toJson`; `outputsJson` re-emits outputs via `describe` + per-preset footprints |

Problems this causes:

- **No single contract.** It's unclear which method is the "truth", which is the
  wire format, and which is the persistence format. They've diverged.
- **`Output::appendBasicInfo()` is dead code** — declared on the interface,
  overridden in `LEDBase`, **never called**. Delete it.
- `describe` vs `toJson` is a naming coin-flip; a reader can't predict which
  fields land where.

### 3b. Persistence and "describe" are conflated, and the round-trip is open

This is the real architectural issue. **What is persisted is not enough to
rebuild a fixture.**

- `persistFixture()` stores `Fixture::toJsonDoc()` = **config only** (name,
  universe, address, presetIndex…). It contains **no outputs and no preset
  language**.
- On boot the outputs and presets come from the **compiled C++ type**
  (`addFixture<StripWS2815<24,5>>`), and `fromJson()` only *overlays* the stored
  config on top.

Consequences:

- The stored `"fixtureN"` is meaningless without the matching firmware build.
- Persistence is **positional/index-based** (`"fixtureN"`, presets are
  `settings[k] -> outputs[k]`). Reorder the fixtures in `main.cpp`, or change a
  compiled fixture's outputs, and stored records silently bind to the wrong
  fixture / wrong outputs.
- `Deserializable::fromJson` is declared on `FixtureHandler` but its body is
  empty (`FixtureHandler.cpp:174` — "Handled per-fixture") — an interface method
  that does nothing is a smell.

### 3c. Mechanical JSON inefficiencies (matter on an ESP32)

- **`fromJson(std::string)` re-parses at every layer.** A command line is parsed
  in the dispatcher, then the string is handed down and `deserializeJson`'d again
  inside `Fixture::fromJson`. Pass the already-parsed `JsonObjectConst` instead.
- **`JsonDocument` returned by value everywhere** (`toJsonDoc`, `configJson`,
  `presetsJson`, `outputsJson`). ArduinoJson v7 docs are heap-backed; returning
  them by value churns the heap. Prefer filling a caller-provided `JsonObject&`
  (which `toJson` already does — so the `*Doc()` wrappers are mostly redundant).
- `getElement<T>` / `updateElement<T>` are duplicated for `const` and non-`const`
  `JsonDocument&` — collapse with a template.

### Verdict on the JSON approach

**Keep JSON as the schema — drop the multiplicity.** The fix is not "less JSON",
it's *one* authoritative JSON document per fixture that is simultaneously the
wire payload, the NVS record, and the preset source — and from which a fixture
can be **fully rebuilt** (outputs included). That is exactly what the proposed
architecture in [`json_fixture_architecture.puml`](./json_fixture_architecture.puml)
does. See [`improvement.md`](./improvement.md) for the plan and trade-offs.

---

## 4. Other findings (smaller)

- **Raw `new` for outputs and fixtures, never `delete`d.** `Fixture` holds
  `vector<Output*>` from `new TOutput(...)`; `FixtureHandler` holds
  `vector<Fixture*>` from `new TFixture()`. Acceptable for objects that live
  forever, but `Fixture`'s destructor leaks its outputs. Prefer
  `vector<unique_ptr<...>>`.
- **`InputHandler` hardcodes `SacnSource`** (`InputHandler.cpp:21`) — fine for
  now, but the only input protocol is wired in. An input factory is the natural
  next step (the proposed diagram notes this as a *later* phase).
- **`CommandDispatcher` reads `doc["request"]`** but logs `"Missing 'cmd'"` — the
  message is stale vs the field name.
- **`Serializable::toJsonDoc()` has a default returning an empty doc**, so
  forgetting to override it fails silently (empty JSON) instead of at compile
  time.
- **`Fixture::update()` highlight branch** writes `m_lastOffset` bytes at
  `&m_buffer[m_config.address]` — if `address + lastOffset` exceeds the buffer
  (mis-set address), that's an OOB write. Worth a bounds check.

---

## 5. What's already good (don't lose it in the refactor)

- Clean separation of **Interfaces** (Output / Input / Transport / Command).
- The **driver-templated Output** hierarchy (`LEDBase<TDriver>`) cleanly isolates
  FastLED vs native WS2815.
- The **threading/locking** model is carefully thought through and commented.
- **Ref-counted universe subscriptions** in `InputHandler` are elegant.
- The **Engine composition root** keeps wiring in one place.
