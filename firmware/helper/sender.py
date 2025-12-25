# import socket








# if __name__ == "__main__":
#     ESP_IP = "10.26.124.138"
#     # ESP_IP = "10.26.124.119"
#     ESP_PORT = 8888
#     # data = "{ \"request\": \"reboot\"}"
#     # data = "{ \"request\": \"wifi_animation\", \"value\": false}"
#     # data = "{ \"request\": \"highlight\", \"id\": 0}"
#     # data = "{ \"request\": \"getfixture\", \"id\": 0}"
#     # data = "{ \"request\": \"setfixture\", \"id\": 0, \"name\": \"omara\" }"
#     # data = "{ \"request\": \"setfixture\", \"id\": 0, \"presetIndex\": 0 }"
#     data = "{ \"request\": \"setfixture\", \"id\": 0, \"address\": 180 }"
#     # data = "{ \"request\": \"setfixture\", \"id\": 0, \"presetIndex\": 0, \"name\": \"postelja\"}"

#     try:
#         sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#         sock.connect((ESP_IP, ESP_PORT))
#         sock.sendall(data.encode())
#         sock.close()
#         print("Delal je")
#     except Exception as e:
#         print(f"Error: {e}")

import socket
import json
from contextlib import closing


ESP_IP = "10.26.124.119"
# ESP_IP = "10.26.124.138"
# ESP_IP = "10.42.0.148"
ESP_PORT = 8888


def send_request(payload: dict) -> None:
    data = json.dumps(payload)

    try:
        with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
            sock.connect((ESP_IP, ESP_PORT))
            sock.sendall(data.encode("utf-8"))

        print("Request sent successfully")

    except OSError as e:
        print(f"Socket error: {e}")


if __name__ == "__main__":
    request = {
        "request": "wifi_animation",
        # "request": "reboot",
        "value": False
        # "id": 0,
        # "address": 180,
    }

    send_request(request)