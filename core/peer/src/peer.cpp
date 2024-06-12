#include "peer.h"
#include <zmq.h>
#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "qwistys_macro.h"

Peer::~Peer() {
    _connections.clear();
}

void Peer::connect(const std::string& ip, int port) {
    QWISTYS_TODO_MSG("Make sure you test the performers vector vs list of <Connection>");
    _handle_new_connection(ip, port);
}

void Peer::send(void* data, size_t data_length) {
    DEBUG("Len of connection vector {}", _connections.size());
}

void Peer::_connect(Connection& conn, const std::string& ip, int port, Connection::Transmitters client_type) {
    auto socket_type = 0;

    switch (client_type) {
        case Connection::USER:
            socket_type = ZMQ_REQ;
            break;
        case Connection::PROTO:
            socket_type = ZMQ_REP;
            break;
        default:
            break;
    }

    conn.clients.emplace_back(&_ctx_out, socket_type, &_errors);
    conn.clients[client_type].connect(ip, port);

    switch (client_type) {
        case Connection::PROTO:
            _setup_proto(conn);
            break;
        default:
            break;
    }
}

void Peer::_setup_proto(Connection& conn) {
    QWISTYS_TODO_MSG("Handle setup proto");
    conn.trusted = true;
}

void Peer::_handle_new_connection(const std::string& ip, int port) {
    // Create place for new connection
    _connections.emplace_back();
    auto& conn = _connections.back();
    // Connect to user
    _connect(conn, ip, port, Connection::USER);

    QWISTYS_TODO_MSG("Add retrieve the port for protocol")
    auto ret = conn.clients[Connection::USER].req_data(GENERIC_DATA);
    // _connect(ip, 5656, Connection::PROTO);

    QWISTYS_TODO_MSG("Handle when fail create connection")
    if (1) {
        DEBUG("Connection created successfully");
    } else {
        ERROR("Fail establish connection to {}:{}", ip, port);
        _connections.pop_back();
    }
}