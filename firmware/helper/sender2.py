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

    # ---- commands ----
    def describe(self):
        return self.send({"request": "describe"})

    def fixtures(self):
        return self.send({"request": "fixtures"})

    def get_fixture(self, fid: int, field: str = None):
        req = {"request": "getfixture", "id": fid}
        if field:
            req["value"] = field
        return self.send(req)

    def set_fixture(self, fid: int, **kwargs):
        return self.send({"request": "setfixture", "id": fid, **kwargs})

    def inputs(self):
        return self.send({"request": "inputs"})

    def wifi(self, ssid: str = None, password: str = None):
        req = {"request": "wifi"}
        if ssid:
            req["ssid"] = ssid
        if password:
            req["password"] = password
        return self.send(req)

    def reboot(self):
        return self.send({"request": "reboot"}, timeout=2.0)

    def factory_reset(self):
        return self.send({"request": "factory_reset"}, timeout=2.0)

    def serial_report(self, value: bool = None):
        req = {"request": "serial_report_task"}
        if value is not None:
            req["value"] = value
        return self.send(req)

    def wireless_report(self, value: bool = None):
        req = {"request": "wireless_report_task"}
        if value is not None:
            req["value"] = value
        return self.send(req)

    def wifi_animation(self, value: bool = None):
        req = {"request": "wifi_animation"}
        if value is not None:
            req["value"] = value
        return self.send(req)


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

    p.add_argument("--wifi", nargs="*", metavar=("SSID", "PASS"))
    p.add_argument("--serial-report", nargs="?", const="query", metavar="BOOL")
    p.add_argument("--wireless-report", nargs="?", const="query", metavar="BOOL")
    p.add_argument("--wifi-animation", nargs="?", const="query", metavar="BOOL")

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
        pretty(c.set_fixture(fid, presetIndex=val))

    if args.set_name:
        fid, val = parse_id_value(args.set_name, cast=str)
        pretty(c.set_fixture(fid, name=val))

    if args.wifi is not None:
        if len(args.wifi) >= 2:
            pretty(c.wifi(args.wifi[0], args.wifi[1]))
        else:
            pretty(c.wifi())

    if args.serial_report is not None:
        val = None if args.serial_report == "query" else args.serial_report.lower() in ("true", "1")
        pretty(c.serial_report(val))

    if args.wireless_report is not None:
        val = None if args.wireless_report == "query" else args.wireless_report.lower() in ("true", "1")
        pretty(c.wireless_report(val))

    if args.wifi_animation is not None:
        val = None if args.wifi_animation == "query" else args.wifi_animation.lower() in ("true", "1")
        pretty(c.wifi_animation(val))


if __name__ == "__main__":
    main()
