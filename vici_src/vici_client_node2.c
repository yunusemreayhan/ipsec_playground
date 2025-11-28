#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libvici.h>

static int load_connection(vici_conn_t *conn) {
    vici_req_t *req = vici_begin("load-conn");
    vici_begin_section(req, "tunnel");
    vici_add_key_valuef(req, "version", "2");
    vici_add_key_valuef(req, "local_addrs", "172.20.0.20");
    vici_add_key_valuef(req, "remote_addrs", "172.20.0.10");
    vici_begin_section(req, "local");
    vici_add_key_valuef(req, "auth", "psk");
    vici_end_section(req);
    vici_begin_section(req, "remote");
    vici_add_key_valuef(req, "auth", "psk");
    vici_end_section(req);
    vici_begin_section(req, "children");
    vici_begin_section(req, "tunnel");
    vici_add_key_valuef(req, "local_ts", "172.20.0.20/32");
    vici_add_key_valuef(req, "remote_ts", "172.20.0.10/32");
    vici_end_section(req);
    vici_end_section(req);
    vici_end_section(req);
    
    vici_res_t *res = vici_submit(req, conn);
    if (res) {
        printf("Node2: Connection loaded\n");
        vici_free_res(res);
        return 0;
    }
    return -1;
}

static int load_psk(vici_conn_t *conn) {
    vici_req_t *req = vici_begin("load-shared");
    vici_begin_section(req, "psk");
    vici_add_key_valuef(req, "type", "IKE");
    vici_add_key_valuef(req, "data", "strongswan123");
    vici_add_key_valuef(req, "owners", "172.20.0.10 172.20.0.20");
    vici_end_section(req);
    
    vici_res_t *res = vici_submit(req, conn);
    if (res) {
        printf("Node2: PSK loaded\n");
        vici_free_res(res);
        return 0;
    }
    return -1;
}

void ike_updown_cb(void *user, char *name, vici_res_t *msg) {
    printf("Node2: IKE event: %s\n", name);
    if (strcmp(name, "ike-updown") == 0) {
        char *up = vici_find_str(msg, "", "up");
        if (up && strcmp(up, "yes") == 0) {
            printf("Node2: IKE SA established\n");
        }
    }
}

void child_updown_cb(void *user, char *name, vici_res_t *msg) {
    printf("Node2: Child event: %s\n", name);
    if (strcmp(name, "child-updown") == 0) {
        char *up = vici_find_str(msg, "", "up");
        if (up && strcmp(up, "yes") == 0) {
            printf("Node2: Child SA established - tunnel is up!\n");
        }
    }
}

int main() {
    vici_conn_t *conn;
    
    printf("Node2: Starting VICI client (responder)\n");
    fflush(stdout);
    
    printf("Node2: Initializing VICI library\n");
    fflush(stdout);
    vici_init();
    
    printf("Node2: Attempting to connect to VICI socket\n");
    fflush(stdout);
    conn = vici_connect(NULL);
    if (!conn) {
        fprintf(stderr, "Node2: Connection failed: %s\n", strerror(errno));
        vici_deinit();
        return 1;
    }
    
    printf("Node2: Connected to VICI successfully\n");
    fflush(stdout);
    
    printf("Node2: Loading connection configuration\n");
    fflush(stdout);
    if (load_connection(conn) == 0) {
        printf("Node2: Loading PSK\n");
        fflush(stdout);
        if (load_psk(conn) == 0) {
            printf("Node2: Registering for IKE events\n");
            fflush(stdout);
            vici_register(conn, "ike-updown", ike_updown_cb, NULL);
            vici_register(conn, "child-updown", child_updown_cb, NULL);
            
            printf("Node2: Ready to accept connections, listening for events\n");
            fflush(stdout);
            for (int i = 0; i < 60; i++) {
                printf("Node2: Listening %d/60\n", i+1);
                fflush(stdout);
                sleep(1);
            }
        } else {
            printf("Node2: Failed to load PSK\n");
            fflush(stdout);
        }
    } else {
        printf("Node2: Failed to load connection\n");
        fflush(stdout);
    }
    
    printf("Node2: Disconnecting\n");
    fflush(stdout);
    vici_disconnect(conn);
    vici_deinit();
    return 0;
}