#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libvici.h>

int get_charon_pid() {
    FILE *fp = fopen("/run/charon.pid", "r");
    if (!fp) {
        printf("Cannot open /run/charon.pid: %s\n", strerror(errno));
        return -1;
    }
    
    int pid;
    if (fscanf(fp, "%d", &pid) != 1) {
        printf("Cannot read PID from /run/charon.pid\n");
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    printf("Found charon PID: %d\n", pid);
    return pid;
}

int test_pid_connection() {
    int pid = get_charon_pid();
    if (pid <= 0) {
        return -1;
    }
    
    // Create URI string for PID-based connection
    char uri[64];
    snprintf(uri, sizeof(uri), "unix://%d", pid);
    
    printf("Testing PID-based connection with URI: %s\n", uri);
    vici_conn_t *conn = vici_connect(uri);
    if (conn) {
        printf("✓ PID-based connection successful\n");
        
        // Test functionality
        vici_req_t *req = vici_begin("version");
        vici_res_t *res = vici_submit(req, conn);
        if (res) {
            char *version = vici_find_str(res, "", "version");
            printf("  strongSwan version: %s\n", version ? version : "unknown");
            vici_free_res(res);
        }
        
        vici_disconnect(conn);
        return 0;
    } else {
        printf("✗ PID-based connection failed: %s\n", strerror(errno));
        return -1;
    }
}

int test_unix_socket_uri() {
    printf("Testing Unix socket URI: unix:///run/charon.vici\n");
    vici_conn_t *conn = vici_connect("unix:///run/charon.vici");
    if (conn) {
        printf("✓ Unix socket URI connection successful\n");
        vici_disconnect(conn);
        return 0;
    } else {
        printf("✗ Unix socket URI connection failed: %s\n", strerror(errno));
        return -1;
    }
}

int main() {
    printf("=== VICI Connection Methods Test ===\n");
    
    vici_init();
    
    // Test 1: Default connection
    printf("\n1. Testing default connection (NULL)...\n");
    vici_conn_t *conn = vici_connect(NULL);
    if (conn) {
        printf("✓ Default connection works\n");
        vici_disconnect(conn);
    } else {
        printf("✗ Default connection failed: %s\n", strerror(errno));
    }
    
    // Test 2: PID-based connection
    printf("\n2. Testing PID-based connection...\n");
    test_pid_connection();
    
    // Test 3: Unix socket URI
    printf("\n3. Testing Unix socket URI...\n");
    test_unix_socket_uri();
    
    // Test 4: Direct socket path (we know this fails)
    printf("\n4. Testing direct socket path...\n");
    conn = vici_connect("/run/charon.vici");
    if (conn) {
        printf("✓ Direct socket path works\n");
        vici_disconnect(conn);
    } else {
        printf("✗ Direct socket path failed: %s\n", strerror(errno));
    }
    
    vici_deinit();
    return 0;
}
