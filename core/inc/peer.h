#ifndef PEER_H
#define PEER_H

#include <vector>
#include "transmitter.h"
#include "context.h"

class Peer {
public:
    Peer(int port);
    ~Peer();
    void connect(const std::string& ip, int port);
private:
    std::vector<Transmiter> _transmitters;
    Context _ctx_out;
    Context _ctx_in;
};

#endif // PEER_H
