#!/bin/bash

# Clean up any existing processes
killall charon 2>/dev/null || true
rm -f /var/run/charon.pid 2>/dev/null || true

# Copy configuration files to proper location
cp /etc/ipsec.d/ipsec.conf /etc/ipsec.conf 2>/dev/null || true
cp /etc/ipsec.d/ipsec.secrets /etc/ipsec.secrets 2>/dev/null || true

# Set proper permissions
chmod 600 /etc/ipsec.secrets 2>/dev/null || true

# Create necessary directories
mkdir -p /var/run
mkdir -p /etc/swanctl/conf.d

# Start charon daemon with VICI support
echo "Starting charon with VICI..."
/usr/local/libexec/ipsec/charon &
CHARON_PID=$!

# Wait for charon to start and VICI socket to be available
echo "Waiting for VICI socket..."
for i in {1..30}; do
    if [ -S /var/run/charon.vici ]; then
        echo "VICI socket available at /var/run/charon.vici"
        break
    fi
    sleep 1
done

if [ ! -S /var/run/charon.vici ]; then
    echo "Error: VICI socket not available after 30 seconds"
    exit 1
fi

# Compile VICI client
if [ -f /app/vici_src/vici_client.c ]; then
    cd /app/vici_src
    make clean && make
    if [ $? -eq 0 ]; then
        echo "C VICI client compiled successfully"
    else
        echo "Warning: Failed to compile VICI client"
    fi
fi

echo "StrongSwan started on $(hostname)"
echo "Node: $NODE_NAME, Peer: $PEER_IP"

# Wait a bit more for everything to settle
sleep 2

# Run VICI client to establish connection
if [ -f /app/vici_src/vici_client ]; then
    echo "Running VICI client to establish IPsec connection..."
    timeout 30 /app/vici_src/vici_client
    if [ $? -eq 124 ]; then
        echo "VICI client timed out after 30 seconds"
    fi
fi

# Keep container running
tail -f /dev/null