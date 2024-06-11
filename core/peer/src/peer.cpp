#include "5thdlogger.h"
#include "peer.h"
#include "qwistys_macro.h"

Peer::Peer(int port) {
    DEBUG("CTOR Peer");
}

Peer::~Peer() {
    _transmitters.clear();
}

void Peer::connect(const std::string& ip, int port) {
    Transmitter conn(&_ctx_out, &_errors);

    conn.connect("address", 4444);
    _transmitters.push_back(std::move(conn));
}
