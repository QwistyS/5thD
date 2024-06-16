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

void Receiver::init() {
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

void Receiver::close() {
    if (_socket_rout) {
        zmq_close(_socket_rout);
        _socket_rout = nullptr;
    }
}