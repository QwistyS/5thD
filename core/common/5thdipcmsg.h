#ifndef IPC_MESSAGE_H
#define IPC_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "qwistys_macro.h"

extern const char* IPC_ENDPOINT;

#define CATEGORY_LENGTH_BYTES 30
#define DATA_LENGTH_BYTES 256

enum Clients { MANAGER = 0, PEER, UI, ROUTER, CLIENTS_TOTAL };

extern const char* CLIENTS_IDS[];



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

void print_ipc_msg(ipc_msg_t* msg);

#ifdef __cplusplus
}
#endif

#endif  // IPC_MESSAGE_H
