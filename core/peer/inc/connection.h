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
    bool trusted;
};

#endif  // CONNECTION_H