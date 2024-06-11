#include "izmq.h"
#include "5thdlogger.h"
#include "5thderror_handler.h"

#include <zmq.h>

ZMQWContext::~ZMQWContext() {
    if (_context) {
        if (zmq_ctx_destroy(_context) != 0) {
            ERROR("ZMQ FAIL clear ctx");
        } else {
            DEBUG("ZMQ clear ctx success");
        }
    }
}

void* ZMQWContext::get_context() {
    return _context;
}

void ZMQWContext::close() {
    zmq_ctx_destroy(_context);
}

void ZMQWContext::set_context() {
    _context = zmq_ctx_new();
    if (_context) {
        DEBUG("ZMQ Context created success");
    } else {
        ERROR("ZMQ Context fail to create");
    }
}