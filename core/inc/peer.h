#ifndef PEER_H
#define PEER_H

#include <list>
#include "transmitter.h"

class Peer {
public:
    Peer(int port);
    ~Peer();
    void connect(const std::string& ip, int port);
private:
    std::list<Transmiter> _transmitters;
};

#endif // PEER_H
