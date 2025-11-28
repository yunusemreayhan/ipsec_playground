# IPsec Playground with VICI Support

A containerized environment for experimenting with strongSwan IPsec connections and VICI (Versatile IKE Control Interface) API.

## Features

- **strongSwan built from source** with VICI plugin enabled
- **VICI headers available** at `/usr/local/include/libvici.h` 
- **C and Python VICI clients** for programmatic IPsec control
- **Two strongSwan nodes** with pre-configured IPsec tunnels
- **Isolated network environment** for safe testing

## Quick Start

### Test with Configuration Files
```bash
python3 test.py --start-using-config-files
```

### Test with C VICI Interface
```bash
python3 test.py --start-using-vici
```

### Test VICI functionality directly
```bash
./quick_test.sh
```

### Test Node-Specific VICI Clients
```bash
# Start containers
podman-compose up -d

# Start node2 (responder) in background
podman exec -d ipsec_node2 bash -c "LD_LIBRARY_PATH=/usr/local/lib/ipsec /app/vici_src/vici_client_node2"

# Start node1 (initiator)
podman exec ipsec_node1 bash -c "LD_LIBRARY_PATH=/usr/local/lib/ipsec /app/vici_src/vici_client_node1"

# Cleanup
podman-compose down
```

## VICI Development

The `vici_src/` directory contains specialized VICI clients for programmatic IPsec control:
- `vici_client_node1.c` - Initiator client (172.20.0.10 → 172.20.0.20)
- `vici_client_node2.c` - Responder client (172.20.0.20 → 172.20.0.10)
- `build.sh` - Build script for VICI clients

### VICI API Features
- **Connection management** - Load IPsec configurations via `load-conn`
- **PSK management** - Load pre-shared keys via `load-shared`
- **Tunnel initiation** - Start connections via `initiate`
- **Event monitoring** - Subscribe to IKE/Child SA events
- **Real-time control** - No service restarts required

### Building VICI Clients

Inside the container:
```bash
cd /app/vici_src
./build.sh
```

### Running VICI Clients

Node1 (initiator):
```bash
LD_LIBRARY_PATH=/usr/local/lib/ipsec ./vici_client_node1
```

Node2 (responder):
```bash
LD_LIBRARY_PATH=/usr/local/lib/ipsec ./vici_client_node2
```

## Structure

- `docker-compose.yml` - Container orchestration
- `Dockerfile` - StrongSwan container with VICI support
- `node1/`, `node2/` - IPsec configuration files
- `vici_src/` - C VICI client source code and build tools
- `test.py` - Test automation script
- `quick_test.sh` - Quick development test cycle

## Container Details

- **Base**: Ubuntu with strongSwan compiled from source
- **VICI Plugin**: Enabled with development headers
- **Network**: 172.20.0.0/24 with nodes at .10 and .20
- **strongSwan Version**: Latest from Git with VICI support

## Manual Testing

```bash
# Start containers
podman-compose up -d

# Check tunnel status
podman exec ipsec_node1 ipsec status
podman exec ipsec_node2 ipsec status

# Test connectivity
podman exec ipsec_node1 ping 172.20.0.20
podman exec ipsec_node2 ping 172.20.0.10

# Test VICI clients
./quick_test.sh

# Cleanup
podman-compose down
```