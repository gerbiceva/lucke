import socket
import json
import sys
import threading
import argparse
from contextlib import closing
from queue import Queue, Empty

class ESPClient:
    def __init__(self, ip: str, await_response: bool = True):
        self.ip = ip
        self.tcp_port = 8888   # send request port
        self.udp_port = 12345  # receive response port
        self._listener_thread = None
        self._response_queue = Queue()
        self._listening = False
        self.await_response = await_response

        self.start_listener()

    # ---------------- UDP Listener ----------------
    def _listen_responses(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(("0.0.0.0", self.udp_port))
        print(f"Listening for responses on UDP port {self.udp_port}...")
        self._listening = True

        while self._listening:
            try:
                data, addr = sock.recvfrom(2048)
                try:
                    message = json.loads(data.decode("utf-8"))
                except json.JSONDecodeError:
                    message = data.decode("utf-8", errors="replace")

                # Put every incoming response in the queue
                self._response_queue.put(message)

            except OSError as e:
                print(f"UDP socket error: {e}")
                break

    def start_listener(self):
        """Start UDP listener in background thread."""
        if not self._listener_thread:
            self._listener_thread = threading.Thread(target=self._listen_responses, daemon=True)
            self._listener_thread.start()

    def stop_listener(self):
        self._listening = False

    # ---------------- TCP Requests ----------------
    def send_request(self, payload: dict, timeout: float = 5.0):
        """Send TCP request. Optionally wait for a UDP response."""
        data = json.dumps(payload)

        try:
            with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
                sock.connect((self.ip, self.tcp_port))
                sock.sendall(data.encode("utf-8"))
            print(f"Sent request: {payload}")

            if self.await_response:
                try:
                    response = self._response_queue.get(timeout=timeout)
                    if isinstance(response, dict) and "response" in response:
                        try:
                            inner = json.loads(response["response"])
                            return response["status"], json.dumps(inner, indent=2)
                        except json.JSONDecodeError:
                            return response["status"], response["response"]
                    else:
                        return "OK", response
                except Empty:
                    print(f"No response received within {timeout} seconds")
                    return None

        except OSError as e:
            print(f"Socket error: {e}")
            return None

    # ---------------- Fixture Commands ----------------
    def describe(self) -> None:
        return self.send_request({"request": "describe"})
    
    def fixtures(self) -> None:
        return self.send_request({"request": "fixtures"})

    def inputs(self) -> None:
        return self.send_request({"request": "inputs"})

    def reboot(self) -> None:
        return self.send_request({"request": "reboot"})

    def factory_reset(self) -> None:
        return self.send_request({"request": "factory_reset"})


    def enable_wifi_animation(self, value: bool) -> None:
        return self.send_request({"request": "wifi_animation", "value": value})

    def enable_serial_print(self, value: bool) -> None:
        return self.send_request({"request": "print_task", "value": value})

    def enable_wireless_logging(self, value: bool) -> None:
        return self.send_request({"request": "auto_report_task", "value": value})
        

    def set_wifi(self, ssid: str, password: str) -> None:
        return self.send_request({"request": "wifi", "ssid": ssid, "password": password})
    
    
    def get_fixture(self, id:int = 0) -> None:
        return self.send_request({"request": "getfixture", "id": id})

    def set_universe(self, universe: int, id:int = 0) -> None:
        return self.send_request({"request": "setfixture", "id": id, "universe": universe})

    def set_address(self, address: int, id:int = 0) -> None:
        return self.send_request({"request": "setfixture", "id": id, "address": address})

    def set_preset(self, preset: int, id:int = 0) -> None:
        return self.send_request({"request": "setfixture", "id": id, "presetIndex": preset})

    def set_name(self, name: str, id:int = 0) -> None:
        return self.send_request({"request": "setfixture", "id": id, "name": name})

    def highlight(self, id:int = 0) -> None:
        return self.send_request({"request": "setfixture", "id": id, "highlight": True})

def initArguments():
    parser = argparse.ArgumentParser(description="ESPClient command-line tool")

    parser.add_argument("--ip", type=str, default="192.168.1.75", help="IP address of the ESP device")

    # Boolean flags
    parser.add_argument("--describe", action="store_true", help="Request engine description")
    parser.add_argument("--fixtures", "--fixs", action="store_true", help="Request all fixtures")
    parser.add_argument("--inputs", action="store_true", help="Request all inputs")
    parser.add_argument("--reboot", action="store_true", help="Reboot system")
    parser.add_argument("--factory-reset", action="store_true", help="Do a factory reset")

    parser.add_argument("--wifi-animation", action="store_true", help="Enable WiFi animation task")
    parser.add_argument("--serial-print", action="store_true", help="Enable serial log task")
    parser.add_argument("--wireless-print", action="store_true", help="Enable wireless log task")

    # WiFi credentials
    parser.add_argument("--wifi-conf", nargs=2, metavar=("SSID", "PASSWORD"), help="Set WiFi credentials (SSID and PASSWORD)")

    # Fixture commands
    parser.add_argument("--get-fixture", nargs="*", metavar="ID", help="Get fixture by ID")
    parser.add_argument("--highlight", nargs="*", metavar="ID", help="Highlight fixture by ID")

    parser.add_argument(
        "--set-universe", nargs="*", metavar=("ID", "UNIVERSE"),
        help="Set fixture universe. If ID is omitted, defaults to 0"
    )
    parser.add_argument(
        "--set-address", nargs="*", metavar=("ID", "ADDRESS"),
        help="Set fixture address. If ID is omitted, defaults to 0"
    )
    parser.add_argument(
        "--set-preset", nargs="*", metavar=("ID", "PRESET"),
        help="Set fixture preset. If ID is omitted, defaults to 0"
    )
    parser.add_argument(
        "--set-name", nargs="*", metavar=("ID", "NAME"),
        help="Set fixture name. If ID is omitted, defaults to 0"
    )

    return parser.parse_args()


# ---------------- Example Usage ----------------
if __name__ == "__main__":
    
    args = initArguments()
    client = ESPClient(args.ip)

    # ---------------- Execute commands ----------------
    def run_command(func, *fargs):
        result = func(*fargs)
        if result is not None:
            if isinstance(result, tuple):
                status, response = result
                print("Status:", status)
                print("Response:", response)
            else:
                print("Result:", result)

    if args.describe:
        run_command(client.describe)

    if args.fixtures:
        run_command(client.fixtures)

    if args.inputs:
        run_command(client.inputs)

    if args.reboot:
        run_command(client.reboot)

    if args.factory_reset:
        run_command(client.factory_reset)

    if args.wifi_animation:
        run_command(client.enable_wifi_animation, True)

    if args.serial_print:
        run_command(client.enable_serial_print, True)

    if args.wireless_print:
        run_command(client.enable_wireless_logging, True)

    if args.wifi_conf:
        ssid, password = args.wifi_conf
        run_command(client.set_wifi, ssid, password)

    # if args.get_fixture is not None:
        # run_command(client.get_fixture, args.get_fixture)

    if args.get_fixture:
        if len(args.get_fixture) == 0:
            fid = 0
        else:
            fid = int(args.get_fixture[0])
        run_command(client.get_fixture, fid)

    # if args.highlight:
    #     if len(args.highlight) == 0:
    #         fid = 0
    #     else:
    #         fid = int(args.highlight[0])
    #     run_command(client.highlight, fid)

    if args.highlight is not None:
        run_command(client.highlight, args.highlight)

    if args.set_universe:
        if len(args.set_universe) == 1:
            fid = 0
            universe = int(args.set_universe[0])
        else:
            fid, universe = map(int, args.set_universe)
        run_command(client.set_universe, universe, fid)

    if args.set_address:
        if len(args.set_address) == 1:
            fid = 0
            address = int(args.set_address[0])
        else:
            fid, address = map(int, args.set_address)
        run_command(client.set_address, address, fid)

    if args.set_preset:
        if len(args.set_preset) == 1:
            fid = 0
            preset = int(args.set_preset[0])
        else:
            fid, preset = map(int, args.set_preset)
        run_command(client.set_preset, preset, fid)

    if args.set_name:
        if len(args.set_name) == 1:
            fid = 0
            name = args.set_name[0]
        else:
            fid, name = args.set_name
            fid = int(fid)
        run_command(client.set_name, name, fid)
        
        {"request": "reboot"}
        {"request": "describe"}
        {"request": "auto_report_task", "value": false}
        {"request": "auto_report_task", "value": true}