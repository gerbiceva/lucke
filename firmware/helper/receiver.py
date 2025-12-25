import socket
import json

def packet_ingest():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", 12345))

    print("Listening on UDP port 12345")

    while True:
        try:
            data, addr = sock.recvfrom(2048)
            print(data)
        except OSError as e:
            print("ERR reading", e)
            continue

if __name__ == "__main__":
    packet_ingest()
        # try:
        #     res = json.loads(data.decode("utf-8"))
        # except json.JSONDecodeError as e:
        #     print("err parsing", e)
        #     continue

        # # key = res["Universe"]
        # key = res.get("LocalIP")

        # if key is None:
        #     continue

        # val = stats.get(key)