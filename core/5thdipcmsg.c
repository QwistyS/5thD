#include <string.h>
#include <time.h>
#include "5thdipcmsg.h"

#ifdef _WIN32
const char* IPC_ENDPOINT = "ipc://secure_ipc";
#else
const char* IPC_ENDPOINT = "ipc:///tmp/secure_ipc";
#endif

void ipc_msg(ipc_msg_t* msg, int src, int dist) {
    memset(msg, 0, sizeof(ipc_msg_t));
    msg->src_id = src;
    msg->dist_id = dist;
    msg->timestamp = time(NULL);
}

void print_ipc_msg(ipc_msg_t* msg) {
    printf("--- Frame ---\n");
    printf("src: %d\n", msg->src_id);
    printf("dist: %d\n", msg->dist_id);
    printf("timestamp: %ld\n", msg->timestamp);
    printf("category: %s\n", msg->category);
    printf("data: ");
    for (int i = 0; i < DATA_LENGTH_BYTES; i++) printf("%x", msg->data[i]);
    printf("\n");
}



const char* CLIENTS_IDS[] = {
    "manager",
    "peerxxx",
    "uixxxxx",
    "ipcrout",
};