# VICI Connection Test Results

## Summary

The VICI (Versatile IKE Control Interface) connection functionality has been tested with the following results:

### Working Connection Methods:

1. **Default Connection (`vici_connect(NULL)`)**
   - ✅ **WORKS** - This is the recommended method
   - Uses the default socket path determined by the VICI library
   - Most reliable and portable approach

2. **Unix Socket URI (`vici_connect("unix:///run/charon.vici")`)**
   - ✅ **WORKS** - Explicit Unix socket URI format
   - Requires the full URI format with `unix://` prefix
   - Good for explicit socket path specification

### Non-Working Connection Methods:

1. **Direct Socket Path (`vici_connect("/run/charon.vici")`)**
   - ❌ **FAILS** - Direct path without URI scheme doesn't work
   - Returns "No such file or directory" even though socket exists
   - The VICI library expects URI format, not raw paths

2. **PID-based Connection (`vici_connect("unix://PID")`)**
   - ❌ **FAILS** - PID-based connection not supported in this build
   - Returns "No such file or directory"
   - May require different strongSwan configuration or build options

### Socket Information:

- **Socket Location**: `/run/charon.vici` (also accessible via `/var/run/charon.vici` symlink)
- **Socket Permissions**: 770 (owner: root, group: root)
- **Charon PID**: Available in `/run/charon.pid`
- **strongSwan Version**: 6.0.3

### Recommendations:

1. **Use `vici_connect(NULL)`** for most applications - it's the most reliable
2. **Use `vici_connect("unix:///run/charon.vici")`** if you need explicit socket specification
3. **Avoid direct socket paths** - they don't work with the VICI library
4. **PID-based connections** are not functional in this environment

### Test Environment:

- Container runtime: Podman
- strongSwan built from source with VICI plugin
- Ubuntu-based containers with custom strongSwan build
- Network: 172.20.0.0/24 with nodes at .10 and .20

The existing VICI clients in the project work correctly and successfully establish IPsec tunnels using the default connection method.
