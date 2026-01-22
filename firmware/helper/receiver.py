import socket
import json
import argparse
import sys

def packet_ingest(simple: bool = True):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", 12344))

    print("Listening on UDP port 12344")

    while True:
        try:
            data, addr = sock.recvfrom(2048)

            # Decode bytes → string → JSON → pretty string
            message = json.loads(data.decode("utf-8"))
            pretty = ""

            if simple == True:
                try:
                    ip = message["engine"]["wifi"]["local_ip"]
                    fixtures = message["engine"]["fixture_handler"]["fixtures"]

                    parsed = [
                        {
                            "universe": f.get("universe"),
                            "address": f.get("address"),
                            "presetIndex": f.get("presetIndex"),
                            "name": f.get("name"),
                            "type": f.get("type"),
                        }
                        for f in fixtures
                    ]
                    if len(parsed) == 1:
                        parsed = parsed[0]

                    pretty = json.dumps(parsed, indent=2)

                except (KeyError, TypeError) as e:
                    print("ERR parsing expected fields:", e)
                    print("Raw JSON:", message)
            else:
                pretty = json.dumps(message["engine"], indent=2)

            print(f"\nFrom {addr}:")
            print(pretty)

        except json.JSONDecodeError as e:
            print("ERR decoding JSON:", e)
            print("Raw data:", data)

        except OSError as e:
            print("ERR reading:", e)
            continue


if __name__ == "__main__":
    simple = True

    if len(sys.argv) > 1:
        simple = sys.argv[1].lower() not in ["false", "0", "no", "f"]

    packet_ingest(simple)