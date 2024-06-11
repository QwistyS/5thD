#ifndef PEER_H
#define PEER_H

#include <vector>
#include "izmq.h"

class Peer {
public:
    Peer(int port) : _ctx_out(&_errors), _ctx_in(&_errors) {};
    ~Peer();
    void connect(const std::string& ip, int port);

private:
    std::vector<Transmitter> _transmitters;
    ZMQWContext _ctx_out;
    ZMQWContext _ctx_in;
    NetworkError _errors;
};

#endif  // PEER_H
