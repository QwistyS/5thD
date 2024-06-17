#include "izmq.h"
#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "keys.h"

#include <zmq.h>

keys* generate_keys() {
    keys* _keys = (keys*) malloc(sizeof(keys));

    if (!_keys) {
        ERROR("FAIL ALLOCATE MEE");
        return nullptr;
    };

    init(_keys);
    if (zmq_curve_keypair(_keys->public_key, _keys->secret_key) != Errors::OK) {
        ERROR("FAIL TO GENERATE KEYS");
        free(_keys);
        _keys = nullptr;
    }
    return _keys;
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