# Improvements / TODO

Open work only. Current design: [`architecture.md`](./architecture.md).

## 1. JSON-authoritative fixtures (next big feature)

Make one JSON document the single source of truth for a fixture: the same
document is the transport payload (`get_fixture` / `set_fixture`), the NVS
record, and what the fixture is **built from**. Today fixtures are compiled
types (`Fixtures::Strip<Type, TDriver, N, Pin>`) and NVS only stores a config
overlay, so a stored record can't rebuild a fixture.

Target document:

```jsonc
{
  "id": 0,
  "name": "stage-left",
  "type": "StripWS2815",
  "universe": 1,            // base universe; the span is derived, never stored
  "address": 0,
  "preset_index": 0,
  "outputs": [
    { "shape": "1d", "driver": "ws2815", "pin": 5, "num_leds": 24 }
    // grid: { "shape": "grid", "driver": "ws2815", "width": 8, "height": 24,
    //         "strips": [ { "pin": 5, "num_leds": 192, "x": [0,8], "y": [0,24] } ] }
  ],
  "presets": {
    "groups": [
      { "name": "group by 1",  "settings": [ { "num_groups": 24 } ] },
      { "name": "group by 24", "settings": [ { "num_groups": 1  } ] }
    ]
  }
}
```

`settings[k]` maps positionally to `outputs[k]`.

Changes:

- **`Output::Factory`** (new): `create(JsonObjectConst)`, switching on
  `shape` + `driver`. Unknown → log and skip. The only place that knows
  concrete output types.
- **`Fixture::fromJson`** builds the fixture: config → presets → outputs via
  the factory → `setPreset`. **`toJson`** emits the full definition, closing
  the round-trip.
- **`addFixture(defJson)`** replaces `addFixture<T>()`. Boot: stored record if
  present and not factory reset, else the shipped default, then persist.
- Retire per-fixture C++ types and compile-time presets
  (`Presets::c_groupPresets<N>`) on the JSON path.

Constraints and caveats:

- `Drivers::FastLed::make<Pin>` needs the pin at compile time. Either a
  `constexpr` table of supported pins dispatching to template instances, or a
  runtime-pin backend (ESP-IDF RMT). `Drivers::Ws2815` takes a runtime pin but
  supports one strip (SPI2).
- Validation moves to runtime: the factory must degrade gracefully.
- Add a schema version to stored records from day one.
- Default definitions need a home (embedded string or LittleFS asset).
- Keep JSON text in NVS; switch encoding to MessagePack only if size bites.

Sequencing:

1. Consolidate serialization: one `toJson` per object as the full form;
   `describe` / `configJson` / `outputsJson` become slices of it or go away.
   Pass parsed `JsonObjectConst` down instead of strings.
2. Add `Output::Factory`; `fromJson` builds outputs, with `addFixture<T>()`
   still working in parallel.
3. Close the round-trip: persist the full definition, rebuild from NVS at boot.
4. Switch `main.cpp` to `addFixture(defJson)`; retire per-fixture types.
5. Later: input factory and per-universe protocol map.

## 2. Grid output

No grid output exists: `Output/Base/LEDGridBase.h` is commented out and must
be ported to `Output::Driver` (one driver per strip). Design its JSON
(`strips[]`) together with §1.

Carry over from the old `LEDGrid` / `VirtualGrid` prototype
(`unused/src_prev/`), fixing its bugs:

- **One index formula.** Grid coordinate → strip-local index in one place
  (`DeviceMapping::localIndex`); the prototype computed it twice and the copy
  in `addLEDStrip` was wrong (pre-incremented counter, slot 0 never written).
- **Stride fan-out.** A virtual cell covers `xRange.begin + x*xStride + i`
  (the prototype multiplied `x * i`).
- Zero-initialize the cell pointer table so `isComplete()` works.
- Optionally make it generic over the cell type (`Grid<T>`, `Device<T>`), so
  the same addressing works for DMX channel arrays or test vectors.

## 3. Serial text shell

Human-friendly commands over serial, translated into the existing JSON
commands. Handlers and TCP stay unchanged; lines starting with `{` are still
raw JSON.

- `LineEditor`: echo, backspace, history; logger output redraws the prompt.
- `TextParser` + command table (`verb`, `cmd`, positional args, help,
  optional formatter): tokens → `JsonDocument` → `CommandDispatcher`. Local
  commands: `help`, confirm, did-you-mean.
- Replies: a `TextReply` transport decorator renders JSON as text for text
  lines; raw JSON lines get raw JSON back.
- `SerialShell` replaces `m_serialSession` in `Core/Engine.h`.

## 4. Smaller items

- More than one WS2815 strip (only SPI2 today; others must use FastLED).
- sACN: priority merge across sources; on source loss, blank or fade instead
  of holding the last frame.
- Delete the commented-out BLE transport (`Network/BleTransport.*`); it's in
  git history.
- `Config` as pure data is partly done: `Config::Engine` / `Config::Fixture`
  are data + JSON, NVS lives in `Services::SettingsStore`.
