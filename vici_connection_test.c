#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libvici.h>

int test_vici_connect_default() {
    printf("Testing vici_connect(NULL) - default socket path...\n");
    vici_conn_t *conn = vici_connect(NULL);
    if (conn) {
        printf("✓ Default connection successful\n");
        vici_disconnect(conn);
        return 0;
    } else {
        printf("✗ Default connection failed: %s\n", strerror(errno));
        return -1;
    }
}

int test_vici_connect_socket_path() {
    printf("Testing vici_connect(\"/run/charon.vici\") - explicit socket path...\n");
    vici_conn_t *conn = vici_connect("/run/charon.vici");
    if (conn) {
        printf("✓ Socket path connection successful\n");
        vici_disconnect(conn);
        return 0;
    } else {
        printf("✗ Socket path connection failed: %s\n", strerror(errno));
        return -1;
    }
}

int test_version_query(vici_conn_t *conn) {
    printf("Testing version query...\n");
    vici_req_t *req = vici_begin("version");
    vici_res_t *res = vici_submit(req, conn);
    if (res) {
        printf("✓ Version query successful\n");
        char *daemon = vici_find_str(res, "", "daemon");
        char *version = vici_find_str(res, "", "version");
        if (daemon && version) {
            printf("  Daemon: %s, Version: %s\n", daemon, version);
        }
        vici_free_res(res);
        return 0;
    } else {
        printf("✗ Version query failed\n");
        return -1;
    }
}

int main() {
    printf("=== VICI Connection Test ===\n");
    
    vici_init();
    
    // Test 1: Default connection (NULL parameter)
    int result1 = test_vici_connect_default();
    
    // Test 2: Explicit socket path
    int result2 = test_vici_connect_socket_path();
    
    // Test 3: Functional test with version query
    if (result1 == 0 || result2 == 0) {
        printf("\nTesting VICI functionality...\n");
        vici_conn_t *conn = vici_connect(NULL);
        if (conn) {
            test_version_query(conn);
            vici_disconnect(conn);
        }
    }
    
    vici_deinit();
    
    printf("\n=== Test Summary ===\n");
    printf("Default connection: %s\n", result1 == 0 ? "PASS" : "FAIL");
    printf("Socket path connection: %s\n", result2 == 0 ? "PASS" : "FAIL");
    
    return (result1 == 0 || result2 == 0) ? 0 : 1;
}
