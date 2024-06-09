#include "peer.h"
#include "context.h"
#include "lggr.h"
#include "qwistys_macro.h"

Peer::Peer(int port) {
    DEBUG("Init peer");
}

Peer::~Peer() {
    _transmitters.clear();
}

void Peer::connect(const std::string& ip, int port) {
    Transmiter conn(&_ctx_out);

    conn.connect("address", 4444);
    _transmitters.push_back(std::move(conn));
}
