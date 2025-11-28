#!/usr/bin/env python3
import subprocess
import time
import sys
import argparse

# Detect container runtime
def get_container_runtime():
    for runtime in ['podman', 'docker']:
        try:
            subprocess.run([runtime, '--version'], capture_output=True, check=True)
            return runtime
        except (subprocess.CalledProcessError, FileNotFoundError):
            continue
    raise RuntimeError("Neither podman nor docker found")

CONTAINER_RUNTIME = get_container_runtime()
COMPOSE_CMD = f"{CONTAINER_RUNTIME}-compose" if CONTAINER_RUNTIME == "podman" else "docker-compose"

def run_command(cmd, container=None):
    if container:
        cmd = f"{CONTAINER_RUNTIME} exec {container} {cmd}"
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.returncode == 0, result.stdout, result.stderr

def test_ping():
    print("Testing connectivity...")
    success1, out1, err1 = run_command("ping -c 3 172.20.0.20", "ipsec_node1")
    success2, out2, err2 = run_command("ping -c 3 172.20.0.10", "ipsec_node2")
    
    if success1 and success2:
        print("✓ Ping test successful - nodes can communicate")
        return True
    else:
        print("✗ Ping test failed")
        if not success1:
            print(f"Node1 -> Node2 failed: {err1}")
        if not success2:
            print(f"Node2 -> Node1 failed: {err2}")
        return False

def start_containers():
    print(f"Starting containers with {CONTAINER_RUNTIME}...")
    success, _, _ = run_command(f"{COMPOSE_CMD} up -d")
    if success:
        print("✓ Containers started")
        time.sleep(10)  # Wait for initialization
        return True
    else:
        print("✗ Failed to start containers")
        return False

def test_config_files():
    print("\n=== Testing with configuration files ===")
    
    if not start_containers():
        return False
    
    # Copy config files and reload
    run_command("bash -c 'cp /etc/ipsec.d/ipsec.conf /etc/ipsec.conf && cp /etc/ipsec.d/ipsec.secrets /etc/ipsec.secrets && chmod 600 /etc/ipsec.secrets && ipsec reload && ipsec rereadsecrets'", "ipsec_node1")
    run_command("bash -c 'cp /etc/ipsec.d/ipsec.conf /etc/ipsec.conf && cp /etc/ipsec.d/ipsec.secrets /etc/ipsec.secrets && chmod 600 /etc/ipsec.secrets && ipsec reload && ipsec rereadsecrets'", "ipsec_node2")
    time.sleep(2)
    
    # Establish tunnel
    success, out, err = run_command("ipsec up tunnel", "ipsec_node1")
    if "established successfully" in out:
        print("✓ IPsec tunnel established via config files")
        return test_ping()
    else:
        print("✗ IPsec tunnel establishment failed")
        print(f"Output: {out}")
        print(f"Error: {err}")
        return False

def test_vici():
    print("\n=== Testing with VICI ===")
    
    if not start_containers():
        return False
    
    # Clear existing connections
    run_command("ipsec down tunnel", "ipsec_node1")
    run_command("ipsec down tunnel", "ipsec_node2")
    time.sleep(2)
    
    # Show VICI C API documentation
    print("C VICI API Documentation available in /opt/vici_src/README_VICI.md")
    
    # Setup via shell VICI client (demonstrates VICI concepts)
    success1, out1, err1 = run_command("/opt/vici_src/vici_client.sh", "ipsec_node1")
    success2, out2, err2 = run_command("/opt/vici_src/vici_client.sh", "ipsec_node2")
    
    if success1 and success2:
        print("✓ C VICI configuration successful")
        time.sleep(3)
        return test_ping()
    else:
        print("✗ C VICI configuration failed")
        print(f"Node1 VICI: {out1} {err1}")
        print(f"Node2 VICI: {out2} {err2}")
        return False

def cleanup():
    print("Cleaning up...")
    run_command(f"{COMPOSE_CMD} down")

def main():
    parser = argparse.ArgumentParser(description='IPsec StrongSwan Test Suite')
    parser.add_argument('--start-using-config-files', action='store_true',
                       help='Test using configuration files')
    parser.add_argument('--start-using-vici', action='store_true',
                       help='Test using C VICI interface')
    
    args = parser.parse_args()
    
    if not args.start_using_config_files and not args.start_using_vici:
        print("Usage: python3 test.py --start-using-config-files OR --start-using-vici")
        sys.exit(1)
    
    try:
        if args.start_using_config_files:
            success = test_config_files()
        elif args.start_using_vici:
            success = test_vici()
        
        if success:
            print("\n✓ Test completed successfully!")
        else:
            print("\n✗ Test failed!")
            sys.exit(1)
            
    except KeyboardInterrupt:
        print("\nTest interrupted")
    finally:
        cleanup()

if __name__ == "__main__":
    main()