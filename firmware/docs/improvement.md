# Lucke Firmware — Proposed Improvement: JSON-authoritative fixtures

Companion diagram: [`json_fixture_architecture.puml`](./json_fixture_architecture.puml)
(class view + boot/persistence sequence). Baseline: [`current_architecture.md`](./current_architecture.md).

> **Status (October 2026): not started — the next big feature.** Since this
> was written the baseline moved; read the names below with that in mind:
>
> - Fixture types are now `Fixtures::Strip<Type, TDriver, N, Pin>` (aliases
>   `StripWS2815`, `StripWS2812`, `StripSK6812`); `Strip2815`, `Grid2815`,
>   `BlinderTest` and `LEDBase` no longer exist.
> - Outputs drive hardware through `Output::Driver`, made by a driver policy
>   (`Drivers::Ws2815`, `Drivers::FastLed<Chip, Order>`).
> - Presets are generated at compile time by `Presets::c_groupPresets<N>`
>   (`MakeJSON` is gone).
> - `appendBasicInfo` is deleted. Persistence goes through
>   `Services::SettingsStore` (fixture writes are delayed 1 s).
>
> **Constraint the factory must solve:** `Drivers::FastLed::make<Pin>` needs
> the pin at compile time (FastLED's `addLeds<Chip, Pin>`), so an
> `Output::Factory` reading `"pin"` from JSON can't create FastLED outputs
> directly. Options: a `constexpr` table of supported pins that dispatches to
> the right template instance, or a runtime-pin backend (e.g. ESP-IDF RMT
> directly). `Drivers::Ws2815` already takes a runtime pin, but supports one
> strip only (SPI2).

## The one idea

Make **one JSON document the single source of truth** for a fixture. The *same*
document is:

1. the **transport payload** (describe / get / set responses),
2. the **NVS record** (what `persistFixture` writes), and
3. the **preset + output language** a fixture is **built from**.

Today a fixture is built from a compiled C++ type and NVS only stores a config
overlay (see current analysis §3b). The round-trip is open: stored JSON can't
rebuild a fixture. Closing that loop removes per-fixture C++ classes entirely.

## Target shape of the document

```jsonc
{
  "id": 0,
  "name": "stage-left",
  "type": "StripWS2815",
  "universe": 8,            // BASE universe; the span is DERIVED, never stored
  "address": 0,
  "presetIndex": 0,
  "outputs": [
    { "shape": "1d", "driver": "ws2815", "pin": 5, "num_leds": 24 },
    // grid example:
    // { "shape": "grid", "driver": "ws2815", "width": 8, "height": 24,
    //   "strips": [ { "pin": 5, "num_leds": 192, "x": [0,8], "y": [0,24],
    //                 "cellW": 1, "cellH": 1, "horizontal": true } ] }
  ],
  "presets": {
    "groups": [
      { "name": "group by 1",  "settings": [ { "num_groups": 24 } ] },
      { "name": "group by 24", "settings": [ { "num_groups": 1  } ] }
    ]
  }
}
```

`settings[k]` maps positionally to `outputs[k]` (the rule the code already uses).

## What changes (mirrors the `<<new>>`/`<<changed>>`/`<<retired>>` tags in the puml)

| Component | Change |
|-----------|--------|
| **`Output::Factory`** *(new)* | `create(JsonObjectConst) -> Output*`. Switches on `("shape","driver")`. Unknown → log + `nullptr` (caller skips). The only place that knows concrete output types. |
| **`Fixture::fromJson`** *(changed)* | Now **builds** the fixture: parse identity/config → load `presets` → for each `outputs[]` call the factory → `setPreset(selectedPreset)`. No subclass needed. |
| **`Fixture::toJson`** *(changed)* | Emits the **full** definition (config + `outputs[]` + `presets`), i.e. exactly what `fromJson` consumes. Round-trip closed. |
| **`FixtureHandler::addFixture`** *(changed)* | `addFixture(defJson : string, factoryReset)` instead of `addFixture<TFixture>()`. The boot-time default definition is a JSON string (or a `.json` asset), not a C++ type. |
| **`LEDGridBase::toJson`** *(changed)* | Must also emit `"strips"` so a grid round-trips. |
| **Retired** | `Strip2815`/`StripWS2815`/`Grid2815`/`BlinderTest`, FastLED variants on the JSON path, and `MakeJSON<N>` (presets become data, not a compile-time template). |

## Persistence after the change

- `persistFixture()` writes `fix->toJson()` — now a **complete, rebuildable**
  definition. NVS `"fixtureN"` alone reconstructs the fixture; no firmware
  coupling.
- Boot: `isKey("fixtureN") && !factoryReset` → build from stored JSON; else build
  from the shipped default definition, then persist.
- **Storage encoding is orthogonal to the schema** (note in the puml): keep JSON
  text by default; if NVS size bites, switch the *encoding* to MessagePack
  (`serializeMsgPack`) without touching the schema. Don't hand-roll binary.

## Collapsing the "describe" mess (the part you flagged)

Alongside the build refactor, **consolidate the serialization surface**:

- **One** `toJson(JsonObject&)` per object = the authoritative full form.
- Derived **views** (`configJson`/`presetsJson`/`outputsJson`) become thin,
  clearly-named slices *computed from* that form — or are dropped if the client
  can slice the full doc itself.
- **Delete `Output::appendBasicInfo()`** (dead) and fold `describe()` into
  `toJson()` (a `verbose`/`describe` flag if a human-summary subset is still
  wanted).
- **Pass parsed `JsonObjectConst`, not `std::string`**, into `fromJson` — parse
  once at the dispatcher. Stop returning `JsonDocument` by value; fill a
  caller-provided object.

## Trade-offs / my honest opinion

**Do it — the direction is right.** JSON-as-truth is the correct call for a
device driven by a network UI: the UI already speaks JSON, presets are already
JSON, so persistence and construction *should* be the same JSON. The current
split (compiled fixtures + config-overlay NVS) is the source of the fragility.

Caveats to plan for:

1. **Runtime build cost & heap.** Building outputs from JSON at boot allocates on
   the heap instead of in `.bss`/compile-time. On ESP32 this is fine for a
   handful of fixtures but watch fragmentation; build once at boot, not per
   command.
2. **Validation moves to runtime.** A bad compiled type used to fail to compile;
   a bad JSON definition fails at boot. The factory **must** validate and
   degrade gracefully (skip unknown output, clamp bad preset — the code already
   logs+ignores invalid presets, keep that discipline).
3. **Schema versioning.** Once NVS holds full definitions, you need a `"schema"`
   or reuse `"version"` to migrate old records. Add it from day one.
4. **Default definitions need a home.** Replacing `main.cpp`'s
   `addFixture<...>()` with a JSON string means shipping default `.json`
   (embedded as a `PROGMEM` string or LittleFS asset). Decide where.
5. **Keep the good bits.** The threading model, `SyncBuffer`, ref-counted
   subscriptions, and the `LEDBase<TDriver>` split are orthogonal to this change
   — don't disturb them.

## Suggested sequencing

1. **Consolidate serialization first** (delete `appendBasicInfo`, unify
   `toJson`, pass parsed JSON down). Low risk, immediately clarifies the model.
2. **Add `Output::Factory`** and make `Fixture::fromJson` build outputs — but
   keep `addFixture<T>()` working in parallel (T's ctor produces the def JSON).
3. **Close the round-trip**: `toJson` emits full def; `persistFixture` stores it;
   boot rebuilds from NVS.
4. **Flip `main.cpp`** to `addFixture(defJson)` and retire the per-fixture C++
   types + `MakeJSON`.
5. **Later**: input factory + per-universe protocol map (out of scope here, noted
   in the puml).
