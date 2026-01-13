#include <stdio.h>
#include <libvici.h>

int main() {
    vici_init();
    
    // Test different URI formats
    char *uris[] = {
        "unix:///run/charon.vici",
        "unix:///var/run/charon.vici", 
        "unix://run/charon.vici",
        "unix://var/run/charon.vici",
        NULL
    };
    
    for (int i = 0; uris[i]; i++) {
        printf("Testing: %s\n", uris[i]);
        vici_conn_t *conn = vici_connect(uris[i]);
        if (conn) {
            printf("✓ SUCCESS\n");
            vici_disconnect(conn);
        } else {
            printf("✗ FAILED\n");
        }
        printf("\n");
    }
    
    vici_deinit();
    return 0;
}
