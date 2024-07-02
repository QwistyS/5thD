#include <errno.h>
#include "5thdlogger.h"
#include "software_bus.h"
#include "zmq.h"

void recv_message(void *socket, ipc_msg_t *msg) {
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(ipc_msg_t));
    zmq_msg_recv(&message, socket, 0);
    memcpy(msg, zmq_msg_data(&message), sizeof(ipc_msg_t));
    zmq_msg_close(&message);
}

ZMQBus::~ZMQBus() {
    if (_router) {
        zmq_close(_router);
    }

    if (_context) {
        zmq_ctx_destroy(_context);
    }
    _errors->unreg(SOFTWARE_BUS_ID);
}

void ZMQBus::run() {
    ipc_msg_t data;
    zmq_msg_t identity;
    zmq_msg_init(&identity);
    zmq_msg_recv(&identity, _router, 0);

    zmq_msg_t empty;
    zmq_msg_init(&empty);
    zmq_msg_recv(&empty, _router, 0);
    zmq_msg_close(&empty);

    recv_message(_router, &data);

    DEBUG("Received: Id:{}", (char*) zmq_msg_data(&identity));
    DEBUG("Category:{}", data.category);

    zmq_msg_close(&identity);
}

void ZMQBus::_init() {
    _errors->reg(SOFTWARE_BUS_ID);

    _context = zmq_ctx_new();
    if (!_context) {
        ERROR("Fail to init context");
        _errors->handle(SOFTWARE_BUS_ID, Errors::FAIL_OPEN_ZMQ_CTX);
    }
    DEBUG("{} Created context", SOFTWARE_BUS_ID);

    _router = zmq_socket(_context, ZMQ_ROUTER);
    if (!_router) {
        ERROR("Fail to init socket");
        _errors->handle(SOFTWARE_BUS_ID, Errors::FAIL_OPEN_SOCKET);
    }
    DEBUG("{} Created socket", SOFTWARE_BUS_ID);
    zmq_bind(_router, IPC_ROUTER_ADDR);

    if (_timeout) {
        if (zmq_setsockopt(_router, ZMQ_RCVTIMEO, &_timeout, sizeof(_timeout)) != Errors::OK) {
            ERROR("{} Fail to set timeout on socket", SOFTWARE_BUS_ID);
            _errors->handle(SOFTWARE_BUS_ID, Errors::FAIL_SET_SOCKOPT);
        }
        DEBUG("{} timeout set", SOFTWARE_BUS_ID);
    }
}
