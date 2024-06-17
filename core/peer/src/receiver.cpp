#include <zmq.h>

#include "5thdlogger.h"
#include "qwistys_macro.h"
#include "receiver.h"

Receiver::~Receiver() {
    close();
}

void Receiver::listen() {
    __NOP;
}

void Receiver::_init() {
    std::string addr = "tcp://*:" + std::to_string(_port);
    _socket_rout = zmq_socket(_ctx->get_context(), ZMQ_ROUTER);

    if (_socket_rout == nullptr) {
        ERROR("FAIL to init listener on address {}", addr);
        zmq_close(_socket_rout);
        _socket_rout = nullptr;
        _error->handle(Errors::FAIL_INIT_LISTENER);
    }
    DEBUG("Listener init success on {}", addr);
}

void Receiver::_set_curve_server_options(const char* public_key, const char* secret_key) {
    int enable_curve = 1;
    zmq_setsockopt(_socket_rout, ZMQ_CURVE_SERVER, &enable_curve, sizeof(enable_curve));
    zmq_setsockopt(_socket_rout, ZMQ_CURVE_PUBLICKEY, public_key, 40);
    zmq_setsockopt(_socket_rout, ZMQ_CURVE_SECRETKEY, secret_key, 40);
}

void Receiver::close() {
    if (_socket_rout) {
        zmq_close(_socket_rout);
        _socket_rout = nullptr;
    }
}