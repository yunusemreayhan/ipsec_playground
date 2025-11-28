#!/bin/bash

echo "=== Quick Test Cycle ==="

# Restart containers (keeps existing image)
# podman-compose down
# podman-compose up -d

# Wait for startup
# sleep 10

# Compile VICI clients
echo "Compiling VICI clients..."
podman exec ipsec_node1 bash -c "cd /app/vici_src && ./build.sh"

# Start node2 (responder) in background
echo "Starting node2 VICI client (responder)..."
podman exec -d ipsec_node2 bash -c "LD_LIBRARY_PATH=/usr/local/lib/ipsec /app/vici_src/vici_client_node2"

# Run node1 (initiator)
echo "Starting node1 VICI client (initiator)..."
podman exec ipsec_node1 bash -c "LD_LIBRARY_PATH=/usr/local/lib/ipsec timeout 15 /app/vici_src/vici_client_node1"

# Show logs
echo -e "\n=== Container Logs ==="
podman logs ipsec_node1