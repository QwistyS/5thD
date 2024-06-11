#include "5thdlogger.h"
#include "peer.h"
#include "qwistys_macro.h"
#include "5thderror_handler.h"

Peer::~Peer() {
    _transmitters.clear();
}

void Peer::connect(const std::string& ip, int port) {
    Transmitter conn(&_ctx_out, &_errors);

    conn.connect("127.0.0.1", 4444);
    _transmitters.push_back(std::move(conn));
}
