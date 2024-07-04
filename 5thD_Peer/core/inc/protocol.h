#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>

enum MsgType {
    INIT = 0,
    TOTAL
};

struct __attribute__((aligned(8), packed)) Message {
    MsgType msg_type;
    uint8_t *buffer;
    size_t num_bytes;
};

#endif // PROTOCOL_H