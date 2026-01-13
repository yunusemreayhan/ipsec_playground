#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <libvici.h>

void check_socket_permissions(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        printf("Socket %s exists:\n", path);
        printf("  Mode: %o\n", st.st_mode & 0777);
        printf("  UID: %d, GID: %d\n", st.st_uid, st.st_gid);
        printf("  Size: %ld\n", st.st_size);
    } else {
        printf("Socket %s does not exist or cannot be accessed: %s\n", path, strerror(errno));
    }
}

int test_various_paths() {
    const char *paths[] = {
        "/run/charon.vici",
        "/var/run/charon.vici", 
        "/tmp/charon.vici",
        "/usr/local/var/run/charon.vici",
        NULL
    };
    
    printf("=== Socket Path Investigation ===\n");
    
    for (int i = 0; paths[i] != NULL; i++) {
        check_socket_permissions(paths[i]);
        
        printf("Testing connection to %s...\n", paths[i]);
        vici_conn_t *conn = vici_connect(paths[i]);
        if (conn) {
            printf("✓ Connection to %s successful\n", paths[i]);
            vici_disconnect(conn);
        } else {
            printf("✗ Connection to %s failed: %s\n", paths[i], strerror(errno));
        }
        printf("\n");
    }
    
    return 0;
}

int main() {
    printf("=== VICI Socket Path Analysis ===\n");
    
    vici_init();
    
    // Check what the current working directory and environment looks like
    printf("Current working directory: %s\n", getcwd(NULL, 0));
    printf("UID: %d, GID: %d\n", getuid(), getgid());
    printf("\n");
    
    test_various_paths();
    
    // Test default connection again for comparison
    printf("=== Default Connection Test ===\n");
    vici_conn_t *conn = vici_connect(NULL);
    if (conn) {
        printf("✓ Default connection (NULL) works\n");
        
        // Try to get version to confirm it's working
        vici_req_t *req = vici_begin("version");
        vici_res_t *res = vici_submit(req, conn);
        if (res) {
            char *version = vici_find_str(res, "", "version");
            printf("  strongSwan version: %s\n", version ? version : "unknown");
            vici_free_res(res);
        }
        
        vici_disconnect(conn);
    } else {
        printf("✗ Default connection failed: %s\n", strerror(errno));
    }
    
    vici_deinit();
    return 0;
}
