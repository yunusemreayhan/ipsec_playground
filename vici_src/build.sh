#!/bin/bash

echo "Building VICI client with strongSwan headers..."

# Check if we're in container
if [ -f "/usr/local/include/libvici.h" ]; then
    echo "Using strongSwan built from source"
    INCLUDE_PATH="/usr/local/include"
    LIB_PATH="/usr/local/lib"
else
    echo "Using system strongSwan (if available)"
    INCLUDE_PATH="/usr/include"
    LIB_PATH="/usr/lib"
fi

# Build both node-specific clients
gcc -Wall -Wextra -std=c99 \
    -I${INCLUDE_PATH} \
    -L${LIB_PATH}/ipsec \
    -o vici_client_node1 vici_client_node1.c \
    -lvici -lstrongswan

gcc -Wall -Wextra -std=c99 \
    -I${INCLUDE_PATH} \
    -L${LIB_PATH}/ipsec \
    -o vici_client_node2 vici_client_node2.c \
    -lvici -lstrongswan

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Node1 client: ./vici_client_node1"
    echo "Node2 client: ./vici_client_node2"
else
    echo "Build failed - check if libvici headers are available"
    exit 1
fi
