import socket
import json
import sys
import threading
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
                        return "ERROR", response
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
    
    
    def get_fixture(self) -> None:
        return self.send_request({"request": "getfixture", "id": 0})

    def set_universe(self, universe: int) -> None:
        return self.send_request({"request": "setfixture", "id": 0, "universe": universe})

    def set_address(self, address: int) -> None:
        return self.send_request({"request": "setfixture", "id": 0, "address": address})

    def set_preset(self, preset: int) -> None:
        return self.send_request({"request": "setfixture", "id": 0, "presetIndex": preset})

    def set_name(self, name: str) -> None:
        return self.send_request({"request": "setfixture", "id": 0, "name": name})

    def highlight(self) -> None:
        return self.send_request({"request": "setfixture", "id": 0, "highlight": True})


# ---------------- Example Usage ----------------
if __name__ == "__main__":
    ip = "192.168.0.8"
    if len(sys.argv) > 1:
        ip = sys.argv[1]

    client = ESPClient(ip)
    status, response = client.fixtures()

    print("Status:", status, response)
