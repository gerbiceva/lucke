import socket

# Replace with the IP address and port of your ESP32
ESP_IP = "192.168.0.105"  # Replace with the IP address of your ESP32
ESP_PORT = 8888           # Replace with the port number used in the ESP32 code

def send_data_to_esp(data):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((ESP_IP, ESP_PORT))
        print("Dela")
        sock.sendall(data.encode())
        sock.close()
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    # Send the data
    send_data_to_esp("8,0,0\n")