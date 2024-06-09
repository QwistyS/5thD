#ifndef PEER_H
#define PEER_H

#include <vector>
#include "context.h"
#include "transmitter.h"

class Peer {
public:
    Peer(int port);
    ~Peer();
    void connect(const std::string& ip, int port);

private:
    std::vector<Transmitter> _transmitters;
    Context _ctx_out;
    Context _ctx_in;
    NetworkError _errors;
};

#endif  // PEER_H
