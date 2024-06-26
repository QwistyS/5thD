#ifndef CONNECTION_H
#define CONNECTION_H

#include <array>
#include "transmitter.h"

struct Connection {
    enum Transmitters{
        USER = 0,
        PROTO,
        TOTAL
    };
    std::vector<ZMQTransmitter> clients;
    Transmitters variant;
    bool trusted;
};

#endif  // CONNECTION_H