import socket








if __name__ == "__main__":
    ESP_IP = "10.42.0.220"
    # ESP_IP = "10.26.124.119"
    ESP_PORT = 8888
    data = "{ \"request\": \"fixtures\"}"
    # data = "{ \"request\": \"setfixture\", \"id\": 0, \"name\": \"pojstla\" }"
    # data = "{ \"request\": \"setfixture\", \"id\": 0, \"presetIndex\": 0 }"
    # data = "{ \"request\": \"setfixture\", \"id\": 0, \"presetIndex\": 0, \"name\": \"postelja\"}"

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((ESP_IP, ESP_PORT))
        sock.sendall(data.encode())
        sock.close()
        print("Delal je")
    except Exception as e:
        print(f"Error: {e}")