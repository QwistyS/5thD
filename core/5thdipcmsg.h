#ifndef IPC_MESSAGE_H
#define IPC_MESSAGE_H

#include <stdint.h>
#include "qwistys_macro.h"

#define IPC_ROUTER_ADDR "127.0.0.1"
#define IPC_ROUTER_PORT 4545

#define CATEGORY_LENGTH_BYTES 30
#define DATA_LENGTH_BYTES 256

enum Clients { MANAGER = 0, PEER, UI, CLIENTS_TOTAL };

constexpr const char* CLIENTS_IDS[] = {
    "manager",
    "peerxxx",
    "uixxxxx",
};

#if defined(__GNUC__) || defined(__clang__)
typedef struct __attribute__((packed)) {
    int src_id;
    int dist_id;
    int64_t timestamp;
    char category[CATEGORY_LENGTH_BYTES];
    char data[DATA_LENGTH_BYTES];
} ipc_msg_t;
#elif defined(_MSC_VER)
#    pragma pack(push, 1)
typedef struct {
    int src_id;
    int dist_id;
    int64_t timestamp;
    char category[CATEGORY_LENGTH_BYTES];
    char data[DATA_LENGTH_BYTES];
} ipc_msg_t;
#    pragma pack(pop)
#else
#    error "Unsupported compiler/platform"
#endif

#endif  // IPC_MESSAGE_H
