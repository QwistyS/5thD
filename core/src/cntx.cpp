#include <zmq.h>
#include "context.h"
#include "logger.h"

auto context_log = logger5thd::get_logger("network");
#define DEBUG(x, ...) context_log->debug(x, ##__VA_ARGS__)

Context::~Context() {
    if (_context) {
        if (zmq_ctx_destroy(_context) != 0) {
            context_log->error("ZMQ FAIL clear ctx");
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
        context_log->error("ZMQ Context fail to create");
    }
}