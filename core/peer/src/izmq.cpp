#include "izmq.h"
#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "keys.h"

#include <zmq.h>


keys *generate_keys() {
    keys* _keys = (keys*)malloc(sizeof(keys));

    if (!_keys) {
        ERROR("FAIL ALLOCATE MEE");
    };
    init(_keys);
    return _keys;
}

void set_curve_server_options(void* socket, const char* public_key, const char* secret_key) {
    int enable_curve = 1;
    zmq_setsockopt(socket, ZMQ_CURVE_SERVER, &enable_curve, sizeof(enable_curve));
    zmq_setsockopt(socket, ZMQ_CURVE_PUBLICKEY, public_key, 40);
    zmq_setsockopt(socket, ZMQ_CURVE_SECRETKEY, secret_key, 40);
}

void set_curve_client_options(void* socket, const char* public_key, const char* secret_key, const char* server_key) {
    zmq_setsockopt(socket, ZMQ_CURVE_PUBLICKEY, public_key, 40);
    zmq_setsockopt(socket, ZMQ_CURVE_SECRETKEY, secret_key, 40);
    zmq_setsockopt(socket, ZMQ_CURVE_SERVERKEY, server_key, 40);
}

ZMQWContext::~ZMQWContext() {
    if (_context)
        close();
}

void* ZMQWContext::get_context() {
    return _context;
}

void ZMQWContext::close() {
    if (_context) {
        if (zmq_ctx_destroy(_context) != Errors::OK) {
            ERROR("ZMQ FAIL clear ctx");
            _error->handle(Errors::FAIL_CLOSE_ZQM_CTX);
        } else {
            DEBUG("ZMQ clear ctx success");
        }
    }
    _context = nullptr;
}

void ZMQWContext::set_context() {
    _context = zmq_ctx_new();
    if (_context) {
        zmq_ctx_set(_context, ZMQ_IO_THREADS, 2);
        DEBUG("ZMQ Context created success");
    } else {
        _context = nullptr;
        ERROR("ZMQ Context fail to create");
        _error->handle(Errors::FAIL_OPEN_ZMQ_CTX);
    }
}