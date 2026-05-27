# Firmware Command API

All commands are sent as JSON over TCP (port 8888) or Serial.
Responses come back via the same transport.

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
     "version": "0.96", "serial_report_task": true, ...
     "fixtures": {"fixtures": [{"id":0, "name":"...", ...}]},
     "inputs": {"inputs": [{"universe":1, "refcount":2}]}
   }
```

## fixtures / getfixture

```
-> {"request": "fixtures"}
<- {"fixtures": [{"id":0, "name":"strip", "type":"ws2815", ...}]}

-> {"request": "getfixture", "id": 0}
<- {"id":0, "name":"strip", "type":"ws2815", "universe":8, "address":0, "presetIndex":0, "numPresets":3}

-> {"request": "getfixture", "id": 0, "value": "universe"}
<- {"req": "universe", "val": 8}

-> {"request": "getfixture", "id": 99}
<- {"err": "fixture not found"}
```

## setfixture

```
-> {"request": "setfixture", "id": 0, "universe": 5}
-> {"request": "setfixture", "id": 0, "address": 100}
-> {"request": "setfixture", "id": 0, "presetIndex": 2}
-> {"request": "setfixture", "id": 0, "name": "main"}
-> {"request": "setfixture", "id": 0, "highlight": true}
-> {"request": "setfixture", "id": 0, "universe": 5, "address": 10}
<- {"status": "ok", "id":0, "name":"main", "universe":5, ...}

-> {"request": "setfixture"}
<- {"err": "missing id"}
```

## inputs

```
-> {"request": "inputs"}
<- {"inputs": [{"universe": 1, "refcount": 2}, {"universe": 5, "refcount": 1}]}
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
{"version": "0.96", "fixtures": [{"name": "strip", "type": "ws2815"}, ...]}
```
