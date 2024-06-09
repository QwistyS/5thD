#include "peer.h"

Peer::Peer(int port) {}

Peer::~Peer() {
    _transmitters.clear();
}

void Peer::connect(const std::string &ip, int port) {
    Transmiter conn;
    conn.connect(ip, port);

    _transmitters.push_back(std::move(conn));
}
