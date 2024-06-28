#ifndef CONNECTION_H
#define CONNECTION_H

#include <string.h>

typedef struct {
    char id[64];
    std::string addr;
    uint16_t port;
} conn_info_t;

#endif  // CONNECTION_H