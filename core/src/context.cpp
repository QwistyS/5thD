#include <zmq.h>
#include "context.h"
#include "lggr.h"

Context::~Context() {
    if (_context) {
        if (zmq_ctx_destroy(_context) != 0) {
            ERROR("ZMQ FAIL clear ctx");
        } else {
            DEBUG("ZMQ clear ctx success");
        }
    }
}

void* Context::get_context() {
    return _context;
}

void Context::close() {
    zmq_ctx_destroy(_context);
}

void Context::set_context() {
    _context = zmq_ctx_new();
    if (_context) {
        DEBUG("ZMQ Context created success");
    } else {
        ERROR("ZMQ Context fail to create");
    }
}