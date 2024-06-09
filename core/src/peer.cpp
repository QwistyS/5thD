#include "peer.h"
#include "context.h"
#include "logger.h"
#include "qwistys_macro.h"

auto peer_log = logger5thd::get_logger("network");

Peer::Peer(int port) {
    peer_log->debug("Init peer");
}

Peer::~Peer() {
    _transmitters.clear();
}

void Peer::connect(const std::string& ip, int port) {
}
