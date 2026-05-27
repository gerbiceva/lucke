# Firmware Command API

## Network Overview

| Port  | Protocol | Direction | Description                        |
|-------|----------|-----------|------------------------------------|
| 8888  | TCP      | Receive   | Command input                      |
| 12345 | TCP      | Send      | Command response                   |
| 12343 | UDP      | Send      | Heartbeat broadcast (every 500ms)  |
| 12344 | UDP      | Send      | Wireless report (every 10s)        |
| 5568  | UDP      | Receive   | sACN / E1.31 DMX input (multicast) |
| Serial| UART     | Both      | Command input / output             |

---

## Response keys

| Key      | Meaning                        |
|----------|--------------------------------|
| `err`    | Error message                  |
| `status` | Status string (`ok`, etc.)     |
| `req`    | Echo of what was requested     |
| `val`    | Returned value                 |

---

## describe

```
-> {"request": "describe"}
<- {
     "settings": {"version": "0.96", "serial_report_task": true, ...},
     "wifi": {"rssi": -61, "local_ip": "192.168.0.143", "connected": true},
     "fixture_handler": {"fixtures": [{"id":0, "name":"...", ...}]},
     "input_handler": {"num_inputs": 1, "inputs": [{"type": "SACN", "universe": 8}]}
   }
```

## fixtures / getfixture

```
-> {"request": "fixtures"}
<- {"fixtures": [{"id":0, "name":"strip", "type":"Strip2815", ...}]}

-> {"request": "getfixture", "id": 0}
<- {"id":0, "name":"strip", "type":"Strip2815", "universe":8, "address":0, "presetIndex":0, "numPresets":8}

-> {"request": "getfixture", "id": 0, "value": "universe"}
<- {"req": "universe", "val": 8}

-> {"request": "getfixture", "id": 0, "value": "address"}
<- {"req": "address", "val": 0}

-> {"request": "getfixture", "id": 0, "value": "preset"}
<- {"req": "preset", "val": 0}

-> {"request": "getfixture", "id": 0, "value": "name"}
<- {"req": "name", "val": "strip"}

-> {"request": "getfixture", "id": 0, "value": "type"}
<- {"req": "type", "val": "Strip2815"}

-> {"request": "getfixture", "id": 0, "value": "config"}
<- {"req": "config", "val": {
     "id":0, "name":"strip", "type":"Strip2815", "universe":8, "address":0,
     "presetIndex":0, "numPresets":8, "footprint":72,
     "outputs": [{"uuid":0, "type":"FastLED-1D", "hardware_pin":5, "num_leds":24, "num_groups":24}]
   }}

-> {"request": "getfixture", "id": 0, "value": "presets"}
<- {"req": "presets", "val": {
     "selected": 0,
     "presets": {"groups": [
       {"name": "group by 1", "settings": [{"num_groups": 24}]},
       {"name": "group by 2", "settings": [{"num_groups": 12}]},
       ...
     ]}
   }}

-> {"request": "getfixture", "id": 99}
<- {"err": "fixture not found"}
```

## setfixture

```
-> {"request": "setfixture", "id": 0, "universe": 5}
-> {"request": "setfixture", "id": 0, "address": 100}
-> {"request": "setfixture", "id": 0, "presetIndex": 2}
-> {"request": "setfixture", "id": 0, "name": "main"}
-> {"request": "setfixture", "id": 0, "universe": 5, "address": 10}
<- {"status": "ok", "id":0, "name":"main", "universe":5, ...}

-> {"request": "setfixture", "id": 0, "highlight": true}
<- {"status": "ok", ...}
```

Highlight toggles all LEDs to white. Sending `"highlight": true` again toggles it off.

```
-> {"request": "setfixture"}
<- {"err": "missing id"}
```

## inputs

```
-> {"request": "inputs"}
<- {"num_inputs": 1, "inputs": [{"type": "SACN", "universe": 8}]}
```

## wifi

```
-> {"request": "wifi"}
<- {"rssi": -45, "local_ip": "192.168.1.75", "connected": true, "ssid": "Ledique"}

-> {"request": "wifi", "ssid": "MyNet", "password": "pass123"}
<- {"status": "ok", "rssi": -45, "local_ip": "...", "connected": true, "ssid": "MyNet"}
```

Reconnects after 1s.

## reboot

```
-> {"request": "reboot"}
<- {"status": "rebooting"}
```

Restarts after 1s.

## factory_reset

```
-> {"request": "factory_reset"}
<- {"status": "factory reset, rebooting"}
```

Sets flag, saves, restarts. Next boot all fixtures use defaults.

## serial_report_task

```
-> {"request": "serial_report_task"}
<- {"req": "serial_report_task", "val": true}

-> {"request": "serial_report_task", "value": false}
<- {"req": "serial_report_task", "val": false}
```

## wireless_report_task

```
-> {"request": "wireless_report_task"}
<- {"req": "wireless_report_task", "val": true}

-> {"request": "wireless_report_task", "value": true}
<- {"req": "wireless_report_task", "val": true}
```

## wifi_animation

Controls whether a running-pixel animation plays on LED outputs when WiFi is disconnected.
The animation always runs before the first connection. After connecting, it only resumes on disconnect if enabled.

```
-> {"request": "wifi_animation"}
<- {"req": "wifi_animation", "val": false}

-> {"request": "wifi_animation", "value": true}
<- {"req": "wifi_animation", "val": true}
```

## Dispatcher errors

```
-> invalid json
<- {"err": "invalid json"}

-> {"foo": "bar"}
<- {"err": "missing cmd"}

-> {"request": "unknown"}
<- {"err": "unknown cmd"}
```

## Heartbeat (UDP broadcast, port 12343, every 500ms)

```
{"version": "0.96", "fixtures": [{"name": "strip", "type": "Strip2815"}, ...]}
```

## Wireless Report (UDP broadcast, port 12344, every 10s)

Full engine state as JSON (settings, wifi, fixtures, inputs). Only sent when `wireless_report_task` is enabled and WiFi is connected.

## Serial Report (UART, every 10s)

Same full engine state as the wireless report, printed to Serial. Only active when `serial_report_task` is enabled.
