import socket
import json
import argparse
from contextlib import closing

SEND_PORT = 8888


class ESPClient:
    """Sends one JSON line per request; the reply comes back as one JSON
    line on the same connection."""

    def __init__(self, ip: str):
        self.ip = ip

    # ---- send request, read the reply line ----
    def send(self, payload: dict, timeout: float = 5.0):
        try:
            with closing(socket.create_connection((self.ip, SEND_PORT), timeout=timeout)) as s:
                s.sendall((json.dumps(payload) + "\n").encode())
                buf = b""
                while b"\n" not in buf:
                    chunk = s.recv(4096)
                    if not chunk:
                        break
                    buf += chunk
        except OSError as e:
            print(f"Socket error: {e}")
            return None

        line = buf.split(b"\n", 1)[0].decode(errors="replace")
        if not line:
            return None
        try:
            return json.loads(line)
        except json.JSONDecodeError:
            return line

    # ---- commands (protocol 2, see docs/apiv2.md) ----
    def describe(self):
        return self.send({"cmd": "describe"})

    def fixtures(self):
        return self.send({"cmd": "fixtures"})

    def get_fixture(self, fid: int, include: list = None):
        req = {"cmd": "get_fixture", "id": fid}
        if include:
            req["include"] = include
        return self.send(req)

    def set_fixture(self, fid: int, **kwargs):
        return self.send({"cmd": "set_fixture", "id": fid, **kwargs})

    def set_output_setting(self, fid: int, output: int, key: str, value):
        return self.set_fixture(fid, output=output, settings={key: value})

    def inputs(self):
        return self.send({"cmd": "inputs"})

    def get_wifi(self):
        return self.send({"cmd": "get_wifi"})

    def set_wifi(self, ssid: str, password: str = None):
        req = {"cmd": "set_wifi", "ssid": ssid, "confirm": True}
        if password is not None:
            req["password"] = password
        return self.send(req)

    def reboot(self):
        return self.send({"cmd": "reboot"}, timeout=2.0)

    def factory_reset(self):
        return self.send({"cmd": "factory_reset", "confirm": True}, timeout=2.0)

    def get_settings(self):
        return self.send({"cmd": "get_settings"})

    def set_settings(self, **kwargs):
        return self.send({"cmd": "set_settings", **kwargs})


def pretty(obj):
    if obj is None:
        print("No response")
    elif isinstance(obj, dict):
        print(json.dumps(obj, indent=2))
    else:
        print(obj)


def parse_id_value(args, cast=int):
    if len(args) == 1:
        return 0, cast(args[0])
    return int(args[0]), cast(args[1])


def main():
    p = argparse.ArgumentParser(description="ESP fixture controller")
    p.add_argument("--ip", default="192.168.1.75")

    p.add_argument("--describe", action="store_true")
    p.add_argument("--fixtures", action="store_true")
    p.add_argument("--inputs", action="store_true")
    p.add_argument("--reboot", action="store_true")
    p.add_argument("--factory-reset", action="store_true")

    p.add_argument("--get-fixture", nargs="*", metavar="ID")
    p.add_argument("--highlight", type=int, metavar="ID")
    p.add_argument("--set-universe", nargs="+", metavar="V")
    p.add_argument("--set-address", nargs="+", metavar="V")
    p.add_argument("--set-preset", nargs="+", metavar="V")
    p.add_argument("--set-name", nargs="+", metavar="V")
    p.add_argument("--outputs", type=int, metavar="ID", help="show a fixture's outputs and their settings")
    p.add_argument("--set-setting", nargs=4, metavar=("ID", "OUTPUT", "KEY", "VALUE"),
                   help="VALUE is parsed as JSON (true, 120, \"smooth\")")

    p.add_argument("--wifi", nargs="*", metavar=("SSID", "PASS"))
    p.add_argument("--settings", action="store_true")
    p.add_argument("--serial-report", metavar="BOOL")
    p.add_argument("--wireless-report", metavar="BOOL")
    p.add_argument("--wifi-animation", metavar="BOOL")

    args = p.parse_args()
    c = ESPClient(args.ip)

    if args.describe:
        pretty(c.describe())

    if args.fixtures:
        pretty(c.fixtures())

    if args.inputs:
        pretty(c.inputs())

    if args.reboot:
        pretty(c.reboot())

    if args.factory_reset:
        pretty(c.factory_reset())

    if args.get_fixture is not None:
        fid = int(args.get_fixture[0]) if args.get_fixture else 0
        pretty(c.get_fixture(fid))

    if args.highlight is not None:
        pretty(c.set_fixture(args.highlight, highlight=True))

    if args.set_universe:
        fid, val = parse_id_value(args.set_universe)
        pretty(c.set_fixture(fid, universe=val))

    if args.set_address:
        fid, val = parse_id_value(args.set_address)
        pretty(c.set_fixture(fid, address=val))

    if args.set_preset:
        fid, val = parse_id_value(args.set_preset)
        pretty(c.set_fixture(fid, preset_index=val))

    if args.set_name:
        fid, val = parse_id_value(args.set_name, cast=str)
        pretty(c.set_fixture(fid, name=val))

    if args.outputs is not None:
        pretty(c.get_fixture(args.outputs, include=["outputs"]))

    if args.set_setting:
        fid, output, key, value = args.set_setting
        pretty(c.set_output_setting(int(fid), int(output), key, json.loads(value)))

    if args.wifi is not None:
        if len(args.wifi) >= 1:
            pretty(c.set_wifi(args.wifi[0], args.wifi[1] if len(args.wifi) > 1 else None))
        else:
            pretty(c.get_wifi())

    if args.settings:
        pretty(c.get_settings())

    switches = {
        "serial_report": args.serial_report,
        "wireless_report": args.wireless_report,
        "wifi_animation": args.wifi_animation,
    }
    switches = {k: v.lower() in ("true", "1") for k, v in switches.items() if v is not None}
    if switches:
        pretty(c.set_settings(**switches))


if __name__ == "__main__":
    main()
