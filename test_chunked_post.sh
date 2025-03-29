#!/bin/bash
# test_chunked_post.sh - Test large chunked POST requests
# Usage: ./test_chunked_post.sh [size_in_kb]

SIZE_KB=${1:-1024}  # Default to 1MB (1024KB)
PORT=5050
URL="http://localhost:$PORT/post_test.php"
SIZE_BYTES=$((SIZE_KB * 1024))

echo "Generating $SIZE_KB KB of test data..."
# Generate data to a file instead
dd if=/dev/urandom bs=1K count=$SIZE_KB 2>/dev/null | base64 > test_data.txt

# Create a Python script to send chunked request
cat > send_chunked.py << 'EOF'
import socket
import time
import sys

def send_chunked_request(host, port, path, data_file):
    # Load data from file
    with open(data_file, 'r') as f:
        data = "data=" + f.read()
    
    # Create socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    
    # Build request headers
    request = f"POST {path} HTTP/1.1\r\n"
    request += f"Host: {host}:{port}\r\n"
    request += "Transfer-Encoding: chunked\r\n"
    request += "Content-Type: application/x-www-form-urlencoded\r\n"
    request += "\r\n"
    
    # Send headers
    s.sendall(request.encode())
    
    # Send data in chunks of approximately 8KB
    chunk_size = 8192
    sent = 0
    total_size = len(data)
    
    while sent < total_size:
        # Calculate chunk size
        current_chunk_size = min(chunk_size, total_size - sent)
        chunk = data[sent:sent+current_chunk_size]
        
        # Send chunk size in hex followed by CRLF
        s.sendall(f"{current_chunk_size:x}\r\n".encode())
        
        # Send chunk data followed by CRLF
        s.sendall(chunk.encode())
        s.sendall(b"\r\n")
        
        sent += current_chunk_size
        sys.stdout.write(f"\rSent: {sent}/{total_size} bytes ({sent/total_size*100:.1f}%)")
        sys.stdout.flush()
        
        # Small delay between chunks
        time.sleep(0.01)
    
    # End with zero-length chunk
    s.sendall(b"0\r\n\r\n")
    print("\nRequest fully sent, waiting for response...")
    
    # Read response
    response = b""
    while True:
        data = s.recv(4096)
        if not data:
            break
        response += data
    
    s.close()
    return response.decode('utf-8', errors='ignore')

# Load host and port from command line
if len(sys.argv) < 3:
    print("Usage: python3 send_chunked.py <host> <port>")
    sys.exit(1)

host = sys.argv[1]
port = int(sys.argv[2])
data_file = sys.argv[3]

# Send request
response = send_chunked_request(host, port, "/post_test.php", data_file)

# Print status line and some of the response
status_line = response.split('\r\n')[0] if '\r\n' in response else response.split('\n')[0]
print(f"Response status: {status_line}")
print("Response excerpt:")
body_parts = response.split('\r\n\r\n', 1)
if len(body_parts) > 1:
    print('\n'.join(body_parts[1].split('\n')[:20]))
else:
    print("No response body received")
EOF

echo "Sending chunked POST request of $SIZE_KB KB to $URL..."
python3 send_chunked.py localhost $PORT test_data.txt

echo "Test complete!"
rm -f test_data.txt