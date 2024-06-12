#include "peer.h"
#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "qwistys_macro.h"

Peer::~Peer() {
    _transmitters.clear();
}

void Peer::connect(const std::string& ip, int port) {
    _transmitters.emplace_back(&_ctx_out, &_errors);
    auto conn = _transmitters.back();
    conn.connect(ip, port);
}

void Peer::send(void* data, size_t data_length) {
    for (auto& it : _transmitters) {
        it.is_connected();
    }
}


