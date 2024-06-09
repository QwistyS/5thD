#ifndef PEER_H
#define PEER_H

#include <list>
#include "transmitter.h"
#include "context.h"

class Peer {
public:
    Peer(int port);
    ~Peer();
    void connect(const std::string& ip, int port);
private:
    std::list<Transmiter> _transmitters;
    std::unique_ptr<IContext> _ctx_out;
};

#endif // PEER_H
