# Architecture Improvement Proposal

## Table of Contents
1. [Current Problems](#1-current-problems)
2. [Proposed Changes Overview](#2-proposed-changes-overview)
3. [Change 1 — Replace Raw Buffer Alias with SyncBuffer](#3-change-1--replace-raw-buffer-alias-with-syncbuffer)
4. [Change 2 — InputManager: Ref-counted Subscriptions + Universe Validation](#4-change-2--inputmanager-ref-counted-subscriptions--universe-validation)
5. [Change 3 — ITransport + TcpConnection: Same-Socket Response](#5-change-3--itransport--tcpconnection-same-socket-response)
6. [Change 4 — CommandDispatcher: Replace Engine::parseConfig](#6-change-4--commanddispatcher-replace-engineparseconfig)
7. [Change 5 — WiFiManager: Events Instead of Polling](#7-change-5--wifimanager-events-instead-of-polling)
8. [Change 6 — Config as Pure Data](#8-change-6--config-as-pure-data)
9. [Change 7 — Application as Thin Orchestrator](#9-change-7--application-as-thin-orchestrator)
10. [Migration Order](#10-migration-order)

---

## 1. Current Problems

### 1.1 Race condition on DMX buffer
`Fixture::m_srcBuffer` is a raw pointer alias into `InputInterface::m_dmxBuffer`.
The "DMX Input" task writes 512 bytes via `memcpy` while the "Output" task reads from
the same memory. No mutex, no atomic, no double-buffering. Torn reads produce
corrupt frames under load.

### 1.2 Default universe collision
`Config::Fixture::universe` defaults to `8` for every fixture. On first boot both
fixtures call `InputHandler::subscribe(8)` and receive the **same**
`shared_ptr<InputInterface>`, so `m_srcBuffer` in both fixtures points to the
same `m_dmxBuffer`. Identical data on both outputs even when universes differ in storage.

### 1.3 Universe not validated in received sACN packets
`SacnParser::update()` calls `m_udp.parsePacket()` and blindly accepts whatever
UDP datagram arrives on port 5568. The E1.31 universe field at bytes 113–114 of the
packet is never checked. When two `Sacn` objects are active on the same port, they
race to consume each other's packets.

### 1.4 Unconditional `unsubscribe` corrupts the input map
`InputHandler::unsubscribe()` always calls `m_inputs.erase(universe)` regardless
of how many fixtures still hold a `shared_ptr` to that source. If two fixtures share
a universe and one changes its universe, the map entry is removed even though the
other fixture's source is still alive. A subsequent `subscribe()` to the same universe
creates a second `Sacn` object — two live sACN receivers for the same universe competing
on the same port.

### 1.5 TCP response sent via reverse outbound connection
`receiveData` accepts a client on port 8888. `parseConfig` assembles the response and
calls `sendTcpPacket(host, 12345, response)`, which opens a **new outbound** TCP
connection back to the client on port 12345. The client must be a TCP server on port
12345. If the client is behind NAT the response never arrives. The correct model is to
write the response back on the same connection that delivered the request.

### 1.6 `Engine` is a god object
`Engine` handles: WiFi management, fixture management, input management, TCP/serial
command parsing, report generation, task spawning, NVS persistence, and JSON routing.
A 700-line class with 20+ responsibilities makes every change risky.

### 1.7 Task lifecycle not fully managed
WiFi tasks are spawned via raw `xTaskCreate` with `NULL` handles. They cannot be
suspended, resumed, or stopped. All other tasks go through `TaskExecutor`. The two
systems are inconsistent.

### 1.8 `readString()` blocks for 1 second per TCP request
`client.readString()` waits for the Arduino default stream timeout (1000 ms) after the
last received byte before returning. Every request incurs at least 1 second of
artificial latency even when the full payload arrived in one segment.

---

## 2. Proposed Changes Overview

| # | Change | Fixes |
|---|--------|-------|
| 1 | `SyncBuffer` replaces raw `uint8_t*` alias | Race condition 1.1 |
| 2 | `InputManager` with ref-counted subscribe + E1.31 universe validation | Issues 1.2, 1.3, 1.4 |
| 3 | `ITransport` / `TcpConnection` — response on same socket | Issue 1.5 |
| 4 | `CommandDispatcher` — replaces `Engine::parseConfig` | Issue 1.6 |
| 5 | `WiFiManager` — event-driven, not polled | Issues 1.6, 1.7 |
| 6 | `Config` as pure data bags | Issue 1.6 |
| 7 | `Application` as thin orchestrator | Issue 1.6, 1.7 |

Each change is independent enough to be applied incrementally.

---

## 3. Change 1 — Replace Raw Buffer Alias with SyncBuffer

### Problem
`Fixture::m_srcBuffer` is a raw `uint8_t*` pointing into
`InputInterface::m_dmxBuffer[0]`. The Input task writes to it; the Output task reads
from it. No synchronization.

### Solution
Introduce `SyncBuffer` — a mutex-protected 512-byte buffer. The input task **writes**
into it after polling the sACN socket. Each `Fixture::update()` **reads** from it into
a local copy, then feeds that local copy to its outputs.

```cpp
// src/Input/SyncBuffer.h
class SyncBuffer {
    std::array<uint8_t, 512> m_buf{};
    SemaphoreHandle_t        m_mutex;
public:
    SyncBuffer() {
        m_mutex = xSemaphoreCreateMutex();
    }

    // Called by InputManager task
    void write(const uint8_t* src) {
        xSemaphoreTake(m_mutex, portMAX_DELAY);
        memcpy(m_buf.data(), src, 512);
        xSemaphoreGive(m_mutex);
    }

    // Called by Fixture::update()
    void read(uint8_t* dst) {
        xSemaphoreTake(m_mutex, portMAX_DELAY);
        memcpy(dst, m_buf.data(), 512);
        xSemaphoreGive(m_mutex);
    }
};
```

### Changes to `Fixture`
- Replace `uint8_t* m_srcBuffer` with `shared_ptr<SyncBuffer> m_buffer`
- Add `std::array<uint8_t, 512> m_localBuf` as a per-frame local snapshot
- In `update()`: call `m_buffer->read(m_localBuf.data())` first, then feed
  `m_localBuf.data() + m_config.address` to each output

```cpp
void Fixture::update() {
    m_buffer->read(m_localBuf.data());
    uint16_t offset = 0;
    for (IOutputSink* o : m_outputs) {
        o->write(m_localBuf.data() + m_config.address + offset, o->footprint());
        o->flush();
        offset += o->footprint();
    }
}
```

The outputs no longer hold a raw pointer into an InputInterface buffer. They receive
data on every `update()` call through a safe copy.

---

## 4. Change 2 — InputManager: Ref-counted Subscriptions + Universe Validation

### Problem
- `InputHandler::unsubscribe` erases unconditionally
- Both fixtures default to universe 8 → shared buffer
- `SacnParser` accepts any packet on port 5568 without universe check

### 2a — Ref-counted subscribe/unsubscribe

```cpp
// InputManager keeps a refcount alongside each source
class InputManager {
    struct Entry {
        shared_ptr<IInputSource> source;
        shared_ptr<SyncBuffer>   buffer;
        int                      refcount = 0;
    };
    std::unordered_map<uint8_t, Entry> m_entries;

public:
    shared_ptr<SyncBuffer> subscribe(uint8_t universe) {
        auto& e = m_entries[universe];
        if (e.refcount == 0) {
            e.source = make_shared<SacnSource>();
            e.source->begin(universe);
            e.buffer = make_shared<SyncBuffer>();
        }
        e.refcount++;
        return e.buffer;
    }

    void unsubscribe(uint8_t universe) {
        auto it = m_entries.find(universe);
        if (it == m_entries.end()) return;
        if (--it->second.refcount == 0) {
            it->second.source->stop();
            m_entries.erase(it);
        }
    }
};
```

### 2b — Universe validation in SacnParser

The E1.31 (sACN) packet layout places the universe at bytes **113–114** (big-endian,
zero-indexed). `ADDR_SEQ = 111` is already correct in the existing code.

```cpp
// src/Utils/SacnParser.cpp
bool SacnParser::poll(uint8_t* dst) {
    int size = m_udp.parsePacket();
    if (!size) return false;

    m_udp.read(m_packet, size);

    // Validate universe (E1.31 bytes 113-114, big-endian)
    uint16_t pkt_universe = (static_cast<uint16_t>(m_packet[113]) << 8)
                          |  static_cast<uint16_t>(m_packet[114]);
    if (pkt_universe != m_universe) return false;   // discard wrong universe

    memcpy(dst, m_packet + ADDR_DMX, 512);
    m_seq_prev = m_seq;
    m_seq = m_packet[ADDR_SEQ];
    return true;
}
```

### 2c — Remove default universe 8 collision

Each fixture type should declare its own default universe, or the `FixtureRegistry`
should assign universes sequentially on first boot:

```cpp
// In FixtureRegistry::add()
Fixture* fix = new TFixture();
fix->fromConfig();                // loads NVS if available

// On factory reset / no stored config, assign unique universes
if (factoryReset || !m_storage.isKey(key)) {
    fix->setUniverse(m_nextUniverse++);  // 1, 2, 3...
}
```

---

## 5. Change 3 — ITransport + TcpConnection: Same-Socket Response

### Problem
Response is sent via a new outbound TCP connection to the sender's port 12345,
requiring the sender to run a TCP server.

### Solution
Introduce `ITransport` as a thin abstraction over any bidirectional channel
(TCP connection, serial port). The command handler receives an `ITransport&` and
writes the response directly back through it — no reverse connection needed.

```cpp
// src/Network/ITransport.h
class ITransport {
public:
    virtual void        send(const std::string& data) = 0;
    virtual std::string readLine()                    = 0;  // delimiter-terminated
    virtual bool        isAvailable()                 = 0;
    virtual std::string remoteAddress()               = 0;
    virtual void        close()                       = 0;
    virtual ~ITransport() = default;
};
```

```cpp
// src/Network/TcpConnection.h
class TcpConnection : public ITransport {
    WiFiClient m_client;
public:
    explicit TcpConnection(WiFiClient client)
        : m_client(std::move(client))
    {
        m_client.setTimeout(100);   // 100 ms read timeout, not 1000 ms
    }

    void send(const std::string& data) override {
        m_client.print(data.c_str());
        m_client.flush();           // ensure bytes leave the send buffer
    }

    std::string readLine() override {
        return m_client.readStringUntil('\n').c_str();  // delimiter, not timeout
    }

    bool        isAvailable()    override { return m_client.available(); }
    std::string remoteAddress()  override { return m_client.remoteIP().toString().c_str(); }
    void        close()          override { m_client.stop(); }
};
```

```cpp
// src/Network/SerialTransport.h
class SerialTransport : public ITransport {
public:
    void send(const std::string& data) override {
        Serial.println(data.c_str());
    }
    std::string readLine() override {
        return Serial.readStringUntil('\n').c_str();
    }
    bool        isAvailable()   override { return Serial.available(); }
    std::string remoteAddress() override { return "Serial0"; }
    void        close()         override {}
};
```

### Updated receive loop

```cpp
void receiveTask() {
    TcpServer server(8888);
    server.begin();

    while (true) {
        if (!wifi.isConnected()) { vTaskDelay(100); continue; }

        auto opt = server.accept();
        if (opt.has_value()) {
            TcpConnection conn(std::move(opt.value()));
            while (conn.isConnected()) {
                if (conn.isAvailable()) {
                    std::string line = conn.readLine();
                    if (!line.empty()) {
                        dispatcher.dispatch(line, conn);   // response goes HERE
                    }
                }
                vTaskDelay(5);  // yield — no CPU spin
            }
            conn.close();
        }
        vTaskDelay(10);
    }
}
```

**Key points:**
- `dispatch(line, conn)` passes the same connection as the transport — response is
  written back immediately without a reverse connection
- `readLine()` uses a delimiter (`\n`) — no 1-second timeout per request
- `vTaskDelay(5)` inside the connected loop — no CPU spin
- `conn.send()` calls `flush()` — no truncated responses

---

## 6. Change 4 — CommandDispatcher: Replace Engine::parseConfig

### Problem
`Engine::parseConfig` is a 200-line chain of `strcmp` checks. Every new command
requires modifying `Engine`. All command logic is coupled to the god object.

### Solution
`CommandDispatcher` maps command strings to `ICommandHandler` implementations.
Each handler is a separate class with injected dependencies.

```cpp
// src/Commands/CommandDispatcher.h
class CommandDispatcher {
    std::unordered_map<std::string, ICommandHandler*> m_handlers;
public:
    void registerHandler(const std::string& cmd, ICommandHandler* h) {
        m_handlers[cmd] = h;
    }

    void dispatch(const std::string& raw, ITransport& transport) {
        JsonDocument doc;
        if (deserializeJson(doc, raw)) {
            sendError(transport, "Invalid JSON");
            return;
        }
        const char* cmd = doc["request"];
        auto it = m_handlers.find(cmd ? cmd : "");
        if (it == m_handlers.end()) {
            sendError(transport, "Unknown command");
            return;
        }
        it->second->handle(doc, transport);
    }

private:
    void sendError(ITransport& t, const char* msg) {
        JsonDocument r;
        r["status"] = "ERROR";
        r["response"] = msg;
        std::string s; serializeJson(r, s);
        t.send(s);
    }
};
```

Example handler:

```cpp
// src/Commands/SetFixtureHandler.h
class SetFixtureHandler : public ICommandHandler {
    FixtureRegistry& m_registry;
public:
    explicit SetFixtureHandler(FixtureRegistry& r) : m_registry(r) {}

    void handle(const JsonDocument& req, ITransport& transport) override {
        JsonDocument resp;
        if (!req["id"].is<uint8_t>()) {
            resp["status"] = "ERROR"; resp["response"] = "Missing id";
            std::string s; serializeJson(resp, s); transport.send(s); return;
        }
        uint8_t id = req["id"];
        // ... apply changes via m_registry ...
        resp["status"] = "OK"; resp["response"] = "Done";
        std::string s; serializeJson(resp, s);
        transport.send(s);
    }
};
```

### Registration in Application::registerCommands()

```cpp
void Application::registerCommands() {
    m_dispatcher.registerHandler("describe",       new DescribeHandler(m_registry, InputManager::instance()));
    m_dispatcher.registerHandler("getfixture",     new GetFixtureHandler(m_registry));
    m_dispatcher.registerHandler("setfixture",     new SetFixtureHandler(m_registry));
    m_dispatcher.registerHandler("inputs",         new InputsHandler(InputManager::instance()));
    m_dispatcher.registerHandler("wifi",           new WiFiConfigHandler(m_wifi, m_config, m_storage));
    m_dispatcher.registerHandler("reboot",         new RebootHandler());
    m_dispatcher.registerHandler("factory_reset",  new FactoryResetHandler(m_storage));
}
```

Adding a new command = adding one new class + one `registerHandler` call. No
changes to any existing class.

---

## 7. Change 5 — WiFiManager: Events Instead of Polling

### Problem
`monitorConnection` fires the status callback 20×/second unconditionally via
`vTaskDelay(50)` even when nothing has changed. The `m_ssid`/`m_password` raw
pointer fields are dangling-pointer-prone.

### Solution
Use the ESP32 native WiFi event system instead of polling. Register callbacks for
`WIFI_EVENT_STA_CONNECTED` and `WIFI_EVENT_STA_DISCONNECTED`. Store credentials as
`std::string` instead of `const char*`.

```cpp
class WiFiManager {
    std::string m_ssid;
    std::string m_password;
    std::function<void(bool)> m_onStatus;

public:
    void connect(const std::string& ssid, const std::string& password,
                 std::function<void(bool)> onStatus)
    {
        m_ssid     = ssid;       // owned copies, never dangling
        m_password = password;
        m_onStatus = onStatus;

        WiFi.onEvent([this](WiFiEvent_t e, WiFiEventInfo_t) {
            if (e == ARDUINO_EVENT_WIFI_STA_GOT_IP) m_onStatus(true);
            if (e == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) m_onStatus(false);
        });

        WiFi.setScanMethod(WIFI_FAST_SCAN);
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(m_ssid.c_str(), m_password.c_str());
    }

    void reconnect(const std::string& ssid, const std::string& password) {
        m_ssid     = ssid;
        m_password = password;
        WiFi.disconnect(true);
        vTaskDelay(100);
        WiFi.begin(m_ssid.c_str(), m_password.c_str());
    }
};
```

- No `monitorConnection` task needed
- Credentials stored by value — `reconnect()` updates them safely
- Callback fires exactly once on state change, not 20 times/second

---

## 8. Change 6 — Config as Pure Data

### Problem
`Config::Fixture` has a static `s_ID` counter mixed with per-fixture data.
`Config::Engine` mixes WiFi credentials with task flags. Both inherit
`Traits::Deserializable` (a "can serialize" interface) mixing data storage with
serialization behaviour.

### Solution
Make config structs plain data bags. ID assignment moves to `FixtureRegistry`.
Serialization stays in the config struct as free methods but no virtual dispatch.

```cpp
// src/Config/FixtureConfig.h
struct FixtureConfig {
    uint8_t  id           = 0;     // assigned by FixtureRegistry
    std::string name      = "fixture";
    std::string type      = "Generic";
    uint8_t  universe     = 1;     // no shared default; registry assigns unique values
    uint16_t address      = 0;
    uint8_t  preset       = 0;
    uint8_t  numPresets   = 0;
    bool     highlighted  = false;

    void     toJson(JsonObject& obj) const;
    void     fromJson(const JsonObject& obj);
};
```

```cpp
// src/Config/AppConfig.h
struct AppConfig {
    std::string ssid               = "Ledique";
    std::string password           = "...";
    std::string version            = ENGINE_VERSION;
    bool        serialReportTask   = true;
    bool        wirelessReportTask = true;
    bool        wifiAnimation      = false;
    bool        toFactorySettings  = false;

    void        toJson(JsonObject& obj) const;
    void        fromJson(const JsonObject& obj);
    std::string toString() const;
};
```

---

## 9. Change 7 — Application as Thin Orchestrator

### Problem
`Engine` owns all logic: WiFi callbacks, command handling, report tasks, animation
tasks, fixture serialization. It calls `parseConfig` (700 lines) directly.

### Solution
`Application` owns all subsystems but delegates every concern to them.
Its only responsibilities are: wiring, task lifecycle, and reacting to WiFi status.

```cpp
class Application {
    AppConfig          m_config;
    Storage            m_storage;
    WiFiManager        m_wifi;
    TcpServer          m_tcpServer;
    UdpBroadcaster     m_udp;
    SerialTransport    m_serial;
    CommandDispatcher  m_dispatcher;
    FixtureRegistry    m_registry;
    TaskManager        m_tasks;

    uint32_t m_inputTaskId  = 0;
    uint32_t m_outputTaskId = 0;
    uint32_t m_tcpTaskId    = 0;
    uint32_t m_serialTaskId = 0;

public:
    static Application& instance();

    void init() {
        readConfig();
        registerCommands();
        m_wifi.connect(m_config.ssid, m_config.password,
                       [this](bool c){ onWifiStatus(c); });
        spawnTasks();
    }

private:
    void onWifiStatus(bool connected) {
        if (connected) {
            m_tasks.resume(m_inputTaskId);
            m_tcpServer.restart();
        } else {
            m_tasks.suspend(m_inputTaskId);
        }
    }

    void spawnTasks() {
        m_inputTaskId  = m_tasks.spawn("Input",    [this]{ InputManager::instance().update(); },  3, 20000);
        m_outputTaskId = m_tasks.spawn("Output",   [this]{ m_registry.update(); },                3, 20000);
        m_tcpTaskId    = m_tasks.spawn("TCP",      [this]{ tcpReceiveLoop(); },                   3, 12000);
        m_serialTaskId = m_tasks.spawn("Serial",   [this]{ serialReceiveLoop(); },                2, 4000);
        m_tasks.spawn("Heartbeat", [this]{ heartbeatLoop(); }, 1, 2000);
    }
};
```

Every task is managed through `TaskManager` (former `TaskExecutor`), including the
TCP and serial receive loops. All handles are stored, so WiFi disconnect/reconnect
can properly suspend/resume the input task.

---

## 10. Migration Order

Apply changes in this order to keep the firmware buildable at every step:

### Step 1 — `SyncBuffer` (no other changes)
1. Create `src/Input/SyncBuffer.h`
2. In `InputHandler::subscribe()`, wrap the created `InputInterface` so that
   `InputInterface::update()` writes into a `SyncBuffer`
3. In `Fixture`, replace `uint8_t* m_srcBuffer` with `shared_ptr<SyncBuffer> m_buffer`
   and add `array<uint8_t,512> m_localBuf`
4. In `Fixture::update()`, read from `SyncBuffer` into `m_localBuf` before feeding outputs
5. Verify: both tasks build, outputs still driven correctly

### Step 2 — Universe validation + ref-counted InputManager
1. Add universe check to `SacnParser::poll()`
2. Add `m_refcount` map to `InputHandler`; make `unsubscribe()` decrement before erasing
3. Change `FixtureRegistry::add()` to assign sequential universes starting at 1 on
   factory reset (remove the shared universe=8 default)
4. Verify: two fixtures with different universes receive different data

### Step 3 — `ITransport` + `TcpConnection`
1. Create `src/Network/ITransport.h`
2. Create `src/Network/TcpConnection.h` implementing `ITransport`
3. Create `src/Network/SerialTransport.h` implementing `ITransport`
4. Move the TCP server loop out of `Wifi::receiveData` into a standalone
   `tcpReceiveLoop()` that uses `TcpConnection`
5. Thread `ITransport&` through `Engine::parseConfig` as a parameter instead of
   `std::string host` — responses call `transport.send()` rather than
   `sendTcpPacket(host, ...)`
6. Verify: client on port 8888 sends command, receives response on same connection

### Step 4 — `CommandDispatcher`
1. Create `src/Commands/ICommandHandler.h` and `CommandDispatcher.h`
2. Extract each `strcmp` branch from `Engine::parseConfig` into its own handler class
3. Register all handlers in `Engine::init()` (or new `Application::registerCommands()`)
4. Replace `Engine::parseConfig` body with `m_dispatcher.dispatch(data, transport)`
5. Verify: all existing commands work identically

### Step 5 — `WiFiManager` (event-driven)
1. Create `src/Network/WiFiManager.h` storing credentials as `std::string`
2. Register ESP32 WiFi events instead of spawning `monitorConnection` task
3. Replace `Wifi` usage in `Engine` with `WiFiManager`
4. Verify: connect, disconnect, and reconnect all trigger correct task suspend/resume

### Step 6 — `Application` (thin orchestrator)
1. Rename / refactor `Engine` to `Application`
2. Move all task spawning into `Application::spawnTasks()` — include WiFi tasks
3. Remove all command logic from `Application` (done in Step 4)
4. Verify: full system builds and operates correctly

---

## Summary of Benefits

| Area | Before | After |
|------|--------|-------|
| Buffer safety | Raw pointer alias, no lock | `SyncBuffer` with mutex |
| Input subscription | Unconditional erase, universe=8 collision | Ref-counted, unique universe per fixture |
| Universe validation | None — wrong packets accepted | E1.31 bytes 113-114 checked |
| TCP response | Reverse outbound connection to port 12345 | Same socket, `transport.send()` |
| Request latency | ≥1 s (`readString()` timeout) | Delimiter-based, ~0 ms overhead |
| Command routing | 200-line `strcmp` chain in Engine | `CommandDispatcher` + `ICommandHandler` |
| WiFi status | Polled 20×/s unconditionally | Event-driven, fires on state change only |
| Credentials storage | `const char*` (dangling-prone) | `std::string` (owned copies) |
| Task management | Two separate systems (raw + TaskExecutor) | All tasks through `TaskManager` |
| Extensibility | Modify Engine to add any command | Add one new `ICommandHandler` class |
