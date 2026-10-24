import socket
import sys

def packet_ingest():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", 12343))

    print("Listening on UDP port 12343 for heartbeat")
    unique_ips = set()
    # ip_counts = {}

    while True:
        try:
            data, addr = sock.recvfrom(512)
            ip, port = addr

            if ip not in unique_ips:
                unique_ips.add(ip)
                # ip_counts[ip] = 1
                print(f"{ip}: {data}")
            # else:
            #     ip_counts[ip] += 1

            # print(f"{ip} has pinged {ip_counts[ip]} times")
        except OSError as e:
            print("ERR reading:", e)
            continue


if __name__ == "__main__":
    packet_ingest()