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
    vici_add_key_valuef(req, "local_addrs", "172.20.0.10");
    vici_add_key_valuef(req, "remote_addrs", "172.20.0.20");
    vici_begin_section(req, "local");
    vici_add_key_valuef(req, "auth", "psk");
    vici_end_section(req);
    vici_begin_section(req, "remote");
    vici_add_key_valuef(req, "auth", "psk");
    vici_end_section(req);
    vici_begin_section(req, "children");
    vici_begin_section(req, "tunnel");
    vici_add_key_valuef(req, "local_ts", "172.20.0.10/32");
    vici_add_key_valuef(req, "remote_ts", "172.20.0.20/32");
    vici_end_section(req);
    vici_end_section(req);
    vici_end_section(req);
    
    vici_res_t *res = vici_submit(req, conn);
    if (res) {
        printf("Node1: Connection loaded\n");
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
        printf("Node1: PSK loaded\n");
        vici_free_res(res);
        return 0;
    }
    return -1;
}

static int initiate(vici_conn_t *conn) {
    vici_req_t *req = vici_begin("initiate");
    vici_add_key_valuef(req, "child", "tunnel");
    
    vici_res_t *res = vici_submit(req, conn);
    if (res) {
        printf("Node1: Connection initiated\n");
        vici_free_res(res);
        return 0;
    }
    return -1;
}

int main() {
    vici_conn_t *conn;
    
    printf("Node1: Starting VICI client\n");
    fflush(stdout);
    
    printf("Node1: Initializing VICI library\n");
    fflush(stdout);
    vici_init();
    
    printf("Node1: Attempting to connect to VICI socket\n");
    fflush(stdout);
    conn = vici_connect(NULL);
    if (!conn) {
        fprintf(stderr, "Node1: Connection failed: %s\n", strerror(errno));
        vici_deinit();
        return 1;
    }
    
    printf("Node1: Connected to VICI successfully\n");
    fflush(stdout);
    
    printf("Node1: Loading connection configuration\n");
    fflush(stdout);
    if (load_connection(conn) == 0) {
        printf("Node1: Loading PSK\n");
        fflush(stdout);
        if (load_psk(conn) == 0) {
            printf("Node1: Waiting 3 seconds for node2\n");
            fflush(stdout);
            sleep(3);
            printf("Node1: Initiating connection\n");
            fflush(stdout);
            if (initiate(conn) == 0) {
                printf("Node1: Connection initiated successfully, keeping alive\n");
                fflush(stdout);
                for (int i = 0; i < 60; i++) {
                    printf("Node1: Alive %d/60\n", i+1);
                    fflush(stdout);
                    sleep(1);
                }
            } else {
                printf("Node1: Failed to initiate connection\n");
                fflush(stdout);
            }
        } else {
            printf("Node1: Failed to load PSK\n");
            fflush(stdout);
        }
    } else {
        printf("Node1: Failed to load connection\n");
        fflush(stdout);
    }
    
    printf("Node1: Disconnecting\n");
    fflush(stdout);
    vici_disconnect(conn);
    vici_deinit();
    return 0;
}