import socket








if __name__ == "__main__":
    ESP_IP = "192.168.0.106"
    ESP_PORT = 8888
    data = "8,0,0\n"

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((ESP_IP, ESP_PORT))
        sock.sendall(data.encode())
        sock.close()
        print("Delal je")
    except Exception as e:
        print(f"Error: {e}")