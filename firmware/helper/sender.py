import socket
import json
import sys
from contextlib import closing

# def listen_responses(port: int = 12345):
#     sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#     sock.bind(("0.0.0.0", port))
#     print(f"Listening for responses on UDP port {port}...")

#     while True:
#         data, addr = sock.recvfrom(2048)
#         try:
#             message = json.loads(data.decode("utf-8"))
#             pretty = json.dumps(message, indent=2)
#         except json.JSONDecodeError:
#             pretty = data.decode("utf-8", errors="replace")

#         print(f"\nResponse from {addr}:")
#         print(pretty)

class ESPClient:
    def __init__(self, ip: str):
        self.ip = ip
        self.port = 8888

    def send_request(self, payload: dict) -> None:
        data = json.dumps(payload)

        try:
            with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
                sock.connect((self.ip, self.port))
                sock.sendall(data.encode("utf-8"))

            print("Request sent successfully")

        except OSError as e:
            print(f"Socket error: {e}")

    def reboot(self) -> None:
        request = {
            "request": "reboot",
        }
        self.send_request(request)

    def set_wifi(self, ssid: str, password: str) -> None:
        request = {
            "request": "wifi",
            "ssid": ssid,
            "password": password
        }
        self.send_request(request)

    def set_universe(self, universe: int) -> None:
        request = {
            "request": "setfixture",
            "id": 0,
            "universe": universe
        }
        self.send_request(request)

    def set_address(self, address: int) -> None:
        request = {
            "request": "setfixture",
            "id": 0,
            "address": address
        }
        self.send_request(request)

    def set_preset(self, preset: int) -> None:
        request = {
            "request": "setfixture",
            "id": 0,
            "presetIndex": preset
        }
        self.send_request(request)

    def set_name(self, name: str) -> None:
        request = {
            "request": "setfixture",
            "id": 0,
            "name": name
        }
        self.send_request(request)
    
    def highlight(self) -> None:
        request = {
            "request": "setfixture",
            "id": 0,
            "highligth": True
        }
        self.send_request(request)


if __name__ == "__main__":
    ip = "192.168.1.10"
    if len(sys.argv) == 2:
        ip = sys.argv[1]

    sender = ESPClient(ip)
