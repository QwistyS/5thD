#include "peer.h"
#include <zmq.h>
#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "qwistys_macro.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <zmq.h>
#include <cerrno>
#include <cstring>
#include <iostream>

int is_port_available(int port) {
    int _port = port;
    int ret = -1;

    while (true) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            ERROR("TO OPEN SOCKET FOR PORT DISCOVERY");
            return ret;
        }

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) == Errors::OK) {
            ret = _port;
            close(sock);
            break;
        } else {
            _port++;
            close(sock);
            if (_port > 0xFFFF) {
                return ret;
            }
        }
    }

    DEBUG("Chosen socket port {}", _port);
    return _port;
}

Peer::~Peer() {
    _connections.clear();
    _receiver->close();
    _protocol->close();
    if (_ipc_socket) {
        zmq_close(_ipc_socket);
    }
}

void Peer::connect(const std::string& ip, int port) {
    QWISTYS_TODO_MSG("Make sure you test the performers vector vs list of <Connection>");
    _handle_new_connection(ip, port);
}

void Peer::send(void* data, size_t data_length) {
    DEBUG("Len of connection vector {}", _connections.size());
}

void Peer::listen() {
    if(_ipc_socket == nullptr) {
        ERROR("FAIL TO OPEN LISTENER");
        return;
    }
    _receiver = std::make_unique<Receiver>(_port, &_ctx_out, &_errors);
    _protocol = std::make_unique<Receiver>(_port - 1, &_ctx_out, &_errors);
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

void Peer::init() {
    _ipc_socket = zmq_socket(_ctx_out.get_context(), ZMQ_PAIR);
    if (_ipc_socket == nullptr) {
        ERROR("FAIL LISTEN");
        zmq_close(_ipc_socket);
        _errors.handle(Errors::FAIL_OPEN_SOCKET);
    }
    if (zmq_bind(_ipc_socket, "inproc://workers") != Errors::OK) {
        ERROR("IPC Bind fail");
        zmq_close(_ipc_socket);
        _ipc_socket = nullptr;
        _errors.handle(Errors::FAIL_TO_BIND);
    }
}

void Peer::_handle_new_connection(const std::string& ip, int port) {
    QWISTYS_TODO_MSG("Add mutex for access to connections collection");
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