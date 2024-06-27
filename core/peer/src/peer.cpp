#include <zmq.h>
#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "peer.h"
#include "qwistys_macro.h"

Peer::~Peer() {
    _connections.clear();
    _receiver->close();
    // _protocol->close();
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
    if (_ipc_socket == nullptr) {
        ERROR("FAIL TO OPEN LISTENER");
        return;
    }
    _receiver = std::make_unique<Receiver>(_port, &_ctx_out, &_errors);

    auto _keys = Keys::get_instance();
    _receiver->set_curve_server_options(_keys->get_key(PRIVATE_KEY_FLAG), _keys->get_key(PUBLIC_KEY_FLAG));
    _receiver->listen();

    // auto next_port = is_port_available(_port + 1);

    // _protocol = std::make_unique<Receiver>(next_port, &_ctx_out, &_errors);
    // _protocol->set_curve_server_options(_keys->server_public_key, _keys->server_secret_key);
    // _protocol->listen();
}

void Peer::_connect(Connection& conn, const std::string& ip, int port, Connection::Transmitters client_type) {
    auto socket_type = 0;

    switch (client_type) {
        case Connection::USER:
            socket_type = ZMQ_REQ;
            conn.variant = Connection::USER;
            break;
        case Connection::PROTO:
            socket_type = ZMQ_REP;
            conn.variant = Connection::PROTO;
            break;
        default:
            break;
    }

    conn.clients.emplace_back(&_ctx_out, socket_type, &_errors);
    conn.clients[client_type].set_curve_client_options(Keys::get_instance()->get_key(PUBLIC_KEY_FLAG));
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

void Peer::_init() {
    // Init Cache for connections
    _conn_cache = std::make_unique<LRU_Cache<std::string, Connection>>(CACHE_SIZE);

    // init socket
    _ipc_socket = zmq_socket(_ctx_out.get_context(), ZMQ_PAIR);
    if (_ipc_socket == nullptr) {
        ERROR("FAIL LISTEN");
        zmq_close(_ipc_socket);
        _errors.handle(Errors::FAIL_OPEN_SOCKET);
    }

    // init keys
    auto _keys = Keys::get_instance();

    QWISTYS_TODO_MSG("In init of the listener for a peer, do i need a way to configure sockopt? or all @ init?");
    // Set socket opt
    //
    // zmq_setsockopt(_ipc_socket, ZMQ_CURVE_SERVER, _keys->server_public_key, KEY_LENGTH - 1);

    // Bind
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